#pragma once

#include "file_handler.hpp"
#include "custom_types.hpp"

#include <unordered_set>

class InputProcessor : public FileHandler {
    private:

    public:
        InputProcessor(const char* filename);
        void process_input(UsersAndMoviesData& usersAndMovies);
};