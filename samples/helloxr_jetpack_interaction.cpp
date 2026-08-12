#include "helloxr_features.h"
#include "helloxr_input.h"
#include "helloxr_jetpack_ui.h"

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
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

using namespace filament;
using namespace filament::math;

namespace helloxr {
namespace {

constexpr uint32_t CORNER_SEGMENTS = 10;
constexpr uint32_t CIRCLE_SEGMENTS = 32;
constexpr uint32_t RAY_SEGMENTS = 12;
constexpr float PI = 3.14159265358979323846f;
constexpr float GLASS_OFFSET = 0.01f;
constexpr float OUTER_HALF_WIDTH = JetpackUiLayer::WIDTH_METERS * 0.5f + 0.055f;
constexpr float OUTER_HALF_HEIGHT = JetpackUiLayer::HEIGHT_METERS * 0.5f + 0.055f;
constexpr float INNER_HALF_WIDTH = JetpackUiLayer::WIDTH_METERS * 0.5f - 0.01f;
constexpr float INNER_HALF_HEIGHT = JetpackUiLayer::HEIGHT_METERS * 0.5f - 0.01f;
constexpr float OUTER_RADIUS = 0.0375f;
constexpr float INNER_RADIUS = 0.016f;
constexpr float BORDER_WIDTH = 0.004f;

struct Vertex {
    float3 position;
    float2 uv;
};

struct Mesh {
    VertexBuffer* vertices = nullptr;
    IndexBuffer* indices = nullptr;
    utils::Entity entity;
    bool visible = false;
};

void releaseVertices(void* buffer, size_t, void*) {
    delete[] static_cast<Vertex*>(buffer);
}

void releaseIndices(void* buffer, size_t, void*) {
    delete[] static_cast<uint16_t*>(buffer);
}

std::vector<float2> roundedRectangle(float halfWidth, float halfHeight, float radius) {
    std::vector<float2> points;
    points.reserve(CORNER_SEGMENTS * 4);
    float2 const centers[] = {
        { halfWidth - radius, halfHeight - radius },
        { -halfWidth + radius, halfHeight - radius },
        { -halfWidth + radius, -halfHeight + radius },
        { halfWidth - radius, -halfHeight + radius },
    };
    for (uint32_t corner = 0; corner < 4; ++corner) {
        for (uint32_t segment = 0; segment < CORNER_SEGMENTS; ++segment) {
            float const angle = (float(corner) + float(segment) / float(CORNER_SEGMENTS)) *
                                PI * 0.5f;
            points.push_back(centers[corner] + float2{ std::cos(angle), std::sin(angle) } * radius);
        }
    }
    return points;
}

bool roundedRectangleContains(float2 point, float halfWidth, float halfHeight, float radius) {
    float const dx = std::max(std::abs(point.x) - (halfWidth - radius), 0.0f);
    float const dy = std::max(std::abs(point.y) - (halfHeight - radius), 0.0f);
    return dx * dx + dy * dy <= radius * radius;
}

class JetpackInteraction final : public Feature {
public:
    char const* name() const override { return "Jetpack UI interaction"; }

    std::vector<char const*> requiredExtensions() const override { return {}; }

