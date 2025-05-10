#pragma once

#include "cstdio"

using namespace std;

class FileHandler {
    protected:
        FILE* file = nullptr;
    public:
        FileHandler(const char* filename);
        ~FileHandler();

        void close();
};