#include "rating_database.hpp"

void RatingDatabase::add_rating(int userId, MovieRating rating) {
    ratings[userId].push_back(rating);
}

unordered_map<int, vector<MovieRating>> RatingDatabase::get_ratings() {
    return ratings;
}