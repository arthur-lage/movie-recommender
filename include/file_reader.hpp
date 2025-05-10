#pragma once

using namespace std;

class FileReader {
    private:
        FILE* file = nullptr;
    public:
        FileReader(const char* filename);
        ~FileReader();

        void process_ratings();

        void close();
};