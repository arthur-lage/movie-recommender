#include "recommender_cosine.hpp"
#include "config.hpp"
#include "output_manager.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <sstream>
#include <unordered_set>
#include <vector>
#include <numeric>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

RecommenderCosine::RecommenderCosine() {}

void RecommenderCosine::precomputeUserNorms(const UsersAndMoviesData &data)
{
    userNorms.clear();
    userNorms.reserve(data.size());

    for (const auto &[userId, ratings] : data)
    {
        double normSq = 0.0;
        for (const Rating &r : ratings)
        {
            normSq += r.score * r.score;
        }
        userNorms[userId] = std::sqrt(normSq);
    }
}
double RecommenderCosine::computeCosineSimilarity(user_id_t user1, user_id_t user2,
                                                 const UsersAndMoviesData &data) const
{
    // Early exit if either user is not found
    const auto it1 = data.find(user1);
    const auto it2 = data.find(user2);
    if (it1 == data.end() || it2 == data.end())
        return 0.0;

    const auto &ratings1 = it1->second;
    const auto &ratings2 = it2->second;

    // Optimize by processing the smaller collection
    const bool useFirstAsBase = ratings1.size() < ratings2.size();
    const auto &baseRatings = useFirstAsBase ? ratings1 : ratings2;
    const auto &targetRatings = useFirstAsBase ? ratings2 : ratings1;

    double dotProduct = 0.0;
    
    // For unordered_set, we can only use find()
    for (const Rating &r : baseRatings) {
        auto found = targetRatings.find(Rating{r.movie, 0});
        if (found != targetRatings.end()) {
            dotProduct += r.score * found->score;
        }
    }

    // Use precomputed norms
    const double norm1 = userNorms.at(user1);
    const double norm2 = userNorms.at(user2);
    const double normProduct = norm1 * norm2;
    
    return (normProduct > std::numeric_limits<double>::epsilon()) 
           ? (dotProduct / normProduct) 
           : 0.0;
}

