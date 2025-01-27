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

#include "details/SkinningBuffer.h"

#include "details/Engine.h"

namespace filament {

using namespace backend;
using namespace math;

void SkinningBuffer::setBones(Engine& engine,
        RenderableManager::Bone const* transforms, size_t const count, size_t const offset) {
    downcast(this)->setBones(downcast(engine), transforms, count, offset);
}

void SkinningBuffer::setBones(Engine& engine,
        mat4f const* transforms, size_t const count, size_t const offset) {
    downcast(this)->setBones(downcast(engine), transforms, count, offset);
}

size_t SkinningBuffer::getBoneCount() const noexcept {
    return downcast(this)->getBoneCount();
}

} // namespace filament

