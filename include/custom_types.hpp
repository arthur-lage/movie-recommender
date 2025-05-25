#pragma once

#include <unordered_map>
#include <unordered_set>


using namespace std;
using user_id_t = int;
using movie_id_t = int;

struct Rating {
    movie_id_t movie;
    float score;
    
    bool operator==(const Rating& other) const {
        return movie == other.movie;
    }
    
    struct Hash {
        size_t operator()(const Rating& r) const {
            return std::hash<int>()(r.movie);
        }
    };
};

using UsersAndMoviesData = std::unordered_map<user_id_t, std::unordered_set<Rating, Rating::Hash>>;