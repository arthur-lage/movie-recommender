#include "file_handler.hpp"

#include <iostream>

FileHandler::FileHandler(const char* filename, const char* mode) {
    file = fopen(filename, mode);

    if (!file) {
        cout << "Erro ao abrir arquivo" << endl;
        exit(1);
    }
}

FileHandler::~FileHandler() {
    close();
}

void FileHandler::close() {
    if (file) {
        fclose(file);
        file = nullptr;
    }
}