#include "config.hpp"
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

InputPreprocessor::InputPreprocessor(const char* filename) : FileHandler(filename) {}

using CombinedKey = uint64_t;

constexpr CombinedKey make_key(int user, int movie) {
    return (static_cast<uint64_t>(user) << 32) | static_cast<uint32_t>(movie);
}

struct Rating {
    int movie;
    float score;
    
    // Required for unordered_set
    bool operator==(const Rating& other) const {
        return movie == other.movie;
    }
    
    // Custom hash for Rating
    struct Hash {
        size_t operator()(const Rating& r) const {
            return hash<int>()(r.movie);
        }
    };
};

void read_data(ssize_t& read, char* line, size_t& len, FILE* file, unordered_map<int, unordered_set<Rating, Rating::Hash>>& user_data, vector<uint16_t>& movie_counter) {
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

        // Inserir rating único e contar filmes
        Rating rating{movie, score};
        if (user_data[user].insert(rating).second) {
            movie_counter[movie]++;
        }
    }
}

void write_data_to_output(unordered_map<int, unordered_set<Rating, Rating::Hash>>& user_data, vector<uint16_t>& movie_counter) {
    const char* output_path = "datasets/input.dat";
    const size_t BUFFER_SIZE = 1 << 20; // 1MB

    FILE* output_file = fopen(output_path, "w");
    char buffer[BUFFER_SIZE];
    size_t offset = 0;

    for (const auto& [user, ratings] : user_data) {
        if (ratings.size() < 50) continue;

        char line_buffer[16384]; // 16KB por linha
        int pos = snprintf(line_buffer, sizeof(line_buffer), "%d", user);

        for (const auto& rating : ratings) {
            if (movie_counter[rating.movie] >= 50 && pos < (int)sizeof(line_buffer) - 256) {
                pos += snprintf(
                    line_buffer + pos,
                    sizeof(line_buffer) - pos,
                    " %d:%.1f",
                    rating.movie,
                    rating.score
                );
            }
        }

        // Gerenciamento de buffer
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
    vector<uint16_t> movie_counter(230000+1, 0);
    unordered_map<int, unordered_set<Rating, Rating::Hash>> user_data;

    movie_counter.reserve(62000);
    user_data.reserve(103000);

    char *line = nullptr;
    size_t len = 0;
    ssize_t read;

    read = getline(&line, &len, file);
    
    read_data(read, line, len, file, user_data, movie_counter);
    write_data_to_output(user_data, movie_counter);
    
    free(line);
}
