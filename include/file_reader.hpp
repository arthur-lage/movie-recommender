#pragma once

// #include <fstream>

#include "rating_data.hpp"
#include "rating_database.hpp"

using namespace std;

class FileReader {
    private:
        // ifstream file;
    public:
        FileReader(std::string filename);
        ~FileReader();

        void process_ratings(RatingDatabase& db);

        void close();
};