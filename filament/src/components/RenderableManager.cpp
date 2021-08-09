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

#include "FilamentAPI-impl.h"

#include "components/RenderableManager.h"

#include "details/Engine.h"
#include "details/VertexBuffer.h"
#include "details/IndexBuffer.h"
#include "details/Material.h"
#include "details/RenderPrimitive.h"

#include <backend/DriverEnums.h>

#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/debug.h>
#include <filament/RenderableManager.h>


using namespace filament::math;
using namespace utils;

namespace filament {

using namespace backend;

struct RenderableManager::BuilderDetails {
    using Entry = RenderableManager::Builder::Entry;
    std::vector<Entry> mEntries;
    Box mAABB;
    uint8_t mLayerMask = 0x1;
    uint8_t mPriority = 0x4;
    uint8_t mChannels = 1;
    bool mCulling : 1;
    bool mCastShadows : 1;
    bool mReceiveShadows : 1;
    bool mScreenSpaceContactShadows : 1;
    bool mMorphingEnabled : 1;
    bool mSkinningBufferMode : 1;
    size_t mSkinningBoneCount = 0;
    Bone const* mUserBones = nullptr;
    mat4f const* mUserBoneMatrices = nullptr;
    FSkinningBuffer* mSkinningBuffer = nullptr;
    uint32_t mSkinningBufferOffset = 0;

