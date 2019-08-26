#pragma once

#include <stdint.h>

class SocketInput {
public:
    SocketInput();
    ~SocketInput();

    void init();
    bool resize(const int32_t size);

    void fetch(char* const buff, const int size);
    int feed(const int socket_fd);
    void skip(const int size);

    int32_t length() const;

private:
    void print_out();
    char* buffer_ = nullptr;
    int32_t buffer_len_;
    int32_t max_buffer_len_;
    int32_t head_ = 0;
    int32_t tail_ = 0;
};
