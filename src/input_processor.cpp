#include "input_processor.hpp"

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

InputProcessor::InputProcessor(const char* filename) : FileHandler(filename, "rb") {}

inline int fast_atoi(const char*& p) {
    int x = 0;
    while (*p >= '0' && *p <= '9') {
        x = x * 10 + (*p - '0');
        ++p;
    }
    return x;
}

inline float fast_atof(const char*& p) {
    int int_part = 0;
    int frac_part = 0;
    int frac_digits = 0;
    bool negative = false;
    
    if (*p == '-') {
        negative = true;
        ++p;
    }
    
    // Parte inteira
    while (*p >= '0' && *p <= '9') {
        int_part = int_part * 10 + (*p - '0');
        ++p;
    }
    
    // Parte fracionária
    if (*p == '.') {
        ++p;
        while (*p >= '0' && *p <= '9') {
            if (frac_digits < 2) {
                frac_part = frac_part * 10 + (*p - '0');
                ++frac_digits;
            }
            ++p;
        }
    }
    
    float value = int_part;
    if (frac_digits == 1) {
        value += frac_part * 0.1f;
    } else if (frac_digits == 2) {
        value += frac_part * 0.01f;
    }
    return negative ? -value : value;
}

void InputProcessor::process_input(UsersAndMoviesData& usersAndMovies) {
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

            // printf("atoi user id: %d\n", user_id);

            unordered_set<Rating, Rating::Hash> ratings;
            
            // Processar cada rating
            while (p < end && *p != '\n' && *p != '\r') {
                // Ler movie ID
                int movie_id = fast_atoi(p);
                if (*p != ':') break;
                ++p; // Pular ':'
                
                // Ler score
                float score = fast_atof(p);
            // printf("atof score: %f\n", score);

                
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