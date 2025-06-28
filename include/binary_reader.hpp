#pragma once

#include "file_handler.hpp"
#include "custom_types.hpp"

#include <unordered_set>

class BinaryReader : public FileHandler {
    private:

    public:
        BinaryReader(const char* filename);
        void process_input(UsersAndMoviesData& usersAndMovies);
};