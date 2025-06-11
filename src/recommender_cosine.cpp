#include "recommender_cosine.hpp"
#include "config.hpp"
#include "output_manager.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <sstream>
#include <unordered_set>
#include <vector>
#include <numeric>  // Para std::accumulate

RecommenderCosine::RecommenderCosine() {}

void RecommenderCosine::precomputeUserNorms(const UsersAndMoviesData& data) {
    userNorms.clear();
    userNorms.reserve(data.size());
    
    for (const auto& [userId, ratings] : data) {
        double normSq = 0.0;
        for (const Rating& r : ratings) {
            normSq += r.score * r.score;
        }
        userNorms[userId] = std::sqrt(normSq);
    }
}

double RecommenderCosine::computeCosineSimilarity(user_id_t user1, user_id_t user2, 
                                                const UsersAndMoviesData& data) const {
    auto it1 = data.find(user1);
    auto it2 = data.find(user2);
    if (it1 == data.end() || it2 == data.end()) return 0.0;
    
    const auto& ratings1 = it1->second;
    const auto& ratings2 = it2->second;
    
    // Escolhe o menor conjunto para iterar
    const auto& smaller = ratings1.size() < ratings2.size() ? ratings1 : ratings2;
    const auto& larger = ratings1.size() < ratings2.size() ? ratings2 : ratings1;
    
    double dotProduct = 0.0;
    for (const Rating& r : smaller) {
        if (larger.find(Rating{r.movie, 0}) != larger.end()) {
            dotProduct += r.score * larger.find(Rating{r.movie, 0})->score;
        }
    }
    
    double normProduct = userNorms.at(user1) * userNorms.at(user2);
    return (normProduct > 0) ? (dotProduct / normProduct) : 0.0;
}

void RecommenderCosine::generateRecommendations(const UsersAndMoviesData& usersAndMovies, 
                                              const MoviesData& movies) {
    OutputManager outputManager;
    std::vector<uint64_t> genDurations;
    std::vector<uint64_t> writeDurations;

    outputManager.openInOutcome("output.txt");
    std::ostringstream buffer;
    buffer.str().reserve(10000);

    // Passo 1: Pré-computar normas
    // auto startPrecompute = std::chrono::high_resolution_clock::now();
    precomputeUserNorms(usersAndMovies);
    // auto normTime = std::chrono::duration_cast<std::chrono::milliseconds>(
    //     std::chrono::high_resolution_clock::now() - startPrecompute);
    // std::cout << "Tempo pré-computação: " << normTime.count() << "ms\n";
    
    // Passo 2: Ler usuários para recomendar
    std::unordered_set<user_id_t> usersToRecommend;
    std::ifstream file("datasets/explore.dat");
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir explore.dat" << std::endl;
        return;
    }
    
    user_id_t user;
    while (file >> user) {
        usersToRecommend.insert(user);
    }
    file.close();
    
    // Passo 3: Gerar recomendações
    const int k = 15;  // Número de vizinhos
    const int maxRecs = NUMBER_OF_RECOMMENDATIONS_PER_USER;  // 10 recomendações
    
    std::vector<long> processingTimes;  // Armazenar tempos individuais
    int processedUsers = 0;
    
    for (user_id_t targetUser : usersToRecommend) {
        if (usersAndMovies.find(targetUser) == usersAndMovies.end()) {
            std::cout << "Usuário " << targetUser << " não encontrado no dataset.\n";
            continue;
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        processedUsers++;
        
        // Encontrar k usuários mais similares
        std::vector<std::pair<user_id_t, double>> similarUsers;
        similarUsers.reserve(usersAndMovies.size() / 10);  // Estimativa
        
        for (const auto& [otherUser, _] : usersAndMovies) {
            if (targetUser == otherUser) continue;
            
            double similarity = computeCosineSimilarity(targetUser, otherUser, usersAndMovies);
            if (similarity > 0.2) {  // Filtro mínimo
                similarUsers.emplace_back(otherUser, similarity);
            }
        }
        
        // Ordenar e selecionar top-k
        if (similarUsers.size() > k) {
            std::partial_sort(similarUsers.begin(), 
                              similarUsers.begin() + k, 
                              similarUsers.end(),
                              [](const auto& a, const auto& b) { return a.second > b.second; });
            similarUsers.resize(k);
        } else {
            std::sort(similarUsers.begin(), similarUsers.end(),
                      [](const auto& a, const auto& b) { return a.second > b.second; });
        }
        
        std::unordered_map<movie_id_t, std::pair<double, double>> predictions;
        const auto& targetRatings = usersAndMovies.at(targetUser);
        
        for (const auto& [similarUser, similarity] : similarUsers) {
            for (const Rating& r : usersAndMovies.at(similarUser)) {
                if (targetRatings.find(Rating{r.movie, 0}) == targetRatings.end()) {
                    predictions[r.movie].first += r.score * similarity;
                    predictions[r.movie].second += similarity;
                }
            }
        }
        
        std::vector<std::pair<movie_id_t, double>> recommendations;
        recommendations.reserve(predictions.size());
        
        for (const auto& [movie, scores] : predictions) {
            if (scores.second > 0) {
                recommendations.emplace_back(movie, scores.first / scores.second);
            }
        }
        
        if (recommendations.size() > maxRecs) {
            std::partial_sort(recommendations.begin(),
                              recommendations.begin() + maxRecs,
                              recommendations.end(),
                              [](const auto& a, const auto& b) { return a.second > b.second; });
            recommendations.resize(maxRecs);
        } else {
            std::sort(recommendations.begin(), recommendations.end(),
                      [](const auto& a, const auto& b) { return a.second > b.second; });
        }
        
        buffer << "\n\nRecommendations for user " << targetUser << ":\n\n";
        for (const auto& [movieId, score] : recommendations) {
            auto it = movies.find(movieId);
            if(it != movies.end()) {
                buffer << it->second << " (Score: " << score << ")\n";
            }
        }

        outputManager.write(buffer.str().c_str(), buffer.str().size());
        buffer.str("");
    }
    
    if (!processingTimes.empty()) {
        long totalTime = std::accumulate(processingTimes.begin(), processingTimes.end(), 0L);
        double averageTime = static_cast<double>(totalTime) / processingTimes.size();
    }

    outputManager.flush();
}