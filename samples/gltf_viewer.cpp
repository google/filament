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

#define GLTFIO_SIMPLEVIEWER_IMPLEMENTATION

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>
#include <filamentapp/IBL.h>

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filament/Renderer.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/SimpleViewer.h>

#include <camutils/Manipulator.h>

#include <getopt/getopt.h>

#include <utils/NameComponentManager.h>

#include <math/vec3.h>
#include <math/vec4.h>
#include <math/mat3.h>
#include <math/norm.h>

#include <fstream>
#include <iostream>
#include <string>

#include "generated/resources/gltf_viewer.h"

using namespace filament;
using namespace filament::math;
using namespace gltfio;
using namespace utils;

struct App {
    Engine* engine;
    SimpleViewer* viewer;
    Config config;

    AssetLoader* loader;
    FilamentAsset* asset = nullptr;
    NameComponentManager* names;

    MaterialProvider* materials;
    MaterialSource materialSource = GENERATE_SHADERS;

    gltfio::ResourceLoader* resourceLoader = nullptr;

    bool actualSize = false;

    struct ViewOptions {
        float cameraAperture = 16.0f;
        float cameraSpeed = 125.0f;
        float cameraISO = 100.0f;
        float groundShadowStrength = 0.75f;
        bool groundPlaneEnabled = false;
        bool skyboxEnabled = true;
        sRGBColor backgroundColor = { 0.0f };
    } viewOptions;

    View::DepthOfFieldOptions dofOptions;

    struct Scene {
        Entity groundPlane;
        VertexBuffer* groundVertexBuffer;
        IndexBuffer* groundIndexBuffer;
        Material* groundMaterial;
    } scene;
};

static const char* DEFAULT_IBL = "venetian_crossroads_2k";

static void printUsage(char* name) {
    std::string exec_name(Path(name).getName());
    std::string usage(
        "SHOWCASE renders the specified glTF file, or a built-in file if none is specified\n"
        "Usage:\n"
        "    SHOWCASE [options] <gltf path>\n"
        "Options:\n"
        "   --help, -h\n"
        "       Prints this message\n\n"
        "   --api, -a\n"
        "       Specify the backend API: opengl (default), vulkan, or metal\n\n"
        "   --ibl=<path to cmgen IBL>, -i <path>\n"
        "       Override the built-in IBL\n\n"
        "   --actual-size, -s\n"
        "       Do not scale the model to fit into a unit cube\n\n"
        "   --ubershader, -u\n"
        "       Enable ubershaders (improves load time, adds shader complexity)\n\n"
        "   --camera=<camera mode>, -c <camera mode>\n"
        "       Set the camera mode: orbit (default) or flight\n"
        "       Flight mode uses the following controls:\n"
        "           Click and drag the mouse to pan the camera\n"
        "           Use the scroll weel to adjust movement speed\n"
        "           W / S: forward / backward\n"
        "           A / D: left / right\n"
        "           E / Q: up / down\n"
    );
    const std::string from("SHOWCASE");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    std::cout << usage;
}

static int handleCommandLineArguments(int argc, char* argv[], App* app) {
    static constexpr const char* OPTSTR = "ha:i:usc:";
    static const struct option OPTIONS[] = {
        { "help",         no_argument,       nullptr, 'h' },
        { "api",          required_argument, nullptr, 'a' },
        { "ibl",          required_argument, nullptr, 'i' },
        { "ubershader",   no_argument,       nullptr, 'u' },
        { "actual-size",  no_argument,       nullptr, 's' },
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
                if (arg == "opengl") {
                    app->config.backend = Engine::Backend::OPENGL;
                } else if (arg == "vulkan") {
                    app->config.backend = Engine::Backend::VULKAN;
                } else if (arg == "metal") {
                    app->config.backend = Engine::Backend::METAL;
                } else {
                    std::cerr << "Unrecognized backend. Must be 'opengl'|'vulkan'|'metal'.\n";
                }
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
            case 'i':
                app->config.iblDirectory = arg;
                break;
            case 'u':
                app->materialSource = LOAD_UBERSHADERS;
                break;
            case 's':
                app->actualSize = true;
                break;
        }
    }
    return optind;
}

