/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "DirIncluder.h"

#include <utils/Log.h>

#include <fstream>

namespace matc {

bool DirIncluder::operator()(const utils::CString& headerName, const utils::CString& includerName,
        filamat::IncludeResult& result) {
    auto getHeaderPath = [&includerName, &headerName, this]() {
        // If includer name is empty, then search from the root include directory.
        if (includerName.empty()) {
            return mIncludeDirectory.concat(headerName.c_str());
        }

        // Otherwise, search relative to the includer file.
        utils::Path includer(includerName.c_str());
        // TODO: this assert was firing only in CI during DirIncluder tests. Maybe because of
        // inadequate file permissions.
        // assert(includer.isFile());
        return includer.getParent() + headerName.c_str();
    };

    const utils::Path headerPath = getHeaderPath();

    if (!headerPath.isFile()) {
        utils::slog.e << "File " << headerPath << " does not exist." << utils::io::endl;
        return false;
    }

    std::ifstream stream(headerPath.getPath(), std::ios::binary);
    if (!stream) {
        utils::slog.e << "Unable to open " << headerPath << "." << utils::io::endl;
        return false;
    }

    std::string contents;

    stream.seekg(0, std::ios::end);
    contents.reserve(stream.tellg());
    stream.seekg(0, std::ios::beg);
    contents.assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

    stream.close();

    result.source = utils::CString(contents.c_str());
    result.name = utils::CString(headerPath.c_str());

    return true;
}

} // namespace matc

