#include "file_writer.hpp"

#include <iostream>

using namespace std;

FileWriter::FileWriter(std::string filename) {
    file = ofstream(filename);

    if (!file.is_open()) {
        std::cerr << "Erro ao abrir arquivo" << std::endl;
        return;
    }
}

FileWriter::~FileWriter() {
    close();
}

void FileWriter::write_line(std::string line) {
    file << line << endl;
}

void FileWriter::close() {
    file.close();
}