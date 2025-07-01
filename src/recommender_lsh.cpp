#include "recommender_lsh.hpp"
#include "output_manager.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <atomic>

RecommenderLSH::RecommenderLSH(size_t num_bands, size_t band_size, size_t num_hashes)
    : num_bands(num_bands), band_size(band_size), num_hashes(num_hashes) {
    
    // Initialize hash functions
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dist_a(1, LARGE_PRIME - 1);
    std::uniform_int_distribution<uint64_t> dist_b(0, LARGE_PRIME - 1);
    
    hash_functions.resize(num_hashes);
    for (auto& hf : hash_functions) {
        hf.a = dist_a(gen);
        hf.b = dist_b(gen);
        hf.c = LARGE_PRIME;
    }
}

std::vector<uint64_t> RecommenderLSH::computeMinHashSignature(const std::unordered_set<movie_id_t>& movies) const {
    std::vector<uint64_t> signature(num_hashes, std::numeric_limits<uint64_t>::max());
    
    for (movie_id_t movie : movies) {
        for (size_t i = 0; i < num_hashes; i++) {
            uint64_t hash_val = hash_functions[i](movie);
            if (hash_val < signature[i]) {
                signature[i] = hash_val;
            }
        }
    }
    
    return signature;
}

void RecommenderLSH::buildIndex(const UsersAndMoviesData& data) {
    users_data = data;
    lsh_bands.clear();
    lsh_bands.resize(num_bands);
    user_movies.clear();
    userNorms.clear();
    
    precomputeUserNorms();
    
    for (const auto& [user_id, ratings] : users_data) {
        auto signature = computeMinHashSignature(user_movies[user_id]);
        
        for (size_t band = 0; band < num_bands; band++) {
            size_t start = band * band_size;
            size_t end = std::min(start + band_size, signature.size());
            
            std::string band_key;
            band_key.reserve((end - start) * sizeof(uint64_t));
            
            for (size_t i = start; i < end; i++) {
                band_key.append(reinterpret_cast<const char*>(&signature[i]), sizeof(signature[i]));
            }
            
            lsh_bands[band][band_key].push_back(user_id);
        }
    }
}

// ... [rest of the implementation remains exactly the same as in previous correct version]

std::vector<std::pair<user_id_t, double>> RecommenderLSH::findSimilarUsers(user_id_t user, 
                                                                        double threshold) const {
    std::unordered_set<user_id_t> candidate_set;
    auto user_it = user_movies.find(user);
    
    if (user_it == user_movies.end()) {
        return {};
    }
    
    auto signature = computeMinHashSignature(user_it->second);
    
    for (size_t band = 0; band < num_bands; band++) {
        size_t start = band * band_size;
        size_t end = std::min(start + band_size, signature.size());
        
        std::string band_key;
        band_key.reserve((end - start) * sizeof(uint64_t));
        
        for (size_t i = start; i < end; i++) {
            band_key.append(reinterpret_cast<const char*>(&signature[i]), sizeof(signature[i]));
        }
        
        auto band_it = lsh_bands[band].find(band_key);
        if (band_it != lsh_bands[band].end()) {
            for (user_id_t candidate : band_it->second) {
                if (candidate != user) {
                    candidate_set.insert(candidate);
                }
            }
        }
    }
    
    std::vector<std::pair<user_id_t, double>> similar_users;
    for (user_id_t candidate : candidate_set) {
        double similarity = computeCosineSimilarity(user, candidate);
        if (similarity >= threshold) {
            similar_users.emplace_back(candidate, similarity);
        }
    }
    
    std::sort(similar_users.begin(), similar_users.end(),
             [](const auto& a, const auto& b) { return a.second > b.second; });
    
    return similar_users;
}

double RecommenderLSH::computeCosineSimilarity(user_id_t user1, user_id_t user2) const {
    const auto it1 = users_data.find(user1);
    const auto it2 = users_data.find(user2);
    if (it1 == users_data.end() || it2 == users_data.end())
        return 0.0;

    const auto& ratings1 = it1->second;
    const auto& ratings2 = it2->second;

    const bool useFirstAsBase = ratings1.size() < ratings2.size();
    const auto& baseRatings = useFirstAsBase ? ratings1 : ratings2;
    const auto& targetRatings = useFirstAsBase ? ratings2 : ratings1;

    double dotProduct = 0.0;
    
    for (const Rating& r : baseRatings) {
        auto found = targetRatings.find(Rating{r.movie, 0});
        if (found != targetRatings.end()) {
            dotProduct += r.score * found->score;
        }
    }

    const double norm1 = userNorms.at(user1);
    const double norm2 = userNorms.at(user2);
    const double normProduct = norm1 * norm2;
    
    return (normProduct > std::numeric_limits<double>::epsilon()) 
           ? (dotProduct / normProduct) 
           : 0.0;
}

