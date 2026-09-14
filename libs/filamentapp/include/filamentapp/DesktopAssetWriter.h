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

#ifndef TNT_FILAMENTAPP_DESKTOPASSETWRITER_H
#define TNT_FILAMENTAPP_DESKTOPASSETWRITER_H

#include <filamentapp/AssetWriter.h>

#include <utils/Path.h>

#include <cstddef>
#include <cstdint>

namespace filament::app {

class UTILS_PUBLIC DesktopAssetWriter : public AssetWriter {
public:
    DesktopAssetWriter()
            : mRootPath() {}
    explicit DesktopAssetWriter(utils::Path rootPath)
            : mRootPath(std::move(rootPath)) {}
    ~DesktopAssetWriter() override = default;

    bool write(utils::Path const& path, uint8_t const* data, size_t size) const override;
    utils::Path resolve(utils::Path const& path) const override;

    utils::Path const& getRootPath() const noexcept { return mRootPath; }

private:
    const utils::Path mRootPath;
};

} // namespace filament::app

#endif // TNT_FILAMENTAPP_DESKTOPASSETWRITER_H
