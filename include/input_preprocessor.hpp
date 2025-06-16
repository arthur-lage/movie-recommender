#pragma once

#include "file_handler.hpp"
#include "custom_types.hpp"

using namespace std;

class InputPreprocessor : public FileHandler {
    public:
        InputPreprocessor(const char* filename, const char* mode);
        void process_ratings();
};