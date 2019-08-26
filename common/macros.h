#pragma once

#include <stdint.h>

static const int invalid_id = -1;
static const int invalid_index = -1;

static const int MAX_BUFFER_LEN = 1024;

static const int MAX_EVENT = 256;

#define SINGLETON_DECLARE(CLASS)        \
public:                                  \
    static CLASS* instance() {           \
        static CLASS object;             \
        return &object;                  \
    }                                    \
private:                                 \
    CLASS();                             \
    CLASS(const CLASS&);                 \
    CLASS& operator = (const CLASS&);    \

#define SAFE_DELETE(p) \
    if (p) {           \
        delete p;      \
    }
#define SAFE_DELETE_ARRAY(p) \
    if (p) {                 \
        delete[] p;         \
        p = nullptr;         \
    }
