#include "std_lib.h"

void swap_char(char *m, char *n) {
    char temp = *m;
    *m = *n;
    *n = temp;
}

void itos(char *numStr, uint32 num, uint32 mod) {
    if (mod < 2 || mod > 26 || num < 0) {
        return;
    }

    uint32 length, temp;

    length = 0;
    
    while (num) {
        temp = num % mod;
        num /= mod;
        numStr[length] = temp > 9 ? temp - 10 + 'A' : temp + '0';
        ++length;
    }

    if (!length) {
        numStr[0] = '0';
        ++length;
    }

    for (int i = 0, j = length - 1; i < j; ++i, --j) {
        swap_char(&numStr[i], &numStr[j]);
    }

    numStr[length] = '\0';
}

void memset(void *memory, char value, int length) {
    for (int i = 0; i < length; ++i) {
        ((char *)memory)[i] = value;
    }
}

int ceil(const int dividend, const int divisor) {
    return (dividend + divisor - 1) / divisor;
}

void memcpy(void *src, void *dst, uint32 length) {
    for (uint32 i = 0; i < length; ++i) {
        ((char *)dst)[i] = ((char *)src)[i];
    }
}

void strcpy(char *src, char *dst) {
    int i = 0;
    while(src[i]) {
        dst[i] = src[i];
        ++i;
    }
    dst[i] = '\0';
}