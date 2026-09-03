/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include <filamentapp/AssetLoader.h>
#include <filamentapp/DesktopAssetLoader.h>

#include <fstream>

namespace filament::app {

utils::Path DesktopAssetLoader::resolve(utils::Path const& path) const {
    if (path.isAbsolute() || mRootPath.isEmpty()) {
        return path;
    }
    utils::Path candidate = mRootPath + path;
    if (candidate.exists()) {
        return candidate;
    }
    return path;
}

bool DesktopAssetLoader::exists(utils::Path const& path) const { return resolve(path).exists(); }

std::vector<uint8_t> DesktopAssetLoader::load(utils::Path const& path) const {
    utils::Path resolvedPath = resolve(path);
    std::ifstream in(resolvedPath.c_str(), std::ifstream::binary | std::ifstream::ate);
    if (!in.is_open()) {
        if (resolvedPath != path) {
            in.open(path.c_str(), std::ifstream::binary | std::ifstream::ate);
        }
        if (!in.is_open()) {
            return {};
        }
    }

    auto size = in.tellg();
    if (size <= 0) {
        return {};
    }

    in.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!in.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return {};
    }

    return buffer;
}

} // namespace filament::app
