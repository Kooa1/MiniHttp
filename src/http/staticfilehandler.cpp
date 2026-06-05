//
// Created by 66 on 2026/6/5.
//

#include "staticfilehandler.h"

const std::unordered_map<std::string, std::string> StaticFileHandler::kMimeTypes = {
    {".html", "text/html"},
    {".htm", "text/html"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".json", "application/json"},
    {".png", "image/png"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".gif", "image/gif"},
    {".svg", "image/svg+xml"},
    {".ico", "image/x-icon"},
    {".pdf", "application/pdf"},
    {".txt", "text/plain"},
    {".xml", "application/xml"},
    {".zip", "application/zip"},
    {".woff", "font/woff"},
    {".woff2", "font/woff2"},
};

StaticFileHandler::StaticFileHandler(std::string prefix,
                                     std::string root_dir,
                                     size_t max_file_size)
    : prefix_(std::move(prefix)),
      root_dir_(std::move(root_dir)),
      max_file_size_(max_file_size) {
    char *resolved = realpath(root_dir_.c_str(), nullptr);
    if (resolved) {
        root_real_ = resolved;
        free(resolved);
    }

    if (!prefix_.empty() && prefix_.back() == '/') {
        prefix_.pop_back();
    }
}

void StaticFileHandler::operator()(const Request &req, Connection *conn) {
    const std::string &uri = req.uri();

    if (uri.find(prefix_) != 0) {
        conn->mSend("Not Found", 404);
        return;
    }

    std::string relative = uri.substr(prefix_.size());
    if (relative.empty()) relative = "/";

    std::string filepath = resolvePath(relative);
    if (filepath == "___NOT_FOUND___") {
        conn->mSend("Not Found", 404);
        return;
    }

    if (filepath.empty()) {
        conn->mSend("Forbiddem", 403);
        return;
    }

    struct stat st{};
    if (stat(filepath.c_str(), &st) != 0) {
        conn->mSend("Not Found", 404);
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        std::string index = filepath + "/index.html";
        if (stat(index.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            filepath = index;
        } else {
            conn->mSend("Forbidden", 403);
            return;
        }
    }

    if (st.st_size > max_file_size_) {
        conn->mSend("Payload Too Large", 413);
        return;
    }

    bool keep_alive = false;
    std::string conn_hdr = req.header("connection");
    if (conn_hdr == "keep-alive") {
        keep_alive = true;
    } else if (conn_hdr != "close" && req.version() == Request::HTTP_1_1) {
        keep_alive = true;
    }

    if (!sendFile(conn, filepath, keep_alive)) {
        conn->mSend("Internal Server Error", 500);
        conn->cleanupAfterSend();
    }
}

std::string StaticFileHandler::resolvePath(const std::string uri) const {
    std::string candidate = root_dir_ + uri;

    char *resolved = realpath(candidate.c_str(), nullptr);
    if (!resolved) {
        return errno == ENOENT ? "___NOT_FOUND___" : "";
    };

    std::string result(resolved);
    free(resolved);

    if (result.find(root_real_) != 0) return "";

    return result;
}

std::string StaticFileHandler::mimeType(const std::string &ext) const {
    auto it = kMimeTypes.find(ext);
    return it != kMimeTypes.end() ? it->second : "application/octet-stream";
}

bool StaticFileHandler::sendFile(Connection *conn, const std::string &filepath, bool keep_alive) const {
    int fd = open(filepath.c_str(), O_RDONLY);
    if (fd < 0) return false;

    struct stat st{};
    fstat(fd, &st);

    conn->setKeepAlive(keep_alive);

    // 查扩展名 → MIME
    size_t dot = filepath.find_last_of('.');
    std::string ext = (dot != std::string::npos) ? filepath.substr(dot) : "";
    std::string content_type = mimeType(ext);

    // 构造 header
    std::ostringstream hdr;
    hdr << "HTTP/1.1 200 OK\r\n"
            << "Content-Type: " << content_type << "\r\n"
            << "Content-Length: " << st.st_size << "\r\n"
            << "\r\n";
    std::string header_str = hdr.str();

    // MSG_MORE header，与 sendfile 合包
    int conn_fd = conn->fd();
    ::send(conn_fd, header_str.data(), header_str.size(), MSG_MORE);

    // sendfile 循环分片（大文件支持）
    off_t offset = 0;
    size_t remaining = st.st_size;
    while (remaining > 0) {
        ssize_t sent = ::sendfile(conn_fd, fd, &offset, remaining);
        if (sent <= 0) {
            close(fd);
            return false;
        }
        remaining -= sent;
    }

    close(fd);

    conn->setSent(true);

    return true;
}