    bool initialize(FeatureContext const& context) override {
        mContext = context;
        if (mContext.input == nullptr || mContext.jetpackUi == nullptr ||
                !mContext.jetpackUi->isEnabled() || !loadMaterial()) {
            return false;
        }

        mGlassMaterialInstance = mMaterial->createInstance();
        mBorderMaterialInstance = mMaterial->createInstance();
        mRayMaterialInstance = mMaterial->createInstance();
        mCursorMaterialInstance = mMaterial->createInstance();
        mGlowMaterialInstance = mMaterial->createInstance();
        mGlassMaterialInstance->setParameter("color", float4{ 0.10f, 0.60f, 0.75f, 0.24f });
        mBorderMaterialInstance->setParameter("color", float4{ 0.35f, 0.92f, 1.0f, 0.55f });
        mRayMaterialInstance->setParameter("color", float4{ 0.24f, 0.95f, 0.78f, 0.72f });
        mCursorMaterialInstance->setParameter("color", float4{ 0.90f, 1.0f, 0.96f, 0.96f });
        mGlowMaterialInstance->setParameter("color", float4{ 0.20f, 1.0f, 0.82f, 0.28f });
        mGlassMaterialInstance->setDepthWrite(false);
        mBorderMaterialInstance->setDepthWrite(false);
        mGlassMaterialInstance->setParameter("glassEffect", 1.0f);
        mGlassMaterialInstance->setParameter("hitPoint", float2{ 0.5f, 0.5f });
        mGlassMaterialInstance->setParameter("hitStrength", 0.0f);
        for (MaterialInstance* instance: { mBorderMaterialInstance, mRayMaterialInstance,
                 mCursorMaterialInstance, mGlowMaterialInstance }) {
            instance->setParameter("glassEffect", 0.0f);
        }

        if (!createFrame() || !createBorder() || !createRay() ||
            !createDisk(&mCursor, mCursorMaterialInstance) ||
            !createDisk(&mGlow, mGlowMaterialInstance)) {
            return false;
        }
        updatePanelTransforms(mContext.jetpackUi->getPose());
        setVisible(mFrame, true);
        setVisible(mBorder, true);
        XRLOG("Jetpack interaction: rounded glass, aim ray, cursor, and touch enabled");
        return true;
    }

