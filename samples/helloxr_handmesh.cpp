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

// XR_EXT_hand_tracking + XR_FB_hand_tracking_mesh: the runtime hands over a skinned mesh in bind
// pose once, and joint poses every frame. Filament does the skinning on the GPU.

#include "helloxr_features.h"

#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>

#include <geometry/SurfaceOrientation.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>

#if !defined(__ANDROID__)
#include "generated/resources/resources.h"
#endif

#include <vector>

using namespace filament;
using namespace filament::math;

namespace helloxr {
namespace {

constexpr uint32_t kHandCount = 2;
constexpr char const* kHandNames[kHandCount] = { "left", "right" };

class HandMeshes final : public Feature {
public:
    char const* name() const override { return "hand meshes"; }

    std::vector<char const*> requiredExtensions() const override {
        return { XR_EXT_HAND_TRACKING_EXTENSION_NAME, XR_FB_HAND_TRACKING_MESH_EXTENSION_NAME };
    }

    bool initialize(FeatureContext const& context) override {
        mContext = context;

        if (!load("xrCreateHandTrackerEXT", &mCreateHandTracker) ||
                !load("xrDestroyHandTrackerEXT", &mDestroyHandTracker) ||
                !load("xrLocateHandJointsEXT", &mLocateHandJoints) ||
                !load("xrGetHandMeshFB", &mGetHandMesh)) {
            XRLOG("hand meshes: entry points unavailable");
            return false;
        }

        XrSystemHandTrackingPropertiesEXT handProperties = {
            XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT
        };
        XrSystemProperties systemProperties = { XR_TYPE_SYSTEM_PROPERTIES };
        systemProperties.next = &handProperties;
        XrSystemGetInfo systemInfo = { XR_TYPE_SYSTEM_GET_INFO };
        systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
        XrSystemId systemId = XR_NULL_SYSTEM_ID;
        if (XR_SUCCEEDED(xrGetSystem(mContext.instance, &systemInfo, &systemId)) &&
                XR_SUCCEEDED(xrGetSystemProperties(mContext.instance, systemId,
                        &systemProperties)) &&
                handProperties.supportsHandTracking == XR_FALSE) {
            XRLOG("hand meshes: the system reports no hand tracking support");
            return false;
        }

        uint32_t built = 0;
        for (uint32_t hand = 0; hand < kHandCount; ++hand) {
            if (createHand(hand)) {
                built++;
            }
        }        XRLOG("hand meshes: built %u of %u hand meshes", built, kHandCount);
        return built > 0;
    }
    void update(XrTime displayTime) override {
        auto& rcm = mContext.engine->getRenderableManager();
        for (uint32_t hand = 0; hand < kHandCount; ++hand) {
            Hand& state = mHands[hand];
            if (state.tracker == XR_NULL_HANDLE || state.renderable.isNull()) {
                continue;
            }

            XrHandJointLocationEXT joints[XR_HAND_JOINT_COUNT_EXT] = {};
            XrHandJointLocationsEXT locations = { XR_TYPE_HAND_JOINT_LOCATIONS_EXT };
            locations.jointCount = XR_HAND_JOINT_COUNT_EXT;
            locations.jointLocations = joints;

            XrHandJointsLocateInfoEXT locateInfo = { XR_TYPE_HAND_JOINTS_LOCATE_INFO_EXT };
            locateInfo.baseSpace = mContext.appSpace;
            locateInfo.time = displayTime;

            bool const active =
                    XR_SUCCEEDED(mLocateHandJoints(state.tracker, &locateInfo, &locations)) &&
                    locations.isActive == XR_TRUE;

            if (active != state.visible) {
                if (active) {
                    mContext.scene->addEntity(state.renderable);
                } else {
                    mContext.scene->remove(state.renderable);
                }
                state.visible = active;
                XRLOG("hand meshes: %s hand %s", kHandNames[hand],
                        active ? "tracked, now drawn" : "lost tracking, hidden");
            }
            if (!active) {
                continue;
            }

            // Vertices are stored in bind space, so the skinning matrix has to undo the bind pose
            // before applying the joint's current world transform.
            std::vector<mat4f> bones(state.jointCount);
            for (uint32_t joint = 0; joint < state.jointCount; ++joint) {
                constexpr XrSpaceLocationFlags kValid = XR_SPACE_LOCATION_POSITION_VALID_BIT |
                                                        XR_SPACE_LOCATION_ORIENTATION_VALID_BIT;
                if ((joints[joint].locationFlags & kValid) != kValid) {
                    bones[joint] = mat4f();
                    continue;
                }
                bones[joint] = mat4f(poseToMat4(joints[joint].pose)) * state.inverseBind[joint];
            }
            rcm.setBones(rcm.getInstance(state.renderable), bones.data(), state.jointCount, 0);
        }
    }

