#ifndef RECOMMENDER_LSH_HPP
#define RECOMMENDER_LSH_HPP

#include "config.hpp"
#include "custom_types.hpp"
#include "utils.hpp"

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>

class RecommenderLSH {
public:
    RecommenderLSH(size_t num_bands = 20, size_t band_size = 5, size_t num_hashes = 100);
    
    void buildIndex(const UsersAndMoviesData& data);
    std::vector<std::pair<user_id_t, double>> findSimilarUsers(user_id_t user, 
                                                           double threshold = 0.8) const;
    void generateRecommendations(const UsersAndMoviesData& usersAndMovies,
                              const MoviesData& movies);

private:
    struct HashFunction {
        uint64_t a, b, c;
        
        uint64_t operator()(uint64_t x) const {
            return (a * x + b) % c;
        }
    };
    
    double computeCosineSimilarity(user_id_t user1, user_id_t user2) const;
    void precomputeUserNorms();
    std::vector<uint64_t> computeMinHashSignature(const std::unordered_set<movie_id_t>& movies) const;
    
    size_t num_bands;
    size_t band_size;
    size_t num_hashes;
    
    std::vector<HashFunction> hash_functions;
    std::vector<std::unordered_map<std::string, std::vector<user_id_t>>> lsh_bands;
    std::unordered_map<user_id_t, std::unordered_set<movie_id_t>> user_movies;
    std::unordered_map<user_id_t, double> userNorms;
    UsersAndMoviesData users_data;
    
    static constexpr uint64_t LARGE_PRIME = 4294967311;
};

#endif // RECOMMENDER_LSH_HPP