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
            case METHOD:
                if (c == ' ') {
                    if (buffer_ == "GET") request_.setMethod(Request::GET);
                    else if (buffer_ == "POST") request_.setMethod(Request::POST);
                    else if (buffer_ == "HEAD") request_.setMethod(Request::HEAD);
                    else request_.setMethod(Request::UNKNOWN);
                    buffer_.clear();
                    state_ = URI;
                } else {
                    buffer_ += c;
                }
                break;

            case URI:
                if (c == ' ') {
                    request_.setUri(buffer_);
                    buffer_.clear();
                    state_ = VERSION;
                } else {
                    buffer_ += c;
                }
                break;

            case VERSION:
                if (c == '\n') {
                    buffer_.clear();
                    state_ = HEADER_KEY;
                }
                break;

            case HEADER_KEY:
                if (c == ':') {
                    current_key_ = buffer_;
                    buffer_.clear();
                    state_ = HEADER_VALUE;
                } else if (c == '\r') {
                } else if (c == '\n') {
                    buffer_.clear();
                    state_ = BODY;
                } else {
                    buffer_ += c;
                }
                break;

            case HEADER_VALUE:
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
                    buffer_ += c;
                }
                break;

            case BODY:
                request_.setBody(std::string(data + i, len - i));
                i = len;
                state_ = DONE;
                break;

            case DONE:
                break;
        }
    }

    return i;
}

void Parser::reset() {
    state_ = METHOD;
    request_.reset();
    buffer_.clear();
    current_key_.clear();
}
