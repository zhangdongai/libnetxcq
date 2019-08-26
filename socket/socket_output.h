#pragma once

#include <stdlib.h>

class SocketOutput {
public:
    SocketOutput();
    ~SocketOutput();

    void init();
    bool resize(const int size);

    int write(const char* buff, const int size);
    int refresh(const int socket_fd);

    int length() const;

private:
    char* buffer_ = nullptr;
    int buffer_len_;
    int max_buffer_len_;
    int head_ = 0;
    int tail_ = 0;
};
