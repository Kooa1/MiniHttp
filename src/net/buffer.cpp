//
// Created by 66 on 2026/6/5.
//

#include "buffer.h"

Buffer::Buffer()
    : buffer_(kCheapPrepend + kInitialSize),
      read_index_(kCheapPrepend),
      write_index_(kCheapPrepend) {
}

void Buffer::retrieve(size_t len) {
    if (len < readableBytes()) {
        read_index_ += len;
    } else {
        retrieveAll();
    }
}

void Buffer::retrieveAll() {
    read_index_ = kCheapPrepend;
    write_index_ = kCheapPrepend;
}

std::string Buffer::retrieveAsString() {
    std::string result(peek(), readableBytes());
    retrieveAll();
    return result;
}

void Buffer::append(const char *data, size_t len) {
    if (writableBytes() < len) {
        makeSpace(len);
    }
    std::copy(data, data + len, beginWrite());
    write_index_ += len;
}

ssize_t Buffer::readFd(int fd) {
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();

    vec[0].iov_base = begin() + write_index_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    ssize_t n = readv(fd, vec, iovcnt);
    if (n < 0) {
        return n;
    }

    if (static_cast<size_t>(n) <= writable) {
        write_index_ += n;
    } else {
        write_index_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    return n;
}

void Buffer::makeSpace(size_t len) {
    if (prependableBytes() + writableBytes() < len + kCheapPrepend) {
        buffer_.resize(write_index_ + len);
    } else {
        size_t readable = readableBytes();
        std::copy(begin() + read_index_, begin() + write_index_, begin() + kCheapPrepend);
        read_index_ = kCheapPrepend;
        write_index_ = read_index_ + readable;
    }
}
