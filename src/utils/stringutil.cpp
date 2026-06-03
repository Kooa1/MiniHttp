#include "stringutil.h"
#include <sstream>

std::vector<std::string> StringUtil::SplitPath(const std::string &path) {
    std::vector<std::string> segments;
    std::istringstream ss(path);
    std::string segment;
    while (std::getline(ss, segment, '/')) {
        if (!segment.empty()) {
            segments.push_back(segment);
        }
    }
    return segments;
}
