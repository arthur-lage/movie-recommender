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
#include "recommender_manhattan.hpp"

using namespace std;

// Seleciona usuários aleatórios para o explore.dat
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

// Escreve os usuários sorteados no explore.dat
void writeExploreFile(const std::vector<int>& users, const std::string& filename = "datasets/explore.dat") {
    std::ofstream outFile(filename);
    
    if (!outFile.is_open()) {
        std::cerr << "Erro: Não foi possível abrir o arquivo " << filename << " para escrita!\n";
        return;
    }

    for (int userId : users) {
        outFile << userId << "\n";
    }

    outFile.close();
    std::cout << "Escreveu com sucesso " << users.size() << " usuários em " << filename << "\n";
}

int main() {
    InputPreprocessor ratings_file("kaggle-data/ratings.csv", "r");

    cout << "\nGERANDO INPUT.DAT...\n" << endl;
    
    auto start = chrono::high_resolution_clock::now();
    ratings_file.process_ratings();
    auto generateInputDuration = chrono::duration_cast<chrono::milliseconds>(
        chrono::high_resolution_clock::now() - start);

    cout << "\nFINALIZADO. LEVOU " << generateInputDuration.count() << " MS.\n" << endl;

    // Lê os dados processados
    BinaryReader binary_reader("datasets/input.dat");
    UsersAndMoviesData usersAndMovies;
    binary_reader.process_input(usersAndMovies);

    // Lê os filmes
    MoviesData movies;
    MovieReader movieReader("kaggle-data/movies.csv", "r");
    movieReader.getMovies(movies);
    
    // Gera explore.dat com usuários aleatórios
    std::vector<int> randomUsers = selectRandomUsers(usersAndMovies);
    writeExploreFile(randomUsers);
    
    // Recomendação via Manhattan
    RecommenderManhattan recommender;
    recommender.generateRecommendations(usersAndMovies, movies);

    return 0;
}
