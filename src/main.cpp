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
#include "recommender_Pearson.hpp"
#include "output_manager.hpp"

using namespace std;

std::vector<int> selectRandomUsers(const UsersAndMoviesData& usersAndMovies, int sampleSize = 5) {
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
    auto start = chrono::high_resolution_clock::now();
    
    auto ratings_start = chrono::high_resolution_clock::now();

    InputPreprocessor ratings_file("kaggle-data/ratings.csv", "r");
    
    ratings_file.process_ratings();

    auto ratings_duration = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - ratings_start);

    auto binary_start = chrono::high_resolution_clock::now();

    BinaryReader binary_reader("datasets/input.dat");
    UsersAndMoviesData usersAndMovies;
    binary_reader.process_input(usersAndMovies);

    auto binary_duration = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - binary_start);

    auto movies_start = chrono::high_resolution_clock::now();
    
    MoviesData movies;
    MovieReader movieReader("kaggle-data/movies.csv", "r");
    movieReader.getMovies(movies);
    
    auto movies_duration = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - movies_start);

    auto rec_start = chrono::high_resolution_clock::now();
    
    RecommenderPearson recommenderPearson;
    recommenderPearson.generateRecommendations(usersAndMovies, movies);
    
    auto rec_duration = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - rec_start);

    auto end = chrono::high_resolution_clock::now();
    auto total_duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    
    cout << "\nTEMPOS DE EXECUÇÃO:\n" 
         << "1. Processamento de ratings: " << ratings_duration.count() << " ms\n"
         << "2. Leitura binária: " << binary_duration.count() << " ms\n"
         << "3. Leitura de filmes: " << movies_duration.count() << " ms\n"
         << "4. Geração de recomendações: " << rec_duration.count() << " ms\n"
         << "---------------------------------\n"
         << "TEMPO TOTAL: " << total_duration.count() << " ms\n" << endl;
    return 0;
}