#include "socket/socket_input.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <memory.h>

#include "common/macros.h"

// initialize size(bytes)
static constexpr int SOCKET_INPUT_SIZE = 16;//64*1024;
static constexpr int SOCKET_INPUT_SIZE_HALF = SOCKET_INPUT_SIZE >> 1;
// maximum size
static constexpr int SOCKET_INPUT_MAXSIZE = 1024 * 1024;

SocketInput::SocketInput()
    : buffer_len_(SOCKET_INPUT_SIZE),
      max_buffer_len_(SOCKET_INPUT_MAXSIZE) {
    buffer_ = new char[buffer_len_];
    memset(buffer_, 0, buffer_len_);
}

SocketInput::~SocketInput() {
    SAFE_DELETE_ARRAY(buffer_);
}

void SocketInput::init() {
    head_ = 0;
    tail_ = 0;

    buffer_len_ = SOCKET_INPUT_SIZE;
    SAFE_DELETE_ARRAY(buffer_);
    buffer_ = new char[buffer_len_];
    memset(buffer_, 0, buffer_len_);
}

bool SocketInput::resize(const int32_t size) {
    if (size <= 0)
        return false;

    int len = length();

    int tmp_size = (size > SOCKET_INPUT_SIZE_HALF) ? size : SOCKET_INPUT_SIZE_HALF;
    int32_t new_buffer_len = buffer_len_ + tmp_size;

    char* new_buffer = new char[new_buffer_len];
    if (head_ < tail_)
        memcpy(new_buffer, &buffer_[head_], tail_ - head_);
    else if (head_ > tail_){
        memcpy(new_buffer, &buffer_[head_], buffer_len_ - head_);
        memcpy(&new_buffer[buffer_len_ - head_], &buffer_[0], tail_);
    }

    SAFE_DELETE_ARRAY(buffer_);
    buffer_ = new_buffer;
    buffer_len_ = new_buffer_len;

    head_ = 0;
    tail_ = len;

    return true;
}

int32_t SocketInput::length() const {
    if (head_ < tail_)
        return tail_ - head_;
    else if (head_ > tail_)
        return buffer_len_ - head_ + tail_;
    return 0;
}

void SocketInput::fetch(char* const buff, const int size) {
    if (size == 0 || size > length())
        return;

    if (head_ < tail_)
        memcpy(buff, &buffer_[head_], size);
    else {
        int32_t right_size = buffer_len_ - head_;
        if (size < right_size)
            memcpy(buff, &buffer_[head_], size);
        else {
            memcpy(buff, &buffer_[head_], right_size);
            memcpy(&buff[right_size], buffer_, size - right_size);
        }
    }

    head_ = (head_ + size) % buffer_len_;
}

