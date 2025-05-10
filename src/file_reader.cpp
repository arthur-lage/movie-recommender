#include "config.hpp"
#include "file_writer.hpp"
#include "file_reader.hpp"

#include <iostream>
// #include <sstream>
#include <utility>
#include <string.h>
#include <unordered_set>

#include <cstdio>
#include <cstring>

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
    // file = ifstream(filename);

    // if (!file.is_open()) {
    //     cerr << "Erro ao abrir arquivo" << std::endl;
    //     return;
    // }
}

FileReader::~FileReader() {
    close();
}

void FileReader::close() {
    // file.close();
}

void FileReader::process_ratings(RatingDatabase& db) {
    const char* input_path = "kaggle-data/ratings_little.csv";
    const char* output_path = "datasets/input.dat";
    const size_t BUFFER_SIZE = 1 << 20; // BUFFER DE 1 MEGA

    FILE* input_file = fopen(input_path, "r");

    if(!input_file) {
        printf("File not found");
        exit(1);
    }

    char* line = nullptr;
    size_t len = 0;
    ssize_t read;

    // pula header
    getline(&line, &len, input_file);

    while((read = getline(&line, &len, input_file)) != -1) {
        line[strlen(line) - 1] = '\0';
        int userId, movieId;
        float rating;
        if(sscanf(line, "%d,%d,%f", &userId, &movieId, &rating) == 3) {
            printf("User: %d, Movie: %d, Rating: %.1f\n", userId, movieId, rating);
        } else {
            printf("Error parsing line: %s\n", line);
        }
    }

    free(line);
    fclose(input_file);

    // escrever no arquivo input.dat

    FILE* output_file = fopen(output_path, "w");
    char* buffer = new char[BUFFER_SIZE];
    setvbuf(output_file, buffer, _IOFBF, BUFFER_SIZE);

    for (const auto& [userId, ratings] : output_map) {
        fprintf(output_file, "%d", userId);
        for(const auto& [movieId, rating] : ratings) {
            fprintf(output_file, " %d:%.1f", movieId, rating);
        }
        fprintf(output_file, "\n");
    }

    fclose(output_file);
    delete[] buffer;
}

// ============== OLD
// ============== OLD
// ============== OLD
// ============== OLD
// ============== OLD

// void FileReader::process_ratings(RatingDatabase& db) {
//     constexpr size_t EXPECTED_LINES = 25000000;
//     constexpr int MIN_REVIEWS = 50;

//     string line;
//     if (!getline(file, line)) {
//         cerr << "Arquivo vazio ou erro na leitura" << endl;
//         return;
//     }

//     // **1. Otimização: Pré-alocar estruturas para evitar realocações**
//     unordered_map<int, int> user_counts;
//     unordered_map<int, int> movie_counts;
//     user_counts.reserve(100000);   // Supondo ~100k usuários únicos
//     movie_counts.reserve(50000);   // Supondo ~50k filmes únicos

//     // **2. Otimização: Ler todo o arquivo de uma vez (se memória permitir)**
//     vector<string> lines;
//     lines.reserve(EXPECTED_LINES);

//     while (getline(file, line)) {
//         lines.push_back(std::move(line)); // Move evita cópia desnecessária
//     }

//     // **3. Primeira passagem: Contar reviews por usuário e filme**
//     for (const auto& line : lines) {
//         size_t comma1 = line.find(',');
//         if (comma1 == string::npos) continue;

//         size_t comma2 = line.find(',', comma1 + 1);
//         if (comma2 == string::npos) continue;

//         try {
//             int userId = stoi(line.substr(0, comma1));
//             int movieId = stoi(line.substr(comma1 + 1, comma2 - comma1 - 1));

//             user_counts[userId]++;
//             movie_counts[movieId]++;
//         } catch (...) {
//             continue; // Ignora linhas inválidas
//         }
//     }

//     // **4. Filtrar usuários e filmes que não atendem ao mínimo (50 reviews)**
//     unordered_set<int> valid_users;
//     unordered_set<int> valid_movies;
    
//     for (const auto& [user, count] : user_counts) {
//         if (count >= MIN_REVIEWS) valid_users.insert(user);
//     }
    
//     for (const auto& [movie, count] : movie_counts) {
//         if (count >= MIN_REVIEWS) valid_movies.insert(movie);
//     }

