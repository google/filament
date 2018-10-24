/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_GPUBUFFER_H
#define TNT_FILAMENT_DETAILS_GPUBUFFER_H

#include <filament/driver/DriverEnums.h>

#include "driver/DriverApiForward.h"
#include "driver/Handle.h"

#include <utils/Slice.h>

namespace filament {

class GPUBuffer {
public:

    enum class ElementType : uint8_t {
        UINT8,
        INT8,
        UINT16,
        INT16,
        UINT32,
        INT32,
        HALF,
        FLOAT
    };

    struct Element {
        ElementType type : 3;
        uint8_t     size : 3; // 1 to 4 allowed
    };

    GPUBuffer() = default;

    GPUBuffer(driver::DriverApi& driverApi, Element element, size_t rowSize, size_t rowCount);

    GPUBuffer(const GPUBuffer&) = delete;
    GPUBuffer& operator=(const GPUBuffer&) = delete;
    GPUBuffer(GPUBuffer&&) noexcept;
    GPUBuffer& operator=(GPUBuffer&&) noexcept;

    // no need to specify a destructor, we're trivially_destructible

    void terminate(driver::DriverApi& driverApi) noexcept;

    size_t getSize() const noexcept { return mSize; }

    // source data isn't copied and must stay valid until the command-buffer is executed
    void commit(driver::DriverApi& driverApi, void const* begin, void const* end) noexcept {
        commitSlow(driverApi, begin, end);
    }

    template<typename T>
    void commit(driver::DriverApi& driverApi, utils::Slice<T> const& data) noexcept {
        commit(driverApi, data.cbegin(), data.cend());
    }

    void swap(GPUBuffer& rhs) noexcept;

private:
    // this is really hidden implementation details (the fact we're using a texture should be
    // exposed as little as possible)
    friend class SamplerBuffer;
    Handle<HwTexture> getHandle() const noexcept { return mTexture; }
    driver::SamplerParams getSamplerParams() const noexcept { return driver::SamplerParams{}; }

private:
    void commitSlow(driver::DriverApi& driverApi, void const* begin, void const* end) noexcept;

    Handle<HwTexture> mTexture;
    uint32_t mSize = 0;
    uint16_t mWidth;
    uint16_t mHeight;
    uint16_t mRowSizeInBytes;
    Element mElement;
    driver::PixelDataFormat mFormat;
    driver::PixelDataType mType;
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_GPUBUFFER_H
