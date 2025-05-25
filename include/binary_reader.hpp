#pragma once

#include "file_handler.hpp"
#include "custom_types.hpp"

class BinaryReader : public FileHandler {
    private:

    public:
        BinaryReader(const char* filename, const char* mode);
        void process_input(UsersAndMoviesData& usersAndMovies);
};