    void update(XrTime displayTime) override {
        XrPosef aimPose = {};
        uint32_t hand = 1;
        if (!mContext.input->getAimPose(hand, &aimPose)) {
            hand = 0;
        }
        if (!mContext.input->getAimPose(hand, &aimPose)) {
            setVisible(mRay, false);
            setVisible(mCursor, false);
            setVisible(mGlow, false);
            mGlassMaterialInstance->setParameter("hitStrength", 0.0f);
            mDragging = false;
            mTriggerWasPressed = false;
            cancelTouch();
            return;
        }

        mat4f const aimTransform = mat4f(poseToMat4(aimPose));
        float3 const origin = aimTransform[3].xyz;
        float3 const direction = normalize(-aimTransform[2].xyz);
        bool const pressed = mContext.input->getTriggerValue(hand) >= 0.55f;
        XrPosef panelPose = mContext.jetpackUi->getPose();
        if (mDragging) {
            if (pressed) {
                float3 const target = origin + direction * mDragDistance;
                mat3f orientation{ quatf{ panelPose.orientation.w, panelPose.orientation.x,
                        panelPose.orientation.y, panelPose.orientation.z } };
                float3 panelPosition = target - orientation * mGrabPoint;

                XrSpaceLocation head = { XR_TYPE_SPACE_LOCATION };
                constexpr XrSpaceLocationFlags REQUIRED =
                        XR_SPACE_LOCATION_POSITION_VALID_BIT |
                        XR_SPACE_LOCATION_ORIENTATION_VALID_BIT;
                if (XR_SUCCEEDED(xrLocateSpace(mContext.viewSpace, mContext.appSpace,
                            displayTime, &head)) &&
                        (head.locationFlags & REQUIRED) == REQUIRED) {
                    float3 const headPosition = {
                        head.pose.position.x, head.pose.position.y, head.pose.position.z
                    };
                    float3 const forward = normalize(headPosition - panelPosition);
                    float3 up = { 0.0f, 1.0f, 0.0f };
                    if (std::abs(dot(forward, up)) > 0.995f) {
                        up = { 0.0f, 0.0f, 1.0f };
                    }
                    float3 const right = normalize(cross(up, forward));
                    up = cross(forward, right);
                    quatf const facing = mat3f{ right, up, forward }.toQuaternion();
                    panelPose.orientation = { facing.x, facing.y, facing.z, facing.w };
                    orientation = mat3f{ facing };
                    panelPosition = target - orientation * mGrabPoint;
                }
                panelPose.position = {
                    panelPosition.x, panelPosition.y, panelPosition.z
                };
                mContext.jetpackUi->setPose(panelPose);
                updatePanelTransforms(panelPose);
            } else {
                mDragging = false;
            }
        }
        mat4f const worldFromPanel = mat4f(poseToMat4(panelPose));
        mat4f const panelFromWorld = inverse(worldFromPanel);
        float3 const localOrigin = (panelFromWorld * float4{ origin, 1.0f }).xyz;
        float3 const localDirection = (panelFromWorld * float4{ direction, 0.0f }).xyz;
        auto const intersectPlane = [&](float z, float3* worldPoint, float3* localPoint,
                                            float* distance) {
            if (std::abs(localDirection.z) <= 1.0e-5f) {
                return false;
            }
            *distance = (z - localOrigin.z) / localDirection.z;
            if (*distance <= 0.0f) {
                return false;
            }
            *localPoint = localOrigin + localDirection * *distance;
            *worldPoint = (worldFromPanel * float4{ *localPoint, 1.0f }).xyz;
            return true;
        };
        float panelDistance = 0.0f;
        float glassDistance = 0.0f;
        float3 panelPoint = {};
        float3 glassPoint = {};
        float3 panelLocalPoint = {};
        float3 glassLocalPoint = {};
        bool const panelPlaneHit = intersectPlane(
            0.0f, &panelPoint, &panelLocalPoint, &panelDistance);
        bool const glassPlaneHit = intersectPlane(
            -GLASS_OFFSET, &glassPoint, &glassLocalPoint, &glassDistance);

        float2 const panelLocal = panelLocalPoint.xy;
        float2 const glassLocal = glassLocalPoint.xy;
        bool const uiHit = panelPlaneHit &&
                           std::abs(panelLocal.x) <= JetpackUiLayer::WIDTH_METERS * 0.5f &&
                           std::abs(panelLocal.y) <= JetpackUiLayer::HEIGHT_METERS * 0.5f;
        bool const glassHit = glassPlaneHit && !uiHit &&
                              roundedRectangleContains(glassLocal, OUTER_HALF_WIDTH,
                                      OUTER_HALF_HEIGHT, OUTER_RADIUS) &&
                              !roundedRectangleContains(glassLocal, INNER_HALF_WIDTH,
                                      INNER_HALF_HEIGHT, INNER_RADIUS);
        if (!mDragging && glassHit && pressed && !mTriggerWasPressed) {
            mDragging = true;
            mDragDistance = glassDistance;
            mGrabPoint = glassLocalPoint;
        }
        mGlassMaterialInstance->setParameter("hitStrength", glassHit ? 1.0f : 0.0f);
        if (glassHit) {
            mGlassMaterialInstance->setParameter("hitPoint", float2{
                    glassLocal.x / (OUTER_HALF_WIDTH * 2.0f) + 0.5f,
                    0.5f - glassLocal.y / (OUTER_HALF_HEIGHT * 2.0f),
            });
        }

        float const rayDistance = mDragging    ? mDragDistance
                                  : uiHit      ? panelDistance
                                  : glassHit   ? glassDistance
                                               : 2.0f;
        auto& transforms = mContext.engine->getTransformManager();
        transforms.setTransform(transforms.getInstance(mRay.entity),
                aimTransform * mat4f::scaling(
                        float3{ 0.003f, 0.003f, std::min(rayDistance, 3.0f) }));
        setVisible(mRay, true);

        setVisible(mCursor, uiHit);
        if (uiHit) {
            transforms.setTransform(transforms.getInstance(mCursor.entity),
                worldFromPanel * mat4f::translation(float3{
                    panelLocal.x, panelLocal.y, 0.006f }) *
                            mat4f::scaling(0.018f));
        }
        setVisible(mGlow, glassHit);
        if (glassHit) {
            transforms.setTransform(transforms.getInstance(mGlow.entity),
                worldFromPanel * mat4f::translation(float3{
                    glassLocal.x, glassLocal.y, -GLASS_OFFSET + 0.003f }) *
                            mat4f::scaling(0.005f));
        }

        if (!mDragging && uiHit) {
            float const u = panelLocal.x / JetpackUiLayer::WIDTH_METERS + 0.5f;
            float const v = 0.5f - panelLocal.y / JetpackUiLayer::HEIGHT_METERS;
            if (pressed && !mTouchActive) {
                injectTouch(u, v, JetpackUiLayer::TouchAction::DOWN);
                mTouchActive = true;
            } else if (pressed) {
                injectTouch(u, v, JetpackUiLayer::TouchAction::MOVE);
            } else if (mTouchActive) {
                injectTouch(u, v, JetpackUiLayer::TouchAction::UP);
                mTouchActive = false;
            }
            mLastTouch = { u, v };
        } else if (mTouchActive) {
            cancelTouch();
        }
        mTriggerWasPressed = pressed;
    }

