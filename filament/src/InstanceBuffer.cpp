/*
* Copyright (C) 2023 The Android Open Source Project
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

#include "details/InstanceBuffer.h"

#include <filament/InstanceBuffer.h>

#include <math/mat4.h>

#include <cstddef>

namespace filament {

size_t InstanceBuffer::getInstanceCount() const noexcept {
    return downcast(this)->getInstanceCount();
}

void InstanceBuffer::setLocalTransforms(
        math::mat4f const* localTransforms, size_t const count, size_t const offset) {
    downcast(this)->setLocalTransforms(localTransforms, count, offset);
}

math::mat4f const& InstanceBuffer::getLocalTransform(size_t index) {
    return downcast(this)->getLocalTransform(index);
}

} // namespace filament
