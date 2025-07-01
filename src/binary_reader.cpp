// // #include "binary_reader.hpp"
// // #include "utils.hpp"

// // #include <sys/mman.h>
// // #include <sys/stat.h>
// // #include <fcntl.h>
// // #include <unistd.h>
// // #include <cstring>
// // #include <cctype>
// // #include <algorithm>
// // #include <thread>
// // #include <vector>
// // #include <mutex>

// // using namespace std;

// // BinaryReader::BinaryReader(const char* filename) : FileHandler(filename, "rb") {}

// // void BinaryReader::process_input(UsersAndMoviesData& usersAndMovies) {
// //     // Abrir arquivo com mmap
// //     int fd = open("datasets/input.dat", O_RDONLY);
// //     if (fd == -1) {
// //         perror("Error opening file");
// //         return;
// //     }

// //     struct stat sb;
// //     if (fstat(fd, &sb) == -1) {
// //         perror("File stat error");
// //         ::close(fd);
// //         return;
// //     }
// //     size_t file_size = sb.st_size;

// //     char* mapped_data = static_cast<char*>(mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0));
// //     if (mapped_data == MAP_FAILED) {
// //         perror("mmap failed");
// //         ::close(fd);
// //         return;
// //     }

// //     // Processamento paralelo
// //     const unsigned num_threads = max(1u, thread::hardware_concurrency());
// //     vector<thread> threads;
// //     vector<UsersAndMoviesData> thread_data(num_threads);
// //     vector<mutex> mutexes(num_threads);

// //     // Divisão do arquivo em chunks
// //     vector<pair<const char*, const char*>> chunks;
// //     const size_t chunk_size = file_size / num_threads;
// //     const char* chunk_start = mapped_data;

// //     for (unsigned i = 0; i < num_threads; ++i) {
// //         const char* chunk_end = (i == num_threads - 1) 
// //             ? mapped_data + file_size 
// //             : chunk_start + chunk_size;
        
// //         // Alinhar no final da linha
// //         while (chunk_end < mapped_data + file_size && *chunk_end != '\n') ++chunk_end;
// //         if (chunk_end < mapped_data + file_size) ++chunk_end;

// //         chunks.emplace_back(chunk_start, chunk_end);
// //         chunk_start = chunk_end;
// //     }

// //     // Função de processamento para cada thread
// //     auto worker = [&](int thread_id, const char* start, const char* end) {
// //         auto& local_data = thread_data[thread_id];
// //         const char* p = start;

// //         while (p < end) {
// //             // Pular linhas vazias
// //             if (*p == '\n' || *p == '\r') {
// //                 ++p;
// //                 continue;
// //             }

// //             // Ler user ID
// //             int user_id = fast_atoi(p);
// //             if (*p != ' ') {
// //                 while (p < end && *p != '\n') ++p;
// //                 if (p < end) ++p;
// //                 continue;
// //             }
// //             ++p; // Pular espaço

// //             unordered_set<Rating, Rating::Hash> ratings;
            
// //             // Processar cada rating
// //             while (p < end && *p != '\n' && *p != '\r') {
// //                 // Ler movie ID
// //                 int movie_id = fast_atoi(p);
// //                 if (*p != ':') break;
// //                 ++p; // Pular ':'
                
// //                 // Ler score
// //                 float score = fast_atof(p);
                
// //                 // Adicionar rating
// //                 ratings.insert({movie_id, score});
                
// //                 // Pular espaço ou fim de linha
// //                 if (*p == ' ') ++p;
// //             }
            
// //             // Adicionar ao mapa (com lock local)
// //             lock_guard<mutex> lock(mutexes[thread_id]);
// //             local_data[user_id] = move(ratings);
            
// //             // Avançar para próxima linha
// //             while (p < end && *p != '\n') ++p;
// //             if (p < end) ++p;
// //         }
// //     };

// //     // Disparar threads
// //     for (unsigned i = 0; i < num_threads; ++i) {
// //         threads.emplace_back(worker, i, chunks[i].first, chunks[i].second);
// //     }

// //     // Aguardar threads
// //     for (auto& t : threads) {
// //         t.join();
// //     }

// //     // Combinar resultados
// //     for (auto& data : thread_data) {
// //         for (auto& [user_id, ratings] : data) {
// //             usersAndMovies[user_id] = move(ratings);
// //         }
// //     }

// //     // Limpeza
// //     munmap(mapped_data, file_size);
// //     ::close(fd);
// // }

// // #include "binary_reader.hpp"

// // #include <cstdio>
// // #include <cstdlib>
// // #include <cstring>

// // using namespace std;

// // BinaryReader::BinaryReader(const char* filename) : FileHandler(filename, "rb") {}

