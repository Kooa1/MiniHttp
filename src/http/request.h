//
// Created by 66 on 2026/6/3.
//

#ifndef MINIHTTP_REQUEST_H
#define MINIHTTP_REQUEST_H

#include <map>
#include <string>
#include <algorithm>

class Request {
public:
    enum Method {
        GET,
        POST,
        HEAD,
        UNKNOWN
    };

    Method method() const { return method_; };

    std::string uri() const { return uri_; };

    std::string header(const std::string &key) const;

    std::string body() const { return body_; };

    std::string methodStr() const;

    std::string param(const std::string &key) const;

    void setMethod(Method method) { method_ = method; };

    void setUri(const std::string &uri) { uri_ = uri; };

    void addHeader(const std::string &key, const std::string &value);

    void setBody(const std::string &body) { body_ = body; };

    void setParam(const std::string &key, const std::string &value);

    void reset();

private:
    Method method_ = UNKNOWN;
    std::string uri_;
    std::map<std::string, std::string> headers_;
    std::map<std::string, std::string> params_;
    std::string body_;
};


#endif //MINIHTTP_REQUEST_H
