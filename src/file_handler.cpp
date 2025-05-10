#include "file_handler.hpp"

#include <iostream>

FileHandler::FileHandler(const char* filename) {
    file = fopen(filename, "r");

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