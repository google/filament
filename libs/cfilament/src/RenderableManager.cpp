/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "API.h"
#include <filament/Engine.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/vec3.h>

using namespace filament;
using namespace utils;

FBool Filament_RenderableManager_HasComponent(RenderableManager *rm,
        Entity entity) {
    return rm->hasComponent(entity);
}

uint32_t Filament_RenderableManager_GetInstance(RenderableManager *rm,
        Entity entity) {
    return rm->getInstance(entity);
}

void Filament_RenderableManager_Destroy(RenderableManager *rm,
        Entity entity) {
    rm->destroy(entity);
}

RenderableManager::Builder *
Filament_RenderableManager_CreateBuilder(size_t count) {
    return new RenderableManager::Builder(count);
}

void
Filament_RenderableManager_DestroyBuilder(RenderableManager::Builder *builder) {
    delete builder;
}

FBool Filament_RenderableManager_BuilderBuild(
        RenderableManager::Builder *builder, Engine *engine, Entity entity) {
    using namespace math;

    return builder->build(*engine, entity) == RenderableManager::Builder::Success;
}

void Filament_RenderableManager_BuilderGeometry1(
        RenderableManager::Builder *builder, size_t index,
        RenderableManager::PrimitiveType primitiveType, VertexBuffer *vertexBuffer,
        IndexBuffer *indexBuffer) {
    builder->geometry(index, primitiveType, vertexBuffer, indexBuffer);
}

void Filament_RenderableManager_BuilderGeometry2(
        RenderableManager::Builder *builder, size_t index,
        RenderableManager::PrimitiveType primitiveType, VertexBuffer *vertexBuffer,
        IndexBuffer *indexBuffer, size_t offset, size_t count) {

    builder->geometry(index, primitiveType, vertexBuffer, indexBuffer, offset,
            count);
}

void Filament_RenderableManager_BuilderGeometry3(
        RenderableManager::Builder *builder, size_t index,
        RenderableManager::PrimitiveType primitiveType, VertexBuffer *vertexBuffer,
        IndexBuffer *indexBuffer, size_t offset, size_t minIndex, size_t maxIndex,
        size_t count) {

    builder->geometry(index, (RenderableManager::PrimitiveType) primitiveType,
            vertexBuffer, indexBuffer, offset, minIndex, maxIndex,
            count);
}

void Filament_RenderableManager_BuilderMaterial(
        RenderableManager::Builder *builder, size_t index,
        MaterialInstance *materialInstance) {
    builder->material(index, materialInstance);
}

void Filament_RenderableManager_BuilderBlendOrder(
        RenderableManager::Builder *builder, size_t index, uint16_t blendOrder) {

    builder->blendOrder(index, blendOrder);
}

void Filament_RenderableManager_BuilderBoundingBox(
        RenderableManager::Builder *builder, Box *box) {
    builder->boundingBox(*box);
}

void Filament_RenderableManager_BuilderLayerMask(
        RenderableManager::Builder *builder, uint8_t select, uint8_t value) {
    builder->layerMask(select, value);
}

void Filament_RenderableManager_BuilderPriority(
        RenderableManager::Builder *builder, uint8_t priority) {

    builder->priority(priority);
}

void Filament_RenderableManager_BuilderCulling(
        RenderableManager::Builder *builder, FBool enabled) {
    builder->culling(enabled);
}

void Filament_RenderableManager_BuilderCastShadows(
        RenderableManager::Builder *builder, FBool enabled) {
    builder->castShadows(enabled);
}

void Filament_RenderableManager_BuilderReceiveShadows(
        RenderableManager::Builder *builder, FBool enabled) {
    builder->receiveShadows(enabled);
}

void Filament_RenderableManager_BuilderSkinning(
        RenderableManager::Builder *builder, size_t boneCount) {
    builder->skinning(boneCount);
}

int Filament_RenderableManager_BuilderSkinningBones(
        RenderableManager::Builder *builder, RenderableManager::Bone *bones,
        size_t boneCount) {
    builder->skinning(boneCount, bones);
    return 0;
}