// // void process_line (char* line, UsersAndMoviesData& usersAndMovies) {
// //     size_t len = strlen(line);
// //     if(len > 0 && line[len-1] == '\r') {
// //         line[len - 1] = '\0';
// //     }

// //     char* token = strtok(line, " ");
// //     if(!token) return;

// //     const int userId = atoi(token);
// //     std::unordered_set<Rating, Rating::Hash> ratings;

// //     while((token = strtok(nullptr, " "))) {
// //         char* colon = strchr(token, ':');
// //         if(!colon) return;

// //         *colon = '\0';
// //         const int movie_id = atoi(token);
// //         const float score = strtof(colon + 1,  nullptr);

// //         ratings.insert({movie_id, score});
// //     }

// //     usersAndMovies[userId] = move(ratings);
// // }

// // void BinaryReader::process_input(UsersAndMoviesData& usersAndMovies) {
// //     fseek(file, 0, SEEK_END);
// //     const long file_size = ftell(file);
// //     fseek(file, 0, SEEK_SET);

// //     char* buffer = new char[file_size + 1];
// //     const size_t bytes_read = fread(buffer, 1, file_size, file);
// //     buffer[bytes_read] = '\0';

// //     close();

// //     char* line = buffer;
// //     while (line < buffer + bytes_read) {
// //         char* end = strchr(line, '\n');
        
// //         if (end) {
// //             *end = '\0';
// //             process_line(line, usersAndMovies);
// //             line = end + 1;
// //         } else {
// //             process_line(line, usersAndMovies);
// //             break;
// //         }
// //     }

// //     delete[] buffer;
// // }

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

// Parsing otimizado de inteiros
// inline int fast_atoi(const char*& p) {
//     int x = 0;
//     while (isdigit(*p)) {
//         x = x * 10 + (*p - '0');
//         ++p;
//     }
//     return x;
// }

// Parsing otimizado de floats
// inline float fast_atof(const char*& p) {
//     float sign = 1.0f;
//     if (*p == '-') {
//         sign = -1.0f;
//         ++p;
//     }
    
//     float value = 0.0f;
//     while (isdigit(*p)) {
//         value = value * 10.0f + (*p - '0');
//         ++p;
//     }
    
//     if (*p == '.') {
//         ++p;
//         float fraction = 0.1f;
//         while (isdigit(*p)) {
//             value += (*p - '0') * fraction;
//             fraction *= 0.1f;
//             ++p;
//         }
//     }
    
//     return sign * value;
// }

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

// #include "binary_reader.hpp"
// #include <fstream>
// #include <vector>
// #include <thread>
// #include <mutex>
// #include <cctype>
// #include <algorithm>

// using namespace std;

// BinaryReader::BinaryReader(const char* filename) : FileHandler(filename, "rb") {}

// // Parsing otimizado de inteiros
// inline int parse_int(const char*& p) {
//     int x = 0;
//     while (isdigit(*p)) {
//         x = x * 10 + (*p - '0');
//         ++p;
//     }
//     return x;
// }

// Parsing robusto de floats para o formato X.Y
// inline float parse_float(const char*& p) {
//     float sign = 1.0f;
//     if (*p == '-') {
//         sign = -1.0f;
//         ++p;
//     }
    
//     float value = 0.0f;
//     while (isdigit(*p)) {
//         value = value * 10.0f + (*p - '0');
//         ++p;
//     }
    
//     if (*p == '.') {
//         ++p;
//         float fraction = 0.1f;
//         while (isdigit(*p)) {
//             value += (*p - '0') * fraction;
//             fraction *= 0.1f;
//             ++p;
//         }
//     }
    
//     return sign * value;
// }

// inline float fast_parse_float(const char*& p) {
//     // 1. Extração direta de dígitos usando operações inteiras
//     int sign = 1;
//     int int_part = 0;
//     int frac_part = 0;
//     int frac_digits = 0;
    
//     // Tratamento de sinal com branch prediction favorável
//     if (*p == '-') {
//         sign = -1;
//         ++p;
//     } else if (*p == '+') {
//         ++p;
//     }
    
//     // Processamento da parte inteira com loop desenrolado
//     while (*p >= '0' && *p <= '9') {
//         int_part = int_part * 10 + (*p - '0');
//         ++p;
//     }
    
//     // Processamento da parte fracionária otimizado
//     if (*p == '.') {
//         ++p;
        
//         // Primeiro dígito decimal (obrigatório)
//         if (*p >= '0' && *p <= '9') {
//             frac_part = (*p - '0') * 10;
//             ++p;
//             frac_digits = 1;
            
//             // Segundo dígito decimal (opcional)
//             if (*p >= '0' && *p <= '9') {
//                 frac_part += (*p - '0');
//                 ++p;
//                 frac_digits = 2;
//             }
//         }
//     }
    
