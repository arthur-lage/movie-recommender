#include "output_manager.hpp"

#include <stdexcept>
#include <sys/stat.h>
#include <string>

OutputManager::OutputManager() : filePtr(nullptr), buffer(nullptr), bufferSize(4096), bufferPos(0) {
    allocateBuffer(bufferSize);
}

OutputManager::OutputManager(size_t bufferSize) : filePtr(nullptr), buffer(nullptr), bufferSize(bufferSize), bufferPos(0) {
    if (bufferSize == 0) {
        throw std::invalid_argument("Buffer size cannot be zero");
    }
    allocateBuffer(bufferSize);
}

OutputManager::~OutputManager() {
    closeFile();
    freeBuffer();
}

void OutputManager::allocateBuffer(size_t size) {
    buffer = new char[size];
    if (!buffer) {
        throw std::bad_alloc();
    }
}

void OutputManager::freeBuffer() {
    if (buffer) {
        delete[] buffer;
        buffer = nullptr;
    }
}

bool OutputManager::openInOutcome(const char* filename) {
    std::string fullPath = "outcome/";
    fullPath += filename;
    
    return openFile(fullPath.c_str());
}

bool OutputManager::openFile(const char* filename) {
    if (filePtr) {
        closeFile();
    }
    
    filePtr = fopen(filename, "wb");
    if (!filePtr) {
        return false;
    }
    
    bufferPos = 0;
    return true;
}

void OutputManager::closeFile() {
    if (filePtr) {
        flush();
        fclose(filePtr);
        filePtr = nullptr;
    }
}

void OutputManager::write(const char* data, size_t length) {
    if (!filePtr) {
        throw std::runtime_error("No file open for writing");
    }
    
    if (bufferPos + length > bufferSize) {
        flush();
    }
    
    if (length > bufferSize) {
        fwrite(data, 1, length, filePtr);
    } else {
        memcpy(buffer + bufferPos, data, length);
        bufferPos += length;
    }
}

void OutputManager::flush() {
    if (filePtr && bufferPos > 0) {
        fwrite(buffer, 1, bufferPos, filePtr);
        bufferPos = 0;
    }
}

bool OutputManager::isFileOpen() const {
    return filePtr != nullptr;
}

size_t OutputManager::getBufferSize() const {
    return bufferSize;
}