    void terminate() override {
        cancelTouch();
        destroyMesh(&mFrame);
        destroyMesh(&mBorder);
        destroyMesh(&mRay);
        destroyMesh(&mCursor);
        destroyMesh(&mGlow);
        for (MaterialInstance** instance: { &mGlassMaterialInstance, &mBorderMaterialInstance,
                 &mRayMaterialInstance, &mCursorMaterialInstance,
                 &mGlowMaterialInstance }) {
            if (*instance != nullptr) {
                mContext.engine->destroy(*instance);
                *instance = nullptr;
            }
        }
        if (mMaterial != nullptr) {
            mContext.engine->destroy(mMaterial);
            mMaterial = nullptr;
        }
    }

private:
    bool loadMaterial() {
#if defined(__ANDROID__)
        std::vector<uint8_t> package;
        if (!readAsset("xrglass.filamat", &package)) {
            XRLOG("Jetpack interaction: xrglass.filamat is missing from the assets");
            return false;
        }
        mMaterial = Material::Builder().package(package.data(), package.size()).build(
                *mContext.engine);
#else
    return false;
#endif
        return mMaterial != nullptr;
    }

    bool createMesh(Mesh* mesh, std::vector<Vertex> const& vertices,
            std::vector<uint16_t> const& indices, MaterialInstance* material,
            Box const& bounds) {
        Engine& engine = *mContext.engine;
        mesh->vertices = VertexBuffer::Builder()
                                 .vertexCount(uint32_t(vertices.size()))
                                 .bufferCount(1)
                                 .attribute(VertexAttribute::POSITION, 0,
                                     VertexBuffer::AttributeType::FLOAT3,
                                     offsetof(Vertex, position), sizeof(Vertex))
                                 .attribute(VertexAttribute::UV0, 0,
                                     VertexBuffer::AttributeType::FLOAT2,
                                     offsetof(Vertex, uv), sizeof(Vertex))
                                 .build(engine);
        mesh->indices = IndexBuffer::Builder()
                                .indexCount(uint32_t(indices.size()))
                                .bufferType(IndexBuffer::IndexType::USHORT)
                                .build(engine);
        auto* vertexData = new Vertex[vertices.size()];
        std::copy(vertices.begin(), vertices.end(), vertexData);
        mesh->vertices->setBufferAt(engine, 0,
                { vertexData, vertices.size() * sizeof(Vertex), releaseVertices });
        auto* indexData = new uint16_t[indices.size()];
        std::copy(indices.begin(), indices.end(), indexData);
        mesh->indices->setBuffer(engine,
                { indexData, indices.size() * sizeof(uint16_t), releaseIndices });

        mesh->entity = utils::EntityManager::get().create();
        RenderableManager::Builder(1)
                .boundingBox(bounds)
                .material(0, material)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES,
                        mesh->vertices, mesh->indices)
                .culling(false)
                .castShadows(false)
                .receiveShadows(false)
                .build(engine, mesh->entity);
        return !mesh->entity.isNull();
    }

