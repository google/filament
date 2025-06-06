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

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>

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

#include <utils/EntityManager.h>

#include <viewer/ViewerGui.h>

#include <camutils/Manipulator.h>

#include <getopt/getopt.h>

#include <math/half.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <math/mat3.h>
#include <math/norm.h>

#include <imgui.h>

#include <imageio/ImageDecoder.h>

#include <fstream>
#include <iostream>
#include <string>

#include "generated/resources/resources.h"

using namespace filament;
using namespace filament::math;
using namespace filament::viewer;

using namespace image;
using namespace utils;

struct App {
    Engine* engine;
    ViewerGui* viewer;
    Config config;
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

static void printUsage(char* name) {
    std::string exec_name(Path(name).getName());
    std::string usage(
        "IMAGE_VIEWER displays the specified image\n"
        "Usage:\n"
        "    IMAGE_VIEWER [options] <image path>\n"
        "Options:\n"
        "   --help, -h\n"
        "       Prints this message\n\n"
        "API_USAGE"
        "   --camera=<camera mode>, -c <camera mode>\n"
        "       Set the camera mode: orbit (default) or flight\n"
        "       Flight mode uses the following controls:\n"
        "           Click and drag the mouse to pan the camera\n"
        "           Use the scroll weel to adjust movement speed\n"
        "           W / S: forward / backward\n"
        "           A / D: left / right\n"
        "           E / Q: up / down\n\n"
    );
    const std::string from("IMAGE_VIEWER");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    const std::string apiUsage("API_USAGE");
    for (size_t pos = usage.find(apiUsage); pos != std::string::npos; pos = usage.find(apiUsage, pos)) {
        usage.replace(pos, apiUsage.length(), samples::getBackendAPIArgumentsUsage());
    }
    std::cout << usage;
}

static int handleCommandLineArguments(int argc, char* argv[], App* app) {
    static constexpr const char* OPTSTR = "ha:c:";
    static const struct option OPTIONS[] = {
        { "help",         no_argument,       nullptr, 'h' },
        { "api",          required_argument, nullptr, 'a' },
        { "camera",       required_argument, nullptr, 'c' },
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
                app->config.backend = samples::parseArgumentsForBackend(arg);
                break;
            case 'c':
                if (arg == "flight") {
                    app->config.cameraMode = camutils::Mode::FREE_FLIGHT;
                } else if (arg == "orbit") {
                    app->config.cameraMode = camutils::Mode::ORBIT;
                } else {
                    std::cerr << "Unrecognized camera mode. Must be 'flight'|'orbit'.\n";
                }
                break;
        }
    }
    return optind;
}

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
            .format(channels == 3 ?
                    Texture::InternalFormat::RGB16F : Texture::InternalFormat::RGBA16F)
            .sampler(Texture::Sampler::SAMPLER_2D)
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

int main(int argc, char** argv) {
    App app;

    app.config.title = "Filament Image Viewer";

    int optionIndex = handleCommandLineArguments(argc, argv, &app);

    Path filename;
    int num_args = argc - optionIndex;
    if (num_args >= 1) {
        filename = argv[optionIndex];
    }

    auto setup = [&](Engine* engine, View* view, Scene* scene) {
        app.engine = engine;
        app.viewer = new ViewerGui(engine, scene, view, 410);
        app.viewer->getSettings().viewer.autoScaleEnabled = false;
        app.viewer->getSettings().viewer.autoInstancingEnabled = true;
        app.viewer->getSettings().view.bloom.enabled = false;
        app.viewer->getSettings().view.ssao.enabled = false;
        app.viewer->getSettings().view.dithering = Dithering::NONE;
        app.viewer->getSettings().view.antiAliasing = AntiAliasing::NONE;

        createImageRenderable(engine, scene, app);

        loadImage(app, engine, filename);

        app.viewer->setUiCallback([&app] () {
            if (ImGui::CollapsingHeader("Image", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::ColorEdit3("Background color", &app.backgroundColor.r);
            }
        });
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        engine->destroy(app.scene.imageEntity);
        engine->destroy(app.scene.imageVertexBuffer);
        engine->destroy(app.scene.imageIndexBuffer);
        engine->destroy(app.scene.imageMaterial);
        engine->destroy(app.scene.imageTexture);
        engine->destroy(app.scene.defaultTexture);
        engine->destroy(app.colorGrading);

        delete app.viewer;
    };

    auto gui = [&app](Engine* engine, View* view) {
        app.viewer->updateUserInterface();

        FilamentApp::get().setSidebarWidth(app.viewer->getSidebarWidth());
    };

    auto preRender = [&app](Engine* engine, View* view, Scene* scene, Renderer* renderer) {
        auto& rcm = engine->getRenderableManager();

        // This applies clear options, the skybox mask, and some camera settings.
        Camera& camera = view->getCamera();
        Skybox* skybox = scene->getSkybox();
        applySettings(engine, app.viewer->getSettings().viewer, &camera, skybox, renderer);

        // Check if color grading has changed.
        ColorGradingSettings& options = app.viewer->getSettings().view.colorGrading;
        if (options.enabled) {
            if (options != app.lastColorGradingOptions) {
                ColorGrading *colorGrading = createColorGrading(options, engine);
                engine->destroy(app.colorGrading);
                app.colorGrading = colorGrading;
                app.lastColorGradingOptions = options;
            }
            view->setColorGrading(app.colorGrading);
        } else {
            view->setColorGrading(nullptr);
        }

        if (app.showImage) {
            Texture *texture = app.scene.imageTexture;
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

            app.scene.imageMaterial->setDefaultParameter("transform", transform);
            app.scene.imageMaterial->setDefaultParameter(
                    "image", app.scene.imageTexture, app.scene.sampler);
        } else {
            app.scene.imageMaterial->setDefaultParameter(
                    "image", app.scene.defaultTexture, app.scene.sampler);
        }

        app.scene.imageMaterial->setDefaultParameter("showImage", app.showImage ? 1 : 0);
        app.scene.imageMaterial->setDefaultParameter(
                "backgroundColor", RgbType::sRGB, app.backgroundColor);
    };

    FilamentApp& filamentApp = FilamentApp::get();

    filamentApp.setDropHandler([&] (std::string_view path) {
        loadImage(app, app.engine, Path(path));
    });

    filamentApp.run(app.config, setup, cleanup, gui, preRender);

    return 0;
}
