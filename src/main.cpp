#include <iostream>

#include "custom_types.hpp"
#include "data_preprocessor.hpp"
#include "input_processor.hpp"
#include "movie_reader.hpp"
#include "recommender.hpp"

using namespace std;

int main() {
    DataPreprocessor ratings_file("kaggle-data/ratings.csv", "r");
    ratings_file.process_ratings();

    InputProcessor inputProcessor("datasets/input.dat");
    UsersAndMoviesData usersAndMovies;
    inputProcessor.process_input(usersAndMovies);

    MoviesData movies;
    MovieReader movieReader("kaggle-data/movies.csv", "r");
    movieReader.getMovies(movies);
    
    Recommender recommender;
    recommender.generateRecommendations(usersAndMovies, movies);

    return 0;
}