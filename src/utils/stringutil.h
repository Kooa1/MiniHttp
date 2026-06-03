#ifndef MINIHTTP_STRING_UTIL_H
#define MINIHTTP_STRING_UTIL_H

#include <string>
#include <vector>

class StringUtil {
public:
    static std::vector<std::string> SplitPath(const std::string &path);
};

#endif
