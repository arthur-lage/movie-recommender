#pragma once

#include <cstdio>
#include <cstring>
#include <string>

class OutputManager {
public:
    OutputManager();
    explicit OutputManager(size_t bufferSize);
    ~OutputManager();
    
    bool openFile(const char* filename);
    bool openInOutcome(const char* filename);
    void closeFile();
    void write(const char* data, size_t length);
    void flush();
    bool isFileOpen() const;
    size_t getBufferSize() const;

private:
    FILE* filePtr;
    char* buffer;
    size_t bufferSize;
    size_t bufferPos;
    
    void allocateBuffer(size_t size);
    void freeBuffer();
};