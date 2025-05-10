#include "config.hpp"
#include "file_writer.hpp"
#include "file_reader.hpp"
#include "custom_types.hpp"

#include <utility>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <iostream>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdint>

using namespace std;

FileReader::FileReader(const char* filename) {
    file = fopen(filename, "r");

    if (!file) {
        cout << "Erro ao abrir arquivo" << endl;
        exit(1);
    }
}

FileReader::~FileReader() {
    close();
}

void FileReader::close() {
    if (file) {
        fclose(file);
        file = nullptr;
    }
}

using CombinedKey = uint64_t;

constexpr CombinedKey make_key(int user, int movie) {
    return (static_cast<uint64_t>(user) << 32) | static_cast<uint32_t>(movie);
}

void count_users_and_movies(FILE* file, char* line, size_t& len, ssize_t& read, unordered_set<CombinedKey>& unique_pairs, unordered_map<int, int>& user_counts, unordered_map<int, int>& movie_counts) {
    while((read = getline(&line, &len, file)) != -1) {
        char* p = line;
        char* end;
        
        int user = strtol(p, &end, 10);
        if (*end != ',') continue;
        p = end + 1;
        
        int movie = strtol(p, &end, 10);
        if (*end != ',') continue;
        
        CombinedKey key = make_key(user, movie);
        if (unique_pairs.insert(key).second) {
            user_counts[user]++;
            movie_counts[movie]++;
        }
    }
}

void find_valid_users_and_movies (unordered_set<int>& valid_users, unordered_set<int>& valid_movies, unordered_map<int,int>& user_counts, unordered_map<int,int>& movie_counts) {
    for (const auto& [user, count] : user_counts) {
        if (count >= MININUM_REVIEW_COUNT_PER_USER) valid_users.insert(user);
    }

    for (const auto& [movie, count] : movie_counts) {
        if (count >= MININUM_REVIEW_COUNT_PER_MOVIE) valid_movies.insert(movie);
    }
}

void filter_and_collect_valid_ratings(FILE* file, char* line, size_t& len, ssize_t& read, unordered_set<CombinedKey>& unique_pairs, unordered_set<int>& valid_users, unordered_set<int>& valid_movies, UserRatings& userRatings) {
    while ((read = getline(&line, &len, file)) != -1) {
        char* p = line;
        char* end;
        
        int user = strtol(p, &end, 10);
        if (*end != ',') continue;
        p = end + 1;
        
        int movie = strtol(p, &end, 10);
        if (*end != ',') continue;
        p = end + 1;
        
        float rating = strtof(p, &end);
        if (end == p) continue;

        CombinedKey key = make_key(user, movie);
        if (unique_pairs.count(key) && 
            valid_users.count(user) && 
            valid_movies.count(movie)) {
            userRatings[user].emplace_back(movie, rating);
            unique_pairs.erase(key);
        }
    }
}

void FileReader::process_ratings() {
    const char* input_path = "kaggle-data/ratings.csv";
    const char* output_path = "datasets/input.dat";

    char* line = nullptr;
    size_t len = 0;
    ssize_t read;

    // pula header
    getline(&line, &len, file);
    
    unordered_set<CombinedKey> unique_pairs;
    unordered_map<int, int> user_counts;
    unordered_map<int, int> movie_counts;
    unordered_set<int> valid_users;
    unordered_set<int> valid_movies;


    unique_pairs.reserve(25000000);
    user_counts.reserve(200000);
    movie_counts.reserve(200000);

    count_users_and_movies(file, line, len, read, unique_pairs, user_counts, movie_counts);

    free(line);
    close();

    find_valid_users_and_movies(valid_users, valid_movies, user_counts, movie_counts);

    // processar entradas
    file = fopen(input_path, "r");
    getline(&line, &len, file);

    UserRatings userRatings;
    userRatings.reserve(valid_users.size());

    filter_and_collect_valid_ratings(file, line, len, read, unique_pairs, valid_users, valid_movies, userRatings);

    free(line);
    close();

    // escrever no arquivo input.dat

    FileWriter output_file(output_path);
    output_file.generateInputFile(userRatings);
    output_file.close();
}

