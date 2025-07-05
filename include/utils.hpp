#pragma once

inline int fast_atoi(const char*& p) {
    int x = 0;
    while (*p >= '0' && *p <= '9') {
        x = x * 10 + (*p - '0');
        ++p;
    }
    return x;
}

inline float fast_atof(const char* p) {
    int int_part = 0;
    int frac_part = 0;
    int frac_digits = 0;
    bool negative = false;
    
    if (*p == '-') {
        negative = true;
        ++p;
    }
    
    while (*p >= '0' && *p <= '9') {
        int_part = int_part * 10 + (*p - '0');
        ++p;
    }
    
    if (*p == '.') {
        ++p;
        while (*p >= '0' && *p <= '9') {
            if (frac_digits < 2) {
                frac_part = frac_part * 10 + (*p - '0');
                ++frac_digits;
            }
            ++p;
        }
    }
    
    float value = int_part;
    if (frac_digits == 1) {
        value += frac_part * 0.1f;
    } else if (frac_digits == 2) {
        value += frac_part * 0.01f;
    }
    return negative ? -value : value;
}