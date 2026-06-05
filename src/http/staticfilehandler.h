//
// Created by 66 on 2026/6/5.
//

#ifndef MINIHTTP_STATICFILEHANDLER_H
#define MINIHTTP_STATICFILEHANDLER_H

#include <string>
#include <unordered_map>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include <sstream>

#include "request.h"
#include "core/connection.h"

class StaticFileHandler {
public:
    StaticFileHandler(std::string prefix,
                      std::string root_dir,
                      size_t max_file_size = 8 * 1024 * 1024);

    void operator()(const Request &req, Connection *conn);

private:
    std::string resolvePath(const std::string uri) const;

    std::string mimeType(const std::string &ext) const;

    bool sendFile(Connection *conn, const std::string &filepath) const;

    std::string prefix_;
    std::string root_dir_;
    std::string root_real_;
    size_t max_file_size_;

    static const std::unordered_map<std::string, std::string> kMimeTypes;
};


#endif //MINIHTTP_STATICFILEHANDLER_H
