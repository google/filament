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

#include "RenderPrimitive.h"

#include "components/RenderableManager.h"

#include "details/Engine.h"
#include "details/VertexBuffer.h"
#include "details/IndexBuffer.h"
#include "details/InstanceBuffer.h"
#include "details/Material.h"

#include "filament/RenderableManager.h"


#include <backend/DriverEnums.h>

#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/debug.h>
#include <unordered_map>

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
    uint8_t mCommandChannel = RenderableManager::Builder::DEFAULT_CHANNEL;
    uint8_t mLightChannels = 1;
    uint16_t mInstanceCount = 1;
    bool mCulling : 1;
    bool mCastShadows : 1;
    bool mReceiveShadows : 1;
    bool mScreenSpaceContactShadows : 1;
    bool mSkinningBufferMode : 1;
    bool mFogEnabled : 1;
    RenderableManager::Builder::GeometryType mGeometryType : 2;
    size_t mSkinningBoneCount = 0;
    size_t mMorphTargetCount = 0;
    Bone const* mUserBones = nullptr;
    mat4f const* mUserBoneMatrices = nullptr;
    FSkinningBuffer* mSkinningBuffer = nullptr;
    FInstanceBuffer* mInstanceBuffer = nullptr;
    uint32_t mSkinningBufferOffset = 0;
    utils::FixedCapacityVector<math::float2> mBoneIndicesAndWeights;
    size_t mBoneIndicesAndWeightsCount = 0;

    // bone indices and weights defined for primitive index
    std::unordered_map<size_t, utils::FixedCapacityVector<
        utils::FixedCapacityVector<math::float2>>> mBonePairs;

    explicit BuilderDetails(size_t count)
            : mEntries(count), mCulling(true), mCastShadows(false),
              mReceiveShadows(true), mScreenSpaceContactShadows(false),
              mSkinningBufferMode(false), mFogEnabled(true),
              mGeometryType(RenderableManager::Builder::GeometryType::DYNAMIC),
              mBonePairs() {
    }
    // this is only needed for the explicit instantiation below
    BuilderDetails() = default;

    void processBoneIndicesAndWights(Engine& engine, utils::Entity entity);

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
        size_t offset, UTILS_UNUSED size_t minIndex, UTILS_UNUSED size_t maxIndex, size_t count) noexcept {
    std::vector<Entry>& entries = mImpl->mEntries;
    if (index < entries.size()) {
        entries[index].vertices = vertices;
        entries[index].indices = indices;
        entries[index].offset = offset;
        entries[index].count = count;
        entries[index].type = type;
    }
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::geometryType(GeometryType type) noexcept {
    mImpl->mGeometryType = type;
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

RenderableManager::Builder& RenderableManager::Builder::channel(uint8_t channel) noexcept {
    mImpl->mCommandChannel = std::min(channel, uint8_t(0x3));
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::culling(bool enable) noexcept {
    mImpl->mCulling = enable;
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::lightChannel(unsigned int channel, bool enable) noexcept {
    if (channel < 8) {
        const uint8_t mask = 1u << channel;
        mImpl->mLightChannels &= ~mask;
        mImpl->mLightChannels |= enable ? mask : 0u;
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
    mImpl->mSkinningBuffer = downcast(skinningBuffer);
    mImpl->mSkinningBoneCount = count;
    mImpl->mSkinningBufferOffset = offset;
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::enableSkinningBuffers(bool enabled) noexcept {
    mImpl->mSkinningBufferMode = enabled;
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::boneIndicesAndWeights(size_t primitiveIndex,
               math::float2 const* indicesAndWeights, size_t count, size_t bonesPerVertex) noexcept {
    size_t const vertexCount = count / bonesPerVertex;
    utils::FixedCapacityVector<utils::FixedCapacityVector<filament::math::float2>> bonePairs(vertexCount);
    for (size_t iVertex = 0; iVertex < vertexCount; iVertex++) {
        utils::FixedCapacityVector<float2> vertexData(bonesPerVertex);
        std::copy_n(indicesAndWeights + iVertex * bonesPerVertex,
                bonesPerVertex, vertexData.data());
        bonePairs[iVertex] = std::move(vertexData);
    }
    return boneIndicesAndWeights(primitiveIndex, bonePairs);
}

RenderableManager::Builder& RenderableManager::Builder::boneIndicesAndWeights(size_t primitiveIndex,
        utils::FixedCapacityVector<
            utils::FixedCapacityVector<math::float2>> indicesAndWeightsVector) noexcept {
    mImpl->mBonePairs[primitiveIndex] = std::move(indicesAndWeightsVector);
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::fog(bool enabled) noexcept {
    mImpl->mFogEnabled = enabled;
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::morphing(size_t targetCount) noexcept {
    mImpl->mMorphTargetCount = targetCount;
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::morphing(uint8_t, size_t primitiveIndex,
        MorphTargetBuffer* morphTargetBuffer, size_t offset, size_t count) noexcept {
    std::vector<Entry>& entries = mImpl->mEntries;
    if (primitiveIndex < entries.size()) {
        auto& morphing = entries[primitiveIndex].morphing;
        morphing.buffer = morphTargetBuffer;
        morphing.offset = offset;
        morphing.count = count;
    }
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::blendOrder(
        size_t index, uint16_t blendOrder) noexcept {
    if (index < mImpl->mEntries.size()) {
        mImpl->mEntries[index].blendOrder = blendOrder;
    }
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::globalBlendOrderEnabled(
        size_t index, bool enabled) noexcept {
    if (index < mImpl->mEntries.size()) {
        mImpl->mEntries[index].globalBlendOrderEnabled = enabled;
    }
    return *this;
}

UTILS_NOINLINE
void RenderableManager::BuilderDetails::processBoneIndicesAndWights(Engine& engine, Entity entity) {
    size_t maxPairsCount = 0; //size of texture, number of bone pairs
    size_t maxPairsCountPerVertex = 0; //maximum of number of bone per vertex

    for (auto& bonePair: mBonePairs) {
        auto primitiveIndex = bonePair.first;
        auto entries = mEntries;
        ASSERT_PRECONDITION(primitiveIndex < entries.size() && primitiveIndex >= 0,
            "[primitive @ %u] primitiveindex is out of size (%u)", primitiveIndex, entries.size());
        auto entry = mEntries[primitiveIndex];
        auto bonePairsForPrimitive = bonePair.second;
        auto vertexCount = entry.vertices->getVertexCount();
        ASSERT_PRECONDITION(bonePairsForPrimitive.size() == vertexCount,
            "[primitive @ %u] bone indices and weights pairs count (%u) must be equal to vertex count (%u)",
            primitiveIndex, bonePairsForPrimitive.size(), vertexCount);
        auto const& declaredAttributes = downcast(entry.vertices)->getDeclaredAttributes();
        ASSERT_PRECONDITION(declaredAttributes[VertexAttribute::BONE_INDICES]
        || declaredAttributes[VertexAttribute::BONE_WEIGHTS],
            "[entity=%u, primitive @ %u] for advanced skinning set VertexBuffer::Builder::advancedSkinning()",
            entity.getId(), primitiveIndex);
        for (size_t iVertex = 0; iVertex < vertexCount; iVertex++) {
            size_t const bonesPerVertex = bonePairsForPrimitive[iVertex].size();
            maxPairsCount += bonesPerVertex;
            maxPairsCountPerVertex = std::max(bonesPerVertex, maxPairsCountPerVertex);
        }
    }

    size_t pairsCount = 0; // counting of number of pairs stored in texture
    if (maxPairsCount) { // at least one primitive has bone indices and weights
        // final texture data, indices and weights
        mBoneIndicesAndWeights = utils::FixedCapacityVector<float2>(maxPairsCount);
        // temporary indices and weights for one vertex
        auto const tempPairs = std::make_unique<float2[]>(maxPairsCountPerVertex);
        for (auto& bonePair: mBonePairs) {
            auto primitiveIndex = bonePair.first;
            auto bonePairsForPrimitive = bonePair.second;
            if (bonePairsForPrimitive.empty()) {
                continue;
            }
            size_t const vertexCount = mEntries[primitiveIndex].vertices->getVertexCount();
            // temporary indices for one vertex
            auto skinJoints = std::make_unique<uint16_t[]>(4 * vertexCount);
            // temporary weights for one vertex
            auto skinWeights = std::make_unique<float[]>(4 * vertexCount);
            for (size_t iVertex = 0; iVertex < vertexCount; iVertex++) {
                size_t tempPairCount = 0;
                double boneWeightsSum = 0;
                for (size_t k = 0; k < bonePairsForPrimitive[iVertex].size(); k++) {
                    auto boneWeight = bonePairsForPrimitive[iVertex][k][1];
                    auto boneIndex = bonePairsForPrimitive[iVertex][k][0];
                    ASSERT_PRECONDITION(boneWeight >= 0,
                            "[entity=%u, primitive @ %u] bone weight (%f) of vertex=%u is negative ",
                            entity.getId(), primitiveIndex, boneWeight, iVertex);
                    if (boneWeight > 0.0f) {
                        ASSERT_PRECONDITION(boneIndex >= 0,
                            "[entity=%u, primitive @ %u] bone index (%i) of vertex=%u is negative ",
                            entity.getId(), primitiveIndex, (int) boneIndex, iVertex);
                        ASSERT_PRECONDITION(boneIndex < mSkinningBoneCount,
                            "[entity=%u, primitive @ %u] bone index (%i) of vertex=%u is bigger then bone count (%u) ",
                            entity.getId(), primitiveIndex, (int) boneIndex, iVertex, mSkinningBoneCount);
                        boneWeightsSum += boneWeight;
                        tempPairs[tempPairCount][0] = boneIndex;
                        tempPairs[tempPairCount][1] = boneWeight;
                        tempPairCount++;
                    }
                }

                ASSERT_PRECONDITION(boneWeightsSum > 0,
                    "[entity=%u, primitive @ %u] sum of bone weights of vertex=%u is %f, it should be positive.",
                    entity.getId(), primitiveIndex, iVertex, boneWeightsSum);

                // see https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#skinned-mesh-attributes
                double const epsilon = 2e-7 * double(tempPairCount);
                if (abs(boneWeightsSum - 1.0) <= epsilon) {
                    boneWeightsSum = 1.0;
                }
#ifndef NDEBUG
                else {
                    utils::slog.w << "Warning of skinning: [entity=%" << entity.getId()
                        << ", primitive @ %" << primitiveIndex
                        << "] sum of bone weights of vertex=" << iVertex << " is " << boneWeightsSum
                        << ", it should be one. Weights will be normalized." << utils::io::endl;
                }
#endif

                // prepare data for vertex attributes
                auto offset = iVertex * 4;
                // set attributes, indices and weights, for <= 4 pairs
                for (size_t j = 0, c = std::min((int)tempPairCount, 4); j < c; j++) {
                    skinJoints[j + offset] = uint16_t(tempPairs[j][0]);
                    skinWeights[j + offset] = tempPairs[j][1] / float(boneWeightsSum);
                }
                // prepare data for texture
                if (tempPairCount > 4) { // set attributes, indices and weights, for > 4 pairs
                    // number pairs per vertex in texture
                    skinJoints[3 + offset] = (uint16_t)tempPairCount;
                    // negative offset to texture 0..-1, 1..-2
                    skinWeights[3 + offset] = -float(pairsCount + 1);
                    for (size_t j = 3; j < tempPairCount; j++) {
                        mBoneIndicesAndWeights[pairsCount][0] = tempPairs[j][0];
                        mBoneIndicesAndWeights[pairsCount][1] = tempPairs[j][1] / float(boneWeightsSum);
                        pairsCount++;
                    }
                }
            } // for all vertices per primitive
            downcast(mEntries[primitiveIndex].vertices)
                ->updateBoneIndicesAndWeights(downcast(engine),
                                              std::move(skinJoints),
                                              std::move(skinWeights));
        } // for all primitives
    }
    mBoneIndicesAndWeightsCount = pairsCount; // only part of mBoneIndicesAndWeights is used for real data
}

RenderableManager::Builder::Result RenderableManager::Builder::build(Engine& engine, Entity entity) {
    bool isEmpty = true;

    ASSERT_PRECONDITION(mImpl->mSkinningBoneCount <= CONFIG_MAX_BONE_COUNT,
            "bone count > %u", CONFIG_MAX_BONE_COUNT);

    ASSERT_PRECONDITION(mImpl->mInstanceCount <= CONFIG_MAX_INSTANCES || !mImpl->mInstanceBuffer,
            "instance count is %zu, but instance count is limited to CONFIG_MAX_INSTANCES (%zu) "
            "instances when supplying transforms via an InstanceBuffer.",
            mImpl->mInstanceCount,
            CONFIG_MAX_INSTANCES);

    if (mImpl->mGeometryType == GeometryType::STATIC) {
        ASSERT_PRECONDITION(mImpl->mSkinningBoneCount > 0,
                "Skinning can't be used with STATIC geometry");

        ASSERT_PRECONDITION(mImpl->mMorphTargetCount > 0,
                "Morphing can't be used with STATIC geometry");
    }

    if (mImpl->mInstanceBuffer) {
        size_t const bufferInstanceCount = mImpl->mInstanceBuffer->mInstanceCount;
        ASSERT_PRECONDITION(mImpl->mInstanceCount <= bufferInstanceCount,
                "instance count (%zu) must be less than or equal to the InstanceBuffer's instance "
                "count "
                "(%zu).",
                mImpl->mInstanceCount, bufferInstanceCount);
    }

    if (UTILS_LIKELY(mImpl->mSkinningBoneCount || mImpl->mSkinningBufferMode)) {
        mImpl->processBoneIndicesAndWights(engine, entity);
    }

    for (size_t i = 0, c = mImpl->mEntries.size(); i < c; i++) {
        auto& entry = mImpl->mEntries[i];

        // entry.materialInstance must be set to something even if indices/vertices are null
        FMaterial const* material;
        if (!entry.materialInstance) {
            material = downcast(engine.getDefaultMaterial());
            entry.materialInstance = material->getDefaultInstance();
        } else {
            material = downcast(entry.materialInstance->getMaterial());
        }

        // primitives without indices or vertices will be ignored
        if (!entry.indices || !entry.vertices) {
            continue;
        }

        // we want a feature level violation to be a hard error (exception if enabled, or crash)
        ASSERT_PRECONDITION(downcast(engine).hasFeatureLevel(material->getFeatureLevel()),
                "Material \"%s\" has feature level %u which is not supported by this Engine",
                material->getName().c_str_safe(), (uint8_t)material->getFeatureLevel());

        // reject invalid geometry parameters
        ASSERT_PRECONDITION(entry.offset + entry.count <= entry.indices->getIndexCount(),
                "[entity=%u, primitive @ %u] offset (%u) + count (%u) > indexCount (%u)",
                entity.getId(), i,
                entry.offset, entry.count, entry.indices->getIndexCount());

        // this can't be an error because (1) those values are not immutable, so the caller
        // could fix later, and (2) the material's shader will work (i.e. compile), and
        // use the default values for this attribute, which maybe be acceptable.
        AttributeBitset const declared = downcast(entry.vertices)->getDeclaredAttributes();
        AttributeBitset const required = material->getRequiredAttributes();
        if ((declared & required) != required) {
            slog.w << "[entity=" << entity.getId() << ", primitive @ " << i
                   << "] missing required attributes ("
                   << required << "), declared=" << declared << io::endl;
        }

        // we have at least one valid primitive
        isEmpty = false;
    }

    ASSERT_PRECONDITION(
            !mImpl->mAABB.isEmpty() ||
            (!mImpl->mCulling && (!(mImpl->mReceiveShadows || mImpl->mCastShadows)) ||
             isEmpty),
            "[entity=%u] AABB can't be empty, unless culling is disabled and "
                    "the object is not a shadow caster/receiver", entity.getId());

    downcast(engine).createRenderable(*this, entity);
    return Success;
}

RenderableManager::Builder& RenderableManager::Builder::instances(size_t instanceCount) noexcept {
    mImpl->mInstanceCount = clamp((unsigned int)instanceCount, 1u, 32767u);
    return *this;
}

RenderableManager::Builder& RenderableManager::Builder::instances(
        size_t instanceCount, InstanceBuffer* instanceBuffer) noexcept {
    mImpl->mInstanceCount = clamp(instanceCount, (size_t)1, CONFIG_MAX_INSTANCES);
    mImpl->mInstanceBuffer = downcast(instanceBuffer);
    return *this;
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
    Instance const ci = manager.addComponent(entity);
    assert_invariant(ci);

    if (ci) {
        // create and initialize all needed RenderPrimitives
        using size_type = Slice<FRenderPrimitive>::size_type;
        Builder::Entry const * const entries = builder->mEntries.data();
        const size_t entryCount = builder->mEntries.size();
        FRenderPrimitive* rp = new FRenderPrimitive[entryCount];
        auto& factory = mHwRenderPrimitiveFactory;
        for (size_t i = 0; i < entryCount; ++i) {
            rp[i].init(factory, driver, entries[i]);
        }
        setPrimitives(ci, { rp, size_type(entryCount) });

        setAxisAlignedBoundingBox(ci, builder->mAABB);
        setLayerMask(ci, builder->mLayerMask);
        setPriority(ci, builder->mPriority);
        setChannel(ci, builder->mCommandChannel);
        setCastShadows(ci, builder->mCastShadows);
        setReceiveShadows(ci, builder->mReceiveShadows);
        setScreenSpaceContactShadows(ci, builder->mScreenSpaceContactShadows);
        setCulling(ci, builder->mCulling);
        setSkinning(ci, false);
        setMorphing(ci, builder->mMorphTargetCount);
        setFogEnabled(ci, builder->mFogEnabled);
        // do this after calling setAxisAlignedBoundingBox
        static_cast<Visibility&>(mManager[ci].visibility).geometryType = builder->mGeometryType;
        mManager[ci].channels = builder->mLightChannels;

        InstancesInfo& instances = manager[ci].instances;
        instances.count = builder->mInstanceCount;
        instances.buffer = builder->mInstanceBuffer;
        if (instances.buffer) {
            // Allocate our instance buffer for this Renderable. We always allocate a size to match
            // PerRenderableUib, regardless of the number of instances. This is because the buffer
            // will get bound to the PER_RENDERABLE UBO, and we can't bind a buffer smaller than the
            // full size of the UBO.
            instances.handle = driver.createBufferObject(sizeof(PerRenderableUib),
                    BufferObjectBinding::UNIFORM, backend::BufferUsage::DYNAMIC);
        }

        const uint32_t boneCount = builder->mSkinningBoneCount;
        const uint32_t targetCount = builder->mMorphTargetCount;
        if (builder->mSkinningBufferMode) {
            if (builder->mSkinningBuffer) {
                setSkinning(ci, boneCount > 0);
                Bones& bones = manager[ci].bones;
                bones = Bones{
                        .handle = builder->mSkinningBuffer->getHwHandle(),
                        .count = (uint16_t)boneCount,
                        .offset = (uint16_t)builder->mSkinningBufferOffset,
                        .skinningBufferMode = true };
            }
        } else {
            if (UTILS_UNLIKELY(boneCount > 0 || targetCount > 0)) {
                setSkinning(ci, boneCount > 0);
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
                                sizeof(PerRenderableBoneUib),
                                BufferObjectBinding::UNIFORM,
                                backend::BufferUsage::DYNAMIC),
                        .count = (uint16_t)boneCount,
                        .offset = 0,
                        .skinningBufferMode = false };

                if (boneCount) {
                    if (builder->mUserBones) {
                        FSkinningBuffer::setBones(mEngine, bones.handle,
                                builder->mUserBones, boneCount, 0);
                    } else if (builder->mUserBoneMatrices) {
                        FSkinningBuffer::setBones(mEngine, bones.handle,
                                builder->mUserBoneMatrices, boneCount, 0);
                    } else {
                        // initialize the bones to identity
                        auto* out = driver.allocatePod<PerRenderableBoneUib::BoneData>(boneCount);
                        std::uninitialized_fill_n(out, boneCount, FSkinningBuffer::makeBone({}));
                        driver.updateBufferObject(bones.handle, {
                                out, boneCount * sizeof(PerRenderableBoneUib::BoneData) }, 0);
                    }
                }
                else {
                    // When boneCount is 0, do an initialization for the bones uniform array to
                    // avoid crash on adreno gpu.
                    if (UTILS_UNLIKELY(driver.isWorkaroundNeeded(
                            Workaround::ADRENO_UNIFORM_ARRAY_CRASH))) {
                        auto *initBones = driver.allocatePod<PerRenderableBoneUib::BoneData>(1);
                        std::uninitialized_fill_n(initBones, 1, FSkinningBuffer::makeBone({}));
                        driver.updateBufferObject(bones.handle, {
                                initBones, sizeof(PerRenderableBoneUib::BoneData) }, 0);
                    }
                }
            }
        }

        // Create and initialize all needed MorphTargets.
        // It's required to avoid branches in hot loops.
        MorphTargets* const morphTargets = new MorphTargets[entryCount];
        std::generate_n(morphTargets, entryCount,
                [dummy = mEngine.getDummyMorphTargetBuffer()]() -> MorphTargets {
                    return { dummy, 0, 0 };
                });

        mManager[ci].morphTargets = { morphTargets, size_type(entryCount) };

        // Always create skinning and morphing resources if one of them is enabled because
        // the shader always handles both. See Variant::SKINNING_OR_MORPHING.
        if (UTILS_UNLIKELY(boneCount > 0 || targetCount > 0)) {

            auto [sampler, texture] = FSkinningBuffer::createIndicesAndWeightsHandle(
                    downcast(engine), builder->mBoneIndicesAndWeightsCount);
            if (builder->mBoneIndicesAndWeightsCount > 0) {
                FSkinningBuffer::setIndicesAndWeightsData(downcast(engine), texture,
                        builder->mBoneIndicesAndWeights, builder->mBoneIndicesAndWeightsCount);
            }
            Bones& bones = manager[ci].bones;
            bones.handleSamplerGroup = sampler;
            bones.handleTexture = texture;

            // Instead of using a UBO per primitive, we could also have a single UBO for all primitives
            // and use bindUniformBufferRange which might be more efficient.
            MorphWeights& morphWeights = manager[ci].morphWeights;
            morphWeights = MorphWeights {
                .handle = driver.createBufferObject(
                        sizeof(PerRenderableMorphingUib),
                        BufferObjectBinding::UNIFORM,
                        backend::BufferUsage::DYNAMIC),
                .count = targetCount };

            for (size_t i = 0; i < entryCount; ++i) {
                const auto& morphing = builder->mEntries[i].morphing;
                if (!morphing.buffer) {
                    continue;
                }
                morphTargets[i] = { downcast(morphing.buffer), (uint32_t)morphing.offset,
                                    (uint32_t)morphing.count };
            }
            
            // When targetCount equal 0, boneCount>0 in this case, do an initialization for the
            // morphWeights uniform array to avoid crash on adreno gpu.
            if (UTILS_UNLIKELY(targetCount == 0 &&
                    driver.isWorkaroundNeeded(Workaround::ADRENO_UNIFORM_ARRAY_CRASH))) {
                float initWeights[1] = { 0 };
                setMorphWeights(ci, initWeights, 1, 0);
            }
        }
    }
    engine.flushIfNeeded();
}

// this destroys a single component from an entity
void FRenderableManager::destroy(utils::Entity e) noexcept {
    Instance const ci = getInstance(e);
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
            Instance const ci = manager.end() - 1;
            destroyComponent(ci);
            manager.removeComponent(manager.getEntity(ci));
        }
    }
    mHwRenderPrimitiveFactory.terminate(mEngine.getDriverApi());
}

void FRenderableManager::gc(utils::EntityManager& em) noexcept {
    mManager.gc(em, [this](Entity e) {
        destroy(e);
    });
}

// This is basically a Renderable's destructor.
void FRenderableManager::destroyComponent(Instance ci) noexcept {
    auto& manager = mManager;
    FEngine& engine = mEngine;

    FEngine::DriverApi& driver = engine.getDriverApi();

    // See create(RenderableManager::Builder&, Entity)
    destroyComponentPrimitives(mHwRenderPrimitiveFactory, driver, manager[ci].primitives);
    destroyComponentMorphTargets(engine, manager[ci].morphTargets);

    // destroy the bones structures if any
    Bones const& bones = manager[ci].bones;
    if (bones.handle && !bones.skinningBufferMode) {
        driver.destroyBufferObject(bones.handle);
    }
    if (bones.handleSamplerGroup){
        driver.destroySamplerGroup(bones.handleSamplerGroup);
        driver.destroyTexture(bones.handleTexture);
    }

    // destroy the weights structures if any
    MorphWeights const& morphWeights = manager[ci].morphWeights;
    if (morphWeights.handle) {
        driver.destroyBufferObject(morphWeights.handle);
    }

    InstancesInfo const& instances = manager[ci].instances;
    if (instances.handle) {
        driver.destroyBufferObject(instances.handle);
    }
}

void FRenderableManager::destroyComponentPrimitives(
        HwRenderPrimitiveFactory& factory, backend::DriverApi& driver,
        Slice<FRenderPrimitive>& primitives) noexcept {
    for (auto& primitive : primitives) {
        primitive.terminate(factory, driver);
    }
    delete[] primitives.data();
}

void FRenderableManager::destroyComponentMorphTargets(FEngine&,
        utils::Slice<MorphTargets>& morphTargets) noexcept {
    delete[] morphTargets.data();
}

void FRenderableManager::setMaterialInstanceAt(Instance instance, uint8_t level,
        size_t primitiveIndex, FMaterialInstance const* mi) {
    if (instance) {
        Slice<FRenderPrimitive>& primitives = getRenderPrimitives(instance, level);
        if (primitiveIndex < primitives.size()) {
            assert_invariant(mi);
            FMaterial const* material = mi->getMaterial();

            // we want a feature level violation to be a hard error (exception if enabled, or crash)
            ASSERT_PRECONDITION(mEngine.hasFeatureLevel(material->getFeatureLevel()),
                    "Material \"%s\" has feature level %u which is not supported by this Engine",
                    material->getName().c_str_safe(), (uint8_t)material->getFeatureLevel());

            primitives[primitiveIndex].setMaterialInstance(mi);
            AttributeBitset const required = material->getRequiredAttributes();
            AttributeBitset const declared = primitives[primitiveIndex].getEnabledAttributes();
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

void FRenderableManager::setGlobalBlendOrderEnabledAt(Instance instance, uint8_t level,
        size_t primitiveIndex, bool enabled) noexcept {
    if (instance) {
        Slice<FRenderPrimitive>& primitives = getRenderPrimitives(instance, level);
        if (primitiveIndex < primitives.size()) {
            primitives[primitiveIndex].setGlobalBlendOrderEnabled(enabled);
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
            primitives[primitiveIndex].set(mHwRenderPrimitiveFactory, mEngine.getDriverApi(),
                    type, vertices, indices, offset, count);
        }
    }
}

void FRenderableManager::setBones(Instance ci,
        Bone const* UTILS_RESTRICT transforms, size_t boneCount, size_t offset) {
    if (ci) {
        Bones const& bones = mManager[ci].bones;

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
        mat4f const* UTILS_RESTRICT transforms, size_t boneCount, size_t offset) {
    if (ci) {
        Bones const& bones = mManager[ci].bones;

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
        FSkinningBuffer* skinningBuffer, size_t count, size_t offset) {

    Bones& bones = mManager[ci].bones;

    ASSERT_PRECONDITION(bones.skinningBufferMode,
            "Enable skinning buffer mode to use this API");

    ASSERT_PRECONDITION(
            count <= CONFIG_MAX_BONE_COUNT,
            "SkinningBuffer larger than 256 (count=%u)", count);

    // According to the OpenGL ES 3.2 specification in 7.6.3 Uniform
    // Buffer Object Bindings:
    //
    //     the uniform block must be populated with a buffer object with a size no smaller
    //     than the minimum required size of the uniform block (the value of
    //     UNIFORM_BLOCK_DATA_SIZE).
    //

    count = CONFIG_MAX_BONE_COUNT;

    ASSERT_PRECONDITION(
            count + offset <= skinningBuffer->getBoneCount(),
            "SkinningBuffer overflow (size=%u, count=%u, offset=%u)",
            skinningBuffer->getBoneCount(), count, offset);

    bones.handle = skinningBuffer->getHwHandle();
    bones.count = uint16_t(count);
    bones.offset = uint16_t(offset);
}

static void updateMorphWeights(FEngine& engine, backend::Handle<backend::HwBufferObject> handle,
        float const* weights, size_t count, size_t offset) noexcept {
    auto& driver = engine.getDriverApi();
    auto size = sizeof(float4) * count;
    auto* UTILS_RESTRICT out = (float4*)driver.allocate(size);
    std::transform(weights, weights + count, out,
            [](float value) { return float4(value, 0, 0, 0); });
    driver.updateBufferObject(handle, { out, size }, sizeof(float4) * offset);
}

void FRenderableManager::setMorphWeights(Instance instance, float const* weights,
        size_t count, size_t offset) {
    if (instance) {
        ASSERT_PRECONDITION(count + offset <= CONFIG_MAX_MORPH_TARGET_COUNT,
                "Only %d morph targets are supported (count=%d, offset=%d)",
                CONFIG_MAX_MORPH_TARGET_COUNT, count, offset);

        MorphWeights const& morphWeights = mManager[instance].morphWeights;
        if (morphWeights.handle) {
            updateMorphWeights(mEngine, morphWeights.handle, weights, count, offset);
        }
    }
}

void FRenderableManager::setMorphTargetBufferAt(Instance instance, uint8_t level,
        size_t primitiveIndex, FMorphTargetBuffer* morphTargetBuffer, size_t offset, size_t count) {
    assert_invariant(offset == 0 && "Offset not yet supported.");
    assert_invariant(count == morphTargetBuffer->getVertexCount() && "Count not yet supported.");
    if (instance) {
        assert_invariant(morphTargetBuffer);

        MorphWeights const& morphWeights = mManager[instance].morphWeights;
        ASSERT_PRECONDITION(morphWeights.count == morphTargetBuffer->getCount(),
                "Only %d morph targets can be set (count=%d)",
                morphWeights.count, morphTargetBuffer->getCount());

        Slice<MorphTargets>& morphTargets = getMorphTargets(instance, level);
        if (primitiveIndex < morphTargets.size()) {
            morphTargets[primitiveIndex] = { morphTargetBuffer, (uint32_t)offset,
                                             (uint32_t)count };
        }
    }
}

MorphTargetBuffer* FRenderableManager::getMorphTargetBufferAt(Instance instance, uint8_t level,
        size_t primitiveIndex) const noexcept {
    if (instance) {
        const Slice<MorphTargets>& morphTargets = getMorphTargets(instance, level);
        if (primitiveIndex < morphTargets.size()) {
            return morphTargets[primitiveIndex].buffer;
        }
    }
    return nullptr;
}

size_t FRenderableManager::getMorphTargetCount(Instance instance) const noexcept {
    if (instance) {
        const MorphWeights& morphWeights = mManager[instance].morphWeights;
        return morphWeights.count;
    }
    return 0;
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

size_t FRenderableManager::getPrimitiveCount(Instance instance, uint8_t level) const noexcept {
    return getRenderPrimitives(instance, level).size();
}

} // namespace filament
