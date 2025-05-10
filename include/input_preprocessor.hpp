#pragma once

#include "file_handler.hpp"

using namespace std;

class InputPreprocessor : public FileHandler {
    public:
        InputPreprocessor(const char* filename);
        void process_ratings();
};