#include "movie_reader.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

MovieReader::MovieReader(const char* filename, const char* mode) : FileHandler(filename, mode) {}

void MovieReader::getMovies (MoviesData& movies) {
    char buffer[1024];
    

    while(fgets(buffer, sizeof(buffer), file)) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') buffer[--len] = '\0';
        
        // Parse movie ID
        char* ptr = buffer;
        char* end = strchr(ptr, ',');
        if (!end) continue;
        *end = '\0';
        int id = atoi(ptr);
        
        // Parse title
        ptr = end + 1;
        if (*ptr == '"') {
            ptr++; // Skip opening quote
            end = strchr(ptr, '"');
            if (!end) continue; // Missing closing quote
            *end = '\0';
            end++; // Move past closing quote
            if (*end != ',') continue; // Invalid format
        } else {
            end = strchr(ptr, ',');
            if (!end) continue;
            *end = '\0';
        }
        string title(ptr);
        movies[id] = title;
    }

    close();
}