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

#include <filament/RenderableManager.h>
#include <filament/MaterialEnums.h>

#include "details/IndexBuffer.h"
#include "details/MaterialInstance.h"
#include "details/VertexBuffer.h"

#include <private/backend/CommandStream.h>
#include <backend/DriverApiForward.h>

#include <utils/debug.h>

#include <stddef.h>

namespace filament {

void FRenderPrimitive::init(HwRenderPrimitiveFactory& factory, backend::DriverApi& driver,
        FRenderableManager::Entry const& entry) noexcept {

    assert_invariant(entry.materialInstance);

    mMaterialInstance = downcast(entry.materialInstance);
    mBlendOrder = entry.blendOrder;
    mGlobalBlendOrderEnabled = entry.globalBlendOrderEnabled;

    if (entry.indices && entry.vertices) {
        FVertexBuffer const* vertexBuffer = downcast(entry.vertices);
        FIndexBuffer const* indexBuffer = downcast(entry.indices);
        set(factory, driver, entry.type, vertexBuffer, indexBuffer, entry.offset, entry.count);
    }
}

void FRenderPrimitive::terminate(HwRenderPrimitiveFactory& factory, backend::DriverApi& driver) {
    if (mHandle) {
        factory.destroy(driver, mHandle);
    }
}

void FRenderPrimitive::set(HwRenderPrimitiveFactory& factory, backend::DriverApi& driver,
        RenderableManager::PrimitiveType const type,
        FVertexBuffer const* vertexBuffer, FIndexBuffer const* indexBuffer,
        size_t const offset, size_t const count) noexcept {
    if (mHandle) {
        factory.destroy(driver, mHandle);
    }

    AttributeBitset const enabledAttributes = vertexBuffer->getDeclaredAttributes();

    auto const& ebh = vertexBuffer->getHwHandle();
    auto const& ibh = indexBuffer->getHwHandle();

    mHandle = factory.create(driver, ebh, ibh, type);
    mVertexBufferInfoHandle = vertexBuffer->getVertexBufferInfoHandle();

    mPrimitiveType = type;
    mIndexOffset = offset;
    mIndexCount = count;
    mEnabledAttributes = enabledAttributes;
}

} // namespace filament
