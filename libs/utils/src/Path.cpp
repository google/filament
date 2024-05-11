/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <utils/Path.h>

#include <sstream>
#include <ostream>
#include <iterator>

#if defined(WIN32)
#   include <utils/compiler.h>
#   include <utils/win32/stdtypes.h>
#   define SEPARATOR '\\'
#   define SEPARATOR_STR  "\\"
#else
#   include <unistd.h>
#   define SEPARATOR '/'
#   define SEPARATOR_STR "/"
#endif


#include <limits.h>
#include <sys/stat.h>

namespace utils {

Path::Path(const char* path)
    : Path(std::string(path)) {
}

Path::Path(std::string_view pathname)
    : Path(std::string(pathname)) {
}

Path::Path(const std::string& path)
    : m_path(getCanonicalPath(path)) {
}

bool Path::exists() const {
#if defined(_WIN64) || defined(_M_X64)
    struct _stat64 file;
    return _stat64(c_str(), &file) == 0;
#else
    struct stat file;
    return stat(c_str(), &file) == 0;
#endif
}

bool Path::isFile() const {
    struct stat file;
    if (stat(c_str(), &file) == 0) {
        return S_ISREG(file.st_mode);
    }
    return false;
}

bool Path::isDirectory() const {
    struct stat file;
    if (stat(c_str(), &file) == 0) {
        return S_ISDIR(file.st_mode);
    }
    return false;
}

Path Path::concat(const Path& path) const {
    if (path.isEmpty()) return *this;
    if (path.isAbsolute()) return path;
    // std::string::back() is UB if the string is empty, so we rely on short-circuit evaluation
    if (!m_path.empty() && m_path.back() != SEPARATOR) {
        return Path(m_path + SEPARATOR + path.getPath());
    }
    return Path(m_path + path.getPath());
}

void Path::concatToSelf(const Path& path)  {
    if (!path.isEmpty()) {
        if (path.isAbsolute()) {
            m_path = path.getPath();
        // std::string::back() is UB if the string is empty, so we rely on short-circuit evaluation
        } else if (!m_path.empty() && m_path.back() != SEPARATOR) {
            m_path = getCanonicalPath(m_path + SEPARATOR + path.getPath());
        } else {
            m_path = getCanonicalPath(m_path + path.getPath());
        }
    }
}

Path Path::concat(const std::string& root, const std::string& leaf) {
    return Path(root).concat(Path(leaf));
}

Path Path::getCurrentDirectory() {
    char directory[PATH_MAX + 1];
    if (getcwd(directory, PATH_MAX) == nullptr) {
        return Path();
    }
    return Path(directory);
}

Path Path::getAbsolutePath() const {
    if (isEmpty() || isAbsolute()) {
        return *this;
    }
    return getCurrentDirectory().concat(*this);
}


#if !defined(WIN32)
bool Path::isAbsolute() const {
    return !isEmpty() && m_path.front() == '/';
}
#endif

Path Path::getParent() const {
    if (isEmpty()) return "";

    std::string result;

    std::vector<std::string> segments(split());

    // if our path is absolute with a single segment,
    // be sure to keep the prefix component
    if (!isAbsolute() || segments.size() > 1) {
        segments.pop_back(); // peel the last one
    }

    for (auto const& s : segments) {
        result.append(s).append(SEPARATOR_STR);
    }
    return getCanonicalPath(result);
}

Path Path::getAncestor(int n) const {
    Path result = getParent();
    while (n--) {
        result = result.getParent();
    }
    return result;
}

std::string Path::getName() const {
    if (isEmpty()) return "";

    std::vector<std::string> segments(split());
    return segments.back();
}

std::string Path::getExtension() const {
    if (isEmpty() || isDirectory()) {
        return "";
    }

    auto name = getName();
    auto index = name.rfind('.');
    if (index != std::string::npos && index != 0) {
        return name.substr(index + 1);
    } else {
        return "";
    }
}

std::string Path::getNameWithoutExtension() const {
    std::string name = getName();
    size_t index = name.rfind('.');
    if (index != std::string::npos) {
        return name.substr(0, index);
    }
    return name;
}

std::ostream& operator<<(std::ostream& os, const Path& path) {
    os << path.getPath();
    return os;
}

std::vector<std::string> Path::split() const {
    std::vector<std::string> segments;
    if (isEmpty()) return segments;

    size_t current;
    ssize_t next = -1;

    // If there is a leading disk designator such as C:, this naturally becomes the first segment.
    // However if there there is leading slash or back slash, we need to explicitly preserve it.
    // Note that we are guaranteed to have at least one char in the path at this point.
    if (m_path[0] == '/' || m_path[0] == '\\') {
        segments.push_back(m_path.substr(0, 1));
        next = 0;
    }

    do {
      current = size_t(next + 1);
      next = m_path.find_first_of(SEPARATOR, current);

      std::string segment(m_path.substr(current, next - current));
      if (!segment.empty()) segments.push_back(segment);
    } while (next != std::string::npos);

    if (segments.empty()) segments.push_back(m_path);

    return segments;
}

std::string Path::getCanonicalPath(const std::string& path) {
    if (path.empty()) return "";

    std::vector<std::string> segments;

    // If the path starts with a / we must preserve it
    bool starts_with_slash = path.front() == SEPARATOR;
    // If the path does not end with a / we need to remove the
    // extra / added by the join process
    bool ends_with_slash = path.back() == SEPARATOR;

    size_t current;
    ssize_t next = -1;

    do {
        current = size_t(next + 1);
        // Handle both Unix and Windows style separators
        next = path.find_first_of("/\\", current);

        std::string segment(path.substr(current, next - current));
        size_t size = segment.length();

        // skip empty (keep initial)
        if (size == 0 && !segments.empty()) {
            continue;
        }

        // skip . (keep initial)
        if (segment == "." && !segments.empty()) {
            continue;
        }

        // remove ..
        if (segment == ".." && !segments.empty()) {
            if (segments.back().empty()) { // ignore if .. follows initial /
                continue;
            }
            if (segments.back() != "..") {
                segments.pop_back();
                continue;
            }
        }

        segments.push_back(segment);
    } while (next != std::string::npos);

    // Join the vector as a single string, every element is
    // separated by a '/'. This process adds an extra / at
    // the end that might need to be removed
    std::stringstream clean_path;
    std::copy(segments.begin(), segments.end(),
            std::ostream_iterator<std::string>(clean_path, SEPARATOR_STR));
    std::string new_path = clean_path.str();

    if (starts_with_slash && new_path.empty()) {
        new_path = SEPARATOR_STR;
    }

    if (!ends_with_slash && new_path.length() > 1) {
        new_path.pop_back();
    }

    return new_path;
}

bool Path::mkdirRecursive() const {
    if (isEmpty()) {
        return true;
    }
    errno = 0;
    bool success = mkdir();
    if (!success) {
        int saveErrno = errno;
        switch (saveErrno) {
        case EEXIST: {
            bool result = isDirectory();
            errno = saveErrno;
            return result;
        }
        case ENOENT:
            getParent().mkdirRecursive();
            return mkdir();
        default:
            break;
        }
    }
    return success;
}

bool Path::unlinkFile() {
    return ::unlink(m_path.c_str()) == 0;
}

} // namespace utils
