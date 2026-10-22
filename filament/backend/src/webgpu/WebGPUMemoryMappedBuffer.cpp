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

#include "WebGPUMemoryMappedBuffer.h"

namespace filament::backend {

WebGPUMemoryMappedBuffer::WebGPUMemoryMappedBuffer(BufferObjectHandle bo, size_t offset, size_t size,
        MapBufferAccessFlags access) noexcept
    : bufferObject(bo), offset(offset), size(size), access(access) {
}

} // namespace filament::backend