        bool createRoundedRing(Mesh* mesh, MaterialInstance* material, float outerHalfWidth,
            float outerHalfHeight, float outerRadius, float innerHalfWidth,
            float innerHalfHeight, float innerRadius) {
        auto const outer = roundedRectangle(outerHalfWidth, outerHalfHeight, outerRadius);
        auto const inner = roundedRectangle(innerHalfWidth, innerHalfHeight, innerRadius);
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;
        vertices.reserve(outer.size() * 2);
        indices.reserve(outer.size() * 6);
        for (size_t point = 0; point < outer.size(); ++point) {
            auto const uv = [outerHalfWidth, outerHalfHeight](float2 position) {
                return float2{ position.x / (outerHalfWidth * 2.0f) + 0.5f,
                    position.y / (outerHalfHeight * 2.0f) + 0.5f };
            };
            vertices.push_back({ { outer[point], 0.0f }, uv(outer[point]) });
            vertices.push_back({ { inner[point], 0.0f }, uv(inner[point]) });
            uint16_t const next = uint16_t((point + 1) % outer.size());
            uint16_t const outerCurrent = uint16_t(point * 2);
            uint16_t const innerCurrent = outerCurrent + 1;
            uint16_t const outerNext = next * 2;
            uint16_t const innerNext = outerNext + 1;
            indices.insert(indices.end(), { outerCurrent, outerNext, innerCurrent,
                    outerNext, innerNext, innerCurrent });
        }
        if (!createMesh(mesh, vertices, indices, material,
                    { { 0.0f, 0.0f, 0.0f },
                            { outerHalfWidth, outerHalfHeight, 0.001f } })) {
            return false;
        }
        return true;
    }

    bool createFrame() {
        return createRoundedRing(&mFrame, mGlassMaterialInstance, OUTER_HALF_WIDTH,
                OUTER_HALF_HEIGHT, OUTER_RADIUS, INNER_HALF_WIDTH, INNER_HALF_HEIGHT,
                INNER_RADIUS);
    }

    bool createBorder() {
        return createRoundedRing(&mBorder, mBorderMaterialInstance, OUTER_HALF_WIDTH,
                OUTER_HALF_HEIGHT, OUTER_RADIUS, OUTER_HALF_WIDTH - BORDER_WIDTH,
                OUTER_HALF_HEIGHT - BORDER_WIDTH, OUTER_RADIUS - BORDER_WIDTH);
    }

    void updatePanelTransforms(XrPosef const& pose) {
        mat4f const transform = mat4f(poseToMat4(pose)) *
                    mat4f::translation(float3{ 0.0f, 0.0f, -GLASS_OFFSET });
        auto& transforms = mContext.engine->getTransformManager();
        transforms.setTransform(transforms.getInstance(mFrame.entity), transform);
        transforms.setTransform(transforms.getInstance(mBorder.entity), transform);
    }

    bool createRay() {
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;
        vertices.reserve(RAY_SEGMENTS * 2);
        indices.reserve(RAY_SEGMENTS * 6);
        for (uint32_t segment = 0; segment < RAY_SEGMENTS; ++segment) {
            float const angle = 2.0f * PI * float(segment) / float(RAY_SEGMENTS);
            float2 const radial = { std::cos(angle), std::sin(angle) };
            float const u = float(segment) / float(RAY_SEGMENTS);
            vertices.push_back({ { radial, 0.0f }, { u, 0.0f } });
            vertices.push_back({ { radial, -1.0f }, { u, 1.0f } });
            uint16_t const next = uint16_t((segment + 1) % RAY_SEGMENTS);
            uint16_t const nearCurrent = uint16_t(segment * 2);
            uint16_t const farCurrent = nearCurrent + 1;
            uint16_t const nearNext = next * 2;
            uint16_t const farNext = nearNext + 1;
            indices.insert(indices.end(), { nearCurrent, farCurrent, nearNext,
                    nearNext, farCurrent, farNext });
        }
        return createMesh(&mRay, vertices, indices, mRayMaterialInstance,
                { { 0.0f, 0.0f, -0.5f }, { 1.0f, 1.0f, 0.5f } });
    }

