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

#ifndef TNT_FILAMENT_BACKEND_WEBGPU_MEMORYMAPPEDBUFFER_H
#define TNT_FILAMENT_BACKEND_WEBGPU_MEMORYMAPPEDBUFFER_H

#include "DriverBase.h"


#include <backend/DriverEnums.h>

namespace filament::backend {

struct WebGPUMemoryMappedBuffer : public HwMemoryMappedBuffer {
    WebGPUMemoryMappedBuffer(BufferObjectHandle bo, size_t offset, size_t size,
            MapBufferAccessFlags access) noexcept;

    BufferObjectHandle bufferObject;
    size_t offset;
    size_t size;
    MapBufferAccessFlags access;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_WEBGPU_MEMORYMAPPEDBUFFER_H
