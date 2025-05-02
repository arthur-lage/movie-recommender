#include "file_reader.hpp"

#include <iostream>
#include <sstream>
#include <string.h>

using namespace std;

FileReader::FileReader(std::string filename) {
    file = ifstream(filename);

    if (!file.is_open()) {
        cerr << "Erro ao abrir arquivo" << std::endl;
        return;
    }
}

FileReader::~FileReader() {
    close();
}

void FileReader::close() {
    file.close();
}

void FileReader::process_ratings(RatingDatabase& db) {
    string line;
    int current_index = 0;

    // pula primeira linha
    getline(file, line);

    while(getline(file, line)) {
        istringstream ss(line);
        std::string token;
        
        MovieRating rating_data;
        int userId;

        while(getline(ss, token, ',')){
            switch(current_index) {
                case 0:
                    userId = stoi(token);
                    cout << "user id: " << userId << endl;
                    break;
                case 1:
                    rating_data.movieId = stoi(token);
                    cout << "movie id: " << rating_data.movieId << endl;
                    break;
                case 2:
                    rating_data.rating = stod(token);
                    cout << "rating: " << rating_data.rating << endl;
                    break;
            }

            if (current_index == 2) {
            current_index = 0;
            } 
                current_index++;


            db.add_rating(userId, rating_data);
        }
    }
}