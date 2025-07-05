#pragma once

#include "custom_types.hpp"

#include <unordered_set>

class InputProcessor {
    private:

    public:
        InputProcessor();
        void process_input(UsersAndMoviesData& usersAndMovies);
};