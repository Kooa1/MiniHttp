//
// Created by 66 on 2026/6/3.
//

#ifndef MINIHTTP_PARSER_H
#define MINIHTTP_PARSER_H

#include <string>

#include "request.h"

class Parser {
public:
    enum State {
        METHOD,
        URI,
        VERSION,
        HEADER_KEY,
        HEADER_VALUE,
        BODY,
        DONE
    };

    Parser();

    size_t parse(const char *data, size_t len);

    bool isDone() const { return state_ == DONE; }

    const Request &getRequest() const { return request_; };

    void reset();

private:
    State state_ = METHOD;
    Request request_;
    std::string buffer_;
    std::string current_key_;
};


#endif //MINIHTTP_PARSER_H
