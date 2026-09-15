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

#include <filamentapp/AssetWriter.h>
#include <filamentapp/DesktopAssetWriter.h>

#include <fstream>

namespace filament::app {

utils::Path DesktopAssetWriter::resolve(utils::Path const& path) const {
    if (path.isAbsolute() || mRootPath.isEmpty()) {
        return path;
    }
    return mRootPath + path;
}

bool DesktopAssetWriter::write(utils::Path const& path, uint8_t const* data, size_t size) const {
    utils::Path targetPath = resolve(path);
    std::ofstream out(targetPath.c_str(), std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        if (targetPath != path) {
            out.open(path.c_str(), std::ios::binary | std::ios::trunc);
        }
        if (!out.is_open()) {
            return false;
        }
    }
    out.write(reinterpret_cast<const char*>(data), size);
    return out.good();
}

} // namespace filament::app
