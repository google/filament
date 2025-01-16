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

#include "details/IndexBuffer.h"

#include "details/Engine.h"

namespace filament {

void IndexBuffer::setBuffer(Engine& engine,
        BufferDescriptor&& buffer, uint32_t byteOffset) {
    downcast(this)->setBuffer(downcast(engine), std::move(buffer), byteOffset);
}

size_t IndexBuffer::getIndexCount() const noexcept {
    return downcast(this)->getIndexCount();
}

} // namespace filament
