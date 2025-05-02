#pragma once

#include <string>
#include <vector>
#include <sstream>

using namespace std;

struct MovieRating {
    int movieId;
    double rating;
};

string get_formated_user_line(int userId, const std::vector<MovieRating>& ratings);