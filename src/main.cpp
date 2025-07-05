#include <iostream>

#include "custom_types.hpp"
#include "data_preprocessor.hpp"
#include "input_processor.hpp"
#include "movie_reader.hpp"
#include "recommender.hpp"

using namespace std;

int main()
{
    auto t1 = chrono::high_resolution_clock::now();

    auto t_ratings = chrono::high_resolution_clock::now();
    DataPreprocessor ratings_file("kaggle-data/ratings.csv", "r");
    ratings_file.process_ratings();
    cout << "Ratings processing: " << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - t_ratings).count() << " ms" << endl;

    auto t_input = chrono::high_resolution_clock::now();
    InputProcessor inputProcessor("datasets/input.dat");
    UsersAndMoviesData usersAndMovies;
    inputProcessor.process_input(usersAndMovies);
    cout << "Input processing: " << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - t_input).count() << " ms" << endl;

    auto t_movies = chrono::high_resolution_clock::now();
    MoviesData movies;
    MovieReader movieReader("kaggle-data/movies.csv", "r");
    movieReader.getMovies(movies);
    cout << "Movies loading: " << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - t_movies).count() << " ms" << endl;

    auto t_recommend = chrono::high_resolution_clock::now();
    Recommender recommender;
    recommender.generateRecommendations(usersAndMovies, movies);
    cout << "Recommendations generation: " << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - t_recommend).count() << " ms" << endl;

    cout << "Total time: " << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - t1).count() << " ms" << endl;
    return 0;
}