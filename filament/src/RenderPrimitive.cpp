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

#include "RenderPrimitive.h"

#include "details/Engine.h"
#include "details/IndexBuffer.h"
#include "details/Material.h"
#include "details/VertexBuffer.h"

#include <utils/debug.h>

namespace filament {

void FRenderPrimitive::init(HwRenderPrimitiveFactory& factory, backend::DriverApi& driver,
        const RenderableManager::Builder::Entry& entry) noexcept {

    assert_invariant(entry.materialInstance);

    mMaterialInstance = downcast(entry.materialInstance);
    mBlendOrder = entry.blendOrder;

    if (entry.indices && entry.vertices) {
        FVertexBuffer* vertexBuffer = downcast(entry.vertices);
        FIndexBuffer* indexBuffer = downcast(entry.indices);

        AttributeBitset enabledAttributes = vertexBuffer->getDeclaredAttributes();

        auto const& ebh = vertexBuffer->getHwHandle();
        auto const& ibh = indexBuffer->getHwHandle();

        mHandle = factory.create(driver, ebh, ibh, entry.type, (uint32_t)entry.offset,
                (uint32_t)entry.minIndex, (uint32_t)entry.maxIndex, (uint32_t)entry.count);

        mPrimitiveType = entry.type;
        mEnabledAttributes = enabledAttributes;
    }
}

void FRenderPrimitive::terminate(HwRenderPrimitiveFactory& factory, backend::DriverApi& driver) {
    if (mHandle) {
        factory.destroy(driver, mHandle);
    }
}

void FRenderPrimitive::set(HwRenderPrimitiveFactory& factory, backend::DriverApi& driver,
        RenderableManager::PrimitiveType type,
        FVertexBuffer* vertices, FIndexBuffer* indices, size_t offset,
        size_t minIndex, size_t maxIndex, size_t count) noexcept {

    AttributeBitset enabledAttributes = vertices->getDeclaredAttributes();

    auto const& ebh = vertices->getHwHandle();
    auto const& ibh = indices->getHwHandle();

    if (mHandle) {
        factory.destroy(driver, mHandle);
    }

    mHandle = factory.create(driver, ebh, ibh, type,
            (uint32_t)offset, (uint32_t)minIndex, (uint32_t)maxIndex, (uint32_t)count);

    mPrimitiveType = type;
    mEnabledAttributes = enabledAttributes;
}

} // namespace filament
