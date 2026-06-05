//
// Created by 66 on 2026/6/5.
//

#ifndef MINIHTTP_BUFFER_H
#define MINIHTTP_BUFFER_H

#include <vector>
#include <string>
#include <algorithm>
#include <sys/uio.h>
#include <unistd.h>

class Buffer {
public:
    static constexpr size_t kCheapPrepend = 8;
    static constexpr size_t kInitialSize = 1024;

    Buffer();

    size_t readableBytes() const { return write_index_ - read_index_; }

    size_t writableBytes() const { return buffer_.size() - write_index_; }

    size_t prependableBytes() const { return read_index_; }

    const char *peek() const { return begin() + read_index_; }

    char *beginWrite() { return begin() + write_index_; }

    const char *beginWrite() const { return begin() + write_index_; }

    void retrieve(size_t len);

    void retrieveAll();

    std::string retrieveAsString();

    void append(const char *data, size_t len);

    ssize_t readFd(int fd);

private:
    char *begin() { return &*buffer_.begin(); }

    const char *begin() const { return &*buffer_.begin(); }

    void makeSpace(size_t len);

    std::vector<char> buffer_;
    size_t read_index_;
    size_t write_index_;
};


#endif //MINIHTTP_BUFFER_H