void RecommenderCosine::generateRecommendations(const UsersAndMoviesData &usersAndMovies,
                                              const MoviesData &movies) {
    OutputManager outputManager;
    std::vector<uint64_t> genDurations;
    std::vector<uint64_t> writeDurations;

    // Otimização 1: Reduzir alocações iniciais
    genDurations.reserve(1000);
    writeDurations.reserve(1000);

    outputManager.openInOutcome("output.txt");
    
    // Otimização 2: Buffer maior e reutilizável
    thread_local std::ostringstream buffer;
    buffer.str("");
    buffer.clear();
    buffer.str().reserve(50000);

    // Pré-computação otimizada
    precomputeUserNorms(usersAndMovies);

    // Otimização 3: Carregar usuários de forma mais eficiente
    std::unordered_set<user_id_t> usersToRecommend;
    {
        std::ifstream file("datasets/explore.dat");
        if (!file) {
            std::cerr << "Erro ao abrir explore.dat" << std::endl;
            return;
        }
        
        user_id_t user;
        usersToRecommend.reserve(10000); // Tamanho estimado
        while (file >> user) {
            usersToRecommend.insert(user);
        }
    }

    // Otimização 4: Mecanismo de paralelismo mais eficiente
    std::mutex queueMutex;
    std::mutex outputMutex;
    std::mutex consoleMutex;
    std::condition_variable cv;
    std::queue<user_id_t> userQueue;
    std::atomic<bool> done(false);
    std::vector<std::thread> threads;

    // Otimização 5: Pré-carregar a fila de uma vez
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        for (user_id_t user : usersToRecommend) {
            userQueue.push(user);
        }
    }

    auto worker = [&]() {
        const int k = 15;             
        const int maxRecs = NUMBER_OF_RECOMMENDATIONS_PER_USER;
        // Otimização 6: Variáveis locais para reduzir alocações
        std::vector<std::pair<user_id_t, double>> similarUsers;
        std::unordered_map<movie_id_t, std::pair<double, double>> predictions;
        std::vector<std::pair<movie_id_t, double>> recommendations;
        thread_local std::ostringstream localBuffer;
        localBuffer.str("");
        localBuffer.clear();

        while (true) {
            user_id_t targetUser;
            bool emptyQueue = false;

            // Otimização 7: Bloco crítico mais curto
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

            // Otimização 8: Verificação mais rápida
            auto targetIt = usersAndMovies.find(targetUser);
            if (targetIt == usersAndMovies.end()) {
                std::lock_guard<std::mutex> consoleLock(consoleMutex);
                std::cout << "Usuário " << targetUser << " não encontrado no dataset.\n";
                continue;
            }

            // Otimização 9: Pré-reservar espaço
            similarUsers.clear();
            similarUsers.reserve(usersAndMovies.size());

            const auto &targetRatings = targetIt->second;
            
            // Otimização 10: Calcular similaridades de forma mais eficiente
            for (const auto &[otherUser, _] : usersAndMovies) {
                if (targetUser == otherUser) continue;

                double similarity = computeCosineSimilarity(targetUser, otherUser, usersAndMovies);
                if (similarity > 0.2) {
                    similarUsers.emplace_back(otherUser, similarity);
                }
            }

            // Otimização 11: Usar nth_element em vez de partial_sort
            if (!similarUsers.empty()) {
                const size_t sortSize = std::min<size_t>(k, similarUsers.size());
                std::nth_element(similarUsers.begin(), similarUsers.begin() + sortSize, similarUsers.end(),
                               [](const auto &a, const auto &b) { return a.second > b.second; });
                similarUsers.resize(sortSize);
                std::sort(similarUsers.begin(), similarUsers.end(),
                         [](const auto &a, const auto &b) { return a.second > b.second; });
            }

            // Otimização 12: Limpar e reutilizar estruturas
            predictions.clear();
            
            for (const auto &[similarUser, similarity] : similarUsers) {
                for (const Rating &r : usersAndMovies.at(similarUser)) {
                    if (targetRatings.find(Rating{r.movie, 0}) == targetRatings.end()) {
                        predictions[r.movie].first += r.score * similarity;
                        predictions[r.movie].second += similarity;
                    }
                }
            }

            recommendations.clear();
            recommendations.reserve(predictions.size());
            
            for (const auto &[movie, scores] : predictions) {
                if (scores.second > 0) {
                    recommendations.emplace_back(movie, scores.first / scores.second);
                }
            }

            if (!recommendations.empty()) {
                const size_t recSize = std::min<size_t>(maxRecs, recommendations.size());
                std::nth_element(recommendations.begin(), recommendations.begin() + recSize,
                                recommendations.end(),
                                [](const auto &a, const auto &b) { return a.second > b.second; });
                recommendations.resize(recSize);
                std::sort(recommendations.begin(), recommendations.end(),
                         [](const auto &a, const auto &b) { return a.second > b.second; });
            }

            // Otimização 13: Buffer local por thread
            localBuffer.str("");
            localBuffer << "\n\nRecommendations for user " << targetUser << ":\n\n";
            for (const auto &[movieId, score] : recommendations) {
                auto it = movies.find(movieId);
                if (it != movies.end()) {
                    localBuffer << it->second << " (Score: " << score << ")\n";
                }
            }

            // Otimização 14: Bloqueio mínimo para saída
            {
                std::lock_guard<std::mutex> outputLock(outputMutex);
                const std::string &outputStr = localBuffer.str();
                outputManager.write(outputStr.c_str(), outputStr.size());
            }
        }
    };

    // Otimização 15: Número de threads baseado no sistema
    unsigned numThreads = std::min<unsigned>(std::thread::hardware_concurrency(), 32);
    if (numThreads == 0) numThreads = 1;
    
    threads.reserve(numThreads);
    for (unsigned i = 0; i < numThreads; i++) {
        threads.emplace_back(worker);
    }

    // Espera mais eficiente
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [&userQueue](){ return userQueue.empty(); });
    }

    done = true;
    cv.notify_all();

    for (auto &thread : threads) {
        thread.join();
    }

    outputManager.flush();
}