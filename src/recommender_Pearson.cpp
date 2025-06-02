#include "recommender_Pearson.hpp"
#include "config.hpp"
#include "binary_reader.hpp"
#include "output_manager.hpp"

#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <sstream> 

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

inline double computePearsonSimilarity(const unordered_set<Rating, Rating::Hash>& user1,
                                     const unordered_set<Rating, Rating::Hash>& user2) {
    // Extreme fast path for empty cases
    if (user1.empty() || user2.empty()) return -INFINITY;

    // Choose the optimal iteration strategy
    const bool iterate_user1 = user1.size() < user2.size();
    const auto& iter_set = iterate_user1 ? user1 : user2;
    const auto& lookup_set = iterate_user1 ? user2 : user1;

    // Single-pass accumulated values
    double sumA = 0.0, sumB = 0.0;
    double sumAA = 0.0, sumBB = 0.0, sumAB = 0.0;
    size_t n = 0;

    // Optimized hot loop
    for (const Rating& r : iter_set) {
        auto it = lookup_set.find(Rating{r.movie, 0});
        if (it != lookup_set.end()) {
            const double a = r.score;
            const double b = it->score;
            sumA += a;
            sumB += b;
            sumAA += a * a;
            sumBB += b * b;
            sumAB += a * b;
            ++n;
        }
    }

    // Minimum common ratings threshold
    if (n < 2) return -INFINITY;

    // Compute final correlation using optimized formula
    const double inv_n = 1.0 / n;
    const double cov = sumAB - sumA * sumB * inv_n;
    const double varA = sumAA - sumA * sumA * inv_n;
    const double varB = sumBB - sumB * sumB * inv_n;

    // Numerical stability check
    if (varA <= 1e-12 || varB <= 1e-12) return 0.0;

    return cov / sqrt(varA * varB);
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
    OutputManager outputManager;
    vector<uint64_t> genDurations;
    vector<uint64_t> writeDurations;
    
    outputManager.openInOutcome("output.txt");
    std::ostringstream buffer;
    buffer.str().reserve(10000);
    
    const int k = 10;

    for(int currentUser : usersToRecommend) {
        // Medição do tempo de geração
        auto startGenRec = std::chrono::high_resolution_clock::now();
        auto similarUsers = findSimilarUsersPerson(currentUser, usersAndMovies, k);
        auto recommendations = getRecommendations(currentUser, similarUsers, usersAndMovies);
        auto timeToGen = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - startGenRec);
        genDurations.push_back(timeToGen.count());

        // Formatação das recomendações
        buffer << "\n\nRecommendations for user " << currentUser << ":\n\n";
        for (const auto& [movieId, score] : recommendations) {
            auto it = movies.find(movieId);
            if(it != movies.end()) {
                buffer << it->second << " (Score: " << score << ")\n";
            }
        }

        // Medição do tempo de escrita
        auto startWrite = std::chrono::high_resolution_clock::now();
        outputManager.write(buffer.str().c_str(), buffer.str().size());
        auto timeToWrite = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - startWrite);
        writeDurations.push_back(timeToWrite.count());

        buffer.str("");
    }

    // Exibição dos tempos de geração
    cout << "\nGeneration Times:\n";
    for(const auto& dur : genDurations) {
        cout << "- " << dur << " ms\n";
    }

    // Exibição dos tempos de escrita
    cout << "\nWriting Times:\n";
    for(const auto& dur : writeDurations) {
        cout << "- " << dur << " ms\n";
    }

    // Cálculo dos totais
    uint64_t totalGenTime = accumulate(genDurations.begin(), genDurations.end(), 0);
    uint64_t totalWriteTime = accumulate(writeDurations.begin(), writeDurations.end(), 0);
    
    cout << "\nTotal generation time: " << totalGenTime << " ms\n";
    cout << "Total writing time: " << totalWriteTime << " ms\n";
    cout << "Combined total time: " << (totalGenTime + totalWriteTime) << " ms\n";

    outputManager.flush();
}