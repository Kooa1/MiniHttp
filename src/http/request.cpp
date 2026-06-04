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

std::string Request::param(const std::string &key) const {
    auto it = params_.find(key);
    return it != params_.end() ? it->second : "";
}

void Request::addHeader(const std::string &key, const std::string &value) {
    std::string lower_key = key;
    std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
    headers_[lower_key] = value;
}

void Request::setParam(const std::string &key, const std::string &value) {
    params_[key] = value;
}

void Request::reset() {
    method_ = UNKNOWN;
    uri_.clear();
    headers_.clear();
    params_.clear();
    body_.clear();
    version_ = HTTP_UNKNOWN;
}
