#pragma once

#include "file_handler.hpp"
#include "custom_types.hpp"

class MovieReader : public FileHandler {
    public:
        MovieReader(const char* filename, const char* mode);
        void getMovies(MoviesData& movies);
};