#pragma once

#include "file_handler.hpp"
#include "custom_types.hpp"

using namespace std;

class FileWriter : public FileHandler {
    public:
        FileWriter(const char* filename);
        void generateInputFile(const UserRatings& userRatings);
};