#include <iostream>

#include "rating_data.hpp"
#include "file_writer.hpp"
#include "file_reader.hpp"
#include "rating_database.hpp"

using namespace std;

int main () {
    RatingDatabase rating_db;
    FileReader ratings_file("kaggle-data/ratings_little.csv");
    ratings_file.process_ratings(rating_db);

    // for(const auto&cur : rating_db.getRatings()) {
    //     cout << "Usuário: " << cur.first << " -" << endl;

    //     for(const auto& rat : cur.second) {
    //         cout << rat.movieId << ":" << rat.rating << endl;
    //     }
    // }

    return 0;
}