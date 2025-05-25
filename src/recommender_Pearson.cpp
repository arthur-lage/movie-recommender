#include "recommender_Pearson.hpp"
#include "config.hpp"
#include "binary_reader.hpp"

#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>
#include <chrono>
#include <algorithm>
#include <numeric>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

//agr testar

RecommenderPearson::RecommenderPearson() {}

void read_explore_file_Pearson(unordered_set<int>& usersToRecommend) {
    FILE* file = fopen("datasets/explore.dat", "r");
    if(!file) {
        printf("Não foi possível ler o explore.dat\n");
        exit(1);
    }

    char line[256];

    while(fgets(line, 256, file)) 
    {
        char* end;
        long num = strtol(line, &end, 10);

        if (end != line) {
            usersToRecommend.insert(num);
        }
    }

    fclose(file);
}

double computePearsonSimilarity(const unordered_set<Rating, Rating::Hash>& user1,
                              const unordered_set<Rating, Rating::Hash>& user2) {
    vector<double> scores1, scores2;
    int common = 0;
    
    // Coletar ratings dos filmes em comum
    for (const Rating& r1 : user1) {
        auto it = user2.find(Rating{r1.movie, 0});
        if (it != user2.end()) {
            scores1.push_back(r1.score);
            scores2.push_back(it->score);
            common++;
        }
    }
    
    // Se não há filmes em comum, retorna similaridade 0 (ou -infinity)
    if (common == 0) return -INFINITY;
    
    // Calcular médias
    double sum1 = accumulate(scores1.begin(), scores1.end(), 0.0);
    double sum2 = accumulate(scores2.begin(), scores2.end(), 0.0);
    double mean1 = sum1 / common;
    double mean2 = sum2 / common;
    
    // Calcular numerador e denominadores
    double numerator = 0.0, denom1 = 0.0, denom2 = 0.0;
    for (size_t i = 0; i < scores1.size(); ++i) {
        double diff1 = scores1[i] - mean1;
        double diff2 = scores2[i] - mean2;
        numerator += diff1 * diff2;
        denom1 += diff1 * diff1;
        denom2 += diff2 * diff2;
    }
    
    // Evitar divisão por zero
    if (denom1 == 0 || denom2 == 0) return 0.0;
    
    return numerator / (sqrt(denom1) * sqrt(denom2));
}


vector<pair<int, double>> findSimilarUsersPerson(int targetUserId,
                                         const UsersAndMoviesData& data,
                                         int k = 5) {
    vector<pair<int, double>> similarities;
    const auto& targetRatings = data.at(targetUserId);

    for (const auto& [userId, ratings] : data) {
        if (userId == targetUserId) continue;
        
        double similarity = computePearsonSimilarity(targetRatings, ratings);
        if (similarity > -INFINITY) {  // Pode ajustar este limite
            similarities.emplace_back(userId, similarity);
        }
    }

    // Agora ordenamos por similaridade decrescente (maior = mais similar)
    sort(similarities.begin(), similarities.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    if (similarities.size() > k) similarities.resize(k);
    return similarities;
}

vector<pair<int, float>> getRecommendations(int targetUserId,
                                          const vector<pair<int, double>>& similarUsers,
                                          const UsersAndMoviesData& data) {
    const auto& targetRatings = data.at(targetUserId);
    unordered_map<int, pair<double, double>> recommendations;  // movie: (sum, weight)

    for (const auto& [similarUserId, similarity] : similarUsers) {
        const auto& similarUserRatings = data.at(similarUserId);
        double weight = similarity;  // Usamos a similaridade diretamente como peso

        for (const Rating& r : similarUserRatings) {
            // Check if target hasn't rated this movie
            if (!targetRatings.count(Rating{r.movie, 0})) {
                recommendations[r.movie].first += r.score * weight;
                recommendations[r.movie].second += abs(weight);  // Usamos valor absoluto
            }
        }
    }

    // Convert to final scores
    vector<pair<int, float>> results;
    for (const auto& [movie, scores] : recommendations) {
        if (scores.second > 0) {
            results.emplace_back(movie, scores.first / scores.second);
        }
    }


 sort(results.begin(), results.end(), [](auto& a, auto& b) { 
        return a.second > b.second; 
    });
    
    results.erase(
        results.begin() + min(NUMBER_OF_RECOMMENDATIONS_PER_USER, (int)results.size()), 
        results.end()
    );
    return results;
    return results;
}

void RecommenderPearson::generateRecommendations(const UsersAndMoviesData& usersAndMovies, const MoviesData& movies) {
    unordered_set<int> usersToRecommend;
    read_explore_file_Pearson(usersToRecommend);

    vector<uint64_t> durations;
    
    // 10 most similar users
    const int k = 10;

    for(int currentUser : usersToRecommend) {
        auto startGenRec = std::chrono::high_resolution_clock::now();

        auto similarUsers = findSimilarUsersPerson(currentUser, usersAndMovies, k);
        auto recommendations = getRecommendations(currentUser, similarUsers, usersAndMovies);

        auto timeToGen = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startGenRec);

        durations.insert(durations.begin(), timeToGen.count());
        std::cout << "\n\n Recommendations for user " << currentUser << ":\n" << std::endl;

        for (const auto& [movieId, score] : recommendations) {
            auto it = movies.find(movieId);
            
            if(it != movies.end()) {
                cout << it->second << " (Score: " << score << ")\n";
            }
        }

        std::cout << "\n\n";
    }

    for(const auto& dur : durations) {
        std::cout << "Took " << dur << " ms to generate this rec." << std::endl;
    }
}