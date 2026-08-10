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

#include "common/arguments.h"
#include "common/SampleConfig.h"

#include "generated/resources/resources.h"

#include <viewer/ViewerGui.h>

#include <imageio/ImageDecoder.h>

#include <filamentapp/AssetLoader.h>
#include <filamentapp/FilamentApp2.h>

#include <filament/Camera.h>
#include <filament/ColorGrading.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <camutils/Manipulator.h>

#include <utils/EntityManager.h>
#include <utils/getopt.h>

#include <math/half.h>
#include <math/mat3.h>
#include <math/norm.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <imgui.h>

#include <fstream>
#include <iostream>
#include <string>

using namespace filament;
using namespace filament::math;
using namespace filament::viewer;

using namespace image;
using namespace utils;

struct App {
    FilamentApp2* filamentApp;
    Engine* engine;
    ViewerGui* viewer;
    SampleConfig config;
    Camera* mainCamera;

    struct Scene {
        Entity imageEntity;
        VertexBuffer* imageVertexBuffer = nullptr;
        IndexBuffer* imageIndexBuffer = nullptr;
        Material* imageMaterial = nullptr;
        Texture* imageTexture = nullptr;
        Texture* defaultTexture = nullptr;
        TextureSampler sampler;
    } scene;

    bool showImage = false;
    float3 backgroundColor = float3(0.0f);

    ColorGradingSettings lastColorGradingOptions = { .enabled = false };

    ColorGrading* colorGrading = nullptr;
};

static constexpr float4 sFullScreenTriangleVertices[3] = {
        { -1.0f, -1.0f, 1.0f, 1.0f },
        {  3.0f, -1.0f, 1.0f, 1.0f },
        { -1.0f,  3.0f, 1.0f, 1.0f }
};

static const uint16_t sFullScreenTriangleIndices[3] = { 0, 1, 2 };

static void createImageRenderable(Engine* engine, Scene* scene, App& app) {
    auto& em = EntityManager::get();
    Material* material = Material::Builder()
            .package(RESOURCES_IMAGE_DATA, RESOURCES_IMAGE_SIZE)
            .build(*engine);

    VertexBuffer* vertexBuffer = VertexBuffer::Builder()
            .vertexCount(3)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT4, 0)
            .build(*engine);

    vertexBuffer->setBufferAt(
            *engine, 0, { sFullScreenTriangleVertices, sizeof(sFullScreenTriangleVertices) });

    IndexBuffer* indexBuffer = IndexBuffer::Builder()
            .indexCount(3)
            .bufferType(IndexBuffer::IndexType::USHORT)
            .build(*engine);

    indexBuffer->setBuffer(*engine,
            { sFullScreenTriangleIndices, sizeof(sFullScreenTriangleIndices) });

    Entity imageEntity = em.create();
    RenderableManager::Builder(1)
            .boundingBox({{}, {1.0f, 1.0f, 1.0f}})
            .material(0, material->getDefaultInstance())
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vertexBuffer, indexBuffer, 0, 3)
            .culling(false)
            .castShadows(false)
            .receiveShadows(false)
            .build(*engine, imageEntity);

    scene->addEntity(imageEntity);

    app.scene.imageEntity = imageEntity;
    app.scene.imageVertexBuffer = vertexBuffer;
    app.scene.imageIndexBuffer = indexBuffer;
    app.scene.imageMaterial = material;

    Texture* texture = Texture::Builder()
                               .width(1)
                               .height(1)
                               .levels(1)
                               .format(Texture::InternalFormat::RGBA8)
                               .sampler(Texture::Sampler::SAMPLER_2D)
                               .build(*engine);
    static uint32_t pixel = 0;
    Texture::PixelBufferDescriptor buffer(&pixel, 4, Texture::Format::RGBA, Texture::Type::UBYTE);
    texture->setImage(*engine, 0, std::move(buffer));

    app.scene.defaultTexture = texture;
}

