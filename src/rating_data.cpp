#include "rating_data.hpp"

using namespace std;

string get_formated_user_line(int userId, const std::vector<MovieRating>& ratings) {
    stringstream oss;
    oss << userId << " ";

    const size_t ratings_size = ratings.size(); 

    for(size_t i = 0; i < ratings_size; ++i) {
        oss << ratings[i].movieId << ":" << ratings[i].rating;
        if(i != ratings_size - 1) oss << " ";
    }

    return oss.str();
}