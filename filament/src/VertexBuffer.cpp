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

#include "details/VertexBuffer.h"

#include "details/Engine.h"

namespace filament {

size_t VertexBuffer::getVertexCount() const noexcept {
    return downcast(this)->getVertexCount();
}

void VertexBuffer::setBufferAt(Engine& engine, uint8_t const bufferIndex,
        backend::BufferDescriptor&& buffer, uint32_t const byteOffset) {
    downcast(this)->setBufferAt(downcast(engine), bufferIndex, std::move(buffer), byteOffset);
}

void VertexBuffer::setBufferObjectAt(Engine& engine, uint8_t const bufferIndex,
        BufferObject const* bufferObject) {
    downcast(this)->setBufferObjectAt(downcast(engine), bufferIndex, downcast(bufferObject));
}

} // namespace filament
