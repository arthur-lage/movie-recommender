#include <iostream>
#include <chrono>

#include "custom_types.hpp"
#include "input_preprocessor.hpp"
#include "binary_reader.hpp"

using namespace std;

int main() {
    InputPreprocessor ratings_file("kaggle-data/ratings.csv", "r");

    cout << "\nGENERATING INPUT.DAT...\n" << endl;
    
    auto start = chrono::high_resolution_clock::now();

    ratings_file.process_ratings();

    auto generateInputDuration = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start);

    cout << "\nFINISHED GENERATING INPUT.DAT. \n\nTOOK " << generateInputDuration.count() << " MS." << endl;

    BinaryReader binary_reader("datasets/input.dat", "rb");
    UsersAndMoviesData usersAndMovies;
    binary_reader.process_input(usersAndMovies);

    // Mostra os dados de 10 usuarios, mostrando 5 avaliacoes de cada um
    int cont2 = 0;
    for (const auto& [user_id, ratings] : usersAndMovies) {
        if(cont2 == 10) break;
        std::cout << "User " << user_id << " (" << ratings.size() << " ratings):\n";
        int count = 0;
        for(const auto& rating : ratings) {
            if(count++ >= 5) break;
            std::cout << "  Movie " << rating.movie << ": " << rating.score << "\n";
        }
        std::cout << "\n";
        cont2++;
    }

    return 0;
}