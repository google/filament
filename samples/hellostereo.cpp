/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include <private/filament/EngineEnums.h>

#include <utils/EntityManager.h>

#include <filameshio/MeshReader.h>

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>

#include <getopt/getopt.h>

#include <iostream>
#include <vector>

#include "generated/resources/resources.h"
#include "generated/resources/monkey.h"

using namespace filament;
using namespace filamesh;
using namespace filament::math;

struct Vertex {
    float3 position;
    float2 uv;
};

struct App {
    Config config;

    Material* monkeyMaterial;
    MaterialInstance* monkeyMatInstance;
    MeshReader::Mesh monkeyMesh;
    mat4f monkeyTransform;
    utils::Entity lightEntity;

    View* stereoView = nullptr;
    Scene* stereoScene = nullptr;
    Camera* stereoCamera = nullptr;
    Texture* stereoColorTexture = nullptr;
    Texture* stereoDepthTexture = nullptr;
    RenderTarget* stereoRenderTarget = nullptr;

    VertexBuffer* quadVb = nullptr;
    IndexBuffer* quadIb = nullptr;
    Material* quadMaterial = nullptr;
    std::vector<utils::Entity> quadEntities;
    std::vector<MaterialInstance*> quadMatInstances;
};

static void printUsage(char* name) {
    std::string exec_name(utils::Path(name).getName());
    std::string usage(
        "SHOWCASE renders multiple quads displaying the contents of stereoscopic rendering\n"
        "Usage:\n"
        "    SHOWCASE [options]\n"
        "Options:\n"
        "   --help, -h\n"
        "       Prints this message\n\n"
        "   --api, -a\n"
        "       Specify the backend API: opengl (default), vulkan, metal or webgpu\n"
        "   --eyes=<stereoscopic eyes>, -y <stereoscopic eyes>\n"
        "       Sets the number of stereoscopic eyes (default: 2) when stereoscopic rendering is\n"
        "       enabled.\n"
        "   --samples=<number of samples for MSAA>, -m <number of samples for MSAA>\n"
        "       Sets the number of samples for MSAA\n\n"
    );
    const std::string from("SHOWCASE");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    std::cout << usage;
}

static int handleCommandLineArguments(int argc, char* argv[], App* app) {
    static constexpr const char* OPTSTR = "ha:y:m:";
    static const struct option OPTIONS[] = {
        { "help",    no_argument,       nullptr, 'h' },
        { "api",     required_argument, nullptr, 'a' },
        { "eyes",    required_argument, nullptr, 'y' },
        { "samples", required_argument, nullptr, 'm'},
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
            case 'y': {
                int eyeCount = 0;
                try {
                    eyeCount = std::stoi(arg);
                } catch (std::invalid_argument &e) { }
                if (eyeCount >= 2 && eyeCount <= CONFIG_MAX_STEREOSCOPIC_EYES) {
                    app->config.stereoscopicEyeCount = eyeCount;
                } else {
                    std::cerr << "Eye count must be between 2 and CONFIG_MAX_STEREOSCOPIC_EYES ("
                              << (int)CONFIG_MAX_STEREOSCOPIC_EYES << ") (inclusive).\n";
                    exit(1);
                }
                break;
            }
            case 'm': {
                int samples = 0;
                try {
                    samples = std::stoi(arg);
                } catch (std::invalid_argument &e) { }
                if (samples > 0) {
                    app->config.samples = samples;
                } else {
                    std::cerr << "Sample count must be a positive number\n";
                    exit(1);
                }
                break;
            }
        }
    }
    return optind;
}

