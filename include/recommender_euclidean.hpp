#pragma once

#include "custom_types.hpp"

class RecommenderEuclidean {
    public:
        RecommenderEuclidean();
        void generateRecommendations(const UsersAndMoviesData& usersAndMovies, const MoviesData& movies);
};