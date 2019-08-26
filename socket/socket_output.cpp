#include "socket/socket_output.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <memory.h>

#include "common/macros.h"

static constexpr int SOCKET_OUTPUT_SIZE = 16 * 1024;
static constexpr int SOCKET_OUTPUT_SIZE_HALF = SOCKET_OUTPUT_SIZE >> 1;
static constexpr int SOCKET_OUTPUT_MAXSIZE = 2014 * 1024;

SocketOutput::SocketOutput() {
    buffer_len_ = SOCKET_OUTPUT_SIZE;
    max_buffer_len_ = SOCKET_OUTPUT_MAXSIZE;

    buffer_ = new char[SOCKET_OUTPUT_SIZE];
    memset(buffer_, 0, SOCKET_OUTPUT_SIZE);
}

SocketOutput::~SocketOutput() {
    SAFE_DELETE_ARRAY(buffer_);
}

int SocketOutput::write(const char* buff, const int size) {
    int free_bytes = (head_ <= tail_) ?
                     (buffer_len_ - tail_ + head_ - 1) :
                     (head_ - tail_ - 1);

    if (size >= free_bytes) {
        if (!resize(size - free_bytes + 1))
            return 0;
    }

    if (head_ < tail_) {
        if (head_ == 0)
            memcpy(&buffer_[tail_], buff, size);
        else {
            int tail_bytes = buffer_len_ - tail_;
            memcpy(&buffer_[tail_], buff, size);
            if (size > tail_bytes)
                memcpy(&buffer_[0], &buff[tail_bytes], size - tail_bytes);
        }
    }
    else {
        memcpy(&buffer_[tail_], buff, size);
    }

    tail_ = (tail_ + size) % buffer_len_;
    return size;
}

int SocketOutput::refresh(const int socket_fd) {
    int out_bytes = 0;
    int sent_bytes = 0;
    int left_bytes = 0;

    if (head_ < tail_) {
        left_bytes = tail_ - head_;
        while (left_bytes > 0) {
            sent_bytes = send(socket_fd, &buffer_[head_], left_bytes, 0);
            if (sent_bytes < 0)
                return sent_bytes;

            left_bytes -= sent_bytes;
            head_ += sent_bytes;
            out_bytes += sent_bytes;
        }
    }
    else if (head_ > tail_) {
        left_bytes = buffer_len_ - head_;
        while (left_bytes > 0) {
            sent_bytes = send(socket_fd, &buffer_[head_], left_bytes, 0);
            if (sent_bytes < 0)
                return sent_bytes;

            left_bytes -= sent_bytes;
            head_ += sent_bytes;
            out_bytes += sent_bytes;
        }

        head_ = 0;
        left_bytes = tail_;
        while (left_bytes > 0) {
            sent_bytes = send(socket_fd, &buffer_[head_], left_bytes, 0);
            if (sent_bytes < 0)
                return sent_bytes;

            left_bytes -= sent_bytes;
            head_ += sent_bytes;
            out_bytes += sent_bytes;
        }
    }

    head_ = tail_ = 0;

    return out_bytes;
}

void SocketOutput::init() {
    head_ = 0;
    tail_ = 0;

    SAFE_DELETE_ARRAY(buffer_);

    buffer_len_ = SOCKET_OUTPUT_SIZE;
    buffer_ = new char[buffer_len_];
    memset(buffer_, 0, buffer_len_);
}

bool SocketOutput::resize(const int size) {
    if (size <= 0)
        return false;

    int len = length();

    int tmp_size = (size > SOCKET_OUTPUT_SIZE_HALF) ? size : SOCKET_OUTPUT_SIZE_HALF;
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

int SocketOutput::length() const {
    if (head_ < tail_)
        return tail_ - head_;
    else if (head_ > tail_)
        return buffer_len_ - head_ + tail_;
    return 0;
}