void RecommenderLSH::precomputeUserNorms() {
    for (const auto& [userId, ratings] : users_data) {
        double normSq = 0.0;
        for (const Rating& r : ratings) {
            normSq += r.score * r.score;
            user_movies[userId].insert(r.movie);
        }
        userNorms[userId] = std::sqrt(normSq);
    }
}

void RecommenderLSH::generateRecommendations(const UsersAndMoviesData& usersAndMovies,
                                         const MoviesData& movies) {
    OutputManager outputManager;
    std::vector<uint64_t> genDurations;
    std::vector<uint64_t> writeDurations;
    buildIndex(usersAndMovies);  // Adicione esta linha

    genDurations.reserve(1000);
    writeDurations.reserve(1000);

    outputManager.openInOutcome("output.txt");
    
    thread_local std::ostringstream buffer;
    buffer.str("");
    buffer.clear();
    buffer.str().reserve(50000);

    std::unordered_set<user_id_t> usersToRecommend;
    {
        std::ifstream file("datasets/explore.dat");
        if (!file) {
            std::cerr << "Error opening explore.dat" << std::endl;
            return;
        }
        
        user_id_t user;
        usersToRecommend.reserve(10000);
        while (file >> user) {
            usersToRecommend.insert(user);
        }
    }

    std::mutex queueMutex;
    std::mutex outputMutex;
    std::mutex consoleMutex;
    std::condition_variable cv;
    std::queue<user_id_t> userQueue;
    std::atomic<bool> done(false);
    std::vector<std::thread> threads;

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        for (user_id_t user : usersToRecommend) {
            userQueue.push(user);
        }
    }

    auto worker = [&]() {
        const int maxRecs = NUMBER_OF_RECOMMENDATIONS_PER_USER;
        std::vector<std::pair<user_id_t, double>> similarUsers;
        std::unordered_map<movie_id_t, std::pair<double, double>> predictions;
        std::vector<std::pair<movie_id_t, double>> recommendations;
        thread_local std::ostringstream localBuffer;
        localBuffer.str("");
        localBuffer.clear();

        while (true) {
            user_id_t targetUser;
            bool emptyQueue = false;

            {
                std::unique_lock<std::mutex> lock(queueMutex);
                cv.wait(lock, [&](){ return done || !userQueue.empty(); });

                if (done && userQueue.empty()) return;
                if (userQueue.empty()) continue;

                targetUser = userQueue.front();
                userQueue.pop();
                emptyQueue = userQueue.empty();
            }

            if (emptyQueue) {
                cv.notify_one();
            }

            auto targetIt = users_data.find(targetUser);
            if (targetIt == users_data.end()) {
                std::lock_guard<std::mutex> consoleLock(consoleMutex);
                std::cout << "User " << targetUser << " not found in dataset.\n";
                continue;
            }

            similarUsers = findSimilarUsers(targetUser, 0.6);
            
            const auto& targetRatings = targetIt->second;
            predictions.clear();
            
            for (const auto& [similarUser, similarity] : similarUsers) {
                for (const Rating& r : users_data.at(similarUser)) {
                    if (targetRatings.find(Rating{r.movie, 0}) == targetRatings.end()) {
                        predictions[r.movie].first += r.score * similarity;
                        predictions[r.movie].second += similarity;
                    }
                }
            }

            recommendations.clear();
            recommendations.reserve(predictions.size());
            
            for (const auto& [movie, scores] : predictions) {
                if (scores.second > 0) {
                    recommendations.emplace_back(movie, scores.first / scores.second);
                }
            }

            if (!recommendations.empty()) {
                const size_t recSize = std::min<size_t>(maxRecs, recommendations.size());
                std::nth_element(recommendations.begin(), recommendations.begin() + recSize,
                                recommendations.end(),
                                [](const auto& a, const auto& b) { return a.second > b.second; });
                recommendations.resize(recSize);
                std::sort(recommendations.begin(), recommendations.end(),
                         [](const auto& a, const auto& b) { return a.second > b.second; });
            }

            localBuffer.str("");
            localBuffer << "\n\nRecommendations for user " << targetUser << ":\n\n";
            for (const auto& [movieId, score] : recommendations) {
                auto it = movies.find(movieId);
                if (it != movies.end()) {
                    localBuffer << it->second << " (Score: " << score << ")\n";
                }
            }

            {
                std::lock_guard<std::mutex> outputLock(outputMutex);
                const std::string& outputStr = localBuffer.str();
                outputManager.write(outputStr.c_str(), outputStr.size());
            }
        }
    };

    unsigned numThreads = std::min<unsigned>(std::thread::hardware_concurrency(), 32);
    if (numThreads == 0) numThreads = 1;
    
    threads.reserve(numThreads);
    for (unsigned i = 0; i < numThreads; i++) {
        threads.emplace_back(worker);
    }

    {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [&userQueue](){ return userQueue.empty(); });
    }

    done = true;
    cv.notify_all();

    for (auto& thread : threads) {
        thread.join();
    }

    outputManager.flush();
}