#pragma once

#include <fstream>

using namespace std;

class FileWriter {
    private:
        ofstream file;
    public:
        FileWriter(std::string filename);
        ~FileWriter();

        void write_line(std::string line);
        void close();
};