int SocketInput::feed(const int socket_fd) {
    int32_t filled_bytes = 0;
    int32_t received_bytes = 0;
    int32_t free_bytes = 0;

    if (head_ <= tail_) {
        if (head_ == 0) {
            free_bytes = buffer_len_ - tail_ - 1;
            // fill the free space first
            if (free_bytes > 0) {
                received_bytes = recv(socket_fd, &buffer_[tail_], free_bytes, 0);
                if (received_bytes <= 0)
                    return received_bytes;

                tail_ += received_bytes;
                filled_bytes += received_bytes;
            }

            // no space left or some bytes remained in cache
            if (free_bytes <= 0 || free_bytes == received_bytes) {
                int32_t remain_bytes = 0;
                ioctl(socket_fd, FIONREAD, &remain_bytes);
                if (remain_bytes > 0) {
                    // still need to receive.
                    // exceed the maximum size
                    if (buffer_len_ + remain_bytes + 1 > max_buffer_len_) {
                        init();
                        return -1000;
                    }
                    if (!resize(remain_bytes + 1))
                        // resize buffer failed, close the socket.
                        return 0;

                    received_bytes = recv(socket_fd, &buffer_[tail_], free_bytes, 0);
                    if (received_bytes <= 0)
                        return received_bytes;

                    tail_ += received_bytes;
                    filled_bytes += received_bytes;
                }
            }
        }
        else {
            // fill bytes after tail;
            free_bytes = buffer_len_ - tail_;
            received_bytes = recv(socket_fd, &buffer_[tail_], free_bytes, 0);
            if (received_bytes <= 0)
                return received_bytes;

            tail_ = (tail_ + received_bytes) % buffer_len_;
            filled_bytes += received_bytes;

            // are there still bytes remained?
            if (received_bytes == free_bytes) {
                int32_t remain_bytes = 0;
                ioctl(socket_fd, FIONREAD, &remain_bytes);
                if (remain_bytes <= 0)
                    return filled_bytes;

                // ### head_ or head_ - 1 ?
                free_bytes = head_ - 1;
                // left space is not enough
                if (remain_bytes > free_bytes) {
                    // exceed the maximum size
                    if (buffer_len_ + remain_bytes + 1 > max_buffer_len_) {
                        init();
                        return -1000;
                    }
                    if (!resize(remain_bytes + 1))
                        // resize buffer failed, close the socket.
                        return 0;

                    // receive all the remained bytes, start with tail_
                    received_bytes = recv(socket_fd, &buffer_[tail_], remain_bytes, 0);
                }
                else {
                    // left space is enough
                    // receive all the remained bytes, start with 0
                    received_bytes = recv(socket_fd, &buffer_[0], remain_bytes, 0);
                }
                if (received_bytes <= 0)
                    return received_bytes;

                tail_ += received_bytes;
                filled_bytes += received_bytes;
/*
                if (free_bytes > 0) {
                    received_bytes = recv(socket_fd, &buffer_[0], free_bytes, 0);
                    if (received_bytes <= 0)
                        return received_bytes;

                    tail_ += received_bytes;
                    filled_bytes += received_bytes;
                }
*/
/*
                free_bytes = head_ - 1;
                if (free_bytes > 0) {
                    received_bytes = recv(socket_fd, &buffer_[0], free_bytes, 0);
                    if (received_bytes <= 0)
                        return received_bytes;

                    tail_ += received_bytes;
                    filled_bytes += received_bytes;
                }
*/
/*
                // are there still bytes need to be received?
                if (received_bytes == free_bytes) {
                    int32_t remain_bytes = 0;
                    ioctl(socket_fd, FIONREAD, &remain_bytes);
                    if (remain_bytes > 0) {
                        // exceed the maximum size
                        if (buffer_len_ + remain_bytes + 1 > max_buffer_len_) {
                            init();
                            return -1000;
                        }

                        if (!resize(remain_bytes + 1))
                            // resize buffer failed, close the socket.
                            return 0;

                        received_bytes = recv(socket_fd, &buffer_[tail_], remain_bytes, 0);
                        if (received_bytes <= 0)
                            return received_bytes;

                        tail_ += received_bytes;
                        filled_bytes += received_bytes;
                    }
                }
*/
            }
        }
    }
    else {
        free_bytes = head_ - tail_ - 1;
        received_bytes = recv(socket_fd, &buffer_[tail_], free_bytes, 0);
        if (received_bytes <= 0)
            return received_bytes;

        tail_ += received_bytes;
        filled_bytes += received_bytes;

        if (free_bytes == received_bytes) {
            int32_t remain_bytes = 0;
            ioctl(socket_fd, FIONREAD, &remain_bytes);
            if (remain_bytes > 0) {
                if (buffer_len_ + remain_bytes + 1 > max_buffer_len_) {
                    init();
                    return -1000;
                }
                if (!resize(remain_bytes + 1))
                    return 0;

                received_bytes = recv(socket_fd, &buffer_[tail_], remain_bytes, 0);
                if (received_bytes <= 0)
                    return received_bytes;

                tail_ += received_bytes;
                filled_bytes += received_bytes;
            }
        }
    }

    return filled_bytes;
}

void SocketInput::skip(const int size) {
    if (size == 0 || size > length())
        return;
    head_ = (head_ + size) % buffer_len_;
}

void SocketInput::print_out() {
    std::cout << "SocketInput: length = " << length() << std::endl;
    std::cout << "SocketInput: buffer len = " << buffer_len_ << std::endl;
    std::cout << "SocketInput: head_ = " << head_ << ", tail_ = " << tail_ << std::endl;
    if (head_ < tail_) {
        for (int i = head_; i <= tail_; ++i)
            std::cout << buffer_[i];
        std::cout << std::endl;
    }
    else if (head_ > tail_) {
        for (int i = head_; i < buffer_len_; ++i)
            std::cout << buffer_[i];
        for (int i = 0; i <= tail_; ++i)
            std::cout << buffer_[i];
        std::cout << std::endl;
    }
}