static void loadImage(App& app, Engine* engine, const Path& filename) {
    if (app.scene.imageTexture) {
        engine->destroy(app.scene.imageTexture);
        app.scene.imageTexture = nullptr;
    }

    if (!filename.exists()) {
        std::cerr << "The input image does not exist: " << filename << std::endl;
        app.showImage = false;
        return;
    }

    std::ifstream inputStream(filename, std::ios::binary);
    LinearImage* image = new LinearImage(ImageDecoder::decode(
            inputStream, filename, ImageDecoder::ColorSpace::SRGB));

    if (!image->isValid()) {
        std::cerr << "The input image is invalid: " << filename << std::endl;
        app.showImage = false;
        return;
    }

    inputStream.close();

    uint32_t channels = image->getChannels();
    uint32_t w = image->getWidth();
    uint32_t h = image->getHeight();
    Texture* texture = Texture::Builder()
                               .width(w)
                               .height(h)
                               .levels(0xff)
                               .format(channels == 3 ? Texture::InternalFormat::RGB16F
                                                     : Texture::InternalFormat::RGBA16F)
                               .sampler(Texture::Sampler::SAMPLER_2D)
                               .usage(Texture::Usage::DEFAULT | Texture::Usage::GEN_MIPMAPPABLE)
                               .build(*engine);

    Texture::PixelBufferDescriptor::Callback freeCallback = [](void* buf, size_t, void* data) {
        delete reinterpret_cast<LinearImage*>(data);
    };

    Texture::PixelBufferDescriptor buffer(
            image->getPixelRef(),
            size_t(w * h * channels * sizeof(float)),
            channels == 3 ? Texture::Format::RGB : Texture::Format::RGBA,
            Texture::Type::FLOAT,
            freeCallback,
            image
    );

    texture->setImage(*engine, 0, std::move(buffer));
    texture->generateMipmaps(*engine);

    app.scene.sampler.setMagFilter(TextureSampler::MagFilter::LINEAR);
    app.scene.sampler.setMinFilter(TextureSampler::MinFilter::LINEAR_MIPMAP_LINEAR);
    app.scene.sampler.setWrapModeS(TextureSampler::WrapMode::REPEAT);
    app.scene.sampler.setWrapModeT(TextureSampler::WrapMode::REPEAT);

    app.scene.imageTexture = texture;
    app.showImage = true;
}

