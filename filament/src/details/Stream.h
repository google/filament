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

#include "downcast.h"
#include "math/mat3.h"

#include <filament/Stream.h>

#include <backend/Handle.h>

#include <utils/compiler.h>

namespace filament {

class FEngine;

class FStream : public Stream {
public:
    FStream(FEngine& engine, const Builder& builder) noexcept;
    void terminate(FEngine& engine) noexcept;

    backend::Handle<backend::HwStream> getHandle() const noexcept { return mStreamHandle; }

    void setAcquiredImage(void* image, Callback callback, void* userdata, math::mat3f transform) noexcept;
    void setAcquiredImage(void* image, backend::CallbackHandler* handler, Callback callback, void* userdata, math::mat3f transform) noexcept;

    void setDimensions(uint32_t width, uint32_t height) noexcept;

    StreamType getStreamType() const noexcept { return mStreamType; }

    uint32_t getWidth() const noexcept { return mWidth; }

    uint32_t getHeight() const noexcept { return mHeight; }

    int64_t getTimestamp() const noexcept;

private:
    FEngine& mEngine;
    const StreamType mStreamType;
    backend::Handle<backend::HwStream> mStreamHandle;
    void* mNativeStream = nullptr;
    uint32_t mWidth;
    uint32_t mHeight;
};

FILAMENT_DOWNCAST(Stream)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_STREAM_H
