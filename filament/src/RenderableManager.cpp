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
    return downcast(this)->hasComponent(e);
}

size_t RenderableManager::getComponentCount() const noexcept {
    return downcast(this)->getComponentCount();
}

bool RenderableManager::empty() const noexcept {
    return downcast(this)->empty();
}

utils::Entity RenderableManager::getEntity(RenderableManager::Instance i) const noexcept {
    return downcast(this)->getEntity(i);
}

utils::Entity const* RenderableManager::getEntities() const noexcept {
    return downcast(this)->getEntities();
}

RenderableManager::Instance
RenderableManager::getInstance(utils::Entity e) const noexcept {
    return downcast(this)->getInstance(e);
}

void RenderableManager::destroy(utils::Entity e) noexcept {
    return downcast(this)->destroy(e);
}

void RenderableManager::setAxisAlignedBoundingBox(Instance instance, const Box& aabb) {
    downcast(this)->setAxisAlignedBoundingBox(instance, aabb);
}

void RenderableManager::setLayerMask(Instance instance, uint8_t select, uint8_t values) noexcept {
    downcast(this)->setLayerMask(instance, select, values);
}

void RenderableManager::setPriority(Instance instance, uint8_t priority) noexcept {
    downcast(this)->setPriority(instance, priority);
}

void RenderableManager::setChannel(Instance instance, uint8_t channel) noexcept{
    downcast(this)->setChannel(instance, channel);
}

void RenderableManager::setCulling(Instance instance, bool enable) noexcept {
    downcast(this)->setCulling(instance, enable);
}

void RenderableManager::setCastShadows(Instance instance, bool enable) noexcept {
    downcast(this)->setCastShadows(instance, enable);
}

void RenderableManager::setReceiveShadows(Instance instance, bool enable) noexcept {
    downcast(this)->setReceiveShadows(instance, enable);
}

void RenderableManager::setScreenSpaceContactShadows(Instance instance, bool enable) noexcept {
    downcast(this)->setScreenSpaceContactShadows(instance, enable);
}

bool RenderableManager::isShadowCaster(Instance instance) const noexcept {
    return downcast(this)->isShadowCaster(instance);
}

bool RenderableManager::isShadowReceiver(Instance instance) const noexcept {
    return downcast(this)->isShadowReceiver(instance);
}

const Box& RenderableManager::getAxisAlignedBoundingBox(Instance instance) const noexcept {
    return downcast(this)->getAxisAlignedBoundingBox(instance);
}

uint8_t RenderableManager::getLayerMask(Instance instance) const noexcept {
    return downcast(this)->getLayerMask(instance);
}

size_t RenderableManager::getPrimitiveCount(Instance instance) const noexcept {
    return downcast(this)->getPrimitiveCount(instance, 0);
}

void RenderableManager::setMaterialInstanceAt(Instance instance,
        size_t primitiveIndex, MaterialInstance const* materialInstance) {
    downcast(this)->setMaterialInstanceAt(instance, 0, primitiveIndex, downcast(materialInstance));
}

MaterialInstance* RenderableManager::getMaterialInstanceAt(
        Instance instance, size_t primitiveIndex) const noexcept {
    return downcast(this)->getMaterialInstanceAt(instance, 0, primitiveIndex);
}

void RenderableManager::setBlendOrderAt(Instance instance, size_t primitiveIndex, uint16_t order) noexcept {
    downcast(this)->setBlendOrderAt(instance, 0, primitiveIndex, order);
}

void RenderableManager::setGlobalBlendOrderEnabledAt(RenderableManager::Instance instance,
        size_t primitiveIndex, bool enabled) noexcept {
    downcast(this)->setGlobalBlendOrderEnabledAt(instance, 0, primitiveIndex, enabled);
}

AttributeBitset RenderableManager::getEnabledAttributesAt(Instance instance, size_t primitiveIndex) const noexcept {
    return downcast(this)->getEnabledAttributesAt(instance, 0, primitiveIndex);
}

void RenderableManager::setGeometryAt(Instance instance, size_t primitiveIndex,
        PrimitiveType type, VertexBuffer* vertices, IndexBuffer* indices,
        size_t offset, size_t count) noexcept {
    downcast(this)->setGeometryAt(instance, 0, primitiveIndex,
            type, downcast(vertices), downcast(indices), offset, count);
}

void RenderableManager::setBones(Instance instance,
        RenderableManager::Bone const* transforms, size_t boneCount, size_t offset) {
    downcast(this)->setBones(instance, transforms, boneCount, offset);
}

void RenderableManager::setBones(Instance instance,
        mat4f const* transforms, size_t boneCount, size_t offset) {
    downcast(this)->setBones(instance, transforms, boneCount, offset);
}

void RenderableManager::setSkinningBuffer(Instance instance,
        SkinningBuffer* skinningBuffer, size_t count, size_t offset) {
    downcast(this)->setSkinningBuffer(instance, downcast(skinningBuffer), count, offset);
}

void RenderableManager::setMorphWeights(Instance instance, float const* weights,
        size_t count, size_t offset) {
    downcast(this)->setMorphWeights(instance, weights, count, offset);
}

void RenderableManager::setMorphTargetBufferOffsetAt(Instance instance, uint8_t level,
        size_t primitiveIndex,
        size_t offset) {
    downcast(this)->setMorphTargetBufferOffsetAt(instance, level, primitiveIndex, offset);
}

MorphTargetBuffer* RenderableManager::getMorphTargetBuffer(Instance instance) const noexcept {
    return downcast(this)->getMorphTargetBuffer(instance);
}

size_t RenderableManager::getMorphTargetCount(Instance instance) const noexcept {
    return downcast(this)->getMorphTargetCount(instance);
}

void RenderableManager::setLightChannel(Instance instance, unsigned int channel, bool enable) noexcept {
    downcast(this)->setLightChannel(instance, channel, enable);
}

bool RenderableManager::getLightChannel(Instance instance, unsigned int channel) const noexcept {
    return downcast(this)->getLightChannel(instance, channel);
}

void RenderableManager::setFogEnabled(RenderableManager::Instance instance, bool enable) noexcept {
    downcast(this)->setFogEnabled(instance, enable);
}

bool RenderableManager::getFogEnabled(RenderableManager::Instance instance) const noexcept {
    return downcast(this)->getFogEnabled(instance);
}

} // namespace filament