    bool createDisk(Mesh* mesh, MaterialInstance* material) {
        std::vector<Vertex> vertices{ { { 0.0f, 0.0f, 0.0f }, { 0.5f, 0.5f } } };
        std::vector<uint16_t> indices;
        vertices.reserve(CIRCLE_SEGMENTS + 1);
        indices.reserve(CIRCLE_SEGMENTS * 3);
        for (uint32_t segment = 0; segment < CIRCLE_SEGMENTS; ++segment) {
            float const angle = 2.0f * PI * float(segment) / float(CIRCLE_SEGMENTS);
            vertices.push_back({ { std::cos(angle), std::sin(angle), 0.0f },
                    { std::cos(angle) * 0.5f + 0.5f,
                            std::sin(angle) * 0.5f + 0.5f } });
            indices.insert(indices.end(), { 0, uint16_t(segment + 1),
                    uint16_t((segment + 1) % CIRCLE_SEGMENTS + 1) });
        }
        return createMesh(mesh, vertices, indices, material,
                { { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 0.001f } });
    }

    void setVisible(Mesh& mesh, bool visible) {
        if (mesh.visible == visible || mesh.entity.isNull()) {
            return;
        }
        if (visible) {
            mContext.scene->addEntity(mesh.entity);
        } else {
            mContext.scene->remove(mesh.entity);
        }
        mesh.visible = visible;
    }

    void destroyMesh(Mesh* mesh) {
        if (!mesh->entity.isNull()) {
            if (mesh->visible) {
                mContext.scene->remove(mesh->entity);
            }
            mContext.engine->destroy(mesh->entity);
            utils::EntityManager::get().destroy(mesh->entity);
            mesh->entity = {};
        }
        if (mesh->vertices != nullptr) {
            mContext.engine->destroy(mesh->vertices);
            mesh->vertices = nullptr;
        }
        if (mesh->indices != nullptr) {
            mContext.engine->destroy(mesh->indices);
            mesh->indices = nullptr;
        }
    }

    void injectTouch(float u, float v, JetpackUiLayer::TouchAction action) {
        mContext.jetpackUi->injectTouch(u, v, action);
    }

    void cancelTouch() {
        if (mTouchActive && mContext.jetpackUi != nullptr) {
            injectTouch(mLastTouch.x, mLastTouch.y, JetpackUiLayer::TouchAction::CANCEL);
        }
        mTouchActive = false;
    }

    FeatureContext mContext;
    Material* mMaterial = nullptr;
    MaterialInstance* mGlassMaterialInstance = nullptr;
    MaterialInstance* mBorderMaterialInstance = nullptr;
    MaterialInstance* mRayMaterialInstance = nullptr;
    MaterialInstance* mCursorMaterialInstance = nullptr;
    MaterialInstance* mGlowMaterialInstance = nullptr;
    Mesh mFrame;
    Mesh mBorder;
    Mesh mRay;
    Mesh mCursor;
    Mesh mGlow;
    float2 mLastTouch = {};
    float3 mGrabPoint = {};
    float mDragDistance = 0.0f;
    bool mTouchActive = false;
    bool mDragging = false;
    bool mTriggerWasPressed = false;
};

} // anonymous namespace

std::unique_ptr<Feature> createJetpackInteraction() {
    return std::make_unique<JetpackInteraction>();
}

} // namespace helloxr