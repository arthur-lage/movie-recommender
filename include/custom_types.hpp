#pragma once

#include <unordered_map>
#include <unordered_set>


using namespace std;

struct Rating {
    int movie;
    float score;
    
    // Required for unordered_set
    bool operator==(const Rating& other) const {
        return movie == other.movie;
    }
    
    // Custom hash for Rating
    struct Hash {
        size_t operator()(const Rating& r) const {
            return hash<int>()(r.movie);
        }
    };
};

using UsersAndMoviesData = unordered_map<int, unordered_set<Rating, Rating::Hash>>;