static std::ifstream::pos_type getFileSize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

static void createGroundPlane(Engine* engine, Scene* scene, App& app) {
    auto& em = EntityManager::get();
    Material* shadowMaterial = Material::Builder()
            .package(GLTF_VIEWER_GROUNDSHADOW_DATA, GLTF_VIEWER_GROUNDSHADOW_SIZE)
            .build(*engine);
    shadowMaterial->setDefaultParameter("strength", app.viewOptions.groundShadowStrength);

    const static uint32_t indices[] = {
            0, 1, 2, 2, 3, 0
    };

    Aabb aabb = app.asset->getBoundingBox();
    if (!app.actualSize) {
        mat4f transform = fitIntoUnitCube(aabb);
        aabb = aabb.transform(transform);
    }

    float3 planeExtent{10.0f * aabb.extent().x, 0.0f, 10.0f * aabb.extent().z};

    const static float3 vertices[] = {
            { -planeExtent.x, 0, -planeExtent.z },
            { -planeExtent.x, 0,  planeExtent.z },
            {  planeExtent.x, 0,  planeExtent.z },
            {  planeExtent.x, 0, -planeExtent.z },
    };

    short4 tbn = packSnorm16(
            mat3f::packTangentFrame(
                    mat3f{
                            float3{ 1.0f, 0.0f, 0.0f },
                            float3{ 0.0f, 0.0f, 1.0f },
                            float3{ 0.0f, 1.0f, 0.0f }
                    }
            ).xyzw);

    const static short4 normals[] { tbn, tbn, tbn, tbn };

    VertexBuffer* vertexBuffer = VertexBuffer::Builder()
            .vertexCount(4)
            .bufferCount(2)
            .attribute(VertexAttribute::POSITION,
                    0, VertexBuffer::AttributeType::FLOAT3)
            .attribute(VertexAttribute::TANGENTS,
                    1, VertexBuffer::AttributeType::SHORT4)
            .normalized(VertexAttribute::TANGENTS)
            .build(*engine);

    vertexBuffer->setBufferAt(*engine, 0, VertexBuffer::BufferDescriptor(
            vertices, vertexBuffer->getVertexCount() * sizeof(vertices[0])));
    vertexBuffer->setBufferAt(*engine, 1, VertexBuffer::BufferDescriptor(
            normals, vertexBuffer->getVertexCount() * sizeof(normals[0])));

    IndexBuffer* indexBuffer = IndexBuffer::Builder()
            .indexCount(6)
            .build(*engine);

    indexBuffer->setBuffer(*engine, IndexBuffer::BufferDescriptor(
            indices, indexBuffer->getIndexCount() * sizeof(uint32_t)));

    Entity groundPlane = em.create();
    RenderableManager::Builder(1)
            .boundingBox({
                { -planeExtent.x, 0, -planeExtent.z },
                { planeExtent.x, 1e-4f, planeExtent.z }
            })
            .material(0, shadowMaterial->getDefaultInstance())
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES,
                    vertexBuffer, indexBuffer, 0, 6)
            .culling(false)
            .receiveShadows(true)
            .castShadows(false)
            .build(*engine, groundPlane);

    scene->addEntity(groundPlane);

    auto& tcm = engine->getTransformManager();
    tcm.setTransform(tcm.getInstance(groundPlane),
            mat4f::translation(float3{ 0, aabb.min.y, -4 }));

    auto& rcm = engine->getRenderableManager();
    auto instance = rcm.getInstance(groundPlane);
    rcm.setLayerMask(instance, 0xff, 0x00);

    app.scene.groundPlane = groundPlane;
    app.scene.groundVertexBuffer = vertexBuffer;
    app.scene.groundIndexBuffer = indexBuffer;
    app.scene.groundMaterial = shadowMaterial;
}

static LinearColor inverseTonemapSRGB(sRGBColor x) {
    return (x * -0.155) / (x - 1.019);
}

