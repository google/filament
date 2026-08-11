/*
 * Copyright (C) 2026 The Android Open Source Project
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

// Streams a CPU-deformed sphere into Filament every XR frame. The material only shades the
// resulting geometry; all positions and tangent frames are generated on the CPU.

#include "helloxr_features.h"

#if !defined(__ANDROID__)
#include "generated/resources/resources.h"
#endif

#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/norm.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

using namespace filament;
using namespace filament::math;

namespace helloxr {
namespace {

constexpr uint32_t kStackCount = 40;
constexpr uint32_t kSliceCount = 64;
constexpr uint32_t kVerticesPerStack = kSliceCount + 1;
constexpr uint32_t kVertexCount = (kStackCount + 1) * kVerticesPerStack;
constexpr uint32_t kIndexCount = kStackCount * kSliceCount * 6;
constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 2.0f * kPi;
const float3 kSpherePosition = { 1.25f, 0.15f, -2.0f };

struct StreamVertex {
    float3 position;
    short4 tangent;
};

size_t vertexIndex(uint32_t stack, uint32_t slice) {
    return size_t(stack) * kVerticesPerStack + slice;
}

float3 normalizedOr(float3 value, float3 fallback) {
    float const lengthSquared = dot(value, value);
    return lengthSquared > 1.0e-10f ? value * (1.0f / std::sqrt(lengthSquared)) : fallback;
}

float3 spectrum(float phase) {
    return {
        0.5f + 0.5f * std::sin(phase),
        0.5f + 0.5f * std::sin(phase + 2.0943951f),
        0.5f + 0.5f * std::sin(phase + 4.1887902f),
    };
}

class VertexStreaming final : public Feature {
public:
    char const* name() const override { return "CPU vertex streaming"; }

    std::vector<char const*> requiredExtensions() const override { return {}; }

    bool initialize(FeatureContext const& context) override {
        mContext = context;
        buildTopology();
        if (!loadAuraMaterial()) {
            return false;
        }

        Engine& engine = *mContext.engine;
        mVertexBuffer = VertexBuffer::Builder()
                                .vertexCount(kVertexCount)
                                .bufferCount(1)
                                .attribute(VertexAttribute::POSITION, 0,
                                        VertexBuffer::AttributeType::FLOAT3,
                                        offsetof(StreamVertex, position), sizeof(StreamVertex))
                                .attribute(VertexAttribute::TANGENTS, 0,
                                        VertexBuffer::AttributeType::SHORT4,
                                        offsetof(StreamVertex, tangent), sizeof(StreamVertex))
                                .normalized(VertexAttribute::TANGENTS)
                                .build(engine);

        mIndexBuffer = IndexBuffer::Builder()
                               .indexCount(kIndexCount)
                               .bufferType(IndexBuffer::IndexType::USHORT)
                               .build(engine);
        auto* indices = new uint16_t[mIndices.size()];
        for (size_t index = 0; index < mIndices.size(); ++index) {
            indices[index] = mIndices[index];
        }
        mIndexBuffer->setBuffer(engine,
                { indices, mIndices.size() * sizeof(uint16_t), releaseIndices });

        mCoreMaterialInstance = mContext.material->createInstance();
        mCoreMaterialInstance->setParameter("metallic", 0.85f);
        mCoreMaterialInstance->setParameter("roughness", 0.18f);
        mCoreMaterialInstance->setParameter("reflectance", 0.75f);

        mAuraMaterialInstance = mAuraMaterial->createInstance();
        mAuraMaterialInstance->setParameter("fillOpacity", 0.08f);
        mAuraMaterialInstance->setParameter("edgeOpacity", 0.85f);
        mAuraMaterialInstance->setParameter("edgePower", 2.2f);
        mAuraMaterialInstance->setParameter("edgeWidth", 0.45f);

        auto& entityManager = utils::EntityManager::get();
        mCore = entityManager.create();
        RenderableManager::Builder(1)
                .boundingBox({ { 0.0f, 0.0f, 0.0f }, { 0.55f, 0.55f, 0.55f } })
                .material(0, mCoreMaterialInstance)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, mVertexBuffer,
                        mIndexBuffer, 0, kIndexCount)
                .castShadows(false)
                .receiveShadows(false)
                .build(engine, mCore);

        mAura = entityManager.create();
        RenderableManager::Builder(1)
                .boundingBox({ { 0.0f, 0.0f, 0.0f }, { 0.58f, 0.58f, 0.58f } })
                .material(0, mAuraMaterialInstance)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, mVertexBuffer,
                        mIndexBuffer, 0, kIndexCount)
                .culling(false)
                .castShadows(false)
                .receiveShadows(false)
                .build(engine, mAura);

        mContext.scene->addEntity(mCore);
        mContext.scene->addEntity(mAura);
        streamVertices(0.0f);
        updateAppearance(0.0f);
        updateTransforms(0.0f);
        XRLOG("vertex streaming: %u CPU vertices, %u triangles", kVertexCount,
                kIndexCount / 3);
        return true;
    }

    void update(XrTime displayTime) override {
        if (mStartTime == 0) {
            mStartTime = displayTime;
        }
        float const seconds = float(double(displayTime - mStartTime) * 1.0e-9);
        streamVertices(seconds);
        updateAppearance(seconds);
        updateTransforms(seconds);
    }

    void terminate() override {
        auto& entityManager = utils::EntityManager::get();
        if (!mCore.isNull()) {
            mContext.scene->remove(mCore);
            mContext.engine->destroy(mCore);
            entityManager.destroy(mCore);
            mCore = {};
        }
        if (!mAura.isNull()) {
            mContext.scene->remove(mAura);
            mContext.engine->destroy(mAura);
            entityManager.destroy(mAura);
            mAura = {};
        }
        if (mVertexBuffer != nullptr) {
            mContext.engine->destroy(mVertexBuffer);
            mVertexBuffer = nullptr;
        }
        if (mIndexBuffer != nullptr) {
            mContext.engine->destroy(mIndexBuffer);
            mIndexBuffer = nullptr;
        }
        if (mCoreMaterialInstance != nullptr) {
            mContext.engine->destroy(mCoreMaterialInstance);
            mCoreMaterialInstance = nullptr;
        }
        if (mAuraMaterialInstance != nullptr) {
            mContext.engine->destroy(mAuraMaterialInstance);
            mAuraMaterialInstance = nullptr;
        }
        if (mAuraMaterial != nullptr) {
            mContext.engine->destroy(mAuraMaterial);
            mAuraMaterial = nullptr;
        }
    }

private:
    static void releaseIndices(void* buffer, size_t, void*) {
        delete[] static_cast<uint16_t*>(buffer);
    }

    static void releaseVertices(void* buffer, size_t, void*) {
        delete[] static_cast<StreamVertex*>(buffer);
    }

    bool loadAuraMaterial() {
#if defined(__ANDROID__)
        std::vector<uint8_t> package;
        if (!readAsset("xrhand.filamat", &package)) {
            XRLOG("vertex streaming: xrhand.filamat is missing from the assets");
            return false;
        }
        mAuraMaterial = Material::Builder()
                                .package(package.data(), package.size())
                                .build(*mContext.engine);
#else
        mAuraMaterial = Material::Builder()
                                .package(RESOURCES_XRHAND_DATA, RESOURCES_XRHAND_SIZE)
                                .build(*mContext.engine);
#endif
        return mAuraMaterial != nullptr;
    }

    void buildTopology() {
        mDirections.resize(kVertexCount);
        mPositions.resize(kVertexCount);
        for (uint32_t stack = 0; stack <= kStackCount; ++stack) {
            float const polar = kPi * float(stack) / float(kStackCount);
            float const sinPolar = std::sin(polar);
            float const cosPolar = std::cos(polar);
            for (uint32_t slice = 0; slice <= kSliceCount; ++slice) {
                float const azimuth = kTwoPi * float(slice) / float(kSliceCount);
                mDirections[vertexIndex(stack, slice)] = {
                    sinPolar * std::cos(azimuth),
                    cosPolar,
                    sinPolar * std::sin(azimuth),
                };
            }
        }

        mIndices.reserve(kIndexCount);
        for (uint32_t stack = 0; stack < kStackCount; ++stack) {
            for (uint32_t slice = 0; slice < kSliceCount; ++slice) {
                uint16_t const upperLeft = uint16_t(vertexIndex(stack, slice));
                uint16_t const upperRight = uint16_t(vertexIndex(stack, slice + 1));
                uint16_t const lowerLeft = uint16_t(vertexIndex(stack + 1, slice));
                uint16_t const lowerRight = uint16_t(vertexIndex(stack + 1, slice + 1));
                mIndices.insert(mIndices.end(), {
                    upperLeft, upperRight, lowerLeft,
                    upperRight, lowerRight, lowerLeft,
                });
            }
        }
    }

    void streamVertices(float seconds) {
        float const globalPulse = 0.042f * std::sin(seconds * 2.25f);
        for (uint32_t stack = 0; stack <= kStackCount; ++stack) {
            float const polar = kPi * float(stack) / float(kStackCount);
            float const envelope = std::sin(polar) * std::sin(polar);
            for (uint32_t slice = 0; slice <= kSliceCount; ++slice) {
                float const azimuth = kTwoPi * float(slice) / float(kSliceCount);
                float const travelingWave = std::sin(11.0f * polar - 3.8f * seconds +
                        1.4f * std::sin(3.0f * azimuth + 0.7f * seconds));
                float const crossWave = std::sin(7.0f * azimuth + 2.1f * seconds) *
                                        std::sin(5.0f * polar - 1.3f * seconds);
                float const radius = 0.40f + globalPulse +
                                     envelope * (0.034f * travelingWave + 0.018f * crossWave);
                size_t const index = vertexIndex(stack, slice);
                mPositions[index] = mDirections[index] * radius;
            }
        }

        auto* vertices = new StreamVertex[kVertexCount];
        for (uint32_t stack = 0; stack <= kStackCount; ++stack) {
            uint32_t const previousStack = stack > 0 ? stack - 1 : stack;
            uint32_t const nextStack = stack < kStackCount ? stack + 1 : stack;
            for (uint32_t slice = 0; slice <= kSliceCount; ++slice) {
                uint32_t const wrappedSlice = slice == kSliceCount ? 0 : slice;
                uint32_t const previousSlice = (wrappedSlice + kSliceCount - 1) % kSliceCount;
                uint32_t const nextSlice = (wrappedSlice + 1) % kSliceCount;
                size_t const index = vertexIndex(stack, slice);

                float const azimuth = kTwoPi * float(wrappedSlice) / float(kSliceCount);
                float3 const fallbackTangent = { -std::sin(azimuth), 0.0f, std::cos(azimuth) };
                float3 const alongSlice =
                        mPositions[vertexIndex(stack, nextSlice)] -
                        mPositions[vertexIndex(stack, previousSlice)];
                float3 const alongStack =
                        mPositions[vertexIndex(nextStack, slice)] -
                        mPositions[vertexIndex(previousStack, slice)];

                float3 tangent = normalizedOr(alongSlice, fallbackTangent);
                float3 normal = stack == 0 || stack == kStackCount
                                        ? mDirections[index]
                                        : normalizedOr(cross(alongSlice, alongStack),
                                                  mDirections[index]);
                if (dot(normal, mDirections[index]) < 0.0f) {
                    normal = -normal;
                }
                float3 const bitangent = normalizedOr(cross(normal, tangent),
                        float3{ 0.0f, 1.0f, 0.0f });
                tangent = normalizedOr(cross(bitangent, normal), tangent);

                vertices[index] = {
                    .position = mPositions[index],
                    .tangent = packSnorm16(
                            mat3f::packTangentFrame({ tangent, bitangent, normal }).xyzw),
                };
            }
        }

        mVertexBuffer->setBufferAt(*mContext.engine, 0,
                { vertices, kVertexCount * sizeof(StreamVertex), releaseVertices });
    }

    void updateAppearance(float seconds) {
        float3 const primary = spectrum(seconds * 0.7f);
        float3 const secondary = spectrum(seconds * 0.7f + 2.4f);
        mCoreMaterialInstance->setParameter("baseColor", RgbType::LINEAR,
                float3{ 0.04f, 0.08f, 0.12f } + primary * 0.28f);
        mAuraMaterialInstance->setParameter("fillColor", RgbType::LINEAR,
                primary * 0.35f);
        mAuraMaterialInstance->setParameter("edgeColor", RgbType::LINEAR,
                float3{ 0.35f, 0.55f, 0.75f } + secondary * 0.65f);
    }

    void updateTransforms(float seconds) {
        mat4f const rotation = mat4f::rotation(seconds * 0.18f,
                normalize(float3{ 0.2f, 1.0f, 0.1f }));
        auto& transformManager = mContext.engine->getTransformManager();
        transformManager.setTransform(transformManager.getInstance(mCore),
                mat4f::translation(kSpherePosition) * rotation);
        float const auraScale = 1.035f + 0.012f * std::sin(seconds * 3.1f);
        transformManager.setTransform(transformManager.getInstance(mAura),
                mat4f::translation(kSpherePosition) * rotation * mat4f::scaling(auraScale));
    }

    FeatureContext mContext;
    std::vector<float3> mDirections;
    std::vector<float3> mPositions;
    std::vector<uint16_t> mIndices;
    VertexBuffer* mVertexBuffer = nullptr;
    IndexBuffer* mIndexBuffer = nullptr;
    Material* mAuraMaterial = nullptr;
    MaterialInstance* mCoreMaterialInstance = nullptr;
    MaterialInstance* mAuraMaterialInstance = nullptr;
    utils::Entity mCore;
    utils::Entity mAura;
    XrTime mStartTime = 0;
};

} // anonymous namespace

std::unique_ptr<Feature> createVertexStreaming() {
    return std::make_unique<VertexStreaming>();
}

} // namespace helloxr