#pragma once

#include "file_handler.hpp"
#include "custom_types.hpp"

using namespace std;

class DataPreprocessor : public FileHandler {
    public:
        DataPreprocessor(const char* filename, const char* mode);
        void process_ratings();
};