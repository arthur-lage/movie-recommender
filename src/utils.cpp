#include "utils.hpp"

#include <fstream>
#include <algorithm>
#include <random>

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

std::vector<int> selectRandomUsers(const UsersAndMoviesData& usersAndMovies, int sampleSize = 50) {
    std::vector<int> allUsers;
    for (const auto& [userId, _] : usersAndMovies) {
        allUsers.push_back(userId);
    }

    sampleSize = std::min(sampleSize, static_cast<int>(allUsers.size()));
    if (sampleSize <= 0) return {};

    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(allUsers.begin(), allUsers.end(), rng);

    return {allUsers.begin(), allUsers.begin() + sampleSize};
}


void writeExploreFile(const std::vector<int>& users, const std::string& filename = "datasets/explore.dat") {
    std::ofstream outFile(filename);
    
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing!\n";
        return;
    }

    for (int userId : users) {
        outFile << userId << "\n";
    }

    outFile.close();
    std::cout << "Successfully wrote " << users.size() << " users to " << filename << "\n";
}