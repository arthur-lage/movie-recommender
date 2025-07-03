#include "movie_reader.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

MovieReader::MovieReader(const char* filename, const char* mode) : FileHandler(filename, mode) {}

void MovieReader::getMovies (MoviesData& movies) {
    char buffer[1024];
    if(fgets(buffer, sizeof(buffer), file) == NULL) {
        if(feof(file)) {
            printf("Não foi possível ler a lista de filmes corretamente.\n");
            return;
        } else {
            perror("Erro durante a leitura do arquivo de filmes.\n");
            return;
        }
    }

    while(fgets(buffer, sizeof(buffer), file)) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') buffer[--len] = '\0';
        
        char* ptr = buffer;
        char* end = strchr(ptr, ',');
        if (!end) continue;
        *end = '\0';
        int id = atoi(ptr);
        
        ptr = end + 1;
        if (*ptr == '"') {
            ptr++;
            end = strchr(ptr, '"');
            if (!end) continue;
            *end = '\0';
            end++;
            if (*end != ',') continue;
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