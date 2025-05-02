#pragma once

#include "string.h"

using namespace std;

template<typename T>
class Tokenizer {
    private:
        char* tokens;
        char* delimiter;
    public:
        Tokenizer();
        ~Tokenizer();

        T get_data_removing();
        T get_data_without_removing();

        void set_delimiter(char* delimiter);
        void tokenizer(char line[]);
};