    void terminate() override {
        auto& em = utils::EntityManager::get();
        for (Hand& state: mHands) {
            if (!state.renderable.isNull()) {
                mContext.scene->remove(state.renderable);
                mContext.engine->destroy(state.renderable);
                em.destroy(state.renderable);
                state.renderable = {};
            }
            if (state.vertexBuffer != nullptr) {
                mContext.engine->destroy(state.vertexBuffer);
                state.vertexBuffer = nullptr;
            }
            if (state.indexBuffer != nullptr) {
                mContext.engine->destroy(state.indexBuffer);
                state.indexBuffer = nullptr;
            }
            if (state.materialInstance != nullptr) {
                mContext.engine->destroy(state.materialInstance);
                state.materialInstance = nullptr;
            }
            if (state.tracker != XR_NULL_HANDLE) {
                mDestroyHandTracker(state.tracker);
                state.tracker = XR_NULL_HANDLE;
            }
        }
        if (mMaterial != nullptr) {
            mContext.engine->destroy(mMaterial);
            mMaterial = nullptr;
        }
    }

private:
    struct Hand {
        XrHandTrackerEXT tracker = XR_NULL_HANDLE;
        utils::Entity renderable;
        VertexBuffer* vertexBuffer = nullptr;
        IndexBuffer* indexBuffer = nullptr;
        MaterialInstance* materialInstance = nullptr;
        std::vector<mat4f> inverseBind;
        uint32_t jointCount = 0;
        bool visible = false;
    };

    template<typename Fn>
    bool load(char const* fnName, Fn* out) const {
        return XR_SUCCEEDED(xrGetInstanceProcAddr(mContext.instance, fnName,
                       reinterpret_cast<PFN_xrVoidFunction*>(out))) &&
               *out != nullptr;
    }

    bool loadMaterial() {
#if defined(__ANDROID__)
        std::vector<uint8_t> package;
        if (!readAsset("xrhand.filamat", &package)) {
            XRLOG("hand meshes: xrhand.filamat is missing from the assets");
            return false;
        }
        mMaterial = Material::Builder()
                            .package(package.data(), package.size())
                            .build(*mContext.engine);
#else
        mMaterial = Material::Builder()
                            .package(RESOURCES_XRHAND_DATA, RESOURCES_XRHAND_SIZE)
                            .build(*mContext.engine);
#endif
        return mMaterial != nullptr;
    }

