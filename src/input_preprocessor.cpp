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

InputPreprocessor::InputPreprocessor(const char *filename, const char *mode) : FileHandler(filename, mode) {}

using CombinedKey = uint64_t;

constexpr CombinedKey make_key(int user, int movie) {
    return (static_cast<uint64_t>(user) << 32) | static_cast<uint32_t>(movie);
}

// Parsing otimizado para inteiros
inline int fast_atoi(const char*& p) {
    int x = 0;
    while (*p >= '0' && *p <= '9') {
        x = x * 10 + (*p - '0');
        ++p;
    }
    return x;
}

void read_data(char*& line, size_t& capacity, FILE* file,
              std::unordered_map<int, std::unordered_set<Rating, Rating::Hash>>& user_data,
              std::vector<uint16_t>& movie_counter) {

    ssize_t read;
    while ((read = getline(&line, &capacity, file)) != -1) {
        const char* p = line;

        int user = fast_atoi(p);
        if (*p != ',') continue;
        ++p;

        int movie = fast_atoi(p);
        if (*p != ',') continue;
        ++p;

        char* endptr;
        float score = strtof(p, &endptr);
        if (endptr == p) continue;

        auto& user_ratings = user_data[user];

        if (user_ratings.emplace(movie, score).second &&
            static_cast<size_t>(movie) < movie_counter.size()) {
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

    read_data(line, len, file, user_data, movie_counter);
    write_data_to_output(user_data, movie_counter);

    free(line);
}