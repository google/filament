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

#include "details/Stream.h"

namespace filament {

using namespace backend;

StreamType Stream::getStreamType() const noexcept {
    return upcast(this)->getStreamType();
}

void Stream::setAcquiredImage(void* image, Callback callback, void* userdata) noexcept {
    upcast(this)->setAcquiredImage(image, callback, userdata);
}

void Stream::setAcquiredImage(void* image, backend::CallbackHandler* handler, Callback callback, void* userdata) noexcept {
    upcast(this)->setAcquiredImage(image, handler, callback, userdata);
}

void Stream::setDimensions(uint32_t width, uint32_t height) noexcept {
    upcast(this)->setDimensions(width, height);
}

int64_t Stream::getTimestamp() const noexcept {
    return upcast(this)->getTimestamp();
}

} // namespace filament
