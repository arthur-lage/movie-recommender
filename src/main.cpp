#include <iostream>
#include <chrono>

#include "input_preprocessor.hpp"

using namespace std;

int main() {
    InputPreprocessor ratings_file("kaggle-data/ratings.csv");
    cout << "GENERATING INPUT.DAT...\n" << endl;
    
    auto start = chrono::high_resolution_clock::now();

    ratings_file.process_ratings();

    auto generateInputDuration = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start);

    cout << "\nFINISHED GENERATING INPUT.DAT. \n\nTOOK " << generateInputDuration.count() << " MS." << endl;

    return 0;
}