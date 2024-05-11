/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include "details/MorphTargetBuffer.h"

#include "details/Engine.h"

namespace filament {

void MorphTargetBuffer::setPositionsAt(Engine& engine, size_t targetIndex,
        math::float3 const* positions, size_t count, size_t offset) {
    downcast(this)->setPositionsAt(downcast(engine), targetIndex, positions, count, offset);
}

void MorphTargetBuffer::setPositionsAt(Engine& engine, size_t targetIndex,
        math::float4 const* positions, size_t count, size_t offset) {
    downcast(this)->setPositionsAt(downcast(engine), targetIndex, positions, count, offset);
}

void MorphTargetBuffer::setTangentsAt(Engine& engine, size_t targetIndex,
        math::short4 const* tangents, size_t count, size_t offset) {
    downcast(this)->setTangentsAt(downcast(engine), targetIndex, tangents, count, offset);
}

size_t MorphTargetBuffer::getVertexCount() const noexcept {
    return downcast(this)->getVertexCount();
}

size_t MorphTargetBuffer::getCount() const noexcept {
    return downcast(this)->getCount();
}

} // namespace filament

