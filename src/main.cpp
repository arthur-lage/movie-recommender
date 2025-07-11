#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <algorithm>
#include <random>


#include "custom_types.hpp"
#include "data_preprocessor.hpp"
#include "input_processor.hpp"
#include "movie_reader.hpp"
#include "recommender.hpp"

using namespace std;

int main() {
    auto start = chrono::high_resolution_clock::now();

    auto ppS = chrono::high_resolution_clock::now();

    DataPreprocessor ratings_file("kaggle-data/ratings.csv", "r");
    ratings_file.process_ratings();

    auto ppE = chrono::high_resolution_clock::now();

    auto umrS = chrono::high_resolution_clock::now();

    InputProcessor inputProcessor("datasets/input.dat");
    UsersAndMoviesData usersAndMovies;
    inputProcessor.process_input(usersAndMovies);

    auto umrE = chrono::high_resolution_clock::now();

    auto rmS = chrono::high_resolution_clock::now();
    MoviesData movies;
    MovieReader movieReader("kaggle-data/movies.csv", "r");
    movieReader.getMovies(movies);

    auto rmE = chrono::high_resolution_clock::now();
    auto recS = chrono::high_resolution_clock::now();
    
    Recommender recommender;
    recommender.generateRecommendations(usersAndMovies, movies);

    auto recE = chrono::high_resolution_clock::now();

    auto progEnd = chrono::high_resolution_clock::now();
    auto dur = chrono::duration_cast<chrono::milliseconds>(progEnd - start);

    cout << "\n====== TIMES\n";
    cout << "Time to pre process: "<< chrono::duration_cast<chrono::milliseconds>(ppS - ppE).count() << " ms" << endl;
    cout << "Time to read users and movies: "<< chrono::duration_cast<chrono::milliseconds>(umrS - umrE).count() << " ms" << endl;
    cout << "Time to read movies: " << chrono::duration_cast<chrono::milliseconds>(rmS - rmE).count() << " ms" << endl;
    cout << "Time to recommend: " << chrono::duration_cast<chrono::milliseconds>(recS - recE).count() << " ms" << endl;

    cout << "Total time: " << dur.count() << " ms" << endl;

    return 0;
}