//     // **5. Segunda passagem: Processar apenas ratings válidos**
//     struct PairHash {
//         size_t operator()(const pair<int, int>& p) const {
//             return ((size_t)p.first << 32) | p.second; // Hash eficiente para pair<int,int>
//         }
//     };

//     unordered_set<pair<int, int>, PairHash> unique_ratings;
//     unique_ratings.reserve(valid_users.size() * 10); // Estimativa

//     FileWriter inputFile("datasets/input.dat");

//     for (const auto& line : lines) {
//         size_t comma1 = line.find(',');
//         if (comma1 == string::npos) continue;

//         size_t comma2 = line.find(',', comma1 + 1);
//         if (comma2 == string::npos) continue;

//         try {
//             int userId = stoi(line.substr(0, comma1));
//             int movieId = stoi(line.substr(comma1 + 1, comma2 - comma1 - 1));

//             // **6. Verificação rápida com unordered_set (O(1))**
//             if (valid_users.count(userId) && valid_movies.count(movieId)) {
//                 auto key = make_pair(userId, movieId);
//                 if (unique_ratings.insert(key).second) { // Evita duplicatas
//                     double rating = stod(line.substr(comma2 + 1));
//                     db.add_rating(userId, {movieId, rating});
//                 }
//             }
//         } catch (...) {
//             continue; // Ignora linhas inválidas
//         }
//     }

//     // **7. Escrever em batch (se possível)**
//     for (const auto& [user, ratings] : db.get_ratings()) {
//         inputFile.write_line(get_formated_user_line(user, ratings));
//     }

//     inputFile.close();
//     close();
// }

// void FileReader::process_ratings(RatingDatabase& db) {
//     string line;
    
//     if (!getline(file, line)) {
//         cerr << "Arquivo vazio ou erro na leitura" << endl;
//         return;
//     }

//     unordered_map<int, int> user_counts;
//     unordered_map<int, int> movie_counts;
//     unordered_set<pair<int, double>, pair_hash> unique_ratings;


//     // conta quantidades de avaliacoes e filmes para filtrar futuramente
//     while (getline(file, line)) {
//         istringstream ss(line);
//         string token;
//         vector<string> tokens;
        
//         while (getline(ss, token, ',')) {
//             tokens.push_back(token);
//         }

//         if (tokens.size() < 3) {
//             continue;
//         }

//         try {
//             int userId = stoi(tokens[0]);
//             int movieId = stoi(tokens[1]);

//             user_counts[userId]++;
//             movie_counts[movieId]++;
//         } catch (const exception& e) {
//             cerr << "Erro ao processar linha: " << line << endl;
//             cerr << "Erro: " << e.what() << endl;
//         }
//     }

//     file.clear();
//     file.seekg(0);
//     getline(file, line);
     
//     // percorre o arquivo novamente, salvando os filmes que serão usados no database
//     while (getline(file, line)) {
//         istringstream ss(line);
//         string token;
//         vector<string> tokens;
        
//         while (getline(ss, token, ',')) {
//             tokens.push_back(token);
//         }

//         if (tokens.size() < 3) {
//             continue;
//         }

//         try {
//             int userId = stoi(tokens[0]);
//             int movieId = stoi(tokens[1]);

//             if(user_counts[userId] >= MININUM_REVIEW_COUNT_PER_USER && movie_counts[movieId] >= MININUM_REVIEW_COUNT_PER_MOVIE) {
//                 auto key = make_pair(userId, movieId);

//                 if(unique_ratings.find(key) == unique_ratings.end()) {
//                     double movieRating = stod(tokens[2]);

//                     MovieRating rating_data;
//                     rating_data.movieId = movieId;
//                     rating_data.rating = movieRating;

//                     db.add_rating(userId, rating_data);

//                     unique_ratings.insert(key);
//                 }
//             }
            
//         } catch (const exception& e) {
//             cerr << "Erro ao processar linha: " << line << endl;
//             cerr << "Erro: " << e.what() << endl;
//         }
//     }

//     FileWriter inputFile("datasets/input.dat");

//     // formata e adiciona as informações no input.dat
//     for (const auto& [userId, ratings] : db.get_ratings()) {
//         inputFile.write_line(get_formated_user_line(userId, ratings));
//     }

//     // fechar arquivos abertos
//     inputFile.close();
//     close();
// }