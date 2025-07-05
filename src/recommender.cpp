#include "recommender.hpp"
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

Recommender::Recommender() {}

void Recommender::precomputeUserNorms(const UsersAndMoviesData &data)
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

double Recommender::computeCosineSimilarity(user_id_t user1, user_id_t user2,
                                                  const UsersAndMoviesData &data) const
{
    auto it1 = data.find(user1);
    auto it2 = data.find(user2);
    if (it1 == data.end() || it2 == data.end())
        return 0.0;

    const auto &ratings1 = it1->second;
    const auto &ratings2 = it2->second;

    // Escolhe o menor conjunto para iterar
    const auto &smaller = ratings1.size() < ratings2.size() ? ratings1 : ratings2;
    const auto &larger = ratings1.size() < ratings2.size() ? ratings2 : ratings1;

    double dotProduct = 0.0;
    for (const Rating &r : smaller)
    {
        if (larger.find(Rating{r.movie, 0}) != larger.end())
        {
            dotProduct += r.score * larger.find(Rating{r.movie, 0})->score;
        }
    }

    double normProduct = userNorms.at(user1) * userNorms.at(user2);
    return (normProduct > 0) ? (dotProduct / normProduct) : 0.0;
}

std::vector<std::pair<user_id_t, double>> Recommender::findSimilarUsers(
    user_id_t targetUser, 
    const UsersAndMoviesData& usersAndMovies,
    int k) const 
{
    std::vector<std::pair<user_id_t, double>> similarUsers;
    
    for (const auto &[otherUser, _] : usersAndMovies) {
        if (targetUser == otherUser) continue;

        double similarity = computeCosineSimilarity(targetUser, otherUser, usersAndMovies);
        if (similarity > 0.2) {
            similarUsers.emplace_back(otherUser, similarity);
        }
    }

    if (similarUsers.size() > k) {
        std::partial_sort(similarUsers.begin(), similarUsers.begin() + k, similarUsers.end(),
                          [](const auto &a, const auto &b) { return a.second > b.second; });
        similarUsers.resize(k);
    } else {
        std::sort(similarUsers.begin(), similarUsers.end(),
                  [](const auto &a, const auto &b) { return a.second > b.second; });
    }
    
    return similarUsers;
}