    bool createHand(uint32_t hand) {
        Hand& state = mHands[hand];

        if (mMaterial == nullptr && !loadMaterial()) {
            return false;
        }

        XrHandTrackerCreateInfoEXT createInfo = { XR_TYPE_HAND_TRACKER_CREATE_INFO_EXT };
        createInfo.hand = hand == 0 ? XR_HAND_LEFT_EXT : XR_HAND_RIGHT_EXT;
        createInfo.handJointSet = XR_HAND_JOINT_SET_DEFAULT_EXT;
        if (XR_FAILED(mCreateHandTracker(mContext.session, &createInfo, &state.tracker))) {
            XRLOG("hand meshes: xrCreateHandTrackerEXT failed for the %s hand", kHandNames[hand]);
            return false;
        }

        // Two-call idiom, except that this one sizes three arrays at once.
        XrHandTrackingMeshFB mesh = { XR_TYPE_HAND_TRACKING_MESH_FB };
        if (XR_FAILED(mGetHandMesh(state.tracker, &mesh))) {
            XRLOG("hand meshes: xrGetHandMeshFB sizing failed for the %s hand", kHandNames[hand]);
            return false;
        }

        uint32_t const jointCount = mesh.jointCountOutput;
        uint32_t const vertexCount = mesh.vertexCountOutput;
        uint32_t const indexCount = mesh.indexCountOutput;
        if (jointCount == 0 || vertexCount == 0 || indexCount == 0) {
            XRLOG("hand meshes: the runtime returned an empty mesh for the %s hand",
                    kHandNames[hand]);
            return false;
        }

        std::vector<XrPosef> bindPoses(jointCount);
        std::vector<float> radii(jointCount);
        std::vector<XrHandJointEXT> parents(jointCount);
        std::vector<XrVector3f> positions(vertexCount);
        std::vector<XrVector3f> normals(vertexCount);
        std::vector<XrVector2f> uvs(vertexCount);
        std::vector<XrVector4sFB> blendIndices(vertexCount);
        std::vector<XrVector4f> blendWeights(vertexCount);
        std::vector<int16_t> indices(indexCount);

        mesh.jointCapacityInput = jointCount;
        mesh.jointBindPoses = bindPoses.data();
        mesh.jointRadii = radii.data();
        mesh.jointParents = parents.data();
        mesh.vertexCapacityInput = vertexCount;
        mesh.vertexPositions = positions.data();
        mesh.vertexNormals = normals.data();
        mesh.vertexUVs = uvs.data();
        mesh.vertexBlendIndices = blendIndices.data();
        mesh.vertexBlendWeights = blendWeights.data();
        mesh.indexCapacityInput = indexCount;
        mesh.indices = indices.data();
        if (XR_FAILED(mGetHandMesh(state.tracker, &mesh))) {
            XRLOG("hand meshes: xrGetHandMeshFB failed for the %s hand", kHandNames[hand]);
            return false;
        }

        state.jointCount = jointCount;
        state.inverseBind.resize(jointCount);
        for (uint32_t joint = 0; joint < jointCount; ++joint) {
            state.inverseBind[joint] = mat4f(inverse(poseToMat4(bindPoses[joint])));
        }

        // Filament wants a tangent frame rather than a bare normal.
        std::vector<quatf> tangents(vertexCount);
        auto* orientation = geometry::SurfaceOrientation::Builder()
                                    .vertexCount(vertexCount)
                                    .normals(reinterpret_cast<float3 const*>(normals.data()))
                                    .build();
        orientation->getQuats(tangents.data(), vertexCount);
        delete orientation;

        Engine& engine = *mContext.engine;
        state.vertexBuffer = VertexBuffer::Builder()
                                     .vertexCount(vertexCount)
                                     .bufferCount(4)
                                     .attribute(VertexAttribute::POSITION, 0,
                                             VertexBuffer::AttributeType::FLOAT3)
                                     .attribute(VertexAttribute::TANGENTS, 1,
                                             VertexBuffer::AttributeType::FLOAT4)
                                     .attribute(VertexAttribute::BONE_INDICES, 2,
                                             VertexBuffer::AttributeType::USHORT4)
                                     .attribute(VertexAttribute::BONE_WEIGHTS, 3,
                                             VertexBuffer::AttributeType::FLOAT4)
                                     .build(engine);

        auto copy = [](void const* source, size_t size) {
            auto* bytes = new uint8_t[size];
            memcpy(bytes, source, size);
            return bytes;
        };
        auto free = [](void* buffer, size_t, void*) { delete[] static_cast<uint8_t*>(buffer); };

        size_t const positionsSize = vertexCount * sizeof(XrVector3f);
        size_t const tangentsSize = vertexCount * sizeof(quatf);
        size_t const blendIndicesSize = vertexCount * sizeof(XrVector4sFB);
        size_t const blendWeightsSize = vertexCount * sizeof(XrVector4f);
        state.vertexBuffer->setBufferAt(engine, 0,
                { copy(positions.data(), positionsSize), positionsSize, free });
        state.vertexBuffer->setBufferAt(engine, 1,
                { copy(tangents.data(), tangentsSize), tangentsSize, free });
        state.vertexBuffer->setBufferAt(engine, 2,
                { copy(blendIndices.data(), blendIndicesSize), blendIndicesSize, free });
        state.vertexBuffer->setBufferAt(engine, 3,
                { copy(blendWeights.data(), blendWeightsSize), blendWeightsSize, free });

        size_t const indicesSize = indexCount * sizeof(int16_t);
        state.indexBuffer = IndexBuffer::Builder()
                                    .indexCount(indexCount)
                                    .bufferType(IndexBuffer::IndexType::USHORT)
                                    .build(engine);
        state.indexBuffer->setBuffer(engine, { copy(indices.data(), indicesSize), indicesSize,
                free });

        state.materialInstance = mMaterial->createInstance();
        state.materialInstance->setParameter("fillColor", RgbType::LINEAR,
                float3{ 0.35f, 0.55f, 0.85f });
        state.materialInstance->setParameter("edgeColor", RgbType::LINEAR,
                float3{ 0.75f, 0.90f, 1.0f });
        state.materialInstance->setParameter("fillOpacity", 0.25f);
        state.materialInstance->setParameter("edgeOpacity", 0.9f);
        state.materialInstance->setParameter("edgePower", 2.5f);
        state.materialInstance->setParameter("edgeWidth", 0.35f);

        state.renderable = utils::EntityManager::get().create();
        RenderableManager::Builder(1)
                .boundingBox({ { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f } })
                .material(0, state.materialInstance)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, state.vertexBuffer,
                        state.indexBuffer, 0, indexCount)
                .culling(false)
                .castShadows(false)
                .receiveShadows(false)
                .skinning(jointCount)
                .build(engine, state.renderable);

        XRLOG("hand meshes: %s hand has %u vertices, %u indices, %u joints", kHandNames[hand],
                vertexCount, indexCount, jointCount);
        return true;
    }

    FeatureContext mContext;
    Hand mHands[kHandCount];
    Material* mMaterial = nullptr;

    PFN_xrCreateHandTrackerEXT mCreateHandTracker = nullptr;
    PFN_xrDestroyHandTrackerEXT mDestroyHandTracker = nullptr;
    PFN_xrLocateHandJointsEXT mLocateHandJoints = nullptr;
    PFN_xrGetHandMeshFB mGetHandMesh = nullptr;
};

} // anonymous namespace

std::unique_ptr<Feature> createHandMeshes() {
    return std::make_unique<HandMeshes>();
}

} // namespace helloxr
