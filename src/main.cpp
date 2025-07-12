#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <random>


#include "custom_types.hpp"
#include "data_preprocessor.hpp"
#include "input_processor.hpp"
#include "recommender.hpp"

using namespace std;

int main() {
    DataPreprocessor ratings_file("kaggle-data/ratings.csv", "r");
    ratings_file.process_ratings();

    InputProcessor inputProcessor("datasets/input.dat");
    UsersAndMoviesData usersAndMovies;
    inputProcessor.process_input(usersAndMovies);

    Recommender recommender;
    recommender.generateRecommendations(usersAndMovies);

    return 0;
}