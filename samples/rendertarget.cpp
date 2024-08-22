/*
 * Copyright (C) 2021 The Android Open Source Project
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
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/RenderTarget.h>
#include <filament/Scene.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>
#include <utils/NameComponentManager.h>
#include <utils/EntityManager.h>

#include <imgui.h>
#include <filagui/ImGuiExtensions.h>
#include <viewer/ViewerGui.h>
#include <filameshio/MeshReader.h>
#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>

#include <getopt/getopt.h>

#include <iostream>

#include "generated/resources/resources.h"
//#include "generated/resources/monkey.h"
#include "generated/resources/gltf_demo.h"

using namespace filament;
using namespace filamesh;
using namespace filament::math;
using namespace filament::viewer;
using namespace filament::gltfio;

#include <utils/Log.h>

struct Vertex {
    float3 position;
    float2 uv;
};

struct App {
    utils::Entity lightEntity;
    Material* meshMaterial;
    MaterialInstance* meshMatInstance;
    mat4f transform;

    Texture* offscreenColorTexture = nullptr;
    Texture* offscreenDepthTexture = nullptr;
    RenderTarget* offscreenRenderTarget = nullptr;
    View* offscreenView = nullptr;
    Scene* offscreenScene = nullptr;
    Camera* offscreenCamera = nullptr;

    Config config;

    utils::Entity quadEntity;
    VertexBuffer* quadVb = nullptr;
    IndexBuffer* quadIb = nullptr;
    Material* quadMaterial = nullptr;
    MaterialInstance* quadMatInstance = nullptr;

    MaterialProvider* materials;

    AssetLoader* assetLoader;
    ResourceLoader* resourceLoader;
    FilamentAsset* asset = nullptr;
    utils::NameComponentManager* names;
    gltfio::TextureProvider* stbDecoder = nullptr;
    gltfio::TextureProvider* ktxDecoder = nullptr;

    Engine* engine;

    ViewerGui* viewer;
    utils::Entity rootTransformEntity;
};

static const char* DEFAULT_IBL = "assets/ibl/lightroom_14b";

static void printUsage(char* name) {
    std::string exec_name(utils::Path(name).getName());
    std::string usage(
        "SHOWCASE renders suzanne with planar reflection\n"
        "Usage:\n"
        "    SHOWCASE [options]\n"
        "Options:\n"
        "   --help, -h\n"
        "       Prints this message\n\n"
        "   --api, -a\n"
        "       Specify the backend API: opengl (default), vulkan, or metal\n"
        "   --mode, -m\n"
        "       Specify the reflection mode: camera (default), or renderables\n\n"
    );
    const std::string from("SHOWCASE");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    std::cout << usage;
}

static int handleCommandLineArguments(int argc, char* argv[], App* app) {
    static constexpr const char* OPTSTR = "ha:m:";
    static const struct option OPTIONS[] = {
        { "help", no_argument,       nullptr, 'h' },
        { "api",  required_argument, nullptr, 'a' },
        { "mode", required_argument, nullptr, 'm' },
        { nullptr, 0, nullptr, 0 }
    };
    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, &option_index)) >= 0) {
        std::string arg(optarg ? optarg : "");
        switch (opt) {
            default:
            case 'h':
                printUsage(argv[0]);
                exit(0);
            case 'a':
                if (arg == "opengl") {
                    app->config.backend = Engine::Backend::OPENGL;
                } else if (arg == "vulkan") {
                    app->config.backend = Engine::Backend::VULKAN;
                } else if (arg == "metal") {
                    app->config.backend = Engine::Backend::METAL;
                } else {
                    std::cerr << "Unrecognized backend. Must be 'opengl'|'vulkan'|'metal'.\n";
                    exit(1);
                }
                break;
            case 'm':
                break;
        }
    }
    return optind;
}

int main(int argc, char** argv) {
    App app{};
    app.config.title = "rendertarget";
    app.config.iblDirectory = FilamentApp::getRootAssetsPath() + DEFAULT_IBL;
    handleCommandLineArguments(argc, argv, &app);

    auto gui = [&app](Engine*, View*) {
        app.viewer->updateUserInterface();
        FilamentApp::get().setSidebarWidth(app.viewer->getSidebarWidth());
    };

    auto loadResources = [&app] () {
        // Load external textures and buffers.
        std::string const gltfPath;
        ResourceConfiguration configuration = {};
        configuration.engine = app.engine;
        configuration.gltfPath = gltfPath.c_str();
        configuration.normalizeSkinningWeights = true;

        app.resourceLoader = new gltfio::ResourceLoader(configuration);
        app.stbDecoder = createStbProvider(app.engine);
        app.ktxDecoder = createKtx2Provider(app.engine);
        app.resourceLoader->addTextureProvider("image/png", app.stbDecoder);
        app.resourceLoader->addTextureProvider("image/jpeg", app.stbDecoder);
        app.resourceLoader->addTextureProvider("image/ktx2", app.ktxDecoder);

        if (!app.resourceLoader->loadResources(app.asset)) {
            std::cerr << "Unable to start loading resources" << std::endl;
            exit(1);
        }

        app.asset->getInstance()->recomputeBoundingBoxes();
        app.asset->releaseSourceData();

        // Enable stencil writes on all material instances.
        auto instance = app.asset->getInstance();
        const size_t matInstanceCount = instance->getMaterialInstanceCount();
        MaterialInstance* const* const instances = instance->getMaterialInstances();
        for (int mi = 0; mi < matInstanceCount; mi++) {
            instances[mi]->setStencilWrite(true);
            instances[mi]->setStencilOpDepthStencilPass(MaterialInstance::StencilOperation::INCR);
        }

        auto ibl = FilamentApp::get().getIBL();
        if (ibl) {
            app.viewer->setIndirectLight(ibl->getIndirectLight(), ibl->getSphericalHarmonics());
            app.viewer->getSettings().view.fogSettings.fogColorTexture = ibl->getFogTexture();
        }
    };

    auto setup = [&app, loadResources](Engine* engine, View* view, Scene* scene) {
        app.engine = engine;
        auto& em = utils::EntityManager::get();
        auto& tcm = engine->getTransformManager();
        auto& rcm = engine->getRenderableManager();
        
        // Offscreen view
        {
            // Instantiate offscreen render target.
            app.offscreenView = engine->createView();
            app.offscreenScene = engine->createScene();
            app.offscreenView->setScene(app.offscreenScene);

            app.viewer = new ViewerGui(engine, app.offscreenScene, app.offscreenView, 410);
            app.viewer->setUiCallback([&app, scene, view, engine]() {
                if (ImGui::CollapsingHeader("Debug")) {
                    auto& debug = engine->getDebugRegistry();
                    ImGui::Checkbox("Disable buffer padding",
                            debug.getPropertyAddress<bool>("d.renderer.disable_buffer_padding"));
                    ImGui::Checkbox("Disable sub-passes",
                            debug.getPropertyAddress<bool>("d.renderer.disable_subpasses"));
                }
            });
            Viewport vp = view->getViewport();
            vp.width = vp.width - app.viewer->getSidebarWidth();

            app.offscreenColorTexture =
                    Texture::Builder()
                            .width(vp.width)
                            .height(vp.height)
                            .levels(1)
                            .usage(Texture::Usage::COLOR_ATTACHMENT | Texture::Usage::SAMPLEABLE)
                            .format(Texture::InternalFormat::RGBA8)
                            .build(*engine);
            app.offscreenDepthTexture = Texture::Builder()
                                                .width(vp.width)
                                                .height(vp.height)
                                                .levels(1)
                                                .usage(Texture::Usage::DEPTH_ATTACHMENT)
                                                .format(Texture::InternalFormat::DEPTH32F)
                                                .build(*engine);
            app.offscreenRenderTarget = RenderTarget::Builder()
                                                .texture(RenderTarget::AttachmentPoint::COLOR,
                                                        app.offscreenColorTexture)
                                                .texture(RenderTarget::AttachmentPoint::DEPTH,
                                                        app.offscreenDepthTexture)
                                                .build(*engine);
            app.offscreenView->setRenderTarget(app.offscreenRenderTarget);
            app.offscreenView->setViewport({0, 0, vp.width, vp.height});
            app.offscreenCamera = engine->createCamera(em.create());
            app.offscreenView->setCamera(app.offscreenCamera);
            FilamentApp::get().addOffscreenView(app.offscreenView);
        }

        // damaged helmet gltf
        {
            app.names = new utils::NameComponentManager(em);
            app.materials = createJitShaderProvider(engine, true);
            app.assetLoader = AssetLoader::create({engine, app.materials, app.names});

            app.asset = app.assetLoader->createAsset(GLTF_DEMO_DAMAGEDHELMET_DATA,
                    GLTF_DEMO_DAMAGEDHELMET_SIZE);

            loadResources();

            app.offscreenScene->addEntities(app.asset->getLightEntities(),
                    app.asset->getLightEntityCount());
            static constexpr int kNumAvailable = 128;
            utils::Entity renderables[kNumAvailable];
            gltfio::FilamentAsset::SceneMask mask;
            mask.set(0);
            while (size_t numWritten = app.asset->popRenderables(renderables, kNumAvailable)) {
                app.asset->addEntitiesToScene(*app.offscreenScene, renderables, numWritten, mask);
            }

            app.rootTransformEntity = engine->getEntityManager().create();
            tcm.create(app.rootTransformEntity);
            TransformManager::Instance const& root = tcm.getInstance(app.rootTransformEntity);
            tcm.setParent(tcm.getInstance(app.asset->getRoot()), root);
            tcm.setTransform(root, mat4::translation(float3(0, 0, -4)));
        }

        // full-screen quad
        {
            static Vertex kQuadVertices[4] = {{{1, 1, 0}, {1, 1}}, {{1, -1, 0}, {1, 0}},
                {{-1, -1, 0}, {0, 0}}, {{-1, 1, 0}, {0, 1}}};
            // Create quad vertex buffer.
            static_assert(sizeof(Vertex) == 20, "Strange vertex size.");
            app.quadVb = VertexBuffer::Builder()
                                 .vertexCount(4)
                                 .bufferCount(1)
                                 .attribute(VertexAttribute::POSITION, 0,
                                         VertexBuffer::AttributeType::FLOAT3, 0, 20)
                                 .attribute(VertexAttribute::UV0, 0,
                                         VertexBuffer::AttributeType::FLOAT2, 12, 20)
                                 .build(*engine);
            app.quadVb->setBufferAt(*engine, 0,
                    VertexBuffer::BufferDescriptor(kQuadVertices, 80, nullptr));

            // Create quad index buffer.
            static constexpr uint16_t kQuadIndices[6] = {0, 2, 1, 2, 0, 3};
            app.quadIb = IndexBuffer::Builder()
                                 .indexCount(6)
                                 .bufferType(IndexBuffer::IndexType::USHORT)
                                 .build(*engine);
            app.quadIb->setBuffer(*engine,
                    IndexBuffer::BufferDescriptor(kQuadIndices, 12, nullptr));

            // Create quad material and renderable.
            // NOTE: this material is VertexDomain=device
            app.quadMaterial = Material::Builder()
                                       .package(RESOURCES_MIRROR_DATA, RESOURCES_MIRROR_SIZE)
                                       .build(*engine);
            app.quadMatInstance = app.quadMaterial->createInstance();
            TextureSampler sampler(TextureSampler::MinFilter::LINEAR,
                    TextureSampler::MagFilter::LINEAR);
            app.quadMatInstance->setParameter("albedo", app.offscreenColorTexture, sampler);
            app.quadEntity = em.create();
            RenderableManager::Builder(1)
                    .boundingBox({{-1, -1, -1}, {1, 1, 1}})
                    .material(0, app.quadMatInstance)
                    .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.quadVb,
                            app.quadIb, 0, 6)
                    .culling(false)
                    .receiveShadows(false)
                    .castShadows(false)
                    .build(*engine, app.quadEntity);
            scene->addEntity(app.quadEntity);
        }
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {

        auto& em = utils::EntityManager::get();
        auto camera = app.offscreenCamera->getEntity();
        engine->destroyCameraComponent(camera);
        em.destroy(camera);

        engine->destroy(app.lightEntity);
        engine->destroy(app.quadEntity);
        engine->destroy(app.meshMatInstance);
        engine->destroy(app.meshMaterial);
        engine->destroy(app.offscreenColorTexture);
        engine->destroy(app.offscreenDepthTexture);
        engine->destroy(app.offscreenRenderTarget);
        engine->destroy(app.offscreenScene);
        engine->destroy(app.offscreenView);
        engine->destroy(app.quadVb);
        engine->destroy(app.quadIb);
        engine->destroy(app.quadMatInstance);
        engine->destroy(app.quadMaterial);
    };

    auto preRender = [&app](Engine* engine, View* view, Scene*, Renderer* renderer) {
        renderer->setClearOptions({.clearColor = {0.1,0.2,0.4,1.0}, .clear = true});

        utils::slog.e <<"offscreen view=" << app.offscreenView << " primary view=" <<
                view << utils::io::endl;

        Camera const& camera = view->getCamera();
        const auto renderingProjection = camera.getProjectionMatrix();
        const auto cullingProjection = camera.getCullingProjectionMatrix();
        app.offscreenCamera->setCustomProjection(renderingProjection, cullingProjection,
                camera.getNear(), camera.getCullingFar());
        const auto model = camera.getModelMatrix();
        app.offscreenCamera->setModelMatrix(model);
    };

    FilamentApp::get().run(app.config, setup, cleanup, gui, preRender);

    return 0;
}
