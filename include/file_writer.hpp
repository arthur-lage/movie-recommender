#pragma once

#include "custom_types.hpp"

#include <fstream>

using namespace std;

class FileWriter {
    private:
        FILE* file = nullptr;
    public:
        FileWriter(const char* filename);
        ~FileWriter();

        void close();
        void generateInputFile(const UserRatings& userRatings);
};