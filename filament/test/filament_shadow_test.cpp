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

#include "DynamicSpecConstKey.h"
#include "LocalProgramCache.h"

#include "details/Material.h"
#include "details/View.h"

#include <private/filament/Variant.h>

#include <filament/Box.h>
#include <filament/Camera.h>
#include <filament/DebugRegistry.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialEnums.h>
#include <filament/MaterialInstance.h>
#include <filament/Options.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/SwapChain.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <filamat/MaterialBuilder.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>
#include <utils/FixedCapacityVector.h>

#include <math/vec3.h>

#include <gtest/gtest.h>

#include <utility>
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
        for (Material* const material : mMaterials) {
            mEngine->destroy(material);
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

    Material* createLitMaterial() {
        filamat::MaterialBuilder builder;
        builder.init();
        builder.name("ShadowTestLit");
        builder.shading(Shading::LIT);
        filamat::Package const package = builder.build(mEngine->getJobSystem());
        if (!package.isValid()) {
            return nullptr;
        }
        Material* const material = Material::Builder()
                .package(package.getData(), package.getSize())
                .build(*mEngine);
        if (material) {
            mMaterials.push_back(material);
        }
        return material;
    }

    // The shadow code only uses the bounding box; the geometry is a placeholder.
    void addBox(float3 const& center, float3 const& halfExtent,
            bool const castShadows, bool const receiveShadows,
            MaterialInstance const* const material = nullptr) {
        VertexBuffer* const vb = VertexBuffer::Builder()
                .vertexCount(3)
                .bufferCount(2)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
                .attribute(VertexAttribute::TANGENTS, 1, VertexBuffer::AttributeType::FLOAT4)
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
                .material(0, material)
                .castShadows(castShadows)
                .receiveShadows(receiveShadows)
                .build(*mEngine, e);
        mScene->addEntity(e);
    }

    // Floor and caster use the default material (variantFilter: [vsm]), so they cannot cast
    // VSM shadows.
    void addLitScene(bool const lightCastsShadows, Material const* const material) {
        Entity const sun = createEntity();
        LightManager::Builder(LightManager::Type::DIRECTIONAL)
                .direction({ 0, -1, 0 })
                .castShadows(lightCastsShadows)
                .build(*mEngine, sun);
        mScene->addEntity(sun);

        addBox({ 0, -0.5f, 0 }, { 50, 0.5f, 50 }, false, true);
        addBox({ 0, 30, 0 }, { 1, 1, 1 }, true, false);
        addBox({ 0, 1, 0 }, { 1, 1, 1 }, false, true, material->getDefaultInstance());

        mCamera->setProjection(45.0, 1.0, 0.1, 100.0);
        mCamera->lookAt({ 0, 1.6, 10 }, { 0, 0, 0 }, { 0, 1, 0 });
    }

    void render() {
        mVsyncNanos += 16'666'667;
        ASSERT_TRUE(mRenderer->beginFrame(mSwapChain, mVsyncNanos));
        mRenderer->render(mView);
        mRenderer->endFrame();
        mEngine->flushAndWait();
    }

    // Renders a frame, then returns the near and far planes of the first directional cascade.
    void renderFrame(double* outNear, double* outFar) {
        ASSERT_NO_FATAL_FAILURE(render());

        utils::FixedCapacityVector<Camera const*> const cameras =
                mView->getDirectionalShadowCameras();
        ASSERT_FALSE(cameras.empty());
        ASSERT_NE(cameras[0], nullptr) << "no visible directional shadows";
        *outNear = cameras[0]->getNear();
        *outFar = cameras[0]->getCullingFar();
    }

    // { variant, whether the program receives shadows }
    using ColorPrograms = std::vector<std::pair<Variant::type_t, bool>>;

    // The color programs `material` was drawn with: a program slot stays empty until a draw call
    // needs it.
    static ColorPrograms getColorPrograms(Material const* const material) {
        LocalProgramCache::Programs const programs =
                downcast(material)->getPrograms().getPrograms();
        ColorPrograms result;
        for (Variant const variant : VariantUtils::getLitVariants()) {
            for (DynamicSpecConstKey const specKey : DynamicSpecConstKey::getValidKeys(
                         variant, MaterialDomain::SURFACE, true)) {
                if (programs[LocalProgramCache::mapCacheEntryKey(
                            variant, specKey, programs.size())]) {
                    result.emplace_back(variant.key, specKey.hasShadowReceiver());
                }
            }
        }
        return result;
    }

    Engine* mEngine = nullptr;
    Renderer* mRenderer = nullptr;
    SwapChain* mSwapChain = nullptr;
    Scene* mScene = nullptr;
    View* mView = nullptr;
    Camera* mCamera = nullptr;
    Entity mCameraEntity;
    std::vector<Entity> mEntities;
    std::vector<Material*> mMaterials;
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

    // The floor, in view. It only receives shadows: in light space it lies entirely behind the
    // farthest caster (the box below), and the light frustum must still reach it.
    addBox({ 0, -0.5f, 0 }, { 50, 0.5f, 50 }, false, true);
    // A box far above the camera's view volume, whose shadow falls on the visible floor. The view
    // volume never reaches it, so if the light frustum were clipped to the casters' depth range
    // before being intersected with the view volume, there would be no shadow map at all.
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

// Non-PCF views use the S2D variant even with no active shadows; shadow receiving stays disabled.
TEST_F(ShadowTest, VsmViewWithoutShadowsRendersLitMaterial) {
    Material const* const material = createLitMaterial();
    ASSERT_NE(material, nullptr);
    mView->setShadowType(ShadowType::VSM);
    addLitScene(/* lightCastsShadows= */ false, material);
    ASSERT_NO_FATAL_FAILURE(render());

    EXPECT_FALSE(downcast(mView)->hasShadowing());
    EXPECT_EQ(getColorPrograms(material), ColorPrograms({ { Variant::S2D, false } }));
}

TEST_F(ShadowTest, PcfViewRendersLitShadowReceiver) {
    Material const* const material = createLitMaterial();
    ASSERT_NE(material, nullptr);
    mView->setShadowType(ShadowType::PCF);
    addLitScene(/* lightCastsShadows= */ true, material);
    ASSERT_NO_FATAL_FAILURE(render());

    EXPECT_TRUE(downcast(mView)->hasShadowing());
    EXPECT_EQ(getColorPrograms(material), ColorPrograms({ { Variant::NO_VARIANT, true } }));
}
