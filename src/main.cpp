#include <iostream>
#include <chrono>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <unordered_set>

#include "custom_types.hpp"
#include "input_preprocessor.hpp"
#include "binary_reader.hpp"

using namespace std;

int main() {
    // Etapa 1: Processar arquivo CSV e medir tempo
    InputPreprocessor ratings_file("kaggle-data/ratings.csv", "r");
    auto start_csv = chrono::high_resolution_clock::now();
    ratings_file.process_ratings();
    auto duration_csv = chrono::duration_cast<chrono::milliseconds>(
        chrono::high_resolution_clock::now() - start_csv
    );
    cout << "Tempo de processamento do CSV: " << duration_csv.count() << " ms\n";

    // Etapa 2: Ler dados binários e medir tempo
    BinaryReader binary_reader("datasets/input.dat", "rb");
    UsersAndMoviesData usersAndMovies;
    auto start_bin = chrono::high_resolution_clock::now();
    binary_reader.process_input(usersAndMovies);
    auto duration_bin = chrono::duration_cast<chrono::milliseconds>(
        chrono::high_resolution_clock::now() - start_bin
    );
    cout << "Tempo de carregamento do binário: " << duration_bin.count() << " ms\n";

    // Etapa 3: Ler usuários alvo
    ifstream explore_file("datasets/explore.dat");
    vector<int> usuarios_alvo;
    int usuario_id;
    while (explore_file >> usuario_id) {
        usuarios_alvo.push_back(usuario_id);
    }
    explore_file.close();

    // Pré-processamento para Jaccard
    unordered_map<int, unordered_set<int>> filmes_por_usuario;
    for (const auto& [user, ratings] : usersAndMovies) {
        unordered_set<int> filmes;
        for (const auto& rating : ratings) {
            filmes.insert(rating.movie);
        }
        filmes_por_usuario[user] = filmes;
    }

    const int K_SIMILARES = 10;
    const int K_RECOMENDACOES = 5;


    for (int usuario_alvo : usuarios_alvo) {
        auto start_user = chrono::high_resolution_clock::now();

        if (!filmes_por_usuario.count(usuario_alvo)) {
            cout << "Usuário " << usuario_alvo << " não encontrado.\n";
            continue;
        }

        const auto& filmes_alvo = filmes_por_usuario[usuario_alvo];
        vector<pair<int, double>> similaridades;


        for (const auto& [outro_user, filmes_outro] : filmes_por_usuario) {
            if (outro_user == usuario_alvo) continue;

            int intersecao = 0;
            for (int filme : filmes_alvo) {
                if (filmes_outro.count(filme)) intersecao++;
            }
            
            int uniao = filmes_alvo.size() + filmes_outro.size() - intersecao;
            double similaridade = (uniao > 0) ? (double)intersecao / uniao : 0.0;
            
            similaridades.emplace_back(outro_user, similaridade);
        }


        sort(similaridades.begin(), similaridades.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

        vector<int> top_similares;
        for (int i = 0; i < min(K_SIMILARES, (int)similaridades.size()); ++i) {
            top_similares.push_back(similaridades[i].first);
        }


        unordered_map<int, int> contagem_filmes;
        for (int user : top_similares) {
            for (int filme : filmes_por_usuario[user]) {
                if (!filmes_alvo.count(filme)) {
                    contagem_filmes[filme]++;
                }
            }
        }


        vector<pair<int, int>> filmes_ordenados(contagem_filmes.begin(), contagem_filmes.end());
        sort(filmes_ordenados.begin(), filmes_ordenados.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

        auto end_user = chrono::high_resolution_clock::now();
        auto duration_user = chrono::duration_cast<chrono::milliseconds>(end_user - start_user);

        cout << "\n=== Usuário " << usuario_alvo << " ===";
        cout << "\nTempo de processamento: " << duration_user.count() << " ms\n";
        cout << "Recomendações:\n";
        int exibidos = 0;
        for (const auto& [filme, contagem] : filmes_ordenados) {
            if (exibidos++ >= K_RECOMENDACOES) break;
            cout << "-> Filme " << filme << " (Recomendado por " << contagem << " usuários)\n";
        }
    }

    return 0;
}