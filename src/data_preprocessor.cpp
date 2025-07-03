#include "config.hpp"
#include "custom_types.hpp"
#include "data_preprocessor.hpp"
#include "utils.hpp"

#include <algorithm>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cmath>

using namespace std;

constexpr size_t READ_BUFFER_SIZE = 32 * 1024 * 1024;  // 32 MB
constexpr size_t WRITE_BUFFER_SIZE = 32 * 1024 * 1024; // 32 MB

DataPreprocessor::DataPreprocessor(const char *filename, const char *mode)
    : FileHandler(filename, mode) {}

#pragma pack(push, 1)
struct RatingEntry {
    uint32_t user;
    uint32_t movie;
    float rating;
};
#pragma pack(pop)

inline char* write_int(char* ptr, int value) {
    char temp[12];
    char* t = temp + sizeof(temp);
    *--t = '\0';
    
    do {
        *--t = '0' + (value % 10);
        value /= 10;
    } while (value > 0);
    
    size_t len = temp + sizeof(temp) - t - 1;
    memcpy(ptr, t, len);
    return ptr + len;
}

inline char* write_rating(char* ptr, float rating) {
    int irating = static_cast<int>(roundf(rating * 10));
    *ptr++ = '0' + irating / 10;
    *ptr++ = '.';
    *ptr++ = '0' + irating % 10;
    return ptr;
}

void write_preprocessed_input(vector<RatingEntry> valid_entries, vector<uint16_t> user_counter_valid) {
    FILE* output_file = fopen("datasets/input.dat", "wb");
    if (!output_file) {
        perror("Erro ao abrir arquivo de saída");
        return;
    }
    
    vector<char> out_buffer(WRITE_BUFFER_SIZE + 64);
    char* aligned_out = out_buffer.data();
    if (reinterpret_cast<uintptr_t>(aligned_out) % 64 != 0) {
        aligned_out += 64 - (reinterpret_cast<uintptr_t>(aligned_out) % 64);
    }
    
    size_t out_pos = 0;
    auto it = valid_entries.begin();

    while (it != valid_entries.end()) {
        int current_user = it->user;
        
        if (static_cast<size_t>(current_user) >= user_counter_valid.size() || 
            user_counter_valid[current_user] < MININUM_REVIEW_COUNT_PER_USER) {
            while (it != valid_entries.end() && it->user == current_user) {
                ++it;
            }
            continue;
        }

        if (out_pos > WRITE_BUFFER_SIZE - 16384) {
            fwrite(aligned_out, 1, out_pos, output_file);
            out_pos = 0;
        }

        char* ptr = aligned_out + out_pos;
        char* start = ptr;
        
        ptr = write_int(ptr, current_user);
        
        while (it != valid_entries.end() && it->user == current_user) {
            *ptr++ = ' ';
            
            ptr = write_int(ptr, it->movie);
            
            *ptr++ = ':';

            ptr = write_rating(ptr, it->rating);
            
            ++it;
        }
        
        *ptr++ = '\n';
        
        out_pos += ptr - start;
    }

    if (out_pos > 0) {
        fwrite(aligned_out, 1, out_pos, output_file);
    }
    
    fclose(output_file);
}

bool parse_line(const char* line, uint32_t& user, uint32_t& movie, float& rating) {
    const char* p = line;
    user = fast_atoi(p);
    if (*p != ',') return false;
    p++;

    movie = fast_atoi(p);
    if (*p != ',') return false;
    p++;

    rating = fast_atof(p);
    return true;
}

void process_valid_line(uint32_t user, uint32_t movie, float rating,
                        std::vector<uint16_t>& user_counter,
                        std::vector<uint16_t>& movie_counter,
                        std::vector<RatingEntry>& entries) {
    if (static_cast<size_t>(movie) < movie_counter.size()) {
        movie_counter[movie]++;
    }
    if (static_cast<size_t>(user) < user_counter.size()) {
        user_counter[user]++;
    }
    entries.push_back({user, movie, rating});
}

void DataPreprocessor::process_ratings() {
    vector<uint16_t> movie_counter(209171, 0);
    vector<uint16_t> user_counter(162541, 0);
    
    vector<RatingEntry> entries;
    entries.reserve(25000000);

    vector<char> buffer(READ_BUFFER_SIZE + 64);
    char* aligned_buffer = buffer.data();

    if (reinterpret_cast<uintptr_t>(aligned_buffer) % 64 != 0) {
        aligned_buffer += 64 - (reinterpret_cast<uintptr_t>(aligned_buffer) % 64);
    }
    
    char* current = aligned_buffer;
    size_t bytes_remaining = 0;
    bool first_line = true;

    while (true) {
        if (bytes_remaining < 256) {
            if (bytes_remaining > 0) {
                memmove(aligned_buffer, current, bytes_remaining);
            }
            
            size_t bytes_read = fread(
                aligned_buffer + bytes_remaining,
                1,
                READ_BUFFER_SIZE - bytes_remaining,
                file
            );
            
            if (bytes_read == 0) {
                if (bytes_remaining == 0) break;
            } else {
                bytes_remaining += bytes_read;
            }
            current = aligned_buffer;
        }

        char* line_end = static_cast<char*>(memchr(current, '\n', bytes_remaining));
        if (!line_end) {
            if (bytes_remaining > 0) {
                memmove(aligned_buffer, current, bytes_remaining);
            }
            size_t bytes_read = fread(
                aligned_buffer + bytes_remaining,
                1,
                READ_BUFFER_SIZE - bytes_remaining,
                file
            );
            if (bytes_read == 0) break;
            bytes_remaining += bytes_read;
            current = aligned_buffer;
            continue;
        }
        
        *line_end = '\0';
        size_t line_length = line_end - current + 1;
        
        if (first_line) {
            first_line = false;
            bytes_remaining -= line_length;
            current = line_end + 1;
            continue;
        }

        uint32_t user = 0, movie = 0;
        float rating = 0.0f;
        if (parse_line(current, user, movie, rating)) {
            process_valid_line(user, movie, rating, user_counter, movie_counter, entries);
        }

        bytes_remaining -= line_length;
        current = line_end + 1;
    }

    vector<bool> valid_movie(movie_counter.size(), false);
    for (size_t i = 0; i < movie_counter.size(); ++i) {
        if (movie_counter[i] >= MININUM_REVIEW_COUNT_PER_MOVIE) {
            valid_movie[i] = true;
        }
    }

    vector<uint16_t> user_counter_valid(user_counter.size(), 0);
    vector<RatingEntry> valid_entries;
    valid_entries.reserve(entries.size());

    for (const auto& entry : entries) {
        if (static_cast<size_t>(entry.movie) < valid_movie.size() && 
            valid_movie[entry.movie]) {
            valid_entries.push_back(entry);
            if (static_cast<size_t>(entry.user) < user_counter_valid.size()) {
                user_counter_valid[entry.user]++;
            }
        }
    }

    write_preprocessed_input(valid_entries, user_counter_valid);
}