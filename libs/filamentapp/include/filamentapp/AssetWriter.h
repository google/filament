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

#ifndef TNT_FILAMENTAPP_ASSETWRITER_H
#define TNT_FILAMENTAPP_ASSETWRITER_H

#include <utils/Path.h>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace filament::app {

class AssetWriter {
public:
    virtual ~AssetWriter() = default;

    /**
     * Writes raw bytes to the specified path or destination identifier.
     *
     * @param path Target destination path or URI.
     * @param data Pointer to the buffer to write.
     * @param size Size in bytes.
     * @return true if the write completed successfully, false otherwise.
     */
    virtual bool write(utils::Path const& path, uint8_t const* data, size_t size) const = 0;

    /**
     * Convenience overload for vector buffers.
     */
    bool write(utils::Path const& path, std::vector<uint8_t> const& data) const {
        return write(path, data.data(), data.size());
    }
};

} // namespace filament::app

#endif // TNT_FILAMENTAPP_ASSETWRITER_H
