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
    
    // Pular cabeçalho
    if (!getline(file, line)) {
        cerr << "Arquivo vazio ou erro na leitura" << endl;
        return;
    }

    while (getline(file, line)) {
        istringstream ss(line);
        string token;
        vector<string> tokens;
        
        // Separar todos os tokens primeiro
        while (getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        // Verificar se temos todos os campos necessários
        if (tokens.size() < 3) {
            cerr << "Linha incompleta: " << line << endl;
            continue;
        }

        try {
            int userId = stoi(tokens[0]);
            MovieRating rating_data;
            rating_data.movieId = stoi(tokens[1]);
            rating_data.rating = stod(tokens[2]);

            db.add_rating(userId, rating_data);
            
            // Debug (opcional)
            /*
            cout << "User: " << userId 
                 << " Movie: " << rating_data.movieId 
                 << " Rating: " << rating_data.rating 
                 << " Timestamp: " << rating_data.timestamp << endl;
            */
        } catch (const exception& e) {
            cerr << "Erro ao processar linha: " << line << endl;
            cerr << "Erro: " << e.what() << endl;
        }
    }
}