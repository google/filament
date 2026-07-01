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

#include <filamentapp/DesktopAssetLoader.h>
#include <filamentapp/AssetLoader.h>

#include <fstream>

namespace filament::app {

std::vector<uint8_t> DesktopAssetLoader::load(utils::Path const& path) const {
    std::ifstream in(path.c_str(), std::ifstream::binary | std::ifstream::ate);
    if (!in.is_open()) {
        return {};
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
