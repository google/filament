/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <filamat/MaterialBuilder.h>

#include <backend/PixelBufferDescriptor.h>

#include <utils/EntityManager.h>

#include <math/vec3.h>
#include <math/vec4.h>

#include <gtest/gtest.h>

using namespace filament;
using namespace backend;

class RenderingTest : public testing::Test {
protected:
    Engine* mEngine = nullptr;
    SwapChain* mSurface = nullptr;
    Renderer* mRenderer = nullptr;
    View* mView = nullptr;
    Skybox* mSkybox = nullptr;
    Scene* mScene = nullptr;
    Camera* mCamera = nullptr;
    utils::Entity mCameraEntity;

    using closure_t = std::function<void(uint8_t const* rgba, uint32_t width, uint32_t height)>;

    void SetUp() override {
        mEngine = Engine::create();
        mSurface = mEngine->createSwapChain(16, 16);
        mRenderer = mEngine->createRenderer();

        mScene = mEngine->createScene();

        mCameraEntity = utils::EntityManager::get().create();
        mCamera = mEngine->createCamera(mCameraEntity);

        mView = mEngine->createView();
        mView->setViewport({0, 0, 16, 16});
        mView->setScene(mScene);
        mView->setCamera(mCamera);
        mView->setPostProcessingEnabled(false);

        mSkybox = Skybox::Builder().build(*mEngine);
        mScene->setSkybox(mSkybox);
    }

    void TearDown() override {
        mEngine->destroyCameraComponent(mCameraEntity);
        utils::EntityManager::get().destroy(mCameraEntity);

        mEngine->destroy(mScene);
        mEngine->destroy(mView);
        mEngine->destroy(mSkybox);
        mEngine->destroy(mRenderer);
        mEngine->destroy(mSurface);
        Engine::destroy(&mEngine);
    }

    void runTest(closure_t closure) {
        auto* user = new closure_t(std::move(closure));

        size_t size = 16 * 16 * 4;
        void* buffer = malloc(size);
        memset(buffer, 0, size);
        PixelBufferDescriptor pd(buffer, size,
                PixelDataFormat::RGBA, PixelDataType::UBYTE,
                callback, user);

        Renderer* pRenderer = mRenderer;
        pRenderer->beginFrame(mSurface);
        pRenderer->render(mView);
        pRenderer->readPixels(0, 0, 16, 16, std::move(pd));
        pRenderer->endFrame();

        // Note: this is where the runTest() callback will be called.
        mEngine->flushAndWait();
    }

private:
    static void callback(void* buffer, size_t size, void* user) {
        closure_t* closure = (closure_t *)user;
        uint8_t const* rgba = (uint8_t const*)buffer;
        (*closure)(rgba, 16, 16);
        delete closure;
        ::free(buffer);
    }
};

TEST_F(RenderingTest, ClearRed) {
    mSkybox->setColor({ 1.0f, 0.0f, 0.0f, 1.0f });
    mView->setDithering(View::Dithering::NONE);
    bool callbackCalled = false;
    runTest([this, &callbackCalled](uint8_t const* rgba, uint32_t width, uint32_t height) {
        EXPECT_EQ(rgba[0], 0xff);
        EXPECT_EQ(rgba[1], 0);
        EXPECT_EQ(rgba[2], 0);
        EXPECT_EQ(rgba[3], 0xff);
        callbackCalled = true;
    });
    EXPECT_TRUE(callbackCalled);
}

// Holds a scene with a full-viewport lit quad, a dominant green directional light and a
// weaker red one. Shared by the extra-directional-lights tests below.
class LitQuadScene {
public:
    void create(Engine& engine, Scene& scene, View& view, Camera& camera) {
        using namespace filament::math;
        using utils::Entity;
        using utils::EntityManager;

        mEngine = &engine;
        mScene = &scene;

        view.setDithering(View::Dithering::NONE);
        camera.setProjection(Camera::Projection::ORTHO, -1, 1, -1, 1, 0.1, 10);

        // full-viewport quad at z = -1, facing the camera (normal +z, identity tangent frame)
        static float3 const positions[4] = {
                { -2, -2, -1 }, { 2, -2, -1 }, { 2, 2, -1 }, { -2, 2, -1 } };
        static float4 const tangents[4] = {
                { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 }, { 0, 0, 0, 1 } };
        static uint16_t const indices[6] = { 0, 1, 2, 0, 2, 3 };

        mVertexBuffer = VertexBuffer::Builder()
                .vertexCount(4)
                .bufferCount(2)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
                .attribute(VertexAttribute::TANGENTS, 1, VertexBuffer::AttributeType::FLOAT4)
                .build(engine);
        mVertexBuffer->setBufferAt(engine, 0, { positions, sizeof(positions) });
        mVertexBuffer->setBufferAt(engine, 1, { tangents, sizeof(tangents) });

        mIndexBuffer = IndexBuffer::Builder()
                .indexCount(6)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(engine);
        mIndexBuffer->setBuffer(engine, { indices, sizeof(indices) });

        // a fully diffuse lit material
        filamat::MaterialBuilder builder;
        builder.init();
        builder.shading(Shading::LIT)
               .material("void material(inout MaterialInputs material) {"
                         "  prepareMaterial(material);"
                         "  material.baseColor.rgb = float3(0.8);"
                         "  material.roughness = 1.0;"
                         "}")
               .targetApi(filamat::MaterialBuilder::TargetApi::ALL);
        filamat::Package const pkg = builder.build(engine.getJobSystem());
        ASSERT_TRUE(pkg.isValid());
        mMaterial = Material::Builder()
                .package(pkg.getData(), pkg.getSize())
                .build(engine);
        ASSERT_NE(mMaterial, nullptr);

        mRenderable = EntityManager::get().create();
        RenderableManager::Builder(1)
                .boundingBox({{ 0, 0, -1 }, { 2, 2, 1 }})
                .material(0, mMaterial->getDefaultInstance())
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES,
                        mVertexBuffer, mIndexBuffer)
                .culling(false)
                .receiveShadows(false)
                .castShadows(false)
                .build(engine, mRenderable);
        scene.addEntity(mRenderable);

