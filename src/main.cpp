#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <algorithm>
#include <random>


#include "custom_types.hpp"
#include "input_preprocessor.hpp"
#include "binary_reader.hpp"
#include "movie_reader.hpp"
#include "recommender_cosine.hpp"

using namespace std;

std::vector<int> selectRandomUsers(const UsersAndMoviesData& usersAndMovies, int sampleSize = 50) {
    std::vector<int> allUsers;
    for (const auto& [userId, _] : usersAndMovies) {
        allUsers.push_back(userId);
    }

    sampleSize = std::min(sampleSize, static_cast<int>(allUsers.size()));
    if (sampleSize <= 0) return {};

    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(allUsers.begin(), allUsers.end(), rng);

    return {allUsers.begin(), allUsers.begin() + sampleSize};
}

// New function to write users to explore.dat
void writeExploreFile(const std::vector<int>& users, const std::string& filename = "datasets/explore.dat") {
    std::ofstream outFile(filename);
    
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing!\n";
        return;
    }

    for (int userId : users) {
        outFile << userId << "\n";
    }

    outFile.close();
    std::cout << "Successfully wrote " << users.size() << " users to " << filename << "\n";
}

int main() {
    InputPreprocessor ratings_file("kaggle-data/ratings.csv", "r");

    cout << "\nGENERATING INPUT.DAT...\n" << endl;
    
    auto start = chrono::high_resolution_clock::now();

    ratings_file.process_ratings();

    auto generateInputDuration = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start);

    cout << "\nFINISHED GENERATING INPUT.DAT. \n\nTOOK " << generateInputDuration.count() << " MS." << endl;

    BinaryReader binary_reader("datasets/input.dat");
    UsersAndMoviesData usersAndMovies;
    binary_reader.process_input(usersAndMovies);

    // read movies

    MoviesData movies;
    MovieReader movieReader("kaggle-data/movies.csv", "r");
    movieReader.getMovies(movies);
    
    // generate random explore.dat

    // std::vector<int> randomUsers = selectRandomUsers(usersAndMovies);
    // writeExploreFile(randomUsers);
    
    RecommenderCosine recommenderCosine;
    recommenderCosine.generateRecommendations(usersAndMovies, movies);

    return 0;
}