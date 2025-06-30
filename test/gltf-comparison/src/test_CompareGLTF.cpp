/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include <gtest/gtest.h>

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>
#include <filamentapp/IBL.h>

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <utils/EntityManager.h>

#include "absl/strings/str_format.h"

#include "ImageExpectations.h"
#include "RunOnMain.h"

#include <atomic>
#include <optional>

using namespace filament;
using utils::Entity;
using utils::EntityManager;

struct App {
    Config config;
    VertexBuffer* vb;
    IndexBuffer* ib;
    Material* mat;
    Camera* cam;
    Entity camera;
    Skybox* skybox;
    Entity renderable;
};

class CompareGLTFContext {
public:
    CompareGLTFContext();
    ~CompareGLTFContext();

    using Command = std::function<void(Engine*, View*, Scene*, Renderer*)>;

    void postRender(Engine* engine, View* view, Scene* scene, Renderer* renderer);

    void addPostRenderCommand(Command command);

    App mApp;

    CrossThreadTask<void(Engine*, View*, Scene*, Renderer*), Engine*, View*, Scene*, Renderer*>
            mPostRenderCommand;
    std::atomic<bool> mStarted = false;
};

class CompareGLTFTest : public testing::TestWithParam<std::string> {
public:
    test::ImageExpectations mExpectations;

    static void SetUpTestSuite();
    void SetUp() override;
    static void TearDownTestSuite();

    static std::optional<CompareGLTFContext> sContext;
    std::atomic<bool> mTestFinished;
};


std::optional<CompareGLTFContext> CompareGLTFTest::sContext;

void CompareGLTFTest::SetUpTestSuite() {
    sContext.emplace();
}

void CompareGLTFTest::SetUp() {
    while (!sContext->mStarted) {}
}

void CompareGLTFTest::TearDownTestSuite() {
    sContext.reset();
}

CompareGLTFContext::CompareGLTFContext() {
    auto setup = [this](Engine* engine, View* view, Scene* scene) {
      mApp.skybox = Skybox::Builder().color({ 0.1, 0.125, 0.25, 1.0 }).build(*engine);
      scene->setSkybox(mApp.skybox);
      view->setPostProcessingEnabled(false);
      mApp.camera = utils::EntityManager::get().create();
      mApp.cam = engine->createCamera(mApp.camera);
      view->setCamera(mApp.cam);
    };

    auto cleanup = [this](Engine* engine, View*, Scene*) {
      engine->destroy(mApp.skybox);
      engine->destroyCameraComponent(mApp.camera);
      utils::EntityManager::get().destroy(mApp.camera);
    };

    FilamentApp::get().animate([this](Engine* engine, View* view, double now) {
      constexpr float ZOOM = 1.5f;
      const uint32_t w = view->getViewport().width;
      const uint32_t h = view->getViewport().height;
      const float aspect = (float) w / h;
      mApp.cam->setProjection(Camera::Projection::ORTHO, -aspect * ZOOM, aspect * ZOOM, -ZOOM,
              ZOOM, 0, 1);
      auto& tcm = engine->getTransformManager();
      tcm.setTransform(tcm.getInstance(mApp.renderable),
              filament::math::mat4f::rotation(now, filament::math::float3{ 0, 0, 1 }));
    });

    mApp.config.backend = filament::Engine::Backend::METAL;

    addPostRenderCommand([this](Engine* engine, View* view, Scene* scene, Renderer* renderer){
      mStarted = true;
    });
    RunOnMain::sTask.queueTask([=,this]() {
        FilamentApp::get().run(mApp.config, setup, cleanup, FilamentApp::ImGuiCallback(),
                FilamentApp::PreRenderCallback(),
                [this](Engine* engine, View* view, Scene* scene, Renderer* renderer) {
                    postRender(engine, view, scene, renderer);
                });
    });
}

CompareGLTFContext::~CompareGLTFContext() {
    std::atomic<bool> appClosed = false;
    addPostRenderCommand([&, this](Engine* engine, View* view, Scene* scene, Renderer* renderer) {
        FilamentApp::get().close();
        appClosed = true;
    });

    while (!appClosed) {}
}

void CompareGLTFContext::addPostRenderCommand(CompareGLTFContext::Command command) {
    while (!mPostRenderCommand.queueTask(command)) {}
}

void CompareGLTFContext::postRender(Engine* engine, View* view, Scene* scene, Renderer* renderer) {
    mPostRenderCommand.runTask(engine, view, scene, renderer);
}

INSTANTIATE_TEST_SUITE_P(GLTFFiles, CompareGLTFTest, testing::Values("BoxTextured", "Box"),
        [](const testing::TestParamInfo<std::string>& info) { return info.param; });

TEST_P(CompareGLTFTest, Compare) {
    sContext->addPostRenderCommand([this](Engine*, View* view, Scene*, Renderer* renderer) {
        EXPECT_IMAGE(renderer, mExpectations,
                test::ScreenshotParams(512, 512, absl::StrFormat("GLTF_%s", GetParam().c_str())));

        std::cout << "postRender: " << GetParam() << std::endl;
        mTestFinished = true;
    });
    while (!mTestFinished) {}

    std::atomic<int> counter = 0;
    auto incrementCounter = [&](Engine*, View* view, Scene*, Renderer* renderer) {
        counter++;
    };
    while (counter < 3) {
        sContext->addPostRenderCommand(incrementCounter);
    }
}