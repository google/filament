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

class DesktopAssetWriter : public AssetWriter {
public:
    ~DesktopAssetWriter() override = default;

    bool write(utils::Path const& path, uint8_t const* data, size_t size) const override;
};

} // namespace filament::app

#endif // TNT_FILAMENTAPP_DESKTOPASSETWRITER_H