int main(int argc, char** argv) {

#if !defined(FILAMENT_SAMPLES_STEREO_TYPE_MULTIVIEW)
    std::cerr << "This sample only works with multiview enabled.\n";
    exit(1);
#endif

    App app{};
    app.config.title = "stereoscopic rendering";
    handleCommandLineArguments(argc, argv, &app);

    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        auto& tcm = engine->getTransformManager();
        auto& rcm = engine->getRenderableManager();
        auto& em = utils::EntityManager::get();
        auto vp = view->getViewport();

        constexpr float3 monkeyPosition{ 0, 0, -4};
        constexpr float3 upVector{ 0, 1, 0};
        const int eyeCount = app.config.stereoscopicEyeCount;
        const uint8_t sampleCount = app.config.samples;

        // Create a mesh material and an instance.
        app.monkeyMaterial = Material::Builder()
              .package(RESOURCES_AIDEFAULTMAT_DATA, RESOURCES_AIDEFAULTMAT_SIZE)
              .build(*engine);
        auto mi = app.monkeyMatInstance = app.monkeyMaterial->createInstance();
        mi->setParameter("baseColor", RgbType::LINEAR, {0.8, 1.0, 1.0});
        mi->setParameter("metallic", 0.0f);
        mi->setParameter("roughness", 0.4f);
        mi->setParameter("reflectance", 0.5f);

        // Add a monkey and a light source into the main scene.
        app.monkeyMesh = MeshReader::loadMeshFromBuffer(
                engine, MONKEY_SUZANNE_DATA, nullptr, nullptr, mi);
        auto ti = tcm.getInstance(app.monkeyMesh.renderable);
        app.monkeyTransform = mat4f{mat3f(1), monkeyPosition } * tcm.getWorldTransform(ti);
        rcm.setCastShadows(rcm.getInstance(app.monkeyMesh.renderable), false);
        scene->addEntity(app.monkeyMesh.renderable);

        app.lightEntity = em.create();
        LightManager::Builder(LightManager::Type::SUN)
                .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
                .intensity(110000)
                .direction({ 0.7, -1, -0.8 })
                .sunAngularRadius(1.9f)
                .castShadows(false)
                .build(*engine, app.lightEntity);
        scene->addEntity(app.lightEntity);

        // Create a stereo render target that will be rendered as an offscreen view.
        app.stereoScene = engine->createScene();
        app.stereoScene->addEntity(app.monkeyMesh.renderable);
        app.stereoScene->addEntity(app.lightEntity);
        app.stereoView = engine->createView();
        app.stereoView->setScene(app.stereoScene);
        app.stereoView->setPostProcessingEnabled(false);
        app.stereoColorTexture = Texture::Builder()
                .width(vp.width)
                .height(vp.height)
                .depth(eyeCount)
                .levels(1)
                .samples(sampleCount)
                .sampler(Texture::Sampler::SAMPLER_2D_ARRAY)
                .format(Texture::InternalFormat::RGBA8)
                .usage(Texture::Usage::COLOR_ATTACHMENT | Texture::Usage::SAMPLEABLE)
                .build(*engine);
        app.stereoDepthTexture = Texture::Builder()
                .width(vp.width)
                .height(vp.height)
                .depth(eyeCount)
                .levels(1)
                .samples(sampleCount)
                .sampler(Texture::Sampler::SAMPLER_2D_ARRAY)
                .format(Texture::InternalFormat::DEPTH32F)
                .usage(Texture::Usage::DEPTH_ATTACHMENT | Texture::Usage::SAMPLEABLE)
                .build(*engine);
        app.stereoRenderTarget = RenderTarget::Builder()
                .texture(RenderTarget::AttachmentPoint::COLOR, app.stereoColorTexture)
                .texture(RenderTarget::AttachmentPoint::DEPTH, app.stereoDepthTexture)
                .multiview(RenderTarget::AttachmentPoint::COLOR, eyeCount, 0)
                .multiview(RenderTarget::AttachmentPoint::DEPTH, eyeCount, 0)
                .samples(sampleCount)
                .build(*engine);
        app.stereoView->setRenderTarget(app.stereoRenderTarget);
        app.stereoView->setViewport({0, 0, vp.width, vp.height});
        app.stereoCamera = engine->createCamera(em.create());
        app.stereoView->setCamera(app.stereoCamera);
        app.stereoView->setStereoscopicOptions({.enabled = true});
        FilamentApp::get().addOffscreenView(app.stereoView);

        // Camera settings for the stereo render target
        constexpr double projNear  = 0.1;
        constexpr double projFar = 100;

        mat4 projections[CONFIG_MAX_STEREOSCOPIC_EYES];
        mat4 eyeModels[CONFIG_MAX_STEREOSCOPIC_EYES];
        static_assert(CONFIG_MAX_STEREOSCOPIC_EYES == 4, "Update matrices");
        projections[0] = Camera::projection(24, 1.0, projNear, projFar);
        projections[1] = Camera::projection(70, 1.0, projNear, projFar);
        projections[2] = Camera::projection(50, 1.0, projNear, projFar);
        projections[3] = Camera::projection(35, 1.0, projNear, projFar);
        app.stereoCamera->setCustomEyeProjection(projections, 4, projections[0], projNear, projFar);

        eyeModels[0] = mat4::lookAt(float3{ -4, 0, 0 }, monkeyPosition, upVector);
        eyeModels[1] = mat4::lookAt(float3{ 4, 0, 0 }, monkeyPosition, upVector);
        eyeModels[2] = mat4::lookAt(float3{ 0, 3, 0 }, monkeyPosition, upVector);
        eyeModels[3] = mat4::lookAt(float3{ 0, -3, 0 }, monkeyPosition, upVector);
        for (int i = 0; i < eyeCount; ++i) {
            app.stereoCamera->setEyeModelMatrix(i, eyeModels[i]);
        }

        // Create a vertex buffer and an index buffer for a quad. This will be used to display the contents
        // of each layer of the stereo texture.
        float3 quadCenter = {0, 0, 0};
        float3 quadNormal = normalize(float3 {0, 0, 1});
        float3 u = normalize(cross(quadNormal, upVector));
        float3 v = cross(quadNormal, u);
        static Vertex quadVertices[4] = {
                {{quadCenter - u - v}, {1, 0}},
                {{quadCenter + u - v}, {0, 0}},
                {{quadCenter - u + v}, {1, 1}},
                {{quadCenter + u + v}, {0, 1}}
        };

        static_assert(sizeof(Vertex) == 20, "Strange vertex size.");
        app.quadVb = VertexBuffer::Builder()
                .vertexCount(4)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3, 0, sizeof(Vertex))
                .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2, 12, sizeof(Vertex))
                .build(*engine);
        app.quadVb->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(quadVertices, sizeof(Vertex) * 4, nullptr));

        static constexpr uint16_t quadIndices[6] = { 0, 1, 2, 3, 2, 1 };
        app.quadIb = IndexBuffer::Builder()
                .indexCount(6)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(*engine);
        app.quadIb->setBuffer(*engine, IndexBuffer::BufferDescriptor(quadIndices, 12, nullptr));

        // Create quad material instances and renderables.
        app.quadMaterial = Material::Builder()
                .package(RESOURCES_ARRAYTEXTURE_DATA, RESOURCES_ARRAYTEXTURE_SIZE)
                .build(*engine);

        for (int i = 0; i < eyeCount; ++i) {
            MaterialInstance* quadMatInst = app.quadMaterial->createInstance();
            TextureSampler sampler(TextureSampler::MinFilter::LINEAR, TextureSampler::MagFilter::LINEAR);
            quadMatInst->setParameter("image", app.stereoColorTexture, sampler);
            quadMatInst->setParameter("layerIndex", i);
            quadMatInst->setParameter("borderEffect", true);
            app.quadMatInstances.push_back(quadMatInst);

            utils::Entity quadEntity = em.create();
            app.quadEntities.push_back(quadEntity);
            RenderableManager::Builder(1)
                    .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
                    .material(0, quadMatInst)
                    .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.quadVb, app.quadIb, 0, 6)
                    .culling(false)
                    .receiveShadows(false)
                    .castShadows(false)
                    .build(*engine, quadEntity);
            scene->addEntity(quadEntity);

            // Place quads at equal intervals.
            TransformManager::Instance quadTi = tcm.getInstance(quadEntity);
            mat4f quadWorld = tcm.getWorldTransform(quadTi);
            constexpr float leftMostPos = -4;
            constexpr float rightMostPos = 4;
            float xpos = leftMostPos + ( (rightMostPos - leftMostPos) / (eyeCount - 1) ) * i;
            tcm.setTransform(quadTi, mat4f::translation(float3(xpos, 2, -8)) * quadWorld);
        }
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {

        auto& em = utils::EntityManager::get();

        for (utils::Entity e : app.quadEntities) {
            engine->destroy(e);
        }
        for (MaterialInstance* mi : app.quadMatInstances) {
            engine->destroy(mi);
        }
        engine->destroy(app.quadMaterial);
        engine->destroy(app.quadIb);
        engine->destroy(app.quadVb);
        engine->destroy(app.stereoRenderTarget);
        engine->destroy(app.stereoDepthTexture);
        engine->destroy(app.stereoColorTexture);
        auto camera = app.stereoCamera->getEntity();
        engine->destroyCameraComponent(camera);
        em.destroy(camera);
        engine->destroy(app.stereoScene);
        engine->destroy(app.stereoView);
        engine->destroy(app.lightEntity);
        engine->destroy(app.monkeyMesh.renderable);
        engine->destroy(app.monkeyMesh.indexBuffer);
        engine->destroy(app.monkeyMesh.vertexBuffer);
        engine->destroy(app.monkeyMatInstance);
        engine->destroy(app.monkeyMaterial);
    };

    auto preRender = [&app](Engine*, View*, Scene*, Renderer* renderer) {
        renderer->setClearOptions({.clearColor = {0.1,0.2,0.4,1.0}, .clear = true});
    };

    FilamentApp::get().animate([&app](Engine* engine, View* view, double now) {
        auto& tcm = engine->getTransformManager();

        // Animate the monkey by spinning and sliding back and forth along Z.
        auto ti = tcm.getInstance(app.monkeyMesh.renderable);
        mat4f xform =  app.monkeyTransform *  mat4f::rotation(now, float3{0, 1, 0 });
        tcm.setTransform(ti, xform);
    });

    FilamentApp::get().run(app.config, setup, cleanup, FilamentApp::ImGuiCallback(), preRender);

    return 0;
}
