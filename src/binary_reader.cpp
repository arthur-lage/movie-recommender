// #include "binary_reader.hpp"

// #include <cstdio>
// #include <cstdlib>
// #include <cstring>

// using namespace std;

// BinaryReader::BinaryReader(const char* filename) : FileHandler(filename, "rb") {}

// void process_line (char* line, UsersAndMoviesData& usersAndMovies) {
//     size_t len = strlen(line);
//     if(len > 0 && line[len-1] == '\r') {
//         line[len - 1] = '\0';
//     }

//     char* token = strtok(line, " ");
//     if(!token) return;

//     const int userId = atoi(token);
//     std::unordered_set<Rating, Rating::Hash> ratings;

//     while((token = strtok(nullptr, " "))) {
//         char* colon = strchr(token, ':');
//         if(!colon) return;

//         *colon = '\0';
//         const int movie_id = atoi(token);
//         const float score = strtof(colon + 1,  nullptr);

//         ratings.insert({movie_id, score});
//     }

//     usersAndMovies[userId] = move(ratings);
// }

// void BinaryReader::process_input(UsersAndMoviesData& usersAndMovies) {
//     fseek(file, 0, SEEK_END);
//     const long file_size = ftell(file);
//     fseek(file, 0, SEEK_SET);

//     char* buffer = new char[file_size + 1];
//     const size_t bytes_read = fread(buffer, 1, file_size, file);
//     buffer[bytes_read] = '\0';

//     close();

//     char* line = buffer;
//     while (line < buffer + bytes_read) {
//         char* end = strchr(line, '\n');
        
//         if (end) {
//             *end = '\0';
//             process_line(line, usersAndMovies);
//             line = end + 1;
//         } else {
//             process_line(line, usersAndMovies);
//             break;
//         }
//     }

//     delete[] buffer;
// }

#include "binary_reader.hpp"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <thread>
#include <vector>
#include <mutex>

using namespace std;

BinaryReader::BinaryReader(const char* filename) : FileHandler(filename, "rb") {}

// Parsing otimizado de inteiros
inline int fast_atoi(const char*& p) {
    int x = 0;
    while (isdigit(*p)) {
        x = x * 10 + (*p - '0');
        ++p;
    }
    return x;
}

// Parsing otimizado de floats
inline float fast_atof(const char*& p) {
    float sign = 1.0f;
    if (*p == '-') {
        sign = -1.0f;
        ++p;
    }
    
    float value = 0.0f;
    while (isdigit(*p)) {
        value = value * 10.0f + (*p - '0');
        ++p;
    }
    
    if (*p == '.') {
        ++p;
        float fraction = 0.1f;
        while (isdigit(*p)) {
            value += (*p - '0') * fraction;
            fraction *= 0.1f;
            ++p;
        }
    }
    
    return sign * value;
}

void BinaryReader::process_input(UsersAndMoviesData& usersAndMovies) {
    // Abrir arquivo com mmap
    int fd = open("datasets/input.dat", O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("File stat error");
        ::close(fd);
        return;
    }
    size_t file_size = sb.st_size;

    char* mapped_data = static_cast<char*>(mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (mapped_data == MAP_FAILED) {
        perror("mmap failed");
        ::close(fd);
        return;
    }

    // Processamento paralelo
    const unsigned num_threads = max(1u, thread::hardware_concurrency());
    vector<thread> threads;
    vector<UsersAndMoviesData> thread_data(num_threads);
    vector<mutex> mutexes(num_threads);

    // Divisão do arquivo em chunks
    vector<pair<const char*, const char*>> chunks;
    const size_t chunk_size = file_size / num_threads;
    const char* chunk_start = mapped_data;

    for (unsigned i = 0; i < num_threads; ++i) {
        const char* chunk_end = (i == num_threads - 1) 
            ? mapped_data + file_size 
            : chunk_start + chunk_size;
        
        // Alinhar no final da linha
        while (chunk_end < mapped_data + file_size && *chunk_end != '\n') ++chunk_end;
        if (chunk_end < mapped_data + file_size) ++chunk_end;

        chunks.emplace_back(chunk_start, chunk_end);
        chunk_start = chunk_end;
    }

    // Função de processamento para cada thread
    auto worker = [&](int thread_id, const char* start, const char* end) {
        auto& local_data = thread_data[thread_id];
        const char* p = start;

        while (p < end) {
            // Pular linhas vazias
            if (*p == '\n' || *p == '\r') {
                ++p;
                continue;
            }

            // Ler user ID
            int user_id = fast_atoi(p);
            if (*p != ' ') {
                while (p < end && *p != '\n') ++p;
                if (p < end) ++p;
                continue;
            }
            ++p; // Pular espaço

            unordered_set<Rating, Rating::Hash> ratings;
            
            // Processar cada rating
            while (p < end && *p != '\n' && *p != '\r') {
                // Ler movie ID
                int movie_id = fast_atoi(p);
                if (*p != ':') break;
                ++p; // Pular ':'
                
                // Ler score
                float score = fast_atof(p);
                
                // Adicionar rating
                ratings.insert({movie_id, score});
                
                // Pular espaço ou fim de linha
                if (*p == ' ') ++p;
            }
            
            // Adicionar ao mapa (com lock local)
            lock_guard<mutex> lock(mutexes[thread_id]);
            local_data[user_id] = move(ratings);
            
            // Avançar para próxima linha
            while (p < end && *p != '\n') ++p;
            if (p < end) ++p;
        }
    };

    // Disparar threads
    for (unsigned i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, i, chunks[i].first, chunks[i].second);
    }

    // Aguardar threads
    for (auto& t : threads) {
        t.join();
    }

    // Combinar resultados
    for (auto& data : thread_data) {
        for (auto& [user_id, ratings] : data) {
            usersAndMovies[user_id] = move(ratings);
        }
    }

    // Limpeza
    munmap(mapped_data, file_size);
    ::close(fd);
}