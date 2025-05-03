#include <iostream>

#include "rating_data.hpp"
#include "file_writer.hpp"
#include "file_reader.hpp"
#include "rating_database.hpp"

using namespace std;

int main() {
    RatingDatabase rating_db;
    FileReader ratings_file("kaggle-data/ratings.csv");
    ratings_file.process_ratings(rating_db);

    return 0;
}