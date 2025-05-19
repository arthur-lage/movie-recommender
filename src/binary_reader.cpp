#include "binary_reader.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

BinaryReader::BinaryReader(const char* filename) : FileHandler(filename, "rb") {}

void process_line (char* line, UsersAndMoviesData& usersAndMovies) {
    size_t len = strlen(line);
    if(len > 0 && line[len-1] == '\r') {
        line[len - 1] = '\0';
    }

    char* token = strtok(line, " ");
    if(!token) return;

    const int userId = atoi(token);
    std::unordered_set<Rating, Rating::Hash> ratings;

    while((token = strtok(nullptr, " "))) {
        char* colon = strchr(token, ':');
        if(!colon) return;

        *colon = '\0';
        const int movie_id = atoi(token);
        const float score = strtof(colon + 1,  nullptr);

        ratings.insert({movie_id, score});
    }

    usersAndMovies[userId] = move(ratings);
}

void BinaryReader::process_input(UsersAndMoviesData& usersAndMovies) {
    fseek(file, 0, SEEK_END);
    const long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = new char[file_size + 1];
    const size_t bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0';

    close();

    char* line = buffer;
    while (line < buffer + bytes_read) {
        char* end = strchr(line, '\n');
        
        if (end) {
            *end = '\0';
            process_line(line, usersAndMovies);
            line = end + 1;
        } else {
            process_line(line, usersAndMovies);
            break;
        }
    }

    delete[] buffer;
}