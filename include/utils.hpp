#pragma once

#include <vector>
#include <iostream>

#include "custom_types.hpp"

inline int fast_atoi(const char*& p);
inline float fast_atof(const char*& p);
std::vector<int> selectRandomUsers(const UsersAndMoviesData& usersAndMovies, int sampleSize);
void writeExploreFile(const std::vector<int>& users, const std::string& filename);