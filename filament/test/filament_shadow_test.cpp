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

#include <filament/Box.h>
#include <filament/Camera.h>
#include <filament/DebugRegistry.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/LightManager.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/SwapChain.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <utils/FixedCapacityVector.h>

#include <math/vec3.h>

#include <gtest/gtest.h>

#include <vector>

#include <stdint.h>

using namespace filament;
using namespace filament::math;
using utils::Entity;

namespace {

class ShadowTest : public testing::Test {
protected:
    void SetUp() override {
        mEngine = Engine::create(Engine::Backend::NOOP);
        ASSERT_NE(mEngine, nullptr);
        mRenderer = mEngine->createRenderer();
        mSwapChain = mEngine->createSwapChain(64, 64);
        mScene = mEngine->createScene();
        mView = mEngine->createView();
        mCameraEntity = mEngine->getEntityManager().create();
        mCamera = mEngine->createCamera(mCameraEntity);
        mView->setScene(mScene);
        mView->setCamera(mCamera);
        mView->setViewport({ 0, 0, 64, 64 });
    }

    // Destroys everything, even after a failed ASSERT: Engine::destroy() aborts on leaked Views.
    void TearDown() override {
        if (!mEngine) {
            return;
        }
        utils::EntityManager& em = mEngine->getEntityManager();
        for (Entity const e : mEntities) {
            mEngine->destroy(e); // all components
            em.destroy(e);
        }
        for (VertexBuffer* const vb : mVertexBuffers) {
            mEngine->destroy(vb);
        }
        for (IndexBuffer* const ib : mIndexBuffers) {
            mEngine->destroy(ib);
        }
        mEngine->destroyCameraComponent(mCameraEntity);
        em.destroy(mCameraEntity);
        mEngine->destroy(mView);
        mEngine->destroy(mScene);
        mEngine->destroy(mSwapChain);
        mEngine->destroy(mRenderer);
        Engine::destroy(&mEngine);
    }

    Entity createEntity() {
        Entity const e = mEngine->getEntityManager().create();
        mEntities.push_back(e);
        return e;
    }

    // The shadow code only uses the bounding box; the geometry is a placeholder.
    void addBox(float3 const& center, float3 const& halfExtent,
            bool const castShadows, bool const receiveShadows) {
        VertexBuffer* const vb = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
                .build(*mEngine);
        mVertexBuffers.push_back(vb);
        IndexBuffer* const ib = IndexBuffer::Builder()
                .indexCount(3)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(*mEngine);
        mIndexBuffers.push_back(ib);
        Entity const e = createEntity();
        RenderableManager::Builder(1)
                .boundingBox({ center, halfExtent })
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vb, ib, 0, 3)
                .castShadows(castShadows)
                .receiveShadows(receiveShadows)
                .build(*mEngine, e);
        mScene->addEntity(e);
    }

    // Renders a frame, then returns the near and far planes of the first directional cascade.
    void renderFrame(double* outNear, double* outFar) {
        mVsyncNanos += 16'666'667;
        ASSERT_TRUE(mRenderer->beginFrame(mSwapChain, mVsyncNanos));
        mRenderer->render(mView);
        mRenderer->endFrame();
        mEngine->flushAndWait();

        utils::FixedCapacityVector<Camera const*> const cameras =
                mView->getDirectionalShadowCameras();
        ASSERT_FALSE(cameras.empty());
        ASSERT_NE(cameras[0], nullptr) << "no visible directional shadows";
        *outNear = cameras[0]->getNear();
        *outFar = cameras[0]->getCullingFar();
    }

    Engine* mEngine = nullptr;
    Renderer* mRenderer = nullptr;
    SwapChain* mSwapChain = nullptr;
    Scene* mScene = nullptr;
    View* mView = nullptr;
    Camera* mCamera = nullptr;
    Entity mCameraEntity;
    std::vector<Entity> mEntities;
    std::vector<VertexBuffer*> mVertexBuffers;
    std::vector<IndexBuffer*> mIndexBuffers;
    uint64_t mVsyncNanos = 0;
};

} // anonymous namespace

// A directional shadow map's near plane may be pulled in to the visible receivers only if the
// shadow pass clamps depth; otherwise the casters in front of it are clipped and cast no shadow.
// The NOOP backend has no depth clamp, so the "d.shadowmap.depth_clamp" debug option, which is
// on by default, must not change the shadow bounds.
TEST_F(ShadowTest, DirectionalBoundsIgnoreDepthClampWhenUnsupported) {
    Entity const sun = createEntity();
    LightManager::Builder(LightManager::Type::DIRECTIONAL)
            .direction({ 0, -1, 0 })
            .castShadows(true)
            .build(*mEngine, sun);
    mScene->addEntity(sun);

    // The floor, in view. It must also cast shadows: with the default
    // "d.shadowmap.far_uses_shadowcasters", the light frustum ends at the farthest caster.
    addBox({ 0, -0.5f, 0 }, { 50, 0.5f, 50 }, true, true);
    // A box far above the camera's view volume, whose shadow falls on the visible floor.
    addBox({ 0, 30, 0 }, { 1, 1, 1 }, true, false);

    mCamera->setProjection(45.0, 1.0, 0.1, 100.0);
    mCamera->lookAt({ 0, 1.6, 10 }, { 0, 0, 0 }, { 0, 1, 0 });

    // The first frame creates the ShadowMapManager, which registers the debug property.
    double warmupNear = 0, warmupFar = 0;
    ASSERT_NO_FATAL_FAILURE(renderFrame(&warmupNear, &warmupFar));

    DebugRegistry& debug = mEngine->getDebugRegistry();
    double nearA = 0, farA = 0, nearB = 0, farB = 0, nearC = 0, farC = 0;
    ASSERT_TRUE(debug.setProperty("d.shadowmap.depth_clamp", true));
    ASSERT_NO_FATAL_FAILURE(renderFrame(&nearA, &farA));
    ASSERT_TRUE(debug.setProperty("d.shadowmap.depth_clamp", false));
    ASSERT_NO_FATAL_FAILURE(renderFrame(&nearB, &farB));
    ASSERT_TRUE(debug.setProperty("d.shadowmap.depth_clamp", true));
    ASSERT_NO_FATAL_FAILURE(renderFrame(&nearC, &farC));

    // The same settings give the same bounds.
    EXPECT_DOUBLE_EQ(nearA, nearC);
    EXPECT_DOUBLE_EQ(farA, farC);

    // Without depth clamp support, the debug option must have no effect.
    EXPECT_DOUBLE_EQ(nearA, nearB) << "the near plane assumes depth clamp";
    EXPECT_DOUBLE_EQ(farA, farB);
}