    explicit BuilderDetails(size_t count)
            : mEntries(count), mCulling(true), mCastShadows(false), mReceiveShadows(true),
              mScreenSpaceContactShadows(false), mMorphingEnabled(false),
              mSkinningBufferMode(false) {
    }
    // this is only needed for the explicit instantiation below
    BuilderDetails() = default;
};

using BuilderType = RenderableManager;
BuilderType::Builder::Builder(size_t count) noexcept
        : BuilderBase<RenderableManager::BuilderDetails>(count) {
    assert_invariant(mImpl->mEntries.size() == count);
}
BuilderType::Builder::~Builder() noexcept = default;
BuilderType::Builder::Builder(BuilderType::Builder&& rhs) noexcept = default;
BuilderType::Builder& BuilderType::Builder::operator=(BuilderType::Builder&& rhs) noexcept = default;


RenderableManager::Builder& RenderableManager::Builder::geometry(size_t index,
        PrimitiveType type, VertexBuffer* vertices, IndexBuffer* indices) noexcept {
    return geometry(index, type, vertices, indices,
            0, 0, vertices->getVertexCount() - 1, indices->getIndexCount());
}

RenderableManager::Builder& RenderableManager::Builder::geometry(size_t index,
        PrimitiveType type, VertexBuffer* vertices, IndexBuffer* indices,
        size_t offset, size_t count) noexcept {
    return geometry(index, type, vertices, indices, offset,
            0, vertices->getVertexCount() - 1, count);
}

RenderableManager::Builder& RenderableManager::Builder::geometry(size_t index,
        PrimitiveType type, VertexBuffer* vertices, IndexBuffer* indices,
        size_t offset, size_t minIndex, size_t maxIndex, size_t count) noexcept {
    std::vector<Entry>& entries = mImpl->mEntries;
    if (index < entries.size()) {
        entries[index].vertices = vertices;
        entries[index].indices = indices;
        entries[index].offset = offset;
        entries[index].minIndex = minIndex;
        entries[index].maxIndex = maxIndex;
        entries[index].count = count;
        entries[index].type = type;
    }
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::material(size_t index,
        MaterialInstance const* materialInstance) noexcept {
    if (index < mImpl->mEntries.size()) {
        mImpl->mEntries[index].materialInstance = materialInstance;
    }
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::boundingBox(const Box& axisAlignedBoundingBox) noexcept {
    mImpl->mAABB = axisAlignedBoundingBox;
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::layerMask(uint8_t select, uint8_t values) noexcept {
    mImpl->mLayerMask = (mImpl->mLayerMask & ~select) | (values & select);
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::priority(uint8_t priority) noexcept {
    mImpl->mPriority = std::min(priority, uint8_t(0x7));
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::culling(bool enable) noexcept {
    mImpl->mCulling = enable;
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::lightChannel(unsigned int channel, bool enable) noexcept {
    if (channel < 8) {
        const uint8_t mask = 1u << channel;
        mImpl->mChannels &= ~mask;
        mImpl->mChannels |= enable ? mask : 0u;
    }
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::castShadows(bool enable) noexcept {
    mImpl->mCastShadows = enable;
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::receiveShadows(bool enable) noexcept {
    mImpl->mReceiveShadows = enable;
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::screenSpaceContactShadows(bool enable) noexcept {
    mImpl->mScreenSpaceContactShadows = enable;
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::skinning(size_t boneCount) noexcept {
    mImpl->mSkinningBoneCount = boneCount;
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::skinning(
        size_t boneCount, Bone const* bones) noexcept {
    mImpl->mSkinningBoneCount = boneCount;
    mImpl->mUserBones = bones;
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::skinning(
        size_t boneCount, mat4f const* transforms) noexcept {
    mImpl->mSkinningBoneCount = boneCount;
    mImpl->mUserBoneMatrices = transforms;
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::skinning(
        SkinningBuffer* skinningBuffer, size_t count, size_t offset) noexcept {
    mImpl->mSkinningBuffer = upcast(skinningBuffer);
    mImpl->mSkinningBoneCount = count;
    mImpl->mSkinningBufferOffset = offset;
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::enableSkinningBuffers(bool enabled) noexcept {
    mImpl->mSkinningBufferMode = enabled;
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::morphing(bool enable) noexcept {
    mImpl->mMorphingEnabled = enable;
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::blendOrder(size_t index, uint16_t blendOrder) noexcept {
    if (index < mImpl->mEntries.size()) {
        mImpl->mEntries[index].blendOrder = blendOrder;
    }
    return *this;
}

RenderableManager::Builder::Result RenderableManager::Builder::build(Engine& engine, Entity entity) {
    bool isEmpty = true;

    if (!ASSERT_PRECONDITION_NON_FATAL(mImpl->mSkinningBoneCount <= CONFIG_MAX_BONE_COUNT,
            "bone count > %u", CONFIG_MAX_BONE_COUNT)) {
        return Error;
    }

    for (size_t i = 0, c = mImpl->mEntries.size(); i < c; i++) {
        auto& entry = mImpl->mEntries[i];

        // entry.materialInstance must be set to something even if indices/vertices are null
        FMaterial const* material = nullptr;
        if (!entry.materialInstance) {
            material = upcast(engine.getDefaultMaterial());
            entry.materialInstance = material->getDefaultInstance();
        } else {
            material = upcast(entry.materialInstance->getMaterial());
        }

        // primitives without indices or vertices will be ignored
        if (!entry.indices || !entry.vertices) {
            continue;
        }

        // reject invalid geometry parameters
        if (!ASSERT_PRECONDITION_NON_FATAL(entry.offset + entry.count <= entry.indices->getIndexCount(),
                "[entity=%u, primitive @ %u] offset (%u) + count (%u) > indexCount (%u)",
                i, entity.getId(),
                entry.offset, entry.count, entry.indices->getIndexCount())) {
            entry.vertices = nullptr;
            return Error;
        }

        if (!ASSERT_PRECONDITION_NON_FATAL(entry.minIndex <= entry.maxIndex,
                "[entity=%u, primitive @ %u] minIndex (%u) > maxIndex (%u)",
                i, entity.getId(),
                entry.minIndex, entry.maxIndex)) {
            entry.vertices = nullptr;
            return Error;
        }

        // this can't be an error because (1) those values are not immutable, so the caller
        // could fix later, and (2) the material's shader will work (i.e. compile), and
        // use the default values for this attribute, which maybe be acceptable.
        AttributeBitset declared = upcast(entry.vertices)->getDeclaredAttributes();
        AttributeBitset required = material->getRequiredAttributes();
        if ((declared & required) != required) {
            slog.w << "[entity=" << entity.getId() << ", primitive @ " << i
                   << "] missing required attributes ("
                   << required << "), declared=" << declared << io::endl;
        }

        // we have at least one valid primitive
        isEmpty = false;
    }

    if (!ASSERT_POSTCONDITION_NON_FATAL(
            !mImpl->mAABB.isEmpty() ||
            (!mImpl->mCulling && (!(mImpl->mReceiveShadows || mImpl->mCastShadows)) ||
             isEmpty),
            "[entity=%u] AABB can't be empty, unless culling is disabled and "
                    "the object is not a shadow caster/receiver", entity.getId())) {
        return Error;
    }

    // we get here only if there was no POSTCONDITION errors.
    upcast(engine).createRenderable(*this, entity);
    return Success;
}

// ------------------------------------------------------------------------------------------------

FRenderableManager::FRenderableManager(FEngine& engine) noexcept : mEngine(engine) {
    // DON'T use engine here in the ctor, because it's not fully constructed yet.
}

FRenderableManager::~FRenderableManager() {
    // all components should have been destroyed when we get here
    // (terminate should have been called from Engine's shutdown())
    assert_invariant(mManager.getComponentCount() == 0);
}

void FRenderableManager::create(
        const RenderableManager::Builder& UTILS_RESTRICT builder, Entity entity) {
    FEngine& engine = mEngine;
    auto& manager = mManager;
    FEngine::DriverApi& driver = engine.getDriverApi();

    if (UTILS_UNLIKELY(manager.hasComponent(entity))) {
        destroy(entity);
    }
    Instance ci = manager.addComponent(entity);
    assert_invariant(ci);

    if (ci) {
        // create and initialize all needed RenderPrimitives
        using size_type = Slice<FRenderPrimitive>::size_type;
        Builder::Entry const * const entries = builder->mEntries.data();
        FRenderPrimitive* rp = new FRenderPrimitive[builder->mEntries.size()];
        for (size_t i = 0, c = builder->mEntries.size(); i < c; ++i) {
            rp[i].init(driver, entries[i]);
        }
        setPrimitives(ci, { rp, size_type(builder->mEntries.size()) });

        setAxisAlignedBoundingBox(ci, builder->mAABB);
        setLayerMask(ci, builder->mLayerMask);
        setPriority(ci, builder->mPriority);
        setCastShadows(ci, builder->mCastShadows);
        setReceiveShadows(ci, builder->mReceiveShadows);
        setScreenSpaceContactShadows(ci, builder->mScreenSpaceContactShadows);
        setCulling(ci, builder->mCulling);
        setSkinning(ci, false);
        setMorphing(ci, builder->mMorphingEnabled);
        setMorphWeights(ci, {0, 0, 0, 0});
        mManager[ci].channels = builder->mChannels;

        const uint32_t count = builder->mSkinningBoneCount;
        if (builder->mSkinningBufferMode) {
            if (builder->mSkinningBuffer) {
                setSkinning(ci, count > 0);
                Bones& bones = manager[ci].bones;
                bones = Bones{
                        .handle = builder->mSkinningBuffer->getHwHandle(),
                        .count = (uint16_t)count,
                        .offset = (uint16_t)builder->mSkinningBufferOffset,
                        .skinningBufferMode = true };
            }
        } else {
            if (UTILS_UNLIKELY(count > 0 || builder->mMorphingEnabled)) {
                setSkinning(ci, count > 0);
                Bones& bones = manager[ci].bones;
                // Note that we are sizing the bones UBO according to CONFIG_MAX_BONE_COUNT rather than
                // mSkinningBoneCount. According to the OpenGL ES 3.2 specification in 7.6.3 Uniform
                // Buffer Object Bindings:
                //
                //     the uniform block must be populated with a buffer object with a size no smaller
                //     than the minimum required size of the uniform block (the value of
                //     UNIFORM_BLOCK_DATA_SIZE).
                //
                // This unfortunately means that we are using a large memory footprint for skinned
                // renderables. In the future we could try addressing this by implementing a paging
                // system such that multiple skinned renderables will share regions within a single
                // large block of bones.
                bones = Bones{
                        .handle = driver.createBufferObject(
                                CONFIG_MAX_BONE_COUNT * sizeof(PerRenderableUibBone),
                                BufferObjectBinding::UNIFORM,
                                backend::BufferUsage::DYNAMIC),
                        .count = (uint16_t)count,
                        .offset = 0,
                        .skinningBufferMode = false };

                if (count) {
                    if (builder->mUserBones) {
                        FSkinningBuffer::setBones(mEngine, bones.handle,
                                builder->mUserBones, count, 0);
                    } else if (builder->mUserBoneMatrices) {
                        FSkinningBuffer::setBones(mEngine, bones.handle,
                                builder->mUserBoneMatrices, count, 0);
                    } else {
                        // initialize the bones to identity
                        size_t size = count * sizeof(PerRenderableUibBone);
                        auto* out = (PerRenderableUibBone*)driver.allocate(size);
                        std::uninitialized_fill_n(out, count, PerRenderableUibBone{});
                        driver.updateBufferObject(bones.handle, { out, size }, 0);
                    }
                }
            }
        }
    }
    engine.flushIfNeeded();
}

// this destroys a single component from an entity
void FRenderableManager::destroy(utils::Entity e) noexcept {
    Instance ci = getInstance(e);
    if (ci) {
        destroyComponent(ci);
        mManager.removeComponent(e);
    }
}

// this destroys all components in this manager
void FRenderableManager::terminate() noexcept {
    auto& manager = mManager;
    if (!manager.empty()) {
#ifndef NDEBUG
        slog.d << "cleaning up " << manager.getComponentCount()
               << " leaked Renderable components" << io::endl;
#endif
        while (!manager.empty()) {
            Instance ci = manager.end() - 1;
            destroyComponent(ci);
            manager.removeComponent(manager.getEntity(ci));
        }
    }
}

// This is basically a Renderable's destructor.
void FRenderableManager::destroyComponent(Instance ci) noexcept {
    auto& manager = mManager;
    FEngine& engine = mEngine;

    FEngine::DriverApi& driver = engine.getDriverApi();

    // See create(RenderableManager::Builder&, Entity)
    destroyComponentPrimitives(engine, manager[ci].primitives);

    // destroy the bones structures if any
    Bones const& bones = manager[ci].bones;
    if (bones.handle && !bones.skinningBufferMode) {
        driver.destroyBufferObject(bones.handle);
    }
}

void FRenderableManager::destroyComponentPrimitives(
        FEngine& engine, Slice<FRenderPrimitive>& primitives) noexcept {
    for (auto& primitive : primitives) {
        primitive.terminate(engine);
    }
    delete[] primitives.data();
}

void FRenderableManager::setMaterialInstanceAt(Instance instance, uint8_t level,
        size_t primitiveIndex, FMaterialInstance const* mi) noexcept {
    if (instance) {
        Slice<FRenderPrimitive>& primitives = getRenderPrimitives(instance, level);
        if (primitiveIndex < primitives.size()) {
            primitives[primitiveIndex].setMaterialInstance(upcast(mi));
            AttributeBitset required = mi->getMaterial()->getRequiredAttributes();
            AttributeBitset declared = primitives[primitiveIndex].getEnabledAttributes();
            if (UTILS_UNLIKELY((declared & required) != required)) {
                slog.w << "[instance=" << instance.asValue() << ", primitive @ " << primitiveIndex
                       << "] missing required attributes ("
                       << required << "), declared=" << declared << io::endl;
            }
        }
    }
}

MaterialInstance* FRenderableManager::getMaterialInstanceAt(
        Instance instance, uint8_t level, size_t primitiveIndex) const noexcept {
    if (instance) {
        const Slice<FRenderPrimitive>& primitives = getRenderPrimitives(instance, level);
        if (primitiveIndex < primitives.size()) {
            // We store the material instance as const because we don't want to change it internally
            // but when the user queries it, we want to allow them to call setParameter()
            return const_cast<FMaterialInstance*>(primitives[primitiveIndex].getMaterialInstance());
        }
    }
    return nullptr;
}

void FRenderableManager::setBlendOrderAt(Instance instance, uint8_t level,
        size_t primitiveIndex, uint16_t order) noexcept {
    if (instance) {
        Slice<FRenderPrimitive>& primitives = getRenderPrimitives(instance, level);
        if (primitiveIndex < primitives.size()) {
            primitives[primitiveIndex].setBlendOrder(order);
        }
    }
}

AttributeBitset FRenderableManager::getEnabledAttributesAt(
        Instance instance, uint8_t level, size_t primitiveIndex) const noexcept {
    if (instance) {
        Slice<FRenderPrimitive> const& primitives = getRenderPrimitives(instance, level);
        if (primitiveIndex < primitives.size()) {
            return primitives[primitiveIndex].getEnabledAttributes();
        }
    }
    return AttributeBitset{};
}

void FRenderableManager::setGeometryAt(Instance instance, uint8_t level, size_t primitiveIndex,
        PrimitiveType type, FVertexBuffer* vertices, FIndexBuffer* indices,
        size_t offset, size_t count) noexcept {
    if (instance) {
        Slice<FRenderPrimitive>& primitives = getRenderPrimitives(instance, level);
        if (primitiveIndex < primitives.size()) {
            primitives[primitiveIndex].set(mEngine, type, vertices, indices, offset,
                    0, vertices->getVertexCount() - 1, count);
        }
    }
}

void FRenderableManager::setGeometryAt(Instance instance, uint8_t level, size_t primitiveIndex,
        PrimitiveType type, size_t offset, size_t count) noexcept {
    if (instance) {
        Slice<FRenderPrimitive>& primitives = getRenderPrimitives(instance, level);
        if (primitiveIndex < primitives.size()) {
            primitives[primitiveIndex].set(mEngine, type, offset, 0, 0, count);
        }
    }
}

void FRenderableManager::setBones(Instance ci,
        Bone const* UTILS_RESTRICT transforms, size_t boneCount, size_t offset) noexcept {
    if (ci) {
        Bones& bones = mManager[ci].bones;

        ASSERT_PRECONDITION(!bones.skinningBufferMode,
                "Disable skinning buffer mode to use this API");

        assert_invariant(bones.handle && offset + boneCount <= bones.count);
        if (bones.handle) {
            boneCount = std::min(boneCount, bones.count - offset);
            FSkinningBuffer::setBones(mEngine, bones.handle, transforms, boneCount, offset);
        }
    }
}

void FRenderableManager::setBones(Instance ci,
        mat4f const* UTILS_RESTRICT transforms, size_t boneCount, size_t offset) noexcept {
    if (ci) {
        Bones& bones = mManager[ci].bones;

        ASSERT_PRECONDITION(!bones.skinningBufferMode,
                "Disable skinning buffer mode to use this API");

        assert_invariant(bones.handle && offset + boneCount <= bones.count);
        if (bones.handle) {
            boneCount = std::min(boneCount, bones.count - offset);
            FSkinningBuffer::setBones(mEngine, bones.handle, transforms, boneCount, offset);
        }
    }
}

void FRenderableManager::setSkinningBuffer(FRenderableManager::Instance ci,
        FSkinningBuffer* skinningBuffer, size_t count, size_t offset) noexcept {

    Bones& bones = mManager[ci].bones;

    ASSERT_PRECONDITION(bones.skinningBufferMode,
            "Enable skinning buffer mode to use this API");

    ASSERT_PRECONDITION(
            count + offset < skinningBuffer->getBoneCount(),
            "SkinningBuffer overflow (size=%u, count=%u, offset=%u)",
            skinningBuffer->getBoneCount(), count, offset);

    // According to the OpenGL ES 3.2 specification in 7.6.3 Uniform
    // Buffer Object Bindings:
    //
    //     the uniform block must be populated with a buffer object with a size no smaller
    //     than the minimum required size of the uniform block (the value of
    //     UNIFORM_BLOCK_DATA_SIZE).
    //
    // So we round-up the "window" of bones set to match UNIFORM_BLOCK_DATA_SIZE, the SkinningBuffer
    // should always contain enough date for this to work.

    count = FSkinningBuffer::getPhysicalBoneCount(count);
    assert_invariant(count + offset < skinningBuffer->getBoneCount());

    bones.handle = skinningBuffer->getHwHandle();
    bones.count = uint16_t(count);
    bones.offset = uint16_t(offset);
}

void FRenderableManager::setMorphWeights(Instance ci, const float4& weights) noexcept {
    if (ci) {
        mManager[ci].morphWeights = weights;
    }
}

void FRenderableManager::setLightChannel(Instance ci, unsigned int channel, bool enable) noexcept {
    if (ci) {
        if (channel < 8) {
            const uint8_t mask = 1u << channel;
            mManager[ci].channels &= ~mask;
            mManager[ci].channels |= enable ? mask : 0u;
        }
    }
}

bool FRenderableManager::getLightChannel(Instance ci, unsigned int channel) const noexcept {
    if (ci) {
        if (channel < 8) {
            const uint8_t mask = 1u << channel;
            return bool(mManager[ci].channels & mask);
        }
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// Trampoline calling into private implementation
// ------------------------------------------------------------------------------------------------

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
        RenderableManager::Bone const* transforms, size_t boneCount, size_t offset) noexcept {
    upcast(this)->setBones(instance, transforms, boneCount, offset);
}

void RenderableManager::setBones(Instance instance,
        mat4f const* transforms, size_t boneCount, size_t offset) noexcept {
    upcast(this)->setBones(instance, transforms, boneCount, offset);
}

void RenderableManager::setMorphWeights(Instance instance, float4 const& weights) noexcept {
    upcast(this)->setMorphWeights(instance, weights);
}

void RenderableManager::setSkinningBuffer(Instance instance,
        SkinningBuffer* skinningBuffer, size_t count, size_t offset) noexcept {
    upcast(this)->setSkinningBuffer(instance, upcast(skinningBuffer), count, offset);
}

void RenderableManager::setLightChannel(Instance instance, unsigned int channel, bool enable) noexcept {
    upcast(this)->setLightChannel(instance, channel, enable);
}

bool RenderableManager::getLightChannel(Instance instance, unsigned int channel) const noexcept {
    return upcast(this)->getLightChannel(instance, channel);
}

} // namespace filament
