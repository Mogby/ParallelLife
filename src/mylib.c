#ifndef MYLIB_H

#define MYLIB_H

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "mylib.h"

void foo() {
    puts("bar");
}

void printInt(int value) {
    size_t bufl = 0;
    char buf[20];

    if (value < 0) {
        buf[bufl++] = '-';
        value = -value;
    }

    while (value) {
        buf[bufl++] = value % 10 + '0';
        value /= 10;
    }

    if (!bufl) {
        buf[bufl++] = '0';
    }
    buf[bufl] = 0;

    size_t leftBound = (buf[0] == '-' ? 1 : 0);
    size_t rightBound = bufl - 1;
    char temporary;
    while (leftBound < rightBound) {
        temporary = buf[leftBound];
        buf[leftBound] = buf[rightBound];
        buf[rightBound] = temporary;
        ++leftBound, --rightBound;
    }

    for (size_t index = 0; index < bufl; ++index) {
        putchar(buf[index]);
    }
}

void printString(const char *string) {
    const char *index = string;
    while (*index) {
        putchar(*index);
        ++index;
    }
}

void myPrintf(const char *format, ...) {
    size_t formatLength = strlen(format);

    va_list values;
    va_start(values, format);
    for (size_t index = 0; index < formatLength; ++index) {
        if (format[index] == '%') {
            ++index;
            switch (format[index]) {
                case 'd':
                    printInt(va_arg(values, int));
                    break;
                case 's':
                    printString(va_arg(values, char*));
                    break;
            }
        } else {
            putchar(format[index]);
        }
    }
    va_end(values);
}

#endif