int Filament_RenderableManager_SetBonesAsMatrices(
        RenderableManager *rm, uint32_t i, math::mat4f *matrices, size_t boneCount,
        size_t offset) {
    rm->setBones(i, matrices, boneCount, offset);
    return 0;
}

int Filament_RenderableManager_SetBonesAsQuaternions(
        RenderableManager *rm, uint32_t i, RenderableManager::Bone *bones,
        size_t boneCount, size_t offset) {
    rm->setBones(i, bones, boneCount, offset);
    return 0;
}

void Filament_RenderableManager_SetAxisAlignedBoundingBox(
        RenderableManager *rm, uint32_t i, float cx, float cy, float cz, float ex,
        float ey, float ez) {
    rm->setAxisAlignedBoundingBox(i, {{cx, cy, cz},
                                      {ex, ey, ez}});
}

void Filament_RenderableManager_SetLayerMask(RenderableManager *rm,
        uint32_t i,
        uint8_t select,
        uint8_t value) {
    rm->setLayerMask(i, select, value);
}

void Filament_RenderableManager_SetPriority(RenderableManager *rm,
        uint32_t i,
        uint8_t priority) {
    rm->setPriority(i, priority);
}

void Filament_RenderableManager_SetCastShadows(RenderableManager *rm,
        uint32_t i,
        FBool enabled) {
    rm->setCastShadows(i, enabled);
}

void Filament_RenderableManager_SetReceiveShadows(
        RenderableManager *rm, uint32_t i, FBool enabled) {
    rm->setReceiveShadows(i, enabled);
}

FBool Filament_RenderableManager_IsShadowCaster(RenderableManager *rm,
        uint32_t i) {
    return (FBool) rm->isShadowCaster(i);
}

FBool Filament_RenderableManager_IsShadowReceiver(RenderableManager *rm,
        uint32_t i) {
    return (FBool) rm->isShadowReceiver(i);
}

void Filament_RenderableManager_GetAxisAlignedBoundingBox(
        RenderableManager *rm, uint32_t i, Box *aabbOut) {
    *aabbOut = rm->getAxisAlignedBoundingBox(i);
}

int Filament_RenderableManager_GetPrimitiveCount(RenderableManager *rm,
        uint32_t i) {
    return rm->getPrimitiveCount(i);
}

void Filament_RenderableManager_SetMaterialInstanceAt(
        RenderableManager *rm, uint32_t i, size_t primitiveIndex,
        MaterialInstance *materialInstance) {
    rm->setMaterialInstanceAt(i, primitiveIndex, materialInstance);
}

void Filament_RenderableManager_SetGeometryAt1(
        RenderableManager *rm, uint32_t i, size_t primitiveIndex,
        RenderableManager::PrimitiveType primitiveType, VertexBuffer *vertexBuffer,
        IndexBuffer *indexBuffer, size_t offset, size_t count) {
    rm->setGeometryAt(i, primitiveIndex, primitiveType, vertexBuffer, indexBuffer,
            offset, count);
}

void Filament_RenderableManager_SetGeometryAt2(
        RenderableManager *rm, uint32_t i, size_t primitiveIndex,
        RenderableManager::PrimitiveType primitiveType, size_t offset,
        size_t count) {
    rm->setGeometryAt(i, primitiveIndex, primitiveType, offset, count);
}

void Filament_RenderableManager_SetBlendOrderAt(RenderableManager *rm,
        uint32_t i,
        size_t primitiveIndex,
        uint16_t blendOrder) {
    rm->setBlendOrderAt(i, primitiveIndex, blendOrder);
}

uint32_t Filament_RenderableManager_GetEnabledAttributesAt(
        RenderableManager *rm, uint32_t i, size_t primitiveIndex) {
    AttributeBitset enabled = rm->getEnabledAttributesAt(i, primitiveIndex);
    return enabled.getValue();
}
