#include "config.hpp"
#include "file_writer.hpp"
#include "file_reader.hpp"

#include <iostream>
#include <sstream>
#include <utility>
#include <string.h>
#include <unordered_set>

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);  // Hash do primeiro elemento
        auto h2 = std::hash<T2>{}(p.second); // Hash do segundo elemento
        return h1 ^ (h2 << 1); // Combina os dois hashes
    }
};

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
    FileWriter inputFile("datasets/input.dat");
    
    // Pular cabeçalho
    if (!getline(file, line)) {
        cerr << "Arquivo vazio ou erro na leitura" << endl;
        return;
    }

    unordered_map<int, int> user_counts;
    unordered_map<int, int> movie_counts;
    unordered_set<pair<int, int>, pair_hash> unique_ratings;

    while (getline(file, line)) {
        istringstream ss(line);
        string token;
        vector<string> tokens;
        
        while (getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        if (tokens.size() < 3) {
            continue;
        }

        try {
            int userId = stoi(tokens[0]);
            int movieId = stoi(tokens[1]);

            user_counts[userId]++;
            movie_counts[movieId]++;
        } catch (const exception& e) {
            cerr << "Erro ao processar linha: " << line << endl;
            cerr << "Erro: " << e.what() << endl;
        }
    }

    file.clear();
    file.seekg(0);
    getline(file, line);
     
    while (getline(file, line)) {
        istringstream ss(line);
        string token;
        vector<string> tokens;
        
        while (getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        if (tokens.size() < 3) {
            continue;
        }

        try {
            int userId = stoi(tokens[0]);
            int movieId = stoi(tokens[1]);

            if(user_counts[userId] >= MININUM_REVIEW_COUNT_PER_USER && movie_counts[movieId] >= MININUM_REVIEW_COUNT_PER_MOVIE) {
                auto key = make_pair(userId, movieId);

                if(unique_ratings.find(key) == unique_ratings.end()) {
                    int movieRating = stoi(tokens[2]);

                    MovieRating rating_data;
                    rating_data.movieId = movieId;
                    rating_data.rating = movieRating;

                    db.add_rating(userId, rating_data);

                    unique_ratings.insert(key);
                }
            }
            
        } catch (const exception& e) {
            cerr << "Erro ao processar linha: " << line << endl;
            cerr << "Erro: " << e.what() << endl;
        }
    }

    for (const auto& [userId, ratings] : db.getRatings()) {
        inputFile.write_line(get_formated_user_line(userId, ratings));
    }
}