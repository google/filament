/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "ResourceLoaderExtended.h"

#include "../FFilamentAsset.h"

#include <backend/BufferDescriptor.h>
#include <filament/BufferObject.h>

#include <cgltf.h>

#include <vector>

static const auto FREE_CALLBACK = [](void* mem, size_t, void*) {
    free(mem);
 };

namespace filament::gltfio {

using filament::backend::BufferDescriptor;

void ResourceLoaderExtended::loadResources(std::vector<BufferSlot> const& slots,
        filament::Engine* engine, std::vector<BufferObject*>& bufferObjects) {
    for (auto& slot: slots) {
        size_t const byteCount = slot.sizeInBytes;
        if (slot.vertices) {
            BufferObject* bo = BufferObject::Builder().size(byteCount).build(*engine);
            bo->setBuffer(*engine, BufferDescriptor(slot.data, byteCount, FREE_CALLBACK));
            slot.vertices->setBufferObjectAt(*engine, slot.slot, bo);
            bufferObjects.push_back(bo);
        }
        if (slot.indices) {
            slot.indices->setBuffer(*engine, BufferDescriptor(slot.data, byteCount, FREE_CALLBACK));
        }
        if (slot.target) {
            assert_invariant(slot.targetData.positions && slot.targetData.tbn);
            slot.target->setPositionsAt(*engine, slot.slot,
                    (float3 const*) slot.targetData.positions, slot.target->getVertexCount());
            slot.target->setTangentsAt(*engine, slot.slot, (short4 const*) slot.targetData.tbn,
                    slot.target->getVertexCount());

            free(slot.targetData.positions);
            free(slot.targetData.tbn);
        }
    }
}

} // namespace filament::gltfio