//     // 2. Cálculo otimizado sem operações float durante o parsing
//     //    Usa aritmética inteira e conversão final única
//     float value = int_part;
//     if (frac_digits == 1) {
//         value += frac_part * 0.1f;
//     } else if (frac_digits == 2) {
//         value += frac_part * 0.01f;
//     }
    
//     return sign * value;
// }


// inline float fast_atof(const char* p) {
//     int int_part = 0;
//     int frac_part = 0;
//     int frac_digits = 0;
//     bool negative = false;
    
//     if (*p == '-') {
//         negative = true;
//         ++p;
//     }
    
//     // Parte inteira
//     while (*p >= '0' && *p <= '9') {
//         int_part = int_part * 10 + (*p - '0');
//         ++p;
//     }
    
//     // Parte fracionária
//     if (*p == '.') {
//         ++p;
//         while (*p >= '0' && *p <= '9') {
//             if (frac_digits < 2) {
//                 frac_part = frac_part * 10 + (*p - '0');
//                 ++frac_digits;
//             }
//             ++p;
//         }
//     }
    
//     float value = int_part;
//     if (frac_digits == 1) {
//         value += frac_part * 0.1f;
//     } else if (frac_digits == 2) {
//         value += frac_part * 0.01f;
//     }
//     return negative ? -value : value;
// }

// void BinaryReader::process_input(UsersAndMoviesData& usersAndMovies) {
//     // Abrir arquivo
//     ifstream file("datasets/input.dat", ios::binary | ios::ate);
//     if (!file) {
//         perror("Error opening file");
//         return;
//     }
    
//     // Obter tamanho do arquivo
//     streamsize file_size = file.tellg();
//     file.seekg(0, ios::beg);
    
//     // Ler arquivo para buffer
//     vector<char> buffer(file_size);
//     if (!file.read(buffer.data(), file_size)) {
//         perror("Error reading file");
//         return;
//     }
//     file.close();

//     const char* data = buffer.data();
//     const char* end = data + file_size;
    
//     // Processamento paralelo
//     const unsigned num_threads = max(1u, thread::hardware_concurrency());
//     vector<thread> threads;
//     vector<UsersAndMoviesData> thread_data(num_threads);
    
//     // Divisão do arquivo em chunks
//     vector<pair<const char*, const char*>> chunks;
//     const size_t chunk_size = file_size / num_threads;
//     const char* chunk_start = data;

//     for (unsigned i = 0; i < num_threads; ++i) {
//         const char* chunk_end = (i == num_threads - 1) 
//             ? end 
//             : min(chunk_start + chunk_size, end);
        
//         // Alinhar no final da linha
//         while (chunk_end < end && *chunk_end != '\n') ++chunk_end;
//         if (chunk_end < end) ++chunk_end;

//         chunks.emplace_back(chunk_start, chunk_end);
//         chunk_start = chunk_end;
//     }

//     // Função de processamento para cada thread
//     auto worker = [](UsersAndMoviesData& local_data, const char* start, const char* end) {
//         const char* p = start;
        
//         while (p < end) {
//             // Pular espaços em branco e quebras de linha
//             while (p < end && (*p == ' ' || *p == '\n' || *p == '\r')) p++;
//             if (p >= end) break;

//             // Ler user ID
//             int user_id = parse_int(p);
//             if (*p != ' ') continue;  // Formato inválido
//             p++;  // Pular espaço

//             unordered_set<Rating, Rating::Hash> ratings;
            
//             // Processar cada rating
//             while (p < end && *p != '\n' && *p != '\r') {
//                 // Ler movie ID
//                 int movie_id = parse_int(p);
//                 if (*p != ':') break;  // Formato inválido
//                 p++;  // Pular ':'
                
//                 // Ler score
//                 float score = fast_atof(p);


                
//                 // Adicionar rating
//                 ratings.insert({movie_id, score});
                
//                 // Pular espaço
//                 if (*p == ' ') p++;
//             }
            
//             // Adicionar ao mapa
//             local_data[user_id] = move(ratings);
            
//             // Pular para próxima linha
//             while (p < end && *p != '\n') p++;
//             if (p < end) p++;
//         }
//     };

//     // Disparar threads
//     for (unsigned i = 0; i < num_threads; ++i) {
//         threads.emplace_back([&, i] {
//             worker(ref(thread_data[i]), chunks[i].first, chunks[i].second);
//         });
//     }

//     // Aguardar threads
//     for (auto& t : threads) {
//         t.join();
//     }

//     // Combinar resultados
//     for (auto& data : thread_data) {
//         for (auto& [user_id, ratings] : data) {
//             usersAndMovies[user_id] = move(ratings);
//         }
//     }
// }