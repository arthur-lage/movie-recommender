#include "config.hpp"
#include "custom_types.hpp"
#include "input_preprocessor.hpp"
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

InputPreprocessor::InputPreprocessor(const char* filename, const char* mode) : FileHandler(filename, mode) {}

void read_data(
    ssize_t& read,
    char* line,
    size_t& len,
    FILE* file,
    unordered_map<int, unordered_set<Rating, Rating::Hash>>& user_data,
    unordered_map<int, int>& movie_counter) {
    
    while ((read = getline(&line, &len, file)) != -1) {
        int user, movie;
        float score;
        char* p = line;
        char* end;

        user = strtoul(p, &end, 10);
        if (*end != ',') continue;
        p = end + 1;
        
        movie = strtoul(p, &end, 10);
        if (*end != ',') continue;
        p = end + 1;
        
        score = strtof(p, &end);
        if (end == p) continue;

        Rating rating{movie, score};
        if (user_data[user].insert(rating).second) {
            movie_counter[movie]++;
        }
    }
}

void write_data_to_output(
    unordered_map<int, unordered_set<Rating, Rating::Hash>>& user_data,
    unordered_map<int, int>& movie_counter) {
    
    const char* output_path = "datasets/input.dat";
    const size_t BUFFER_SIZE = 1 << 20;
    FILE* output_file = fopen(output_path, "w");
    char buffer[BUFFER_SIZE];
    size_t offset = 0;

    for (const auto& [user, ratings] : user_data) {
        if (ratings.size() < MININUM_REVIEW_COUNT_PER_USER) continue;

        char line_buffer[16384];
        int pos = snprintf(line_buffer, sizeof(line_buffer), "%d", user);

        for (const auto& rating : ratings) {
            if (movie_counter[rating.movie] >= MININUM_REVIEW_COUNT_PER_MOVIE && 
                pos < (int)sizeof(line_buffer) - 256) {
                pos += snprintf(
                    line_buffer + pos,
                    sizeof(line_buffer) - pos,
                    " %d:%.1f",
                    rating.movie,
                    rating.score
                );
            }
        }

        line_buffer[pos++] = '\n';
        if (offset + pos > BUFFER_SIZE) {
            fwrite(buffer, 1, offset, output_file);
            offset = 0;
        }
        memcpy(buffer + offset, line_buffer, pos);
        offset += pos;
    }

    fwrite(buffer, 1, offset, output_file);
    fclose(output_file);
}

void InputPreprocessor::process_ratings() {
    char *line = nullptr;
    size_t len = 0;
    if (getline(&line, &len, file) == -1) {
        cerr << "Erro: CSV vazio!" << endl;
        free(line);
        return;
    }

    unordered_map<int, int> movie_counter;
    unordered_map<int, unordered_set<Rating, Rating::Hash>> user_data;

    ssize_t read;
    read_data(read, line, len, file, user_data, movie_counter);

    // Aplicar filtros
    const int MIN_USER = MININUM_REVIEW_COUNT_PER_USER;
    const int MIN_MOVIE = MININUM_REVIEW_COUNT_PER_MOVIE;

    for (auto it = user_data.begin(); it != user_data.end();) {
        if (it->second.size() < MIN_USER) {
            it = user_data.erase(it);
        } else {
            ++it;
        }
    }

    for (auto& [user, ratings] : user_data) {
        unordered_set<Rating, Rating::Hash> novos_ratings;
        for (const auto& rating : ratings) {
            if (movie_counter[rating.movie] >= MIN_MOVIE) {
                novos_ratings.insert(rating);
            }
        }
        ratings = novos_ratings;
    }

    write_data_to_output(user_data, movie_counter);

    
    free(line);
}