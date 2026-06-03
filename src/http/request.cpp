//
// Created by 66 on 2026/6/3.
//

#include "request.h"

std::string Request::header(const std::string &key) const {
    std::string lower_key = key;
    std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
    auto it = headers_.find(lower_key);
    return it != headers_.end() ? it->second : "";
}

std::string Request::methodStr() const {
    switch (method_) {
        case GET: return "GET";
        case POST: return "POST";
        case HEAD: return "HEAD";
        default: return "UNKNOWN";
    }
}

void Request::addHeader(const std::string &key, const std::string &value) {
    std::string lower_key = key;
    std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
    headers_[lower_key] = value;
}

void Request::reset() {
    method_ = UNKNOWN;
    uri_.clear();
    headers_.clear();
    body_.clear();
}
