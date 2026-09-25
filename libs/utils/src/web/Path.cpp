/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include <vector>

#include <dirent.h>

namespace utils {

bool Path::mkdir() const {
    return true;
}

Path Path::getCurrentExecutable() {
    return Path("filament-wasm");
}

Path Path::getUserSettingsDirectory() {
    return Path(".");
}

std::vector<Path> Path::listContents() const {
    // Emscripten's virtual filesystem implements the POSIX directory API, so this is the same
    // traversal the native platforms do.
    if (!isDirectory() || !exists()) {
        return {};
    }

    DIR* dir = opendir(c_str());
    if (dir == nullptr) {
        return {};
    }

    std::vector<Path> directoryContents;
    while (struct dirent* entry = readdir(dir)) {
        const char* file = entry->d_name;
        if (file[0] != '.') {
            directoryContents.push_back(concat(file));
        }
    }

    closedir(dir);
    return directoryContents;
}

} // namespace utils
