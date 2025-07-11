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

#include "utils.hpp"

using namespace std;

InputProcessor::InputProcessor(const char* filename) : FileHandler(filename, "rb") {}

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

    const unsigned num_threads = max(1u, thread::hardware_concurrency());
    vector<thread> threads;
    vector<UsersAndMoviesData> thread_data(num_threads);
    vector<mutex> mutexes(num_threads);

    vector<pair<const char*, const char*>> chunks;
    const size_t chunk_size = file_size / num_threads;
    const char* chunk_start = mapped_data;

    for (unsigned i = 0; i < num_threads; ++i) {
        const char* chunk_end = (i == num_threads - 1) 
            ? mapped_data + file_size 
            : chunk_start + chunk_size;
        
        while (chunk_end < mapped_data + file_size && *chunk_end != '\n') ++chunk_end;
        if (chunk_end < mapped_data + file_size) ++chunk_end;

        chunks.emplace_back(chunk_start, chunk_end);
        chunk_start = chunk_end;
    }

    auto worker = [&](int thread_id, const char* start, const char* end) {
        auto& local_data = thread_data[thread_id];
        const char* p = start;

        while (p < end) {
            if (*p == '\n' || *p == '\r') {
                ++p;
                continue;
            }

            int user_id = fast_atoi(p);
            if (*p != ' ') {
                while (p < end && *p != '\n') ++p;
                if (p < end) ++p;
                continue;
            }
            ++p; 

            unordered_set<Rating, Rating::Hash> ratings;
            
            while (p < end && *p != '\n' && *p != '\r') {
                int movie_id = fast_atoi(p);
                if (*p != ':') break;
                ++p; 
                
                float score = fast_atof(p);
                
                ratings.insert({movie_id, score});
                
                if (*p == ' ') ++p;
            }
            
            lock_guard<mutex> lock(mutexes[thread_id]);
            local_data[user_id] = move(ratings);
            
            while (p < end && *p != '\n') ++p;
            if (p < end) ++p;
        }
    };

    for (unsigned i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, i, chunks[i].first, chunks[i].second);
    }

    for (auto& t : threads) {
        t.join();
    }

    for (auto& data : thread_data) {
        for (auto& [user_id, ratings] : data) {
            usersAndMovies[user_id] = move(ratings);
        }
    }

    munmap(mapped_data, file_size);
    ::close(fd);
}