int main(int argc, char** argv) {
    App app;

    app.config.title = "Filament";
    app.config.iblDirectory = FilamentApp::getRootAssetsPath() + DEFAULT_IBL;

    int optionIndex = handleCommandLineArguments(argc, argv, &app);
    utils::Path filename;
    int num_args = argc - optionIndex;
    if (num_args >= 1) {
        filename = argv[optionIndex];
        if (!filename.exists()) {
            std::cerr << "file " << filename << " not found!" << std::endl;
            return 1;
        }
        if (filename.isDirectory()) {
            auto files = filename.listContents();
            for (auto file : files) {
                if (file.getExtension() == "gltf" || file.getExtension() == "glb") {
                    filename = file;
                    break;
                }
            }
            if (filename.isDirectory()) {
                std::cerr << "no glTF file found in " << filename << std::endl;
                return 1;
            }
        }
    }

    auto loadAsset = [&app](utils::Path filename) {
        // Peek at the file size to allow pre-allocation.
        long contentSize = static_cast<long>(getFileSize(filename.c_str()));
        if (contentSize <= 0) {
            std::cerr << "Unable to open " << filename << std::endl;
            exit(1);
        }

        // Consume the glTF file.
        std::ifstream in(filename.c_str(), std::ifstream::binary | std::ifstream::in);
        std::vector<uint8_t> buffer(static_cast<unsigned long>(contentSize));
        if (!in.read((char*) buffer.data(), contentSize)) {
            std::cerr << "Unable to read " << filename << std::endl;
            exit(1);
        }

        // Parse the glTF file and create Filament entities.
        if (filename.getExtension() == "glb") {
            app.asset = app.loader->createAssetFromBinary(buffer.data(), buffer.size());
        } else {
            app.asset = app.loader->createAssetFromJson(buffer.data(), buffer.size());
        }
        buffer.clear();
        buffer.shrink_to_fit();

        if (!app.asset) {
            std::cerr << "Unable to parse " << filename << std::endl;
            exit(1);
        }
    };

    auto loadResources = [&app] (utils::Path filename) {
        // Load external textures and buffers.
        std::string gltfPath = filename.getAbsolutePath();
        ResourceConfiguration configuration;
        configuration.engine = app.engine;
        configuration.gltfPath = gltfPath.c_str();
        configuration.normalizeSkinningWeights = true;
        configuration.recomputeBoundingBoxes = false;
        if (!app.resourceLoader) {
            app.resourceLoader = new gltfio::ResourceLoader(configuration);
        }
        app.resourceLoader->asyncBeginLoad(app.asset);

        // Load animation data then free the source hierarchy.
        app.asset->getAnimator();
        app.asset->releaseSourceData();

        auto ibl = FilamentApp::get().getIBL();
        if (ibl) {
            app.viewer->setIndirectLight(ibl->getIndirectLight(), ibl->getSphericalHarmonics());
        }
    };

    auto setup = [&](Engine* engine, View* view, Scene* scene) {
        app.engine = engine;
        app.names = new NameComponentManager(EntityManager::get());
        app.viewer = new SimpleViewer(engine, scene, view);
        app.materials = (app.materialSource == GENERATE_SHADERS) ?
                createMaterialGenerator(engine) : createUbershaderLoader(engine);
        app.loader = AssetLoader::create({engine, app.materials, app.names });
        if (filename.isEmpty()) {
            app.asset = app.loader->createAssetFromBinary(
                    GLTF_VIEWER_DAMAGEDHELMET_DATA,
                    GLTF_VIEWER_DAMAGEDHELMET_SIZE);
        } else {
            loadAsset(filename);
        }

        loadResources(filename);

        createGroundPlane(engine, scene, app);

        app.viewer->setUiCallback([&app, scene] () {
            float progress = app.resourceLoader->asyncGetLoadProgress();
            if (progress < 1.0) {
                ImGui::ProgressBar(progress);
            }

            if (ImGui::CollapsingHeader("Stats")) {
                ImGui::Text("%zu entities in the asset", app.asset->getEntityCount());
                ImGui::Text("%zu renderables (excluding UI)", scene->getRenderableCount());
                ImGui::Text("%zu skipped frames", FilamentApp::get().getSkippedFrameCount());
            }

            if (ImGui::CollapsingHeader("Scene")) {
                ImGui::Checkbox("Show skybox", &app.viewOptions.skyboxEnabled);
                ImGui::ColorEdit3("Background color", &app.viewOptions.backgroundColor.r);
                ImGui::Checkbox("Ground shadow", &app.viewOptions.groundPlaneEnabled);
                ImGui::Indent();
                ImGui::SliderFloat("Strength", &app.viewOptions.groundShadowStrength, 0.0f, 1.0f);
                ImGui::Unindent();
            }

            if (ImGui::CollapsingHeader("Camera")) {
                ImGui::SliderFloat("Focal length (mm)", &FilamentApp::get().getCameraFocalLength(), 16.0f, 90.0f);
                ImGui::SliderFloat("Aperture", &app.viewOptions.cameraAperture, 1.0f, 32.0f);
                ImGui::SliderFloat("Speed (1/s)", &app.viewOptions.cameraSpeed, 1000.0f, 1.0f);
                ImGui::SliderFloat("ISO", &app.viewOptions.cameraISO, 25.0f, 6400.0f);
                ImGui::Checkbox("DoF", &app.dofOptions.enabled);
                ImGui::SliderFloat("Focus distance", &app.dofOptions.focusDistance, 0.0f, 30.0f);
                ImGui::SliderFloat("Blur scale", &app.dofOptions.blurScale, 0.1f, 10.0f);
            }
        });

        // Leave FXAA enabled but we also enable MSAA for a nice result. The wireframe looks
        // much better with MSAA enabled.
        view->setSampleCount(4);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        app.loader->destroyAsset(app.asset);
        app.materials->destroyMaterials();

        engine->destroy(app.scene.groundPlane);
        engine->destroy(app.scene.groundVertexBuffer);
        engine->destroy(app.scene.groundIndexBuffer);
        engine->destroy(app.scene.groundMaterial);

        delete app.viewer;
        delete app.materials;
        delete app.names;

        AssetLoader::destroy(&app.loader);
    };

    auto animate = [&app](Engine* engine, View* view, double now) {
        app.resourceLoader->asyncUpdateLoad();

        // Add renderables to the scene as they become ready.
        app.viewer->populateScene(app.asset, !app.actualSize);

        app.viewer->applyAnimation(now);
    };

    auto gui = [&app](Engine* engine, View* view) {
        app.viewer->updateUserInterface();

        FilamentApp::get().setSidebarWidth(app.viewer->getSidebarWidth());
    };

    auto preRender = [&app](Engine* engine, View* view, Scene* scene, Renderer* renderer) {
        auto& rcm = engine->getRenderableManager();
        auto instance = rcm.getInstance(app.scene.groundPlane);
        rcm.setLayerMask(instance,
                0xff, app.viewOptions.groundPlaneEnabled ? 0xff : 0x00);

        Camera& camera = view->getCamera();
        camera.setExposure(
                app.viewOptions.cameraAperture,
                1.0f / app.viewOptions.cameraSpeed,
                app.viewOptions.cameraISO);

        view->setDepthOfFieldOptions(app.dofOptions);

        app.scene.groundMaterial->setDefaultParameter(
                "strength", app.viewOptions.groundShadowStrength);

        auto ibl = FilamentApp::get().getIBL();
        if (ibl) {
            ibl->getSkybox()->setLayerMask(0xff, app.viewOptions.skyboxEnabled ? 0xff : 0x00);
        }

        // we have to clear because the side-bar doesn't have a background, we cannot use
        // a skybox on the ui scene, because the ui view is always full screen.
        renderer->setClearOptions({
                .clearColor = { inverseTonemapSRGB(app.viewOptions.backgroundColor), 1.0f },
                .clear = true  });
    };

    FilamentApp& filamentApp = FilamentApp::get();
    filamentApp.animate(animate);

    filamentApp.setDropHandler([&] (std::string path) {
        app.viewer->removeAsset();
        app.loader->destroyAsset(app.asset);
        loadAsset(path);
        loadResources(path);
    });

    filamentApp.run(app.config, setup, cleanup, gui, preRender);

    return 0;
}
