#pragma once

#include "custom_types.hpp"

class RecommenderManhattan {
    public:
        RecommenderManhattan();
        void generateRecommendations(const UsersAndMoviesData& usersAndMovies, const MoviesData& movies);
};