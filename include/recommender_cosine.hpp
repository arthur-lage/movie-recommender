#pragma once

#include "custom_types.hpp"
#include <unordered_map>
#include <vector>

class RecommenderCosine {
public:
    RecommenderCosine();
    void generateRecommendations(const UsersAndMoviesData& usersAndMovies, 
                               const MoviesData& movies);
    
private:
    void precomputeUserNorms(const UsersAndMoviesData& data);
    double computeCosineSimilarity(user_id_t user1, user_id_t user2, 
                                 const UsersAndMoviesData& data) const;
    
    std::unordered_map<user_id_t, double> userNorms;
};