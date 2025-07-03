#include "utils.hpp"

#include <fstream>
#include <algorithm>
#include <random>

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