#pragma once

#include <stdlib.h>

#include <unistd.h>

#include "common/macros.h"

class EnumHash {
public:
    template <typename T>
    typename std::enable_if<std::is_enum<T>::value, int>::type
    operator () (const T& t) const {
        return static_cast<int>(t);
    }
};

inline int hash_index(const int key, const int max) {
    return key % max;
}

inline int get_procnum() {
    static int proc_num = invalid_index;
    if (proc_num == invalid_index)
        proc_num = sysconf(_SC_NPROCESSORS_ONLN);
    return proc_num;
}
