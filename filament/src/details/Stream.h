/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_STREAM_H
#define TNT_FILAMENT_DETAILS_STREAM_H

#include "upcast.h"

#include "driver/DriverBase.h"

#include <filament/Stream.h>

#include <utils/compiler.h>

namespace filament {
namespace details {

class FEngine;

class FStream : public Stream {
public:
    FStream(FEngine& engine, const Builder& builder) noexcept;
    void terminate(FEngine& engine) noexcept;

    Handle<HwStream> getHandle() const noexcept { return mStreamHandle; }

    void setDimensions(uint32_t width, uint32_t height) noexcept;

    void readPixels(uint32_t xoffset, uint32_t yoffset, uint32_t width, uint32_t height,
            driver::PixelBufferDescriptor&& buffer) noexcept;

    bool isNativeStream() const noexcept { return mNativeStream != nullptr; }

    bool isExternalTextureId() const noexcept { return !isNativeStream(); }

    uint32_t getWidth() const noexcept { return mWidth; }

    uint32_t getHeight() const noexcept { return mHeight; }

    int64_t getTimestamp() const noexcept;

private:
    FEngine& mEngine;
    Handle<HwStream> mStreamHandle;
    void* mNativeStream = nullptr;
    intptr_t mExternalTextureId;
    uint32_t mWidth;
    uint32_t mHeight;
};

FILAMENT_UPCAST(Stream)

} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_STREAM_H
