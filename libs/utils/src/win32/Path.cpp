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

#include <direct.h>
#include <Strsafe.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <windows.h>
#include <shlwapi.h>

#include <filesystem>

namespace utils {

bool Path::mkdir() const {
    return _mkdir(m_path.c_str()) == 0;
}

Path Path::getCurrentExecutable() {
    // First, need to establish resource path.
    CHAR path[MAX_PATH + 1];
    Path result;

    GetModuleFileNameA(NULL, path, MAX_PATH + 1);
    result.setPath(path);

    return result;
}

Path Path::getTemporaryDirectory() {
    CHAR lpTempPathBuffer[MAX_PATH];
    DWORD dwRetVal = GetTempPathA(MAX_PATH, lpTempPathBuffer);
    return Path(lpTempPathBuffer);
}

std::vector<Path> Path::listContents() const {
    // Return an empty vector if the path doesn't exist or is not a directory
    if (!isDirectory() || !exists()) {
        return {};
    }

    CHAR dirName[MAX_PATH];
    StringCchCopyA(dirName, MAX_PATH, c_str());

    WIN32_FIND_DATAA findData;
    HANDLE find = FindFirstFileA(dirName, &findData);

    std::vector<Path> directory_contents;
    do
    {
        if (findData.cFileName[0] != '.') {
            directory_contents.push_back(concat(findData.cFileName));
        }
    } while (FindNextFileA(find, &findData) != 0);

    return directory_contents;
}

bool Path::isAbsolute() const {
    return std::filesystem::path(m_path).is_absolute();
}

} // namespace utils
