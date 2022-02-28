/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "components/RenderableManager.h"

#include "details/Engine.h"
#include "details/VertexBuffer.h"
#include "details/Material.h"

using namespace utils;

namespace filament {

using namespace backend;
using namespace math;

bool RenderableManager::hasComponent(utils::Entity e) const noexcept {
    return upcast(this)->hasComponent(e);
}

RenderableManager::Instance
RenderableManager::getInstance(utils::Entity e) const noexcept {
    return upcast(this)->getInstance(e);
}

void RenderableManager::destroy(utils::Entity e) noexcept {
    return upcast(this)->destroy(e);
}

void RenderableManager::setAxisAlignedBoundingBox(Instance instance, const Box& aabb) noexcept {
    upcast(this)->setAxisAlignedBoundingBox(instance, aabb);
}

void RenderableManager::setLayerMask(Instance instance, uint8_t select, uint8_t values) noexcept {
    upcast(this)->setLayerMask(instance, select, values);
}

void RenderableManager::setPriority(Instance instance, uint8_t priority) noexcept {
    upcast(this)->setPriority(instance, priority);
}

void RenderableManager::setCulling(Instance instance, bool enable) noexcept {
    upcast(this)->setCulling(instance, enable);
}

void RenderableManager::setCastShadows(Instance instance, bool enable) noexcept {
    upcast(this)->setCastShadows(instance, enable);
}

void RenderableManager::setReceiveShadows(Instance instance, bool enable) noexcept {
    upcast(this)->setReceiveShadows(instance, enable);
}

void RenderableManager::setScreenSpaceContactShadows(Instance instance, bool enable) noexcept {
    upcast(this)->setScreenSpaceContactShadows(instance, enable);
}

bool RenderableManager::isShadowCaster(Instance instance) const noexcept {
    return upcast(this)->isShadowCaster(instance);
}

bool RenderableManager::isShadowReceiver(Instance instance) const noexcept {
    return upcast(this)->isShadowReceiver(instance);
}

const Box& RenderableManager::getAxisAlignedBoundingBox(Instance instance) const noexcept {
    return upcast(this)->getAxisAlignedBoundingBox(instance);
}

uint8_t RenderableManager::getLayerMask(Instance instance) const noexcept {
    return upcast(this)->getLayerMask(instance);
}

size_t RenderableManager::getPrimitiveCount(Instance instance) const noexcept {
    return upcast(this)->getPrimitiveCount(instance, 0);
}

void RenderableManager::setMaterialInstanceAt(Instance instance,
        size_t primitiveIndex, MaterialInstance const* materialInstance) noexcept {
    upcast(this)->setMaterialInstanceAt(instance, 0, primitiveIndex, upcast(materialInstance));
}

MaterialInstance* RenderableManager::getMaterialInstanceAt(
        Instance instance, size_t primitiveIndex) const noexcept {
    return upcast(this)->getMaterialInstanceAt(instance, 0, primitiveIndex);
}

void RenderableManager::setBlendOrderAt(Instance instance, size_t primitiveIndex, uint16_t order) noexcept {
    upcast(this)->setBlendOrderAt(instance, 0, primitiveIndex, order);
}

AttributeBitset RenderableManager::getEnabledAttributesAt(Instance instance, size_t primitiveIndex) const noexcept {
    return upcast(this)->getEnabledAttributesAt(instance, 0, primitiveIndex);
}

void RenderableManager::setGeometryAt(Instance instance, size_t primitiveIndex,
        PrimitiveType type, VertexBuffer* vertices, IndexBuffer* indices,
        size_t offset, size_t count) noexcept {
    upcast(this)->setGeometryAt(instance, 0, primitiveIndex,
            type, upcast(vertices), upcast(indices), offset, count);
}

void RenderableManager::setGeometryAt(RenderableManager::Instance instance, size_t primitiveIndex,
        RenderableManager::PrimitiveType type, size_t offset, size_t count) noexcept {
    upcast(this)->setGeometryAt(instance, 0, primitiveIndex, type, offset, count);
}

void RenderableManager::setBones(Instance instance,
        RenderableManager::Bone const* transforms, size_t boneCount, size_t offset) {
    upcast(this)->setBones(instance, transforms, boneCount, offset);
}

void RenderableManager::setBones(Instance instance,
        mat4f const* transforms, size_t boneCount, size_t offset) {
    upcast(this)->setBones(instance, transforms, boneCount, offset);
}

void RenderableManager::setSkinningBuffer(Instance instance,
        SkinningBuffer* skinningBuffer, size_t count, size_t offset) {
    upcast(this)->setSkinningBuffer(instance, upcast(skinningBuffer), count, offset);
}

void RenderableManager::setMorphWeights(Instance instance, float const* weights,
        size_t count, size_t offset) {
    upcast(this)->setMorphWeights(instance, weights, count, offset);
}

void RenderableManager::setMorphTargetBufferAt(Instance instance, uint8_t level, size_t primitiveIndex,
        MorphTargetBuffer* morphTargetBuffer, size_t offset, size_t count) {
    upcast(this)->setMorphTargetBufferAt(instance, level, primitiveIndex,
            upcast(morphTargetBuffer), offset, count);
}

size_t RenderableManager::getMorphTargetCount(Instance instance) const noexcept {
    return upcast(this)->getMorphTargetCount(instance);
}

void RenderableManager::setLightChannel(Instance instance, unsigned int channel, bool enable) noexcept {
    upcast(this)->setLightChannel(instance, channel, enable);
}

bool RenderableManager::getLightChannel(Instance instance, unsigned int channel) const noexcept {
    return upcast(this)->getLightChannel(instance, channel);
}

} // namespace filament
