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

bool RenderableManager::hasComponent(Entity const e) const noexcept {
    return downcast(this)->hasComponent(e);
}

size_t RenderableManager::getComponentCount() const noexcept {
    return downcast(this)->getComponentCount();
}

bool RenderableManager::empty() const noexcept {
    return downcast(this)->empty();
}

Entity RenderableManager::getEntity(Instance const i) const noexcept {
    return downcast(this)->getEntity(i);
}

Entity const* RenderableManager::getEntities() const noexcept {
    return downcast(this)->getEntities();
}

RenderableManager::Instance
RenderableManager::getInstance(Entity const e) const noexcept {
    return downcast(this)->getInstance(e);
}

void RenderableManager::destroy(Entity const e) noexcept {
    return downcast(this)->destroy(e);
}

void RenderableManager::setAxisAlignedBoundingBox(Instance const instance, const Box& aabb) {
    downcast(this)->setAxisAlignedBoundingBox(instance, aabb);
}

void RenderableManager::setLayerMask(Instance const instance, uint8_t const select, uint8_t const values) noexcept {
    downcast(this)->setLayerMask(instance, select, values);
}

void RenderableManager::setPriority(Instance const instance, uint8_t const priority) noexcept {
    downcast(this)->setPriority(instance, priority);
}

void RenderableManager::setChannel(Instance const instance, uint8_t const channel) noexcept{
    downcast(this)->setChannel(instance, channel);
}

void RenderableManager::setCulling(Instance const instance, bool const enable) noexcept {
    downcast(this)->setCulling(instance, enable);
}

void RenderableManager::setCastShadows(Instance const instance, bool const enable) noexcept {
    downcast(this)->setCastShadows(instance, enable);
}

void RenderableManager::setReceiveShadows(Instance const instance, bool const enable) noexcept {
    downcast(this)->setReceiveShadows(instance, enable);
}

void RenderableManager::setScreenSpaceContactShadows(Instance const instance, bool const enable) noexcept {
    downcast(this)->setScreenSpaceContactShadows(instance, enable);
}

bool RenderableManager::isShadowCaster(Instance const instance) const noexcept {
    return downcast(this)->isShadowCaster(instance);
}

bool RenderableManager::isShadowReceiver(Instance const instance) const noexcept {
    return downcast(this)->isShadowReceiver(instance);
}

const Box& RenderableManager::getAxisAlignedBoundingBox(Instance const instance) const noexcept {
    return downcast(this)->getAxisAlignedBoundingBox(instance);
}

uint8_t RenderableManager::getLayerMask(Instance const instance) const noexcept {
    return downcast(this)->getLayerMask(instance);
}

size_t RenderableManager::getPrimitiveCount(Instance const instance) const noexcept {
    return downcast(this)->getPrimitiveCount(instance, 0);
}

void RenderableManager::setMaterialInstanceAt(Instance const instance,
        size_t const primitiveIndex, MaterialInstance const* materialInstance) {
    downcast(this)->setMaterialInstanceAt(instance, 0, primitiveIndex, downcast(materialInstance));
}

void RenderableManager::clearMaterialInstanceAt(Instance instance, size_t primitiveIndex) {
    downcast(this)->clearMaterialInstanceAt(instance, 0, primitiveIndex);
}

MaterialInstance* RenderableManager::getMaterialInstanceAt(
        Instance const instance, size_t const primitiveIndex) const noexcept {
    return downcast(this)->getMaterialInstanceAt(instance, 0, primitiveIndex);
}

void RenderableManager::setBlendOrderAt(Instance const instance, size_t const primitiveIndex, uint16_t const order) noexcept {
    downcast(this)->setBlendOrderAt(instance, 0, primitiveIndex, order);
}

void RenderableManager::setGlobalBlendOrderEnabledAt(Instance const instance,
        size_t const primitiveIndex, bool const enabled) noexcept {
    downcast(this)->setGlobalBlendOrderEnabledAt(instance, 0, primitiveIndex, enabled);
}

AttributeBitset RenderableManager::getEnabledAttributesAt(Instance const instance, size_t const primitiveIndex) const noexcept {
    return downcast(this)->getEnabledAttributesAt(instance, 0, primitiveIndex);
}

void RenderableManager::setGeometryAt(Instance const instance, size_t const primitiveIndex,
        PrimitiveType const type, VertexBuffer* vertices, IndexBuffer* indices,
        size_t const offset, size_t const count) noexcept {
    downcast(this)->setGeometryAt(instance, 0, primitiveIndex,
            type, downcast(vertices), downcast(indices), offset, count);
}

void RenderableManager::setBones(Instance const instance,
        Bone const* transforms, size_t const boneCount, size_t const offset) {
    downcast(this)->setBones(instance, transforms, boneCount, offset);
}

void RenderableManager::setBones(Instance const instance,
        mat4f const* transforms, size_t const boneCount, size_t const offset) {
    downcast(this)->setBones(instance, transforms, boneCount, offset);
}

void RenderableManager::setSkinningBuffer(Instance const instance,
        SkinningBuffer* skinningBuffer, size_t const count, size_t const offset) {
    downcast(this)->setSkinningBuffer(instance, downcast(skinningBuffer), count, offset);
}

void RenderableManager::setMorphWeights(Instance const instance, float const* weights,
        size_t const count, size_t const offset) {
    downcast(this)->setMorphWeights(instance, weights, count, offset);
}

void RenderableManager::setMorphTargetBufferOffsetAt(Instance const instance, uint8_t const level,
        size_t const primitiveIndex,
        size_t const offset) {
    downcast(this)->setMorphTargetBufferOffsetAt(instance, level, primitiveIndex, offset);
}

MorphTargetBuffer* RenderableManager::getMorphTargetBuffer(Instance const instance) const noexcept {
    return downcast(this)->getMorphTargetBuffer(instance);
}

size_t RenderableManager::getMorphTargetCount(Instance const instance) const noexcept {
    return downcast(this)->getMorphTargetCount(instance);
}

void RenderableManager::setLightChannel(Instance const instance, unsigned int const channel, bool const enable) noexcept {
    downcast(this)->setLightChannel(instance, channel, enable);
}

bool RenderableManager::getLightChannel(Instance const instance, unsigned int const channel) const noexcept {
    return downcast(this)->getLightChannel(instance, channel);
}

void RenderableManager::setFogEnabled(Instance const instance, bool const enable) noexcept {
    downcast(this)->setFogEnabled(instance, enable);
}

bool RenderableManager::getFogEnabled(Instance const instance) const noexcept {
    return downcast(this)->getFogEnabled(instance);
}

} // namespace filament
