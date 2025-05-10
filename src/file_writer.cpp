#include "file_writer.hpp"

#include <iostream>
#include <vector>
#include <unordered_map>

#include <cstdlib>
#include <cstdio>

using namespace std;

FileWriter::FileWriter(const char* filename) : FileHandler(filename) {}

void FileWriter::generateInputFile (const UserRatings& userRatings) {
    const size_t BUFFER_SIZE = 1 << 20; // BUFFER DE 1 MEGA

    char* buffer = new char[BUFFER_SIZE];
    setvbuf(file, buffer, _IOFBF, BUFFER_SIZE);

    for (const auto& [userId, ratings] : userRatings) {
        fprintf(file, "%d", userId);
        for(const auto& [movieId, rating] : ratings) {
            fprintf(file, " %d:%.1f", movieId, rating);
        }
        fprintf(file, "\n");
    }

    delete[] buffer;
}