std::unique_ptr<FilamentApp2> createSampleApp(SampleConfig config,
        filament::app::DisplayManager* dm, filament::app::AssetLoader* loader) {
    auto app = std::make_shared<App>();
    app->config = config;

    Path filename;
    if (config.customArgs.find("filename") != config.customArgs.end()) {
        filename = Path(config.customArgs["filename"].c_str());
    }

    auto setup = [app, filename](Engine* engine, View* view, Scene* scene) {
        app->engine = engine;
        app->viewer = new ViewerGui(engine, scene, view, 410);
        app->viewer->getSettings().viewer.autoScaleEnabled = false;
        app->viewer->getSettings().viewer.autoInstancingEnabled = true;
        app->viewer->getSettings().view.bloom.enabled = false;
        app->viewer->getSettings().view.ssao.enabled = false;
        app->viewer->getSettings().view.dithering = Dithering::NONE;
        app->viewer->getSettings().view.antiAliasing = AntiAliasing::NONE;

        createImageRenderable(engine, scene, *app);

        loadImage(*app, engine, filename);

        app->viewer->setUiCallback([app]() {
            if (ImGui::CollapsingHeader("Image", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::ColorEdit3("Background color", &app->backgroundColor.r);
            }
        });
    };

    auto cleanup = [app](Engine* engine, View*, Scene*) {
        engine->destroy(app->scene.imageEntity);
        engine->destroy(app->scene.imageVertexBuffer);
        engine->destroy(app->scene.imageIndexBuffer);
        engine->destroy(app->scene.imageMaterial);
        engine->destroy(app->scene.imageTexture);
        engine->destroy(app->scene.defaultTexture);
        engine->destroy(app->colorGrading);

        delete app->viewer;
    };

    auto gui = [app](Engine* engine, View* view) {
        app->viewer->updateUserInterface();

        app->filamentApp->setSidebarWidth(app->viewer->getSidebarWidth());
    };

    auto preRender = [app](Engine* engine, View* view, Scene* scene, Renderer* renderer) {
        auto& rcm = engine->getRenderableManager();

        // This applies clear options, the skybox mask, and some camera settings.
        Camera& camera = view->getCamera();
        Skybox* skybox = scene->getSkybox();
        applySettings(engine, app->viewer->getSettings().viewer, &camera, skybox, renderer);

        // Check if color grading has changed.
        ColorGradingSettings& options = app->viewer->getSettings().view.colorGrading;
        if (options.enabled) {
            if (options != app->lastColorGradingOptions) {
                ColorGrading *colorGrading = createColorGrading(options, engine);
                engine->destroy(app->colorGrading);
                app->colorGrading = colorGrading;
                app->lastColorGradingOptions = options;
            }
            view->setColorGrading(app->colorGrading);
        } else {
            view->setColorGrading(nullptr);
        }

        if (app->showImage) {
            Texture* texture = app->scene.imageTexture;
            float srcWidth = (float) texture->getWidth();
            float srcHeight = (float) texture->getHeight();
            float dstWidth = (float) view->getViewport().width;
            float dstHeight = (float) view->getViewport().height;

            float srcRatio = srcWidth / srcHeight;
            float dstRatio = dstWidth / dstHeight;

            bool xMajor = dstWidth / srcWidth > dstHeight / srcHeight;

            float sx = 1.0f;
            float sy = dstRatio / srcRatio;

            float tx = 0.0f;
            float ty = ((1.0f - sy) * 0.5f) / sy;

            if (xMajor) {
                sx = srcRatio / dstRatio;
                sy = 1.0;
                tx = ((1.0f - sx) * 0.5f) / sx;
                ty = 0.0f;
            }

            mat3f transform(
                    1.0f / sx,  0.0f,       0.0f,
                    0.0f,       1.0f / sy,  0.0f,
                    -tx,        -ty,         1.0f
            );

            app->scene.imageMaterial->setDefaultParameter("transform", transform);
            app->scene.imageMaterial->setDefaultParameter("image", app->scene.imageTexture,
                    app->scene.sampler);
        } else {
            app->scene.imageMaterial->setDefaultParameter("image", app->scene.defaultTexture,
                    app->scene.sampler);
        }

        app->scene.imageMaterial->setDefaultParameter("showImage", app->showImage ? 1 : 0);
        app->scene.imageMaterial->setDefaultParameter("backgroundColor", RgbType::sRGB,
                app->backgroundColor);
    };
    auto fApp = FilamentApp2::Builder()
                        .displayManager(dm)
                        .title(app->config.title)
                        .backend(app->config.backend)
                        .cameraMode(app->config.cameraMode)
                        .configDisplayManager(static_cast<FilamentApp2::DisplayManager>(
                                app->config.displayManager))
                        .setup(setup)
                        .cleanup(cleanup)
                        .imgui(gui)
                        .preRender(preRender)
                        .dropHandler([app](std::string_view path) {
                            loadImage(*app, app->engine, Path(path));
                        })
                        .build();
    app->filamentApp = fApp.get();

    return fApp;
}

#ifndef __ANDROID__
int main(int argc, char** argv) {
    SampleConfig config;
    config.title = "Filament Image Viewer";
    int optind = samples::handleCommandLineArguments(argc, argv, &config);
    auto dm = samples::getDisplayManager(config);
    int num_args = argc - optind;
    if (num_args >= 1) {
        config.customArgs["filename"] = utils::CString(argv[optind]);
    }
    auto app = createSampleApp(config, dm.get(), nullptr);
    app->run();
    return 0;
}
#endif
