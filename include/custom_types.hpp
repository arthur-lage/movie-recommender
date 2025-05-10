#pragma once

#include <unordered_map>
#include <vector>
#include <utility>

using namespace std;

struct MovieRating {
    int movieId;
    float rating;
};

using UserRatings = unordered_map<int, vector<pair<int, float>>>;
