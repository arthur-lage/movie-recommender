#include "recommender_euclidean.hpp"
#include "config.hpp"
#include "binary_reader.hpp"

#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>
#include <chrono>
#include <algorithm>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>


RecommenderEuclidean::RecommenderEuclidean() {}

void read_explore_file(unordered_set<int>& usersToRecommend) {
    FILE* file = fopen("datasets/explore.dat", "r");
    if(!file) {
        printf("Não foi possível ler o explore.dat\n");
        exit(1);
    }

    char line[256];

    while(fgets(line, 256, file)) 
    {
        char* end;
        long num = strtol(line, &end, 10);

        if (end != line) {
            usersToRecommend.insert(num);
        }
    }

    fclose(file);
}

double computeDistance(const unordered_set<Rating, Rating::Hash>& user1,
                      const unordered_set<Rating, Rating::Hash>& user2) {
    double sum = 0.0;
    int common = 0;
    
    for (const Rating& r1 : user1) {
        auto it = user2.find(Rating{r1.movie, 0});  // Search by movie ID only
        if (it != user2.end()) {
            double diff = r1.score - it->score;
            sum += diff * diff;
            common++;
        }
    }
    return (common > 0) ? sqrt(sum) : INFINITY;
}

vector<pair<int, double>> findSimilarUsers(int targetUserId,
                                          const UsersAndMoviesData& data,
                                          int k = 5) {
    vector<pair<int, double>> similarities;
    const auto& targetRatings = data.at(targetUserId);

    for (const auto& [userId, ratings] : data) {
        if (userId == targetUserId) continue;
        
        double distance = computeDistance(targetRatings, ratings);
        if (distance != INFINITY) {
            similarities.emplace_back(userId, distance);
        }
    }

    // Sort by ascending distance (lower = more similar)
    sort(similarities.begin(), similarities.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });

    if (similarities.size() > k) similarities.resize(k);
    return similarities;
}


vector<pair<int, float>> getRecommendations(int targetUserId,
                                           const vector<pair<int, double>>& similarUsers,
                                           const UsersAndMoviesData& data) {
    const auto& targetRatings = data.at(targetUserId);
    unordered_map<int, pair<double, double>> recommendations;  // movie: (sum, weight)

    for (const auto& [similarUserId, distance] : similarUsers) {
        const auto& similarUserRatings = data.at(similarUserId);
        double weight = 1.0 / (1.0 + distance);

        for (const Rating& r : similarUserRatings) {
            // Check if target hasn't rated this movie
            if (!targetRatings.count(Rating{r.movie, 0})) {
                recommendations[r.movie].first += r.score * weight;
                recommendations[r.movie].second += weight;
            }
        }
    }

    // Convert to final scores
    vector<pair<int, float>> results;
    for (const auto& [movie, scores] : recommendations) {
        if (scores.second > 0) {
            results.emplace_back(movie, scores.first / scores.second);
        }
    }


 sort(results.begin(), results.end(), [](auto& a, auto& b) { 
        return a.second > b.second; 
    });
    
    results.erase(
        results.begin() + min(NUMBER_OF_RECOMMENDATIONS_PER_USER, (int)results.size()), 
        results.end()
    );

    return results;
    return results;
}

void RecommenderEuclidean::generateRecommendations(const UsersAndMoviesData& usersAndMovies, const MoviesData& movies) {
    unordered_set<int> usersToRecommend;
    read_explore_file(usersToRecommend);

    vector<uint64_t> durations;
    
    // 10 most similar users
    const int k = 10;

    for(int currentUser : usersToRecommend) {
        auto startGenRec = std::chrono::high_resolution_clock::now();

        auto similarUsers = findSimilarUsers(currentUser, usersAndMovies, k);
        auto recommendations = getRecommendations(currentUser, similarUsers, usersAndMovies);

        auto timeToGen = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startGenRec);

        durations.insert(durations.begin(), timeToGen.count());
        std::cout << "\n\n Recommendations for user " << currentUser << ":\n" << std::endl;

        for (const auto& [movieId, score] : recommendations) {
            auto it = movies.find(movieId);
            
            if(it != movies.end()) {
                cout << it->second << " (Score: " << score << ")\n";
            }
        }

        std::cout << "\n\n";
    }

    for(const auto& dur : durations) {
        std::cout << "Took " << dur << " ms to generate this rec." << std::endl;
    }
}