        // a dominant green directional light, added to the scene right away, and a weaker
        // red one that tests add when needed
        mGreenLight = EntityManager::get().create();
        LightManager::Builder(LightManager::Type::DIRECTIONAL)
                .color({ 0.0f, 1.0f, 0.0f })
                .intensity(100000.0f)
                .direction({ 0.0f, 0.0f, -1.0f })
                .build(engine, mGreenLight);
        scene.addEntity(mGreenLight);

        mRedLight = EntityManager::get().create();
        LightManager::Builder(LightManager::Type::DIRECTIONAL)
                .color({ 1.0f, 0.0f, 0.0f })
                .intensity(50000.0f)
                .direction({ 0.0f, 0.0f, -1.0f })
                .build(engine, mRedLight);
    }

    void addRedLight() { mScene->addEntity(mRedLight); }

    void destroy() {
        using utils::EntityManager;
        mScene->remove(mRenderable);
        mScene->remove(mGreenLight);
        mScene->remove(mRedLight);
        for (auto e : { mRenderable, mGreenLight, mRedLight }) {
            mEngine->destroy(e);
            EntityManager::get().destroy(e);
        }
        mEngine->destroy(mMaterial);
        mEngine->destroy(mVertexBuffer);
        mEngine->destroy(mIndexBuffer);
    }

    static uint8_t const* centerPixel(uint8_t const* rgba, uint32_t width, uint32_t height) {
        return rgba + ((height / 2) * width + width / 2) * 4;
    }

private:
    Engine* mEngine = nullptr;
    Scene* mScene = nullptr;
    VertexBuffer* mVertexBuffer = nullptr;
    IndexBuffer* mIndexBuffer = nullptr;
    Material* mMaterial = nullptr;
    utils::Entity mRenderable;
    utils::Entity mGreenLight;
    utils::Entity mRedLight;
};

TEST_F(RenderingTest, MultipleDirectionalLights) {
    LitQuadScene quad;
    quad.create(*mEngine, *mScene, *mView, *mCamera);

    // with only the dominant light in the scene, the quad is green
    bool callbackCalled = false;
    runTest([&callbackCalled](uint8_t const* rgba, uint32_t width, uint32_t height) {
        uint8_t const* p = LitQuadScene::centerPixel(rgba, width, height);
        EXPECT_LT(p[0], 16);
        EXPECT_GT(p[1], 64);
        callbackCalled = true;
    });
    EXPECT_TRUE(callbackCalled);

    // the second (extra) directional light adds red
    quad.addRedLight();
    callbackCalled = false;
    runTest([&callbackCalled](uint8_t const* rgba, uint32_t width, uint32_t height) {
        uint8_t const* p = LitQuadScene::centerPixel(rgba, width, height);
        EXPECT_GT(p[0], 32);
        EXPECT_GT(p[1], 64);
        callbackCalled = true;
    });
    EXPECT_TRUE(callbackCalled);

    quad.destroy();
}

TEST_F(RenderingTest, ClearGreen) {
    mSkybox->setColor({ 0.0f, 1.0f, 0.0f, 1.0f });
    mView->setDithering(View::Dithering::NONE);
    bool callbackCalled = false;
    runTest([this, &callbackCalled](uint8_t const* rgba, uint32_t width, uint32_t height) {
        EXPECT_EQ(rgba[0], 0);
        EXPECT_EQ(rgba[1], 0xff);
        EXPECT_EQ(rgba[2], 0);
        EXPECT_EQ(rgba[3], 0xff);
        callbackCalled = true;
    });
    EXPECT_TRUE(callbackCalled);
}

TEST_F(RenderingTest, VisibleRenderableCount) {
    EXPECT_EQ(mView->getVisibleRenderableCount(), -1);

    View* viewB = mEngine->createView();
    viewB->setViewport({0, 0, 16, 16});
    viewB->setScene(mScene);
    viewB->setCamera(mCamera);
    viewB->setPostProcessingEnabled(false);

    EXPECT_EQ(viewB->getVisibleRenderableCount(), -1);

    bool callbackCalled = false;
    runTest([this, viewB, &callbackCalled](uint8_t const*, uint32_t, uint32_t) {
        EXPECT_EQ(mView->getVisibleRenderableCount(), 1);
        EXPECT_EQ(viewB->getVisibleRenderableCount(), -1);
        callbackCalled = true;
    });
    EXPECT_TRUE(callbackCalled);

    mView->setScene(nullptr);
    EXPECT_EQ(mView->getVisibleRenderableCount(), -1);

    mEngine->destroy(viewB);
}