void Recommender::generateRecommendations(const UsersAndMoviesData &usersAndMovies,
                                                const MoviesData &movies)
{
    OutputManager outputManager;
    std::vector<uint64_t> genDurations;
    std::vector<uint64_t> writeDurations;

    outputManager.openInOutcome("output.txt");
    std::ostringstream buffer;
    buffer.str().reserve(10000);

    precomputeUserNorms(usersAndMovies);

    std::unordered_set<user_id_t> usersToRecommend;
    std::ifstream file("datasets/explore.dat");
    if (!file.is_open())
    {
        std::cerr << "Erro ao abrir explore.dat" << std::endl;
        return;
    }

    user_id_t user;
    while (file >> user)
    {
        usersToRecommend.insert(user);
    }
    file.close();

    std::mutex queueMutex;
    std::mutex outputMutex;
    std::mutex consoleMutex;
    std::condition_variable cv;
    std::queue<user_id_t> userQueue;
    std::atomic<bool> done(false);
    std::vector<std::thread> threads;

    for (user_id_t user : usersToRecommend)
    {
        userQueue.push(user);
    }

    auto worker = [&]()
    {
        const int k = 15;             
        const int maxRecs = NUMBER_OF_RECOMMENDATIONS_PER_USER;

        while (true)
        {
            user_id_t targetUser;
            bool emptyQueue = false;

            {
                std::unique_lock<std::mutex> lock(queueMutex);
                cv.wait(lock, [&]()
                        { return done || !userQueue.empty(); });

                if (done && userQueue.empty())
                    return;
                if (userQueue.empty())
                    continue;

                targetUser = userQueue.front();
                userQueue.pop();
                emptyQueue = userQueue.empty();
            }

            if (emptyQueue)
            {
                cv.notify_one();
            }

            if (usersAndMovies.find(targetUser) == usersAndMovies.end())
            {
                std::lock_guard<std::mutex> consoleLock(consoleMutex);
                std::cout << "Usuário " << targetUser << " não encontrado no dataset.\n";
                continue;
            }

            std::vector<std::pair<user_id_t, double>> similarUsers;
            for (const auto &[otherUser, _] : usersAndMovies)
            {
                if (targetUser == otherUser)
                    continue;

                double similarity = computeCosineSimilarity(targetUser, otherUser, usersAndMovies);
                if (similarity > 0.2)
                {
                    similarUsers.emplace_back(otherUser, similarity);
                }
            }

            if (similarUsers.size() > k)
            {
                std::partial_sort(similarUsers.begin(), similarUsers.begin() + k, similarUsers.end(),
                                  [](const auto &a, const auto &b)
                                  { return a.second > b.second; });
                similarUsers.resize(k);
            }
            else
            {
                std::sort(similarUsers.begin(), similarUsers.end(),
                          [](const auto &a, const auto &b)
                          { return a.second > b.second; });
            }

            std::unordered_map<movie_id_t, std::pair<double, double>> predictions;
            const auto &targetRatings = usersAndMovies.at(targetUser);

            for (const auto &[similarUser, similarity] : similarUsers)
            {
                for (const Rating &r : usersAndMovies.at(similarUser))
                {
                    if (targetRatings.find(Rating{r.movie, 0}) == targetRatings.end())
                    {
                        predictions[r.movie].first += r.score * similarity;
                        predictions[r.movie].second += similarity;
                    }
                }
            }

            std::vector<std::pair<movie_id_t, double>> recommendations;
            for (const auto &[movie, scores] : predictions)
            {
                if (scores.second > 0)
                {
                    recommendations.emplace_back(movie, scores.first / scores.second);
                }
            }

            if (recommendations.size() > maxRecs)
            {
                std::partial_sort(recommendations.begin(), recommendations.begin() + maxRecs,
                                  recommendations.end(),
                                  [](const auto &a, const auto &b)
                                  { return a.second > b.second; });
                recommendations.resize(maxRecs);
            }
            else
            {
                std::sort(recommendations.begin(), recommendations.end(),
                          [](const auto &a, const auto &b)
                          { return a.second > b.second; });
            }

            std::ostringstream buffer;
            buffer << "\n\nRecommendations for user " << targetUser << ":\n\n";
            for (const auto &[movieId, score] : recommendations)
            {
                auto it = movies.find(movieId);
                if (it != movies.end())
                {
                    buffer << it->second << " (Score: " << score << ")\n";
                }
            }

            {
                std::lock_guard<std::mutex> outputLock(outputMutex);
                outputManager.write(buffer.str().c_str(), buffer.str().size());
            }
        }
        // while(true) {
        //     user_id_t targetUser;
        //     bool emptyQueue = false;

        //     {
        //         std::unique_lock<std::mutex> lock(queueMutex);
        //         cv.wait(lock, [&]() { return done || !userQueue.empty(); });

        //         if (done && userQueue.empty()) return;
        //         if (userQueue.empty()) continue;

        //         targetUser = userQueue.front();
        //         userQueue.pop();
        //         emptyQueue = userQueue.empty();
        //     }

        //     if(emptyQueue) {
        //         cv.notify_one();
        //     }

        //     processUserRecommendations(targetUser, usersAndMovies, movies, outputManager, outputMutex);
        // }
    };

    unsigned numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0)
        numThreads = 1;
    for (unsigned i = 0; i < numThreads; i++)
    {
        threads.emplace_back(worker);
    }

    {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [&userQueue]()
                { return userQueue.empty(); });
    }

    done = true;
    cv.notify_all();

    for (auto &thread : threads)
    {
        thread.join();
    }

    outputManager.flush();
}