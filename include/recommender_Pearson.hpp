#pragma once

#include "custom_types.hpp"

class RecommenderPearson {
    public:
        RecommenderPearson();
        void generateRecommendations(const UsersAndMoviesData& usersAndMovies, const MoviesData& movies);
};