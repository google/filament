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

#include "components/TransformManager.h"

using namespace utils;

namespace filament {

using namespace math;

void TransformManager::create(Entity entity, Instance parent, const mat4f& worldTransform) {
    upcast(this)->create(entity, parent, worldTransform);
}

void TransformManager::create(Entity entity, Instance parent, const mat4& worldTransform) {
    upcast(this)->create(entity, parent, worldTransform);
}

void TransformManager::create(Entity entity, Instance parent) {
    upcast(this)->create(entity, parent, mat4f{});
}

void TransformManager::destroy(Entity e) noexcept {
    upcast(this)->destroy(e);
}

bool TransformManager::hasComponent(Entity e) const noexcept {
    return upcast(this)->hasComponent(e);
}

TransformManager::Instance TransformManager::getInstance(Entity e) const noexcept {
    return upcast(this)->getInstance(e);
}

void TransformManager::setTransform(Instance ci, const mat4f& model) noexcept {
    upcast(this)->setTransform(ci, model);
}

void TransformManager::setTransform(Instance ci, const mat4& model) noexcept {
    upcast(this)->setTransform(ci, model);
}

const mat4f& TransformManager::getTransform(Instance ci) const noexcept {
    return upcast(this)->getTransform(ci);
}

const mat4 TransformManager::getTransformAccurate(Instance ci) const noexcept {
    return upcast(this)->getTransformAccurate(ci);
}

const mat4f& TransformManager::getWorldTransform(Instance ci) const noexcept {
    return upcast(this)->getWorldTransform(ci);
}

const mat4 TransformManager::getWorldTransformAccurate(Instance ci) const noexcept {
    return upcast(this)->getWorldTransformAccurate(ci);
}

void TransformManager::setParent(Instance i, Instance newParent) noexcept {
    upcast(this)->setParent(i, newParent);
}

utils::Entity TransformManager::getParent(Instance i) const noexcept {
    return upcast(this)->getParent(i);
}

size_t TransformManager::getChildCount(Instance i) const noexcept {
    return upcast(this)->getChildCount(i);
}

size_t TransformManager::getChildren(Instance i, utils::Entity* children,
        size_t count) const noexcept {
    return upcast(this)->getChildren(i, children, count);
}

void TransformManager::openLocalTransformTransaction() noexcept {
    upcast(this)->openLocalTransformTransaction();
}

void TransformManager::commitLocalTransformTransaction() noexcept {
    upcast(this)->commitLocalTransformTransaction();
}

TransformManager::children_iterator TransformManager::getChildrenBegin(
        TransformManager::Instance parent) const noexcept {
    return upcast(this)->getChildrenBegin(parent);
}

TransformManager::children_iterator TransformManager::getChildrenEnd(
        TransformManager::Instance parent) const noexcept {
    return upcast(this)->getChildrenEnd(parent);
}

void TransformManager::setAccurateTranslationsEnabled(bool enable) noexcept {
    upcast(this)->setAccurateTranslationsEnabled(enable);
}

bool TransformManager::isAccurateTranslationsEnabled() const noexcept {
    return upcast(this)->isAccurateTranslationsEnabled();;
}

} // namespace filament
