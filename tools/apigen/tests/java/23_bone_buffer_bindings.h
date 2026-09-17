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

#ifndef TNT_FILAMENT_BONE_BUFFER_BINDINGS_TEST_H
#define TNT_FILAMENT_BONE_BUFFER_BINDINGS_TEST_H

#include <filament/Engine.h>
#include <filament/RenderableManager.h>

#include <utils/compiler.h>
#include <utils/Slice.h>

#include <math/mat4.h>

#include <stddef.h>

namespace filament {

class UTILS_PUBLIC BoneBufferBindingsTest {
public:
    /**
     * Updates the bone transforms in the range [offset, offset + transforms.size()).
     *
     * @param engine Reference to the filament::Engine to associate with.
     * @param transforms slice of Bone transforms
     * @param offset offset in elements (not bytes)
     */
    UTILS_APIGEN_ALTERNATE_NAME(setBonesAsQuaternions)
    void setBones(Engine& engine,
            utils::Slice<const RenderableManager::Bone> transforms,
            size_t offset = 0);

    /**
     * Updates the bone transforms in the range [offset, offset + transforms.size()).
     *
     * @param engine Reference to the filament::Engine to associate with.
     * @param transforms slice of mat4f transforms
     * @param offset offset in elements (not bytes)
     */
    UTILS_APIGEN_ALTERNATE_NAME(setBonesAsMatrices)
    void setBones(Engine& engine,
            utils::Slice<const math::mat4f> transforms,
            size_t offset = 0);
};

} // namespace filament

#endif // TNT_FILAMENT_BONE_BUFFER_BINDINGS_TEST_H
