#include <iostream>

#include "rating_data.hpp"
#include "file_writer.hpp"
#include "file_reader.hpp"
#include "rating_database.hpp"

using namespace std;

int main() {
    RatingDatabase rating_db;
    FileReader ratings_file("kaggle-data/ratings_little.csv");
    ratings_file.process_ratings(rating_db);

    cout << "=== DADOS DE RATINGS ===" << endl << endl;

    for(const auto& user_entry : rating_db.getRatings()) {
        int user_id = user_entry.first;
        const auto& user_ratings = user_entry.second;
        
        cout << "Usuário " << user_id << " (" << user_ratings.size() << " ratings):" << endl;
        
        for(const auto& rating : user_ratings) {
            cout << "  Filme " << rating.movieId 
                 << " - Rating: " << rating.rating 
                 << endl;
        }
        
        cout << endl;  // Espaço entre usuários
    }

    cout << "=== TOTAL DE USUÁRIOS: " << rating_db.getRatings().size() << " ===" << endl;
    
    return 0;
}