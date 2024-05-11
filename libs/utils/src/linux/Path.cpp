/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <dirent.h>
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include <cstdint>

namespace utils {

bool Path::mkdir() const {
    return ::mkdir(m_path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) == 0;
}

Path Path::getCurrentExecutable() {
    // First, need to establish resource path.
    char exec_buf[2048];
    Path result;

    uint32_t buffer_size = sizeof(exec_buf)-1;
    ssize_t sz = readlink("/proc/self/exe", exec_buf, buffer_size);
    if (sz > 0) {
        exec_buf[sz] = 0;
        result.setPath(exec_buf);
    }

    return result;
}

Path Path::getTemporaryDirectory() {
    return Path("/tmp/");
}

Path Path::getUserSettingsDirectory() {
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pwd = getpwuid(getuid());
        if (pwd) {
           home = pwd->pw_dir;
        }
    }
    return Path(home);
}

std::vector<Path> Path::listContents() const {
    // Return an empty vector if the path doesn't exist or is not a directory
    if (!isDirectory() || !exists()) {
        return {};
    }

    struct dirent* directory;
    DIR* dir;

    dir = opendir(c_str());
    if (dir == nullptr) {
        // Path does not exist or could not be read
        return {};
    }

    std::vector<Path> directory_contents;

    while ((directory = readdir(dir)) != nullptr) {
        const char* file = directory->d_name;
        if (file[0] != '.') {
            directory_contents.push_back(concat(directory->d_name));
        }
    }

    closedir(dir);
    return directory_contents;
}
} // namespace utils
