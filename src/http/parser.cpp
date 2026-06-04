//
// Created by 66 on 2026/6/3.
//

#include "parser.h"

Parser::Parser() {
    reset();
}

size_t Parser::parse(const char *data, size_t len) {
    size_t i = 0;

    for (; i < len && state_ != DONE; i++) {
        char c = data[i];

        switch (state_) {
            case METHOD: {
                if (c == ' ') {
                    if (buffer_ == "GET") request_.setMethod(Request::GET);
                    else if (buffer_ == "POST") request_.setMethod(Request::POST);
                    else if (buffer_ == "HEAD") request_.setMethod(Request::HEAD);
                    else request_.setMethod(Request::UNKNOWN);
                    buffer_.clear();
                    state_ = URI;
                } else {
                    if (!checkBuffer(c)) return i;
                }
                break;
            }
            case URI: {
                if (c == ' ') {
                    request_.setUri(buffer_);
                    buffer_.clear();
                    state_ = VERSION;
                } else {
                    if (!checkBuffer(c)) return i;
                }
                break;
            }
            case VERSION: {
                if (c == '\n') {
                    if (buffer_ == "HTTP/1.1") request_.setVersion(Request::HTTP_1_1);
                    else if (buffer_ == "HTTP/1.0") request_.setVersion(Request::HTTP_1_0);
                    buffer_.clear();
                    state_ = HEADER_KEY;
                } else if (c != '\r') {
                    if (!checkBuffer(c)) return i;
                }
                break;
            }
            case HEADER_KEY: {
                if (c == ':') {
                    current_key_ = buffer_;
                    buffer_.clear();
                    state_ = HEADER_VALUE;
                } else if (c == '\r') {
                } else if (c == '\n') {
                    buffer_.clear();
                    state_ = BODY;
                } else {
                    if (!checkBuffer(c)) return i;
                }
                break;
            }
            case HEADER_VALUE: {
                if (c == '\r') {
                } else if (c == '\n') {
                    if (!buffer_.empty() && buffer_[0] == ' ') {
                        request_.addHeader(current_key_, buffer_.substr(1));
                    } else {
                        request_.addHeader(current_key_, buffer_);
                    }
                    buffer_.clear();
                    current_key_.clear();
                    state_ = HEADER_KEY;
                } else {
                    if (!checkBuffer(c)) return i;
                }
                break;
            }
            case BODY: {
                if (body_bytes_remaining_ == 0) {
                    std::string cl = request_.header("content-length");
                    if (cl.empty()) {
                        state_ = DONE;
                        break;
                    }

                    body_bytes_remaining_ = std::stoul(cl);
                    if (body_bytes_remaining_ > kMaxBodySize) {
                        error_ = true;
                        return i;
                    }
                }

                size_t to_read = std::min(body_bytes_remaining_, len - i);
                request_.appendBody(data + i, to_read);
                i += to_read - 1;
                body_bytes_remaining_ -= to_read;
                if (body_bytes_remaining_ == 0) {
                    state_ = DONE;
                }
                break;
            }
            case DONE: {
                break;
            }
        }
    }

    if (state_ == BODY && body_bytes_remaining_ == 0) {
        std::string cl = request_.header("content-length");
        if (cl.empty()) {
            state_ = DONE;
        }
    }

    return i;
}

bool Parser::checkBuffer(char c) {
    if (buffer_.size() >= kMaxTokenSize) {
        error_ = true;
        return false;
    }
    buffer_ += c;
    return true;
}

void Parser::reset() {
    state_ = METHOD;
    request_.reset();
    buffer_.clear();
    current_key_.clear();
    error_ = false;
    body_bytes_remaining_ = 0;
}
