#pragma once

#include "output_manager.hpp"
#include "custom_types.hpp"
#include <unordered_map>
#include <mutex>
#include <vector>

class Recommender {
public:
    Recommender();
    void generateRecommendations(const UsersAndMoviesData& usersAndMovies, 
                               const MoviesData& movies);
    
private:
    std::unordered_map<user_id_t, double> userNorms;

    void precomputeUserNorms(const UsersAndMoviesData& data);
    double computeCosineSimilarity(user_id_t user1, user_id_t user2, 
                             const UsersAndMoviesData& data) const;

    std::vector<std::pair<user_id_t, double>> findSimilarUsers(
        user_id_t targetUser, 
        const UsersAndMoviesData& usersAndMovies,
        int k) const;

    std::unordered_map<movie_id_t, std::pair<double, double>> calculatePredictions(
        user_id_t targetUser,
        const std::vector<std::pair<user_id_t, double>>& similarUsers,
        const UsersAndMoviesData& usersAndMovies) const;
        
    std::vector<std::pair<movie_id_t, double>> generateTopRecommendations(
        const std::unordered_map<movie_id_t, std::pair<double, double>>& predictions,
        int maxRecs) const;
        
    void processUserRecommendations(
        user_id_t targetUser,
        const UsersAndMoviesData& usersAndMovies,
        const MoviesData& movies,
        OutputManager& outputManager,
        std::mutex& outputMutex) const;
};