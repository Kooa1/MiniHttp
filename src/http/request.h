//
// Created by 66 on 2026/6/3.
//

#ifndef MINIHTTP_REQUEST_H
#define MINIHTTP_REQUEST_H
#include <map>
#include <string>


class Request {
public:
    enum Method {
        GET,
        POST,
        HEAD,
        UNKNOWN
    };

    Method method() const;

    std::string uri() const;

    std::string header(const std::string &key) const;

    std::string body() const;

    void setMethod(Method method);

    void setUri(const std::string &uri);

    void addHeader(const std::string &key, const std::string &value);

    void setBody(const std::string &body);

private:
    Method method_;
    std::string uri_;
    std::map<std::string, std::string> headers_;
    std::string body_;
};


#endif //MINIHTTP_REQUEST_H
