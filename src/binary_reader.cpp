#include "binary_reader.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

BinaryReader::BinaryReader(const char* filename, const char* mode) : FileHandler(filename, mode) {}

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

    // Log inicial
    std::cout << "\nLENDO INPUT.DAT...\n";
    std::cout << "Tamanho do arquivo: " << bytes_read << " bytes\n";

    char* line = buffer;
    int line_count = 0;
    
    while (line < buffer + bytes_read) {
        char* end = strchr(line, '\n');
        
        if (end) {
            *end = '\0';
            process_line(line, usersAndMovies);
            line = end + 1;
            
            // Log a cada 1000 linhas
            if (++line_count % 1000 == 0) {
                std::cout << "Linhas processadas: " << line_count 
                          << " | Usuários carregados: " << usersAndMovies.size() 
                          << "\n";
            }
        } else {
            process_line(line, usersAndMovies);
            break;
        }
    }

    // Log final
    std::cout << "TOTAL:\n";
    std::cout << "- Linhas processadas: " << line_count << "\n";
    std::cout << "- Usuários carregados: " << usersAndMovies.size() << "\n";
    std::cout << "- Avaliações por usuário (amostra):\n";
    
    // Exibir 5 usuários para debug
    int count = 0;
    for (const auto& [user, ratings] : usersAndMovies) {
        if (count++ >= 5) break;
        std::cout << "  Usuário " << user << ": " << ratings.size() << " avaliações\n";
    }

    delete[] buffer;
}