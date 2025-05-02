#pragma once

#include <vector>
#include <unordered_map>

#include "rating_data.hpp"

using namespace std;

class RatingDatabase {
    private:
        unordered_map<int, vector<MovieRating>> ratings;
    public:
        void add_rating(int userId, MovieRating rating);
        unordered_map<int, vector<MovieRating>> getRatings();
};