/*
 * Copyright (C) 2016 The Android Open Source Project
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

#include <filamentapp/AssetLoader.h>
#include <filamentapp/Cube.h>
#include <filamentapp/FilamentApp2.h>
#include <filamentapp/IcoSphere.h>
#include <filamentapp/MeshAssimp.h>
#include <filamentapp/Sphere.h>

#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <utils/EntityManager.h>
#include <utils/getopt.h>
#include <utils/Log.h>
#include <utils/Path.h>

#include <math/norm.h>

#include <iostream>
#include <map>
#include <vector>

using namespace filament::math;
using namespace filament;
using namespace filamat;
using namespace utils;
namespace {
float g_meshScale = 1.0f;

struct GroundPlane {
    VertexBuffer* vb = nullptr;
    IndexBuffer* ib = nullptr;
    Material* mat = nullptr;
    Entity renderable;
};

struct App {
    FilamentApp2* filamentApp;
    SampleConfig config;
    std::vector<utils::Path> filenames;
    std::map<utils::CString, MaterialInstance*> materialLibrary;
    std::unique_ptr<MeshAssimp> meshSet;
    std::unique_ptr<Cube> meshAabb;
    std::vector<Sphere> spheres;
    std::vector<Entity> lights;
    GroundPlane plane;
    Entity discoBallEntity;
    float discoAngle = 0;
    float discoAngularSpeed = 0.25f * float(M_PI);
    bool moreLights = false;
    bool shadowPlane = false;
    bool discoBall = false;
};
} // namespace


std::unique_ptr<FilamentApp2> createSampleApp(SampleConfig config,
        filament::app::DisplayManager* dm, filament::app::AssetLoader* loader) {
    auto app = std::make_shared<App>();
    app->config = config;

    app->moreLights = app->config.getBool("more-lights");
    app->shadowPlane = app->config.getBool("shadow-plane");
    app->discoBall = app->config.getBool("disco");
    g_meshScale = app->config.getFloat("scale", 1.0f);

    for (const auto& filename: app->config.positionalArgs) {
        app->filenames.push_back(utils::Path(filename.c_str()));
    }

    auto cleanup = [app](Engine* engine, View* view, Scene* scene) {
        for (auto& item: app->materialLibrary) {
            auto materialInstance = item.second;
            engine->destroy(materialInstance);
        }
        app->meshSet.reset(nullptr);
        app->meshAabb.reset(nullptr);

        auto& em = EntityManager::get();

        if (app->plane.renderable) {
            engine->destroy(app->plane.renderable);
            em.destroy(app->plane.renderable);
        }
        if (app->plane.mat) {
            engine->destroy(app->plane.mat);
        }
        if (app->plane.vb) {
            engine->destroy(app->plane.vb);
        }
        if (app->plane.ib) {
            engine->destroy(app->plane.ib);
        }

        app->spheres.clear();

        for (Entity e: app->lights) {
            engine->destroy(e);
            em.destroy(e);
        }
        if (app->discoBallEntity) {
            em.destroy(app->discoBallEntity);
        }
    };

    auto preRender = [app](filament::Engine* engine, filament::View* view, filament::Scene*,
                             filament::Renderer* renderer) {
        renderer->setClearOptions(
                { .clearColor = { 0.0f, 0.0f, 0.0f, 1.0f }, .clear = !app->filamentApp->getIBL() });
    };

    auto setup = [app](Engine* engine, View* view, Scene* scene) {
        app->meshSet.reset(new MeshAssimp(*engine));
        for (auto& filename: app->filenames) {
            app->meshSet->addFromFile(filename, app->materialLibrary);
        }
        if (app->filenames.empty()) {
            app->meshSet->addFromFile(FilamentApp2::getRootAssetsPath() +
                                              "assets/models/monkey/monkey.obj",
                    app->materialLibrary);
        }

        auto& lcm = engine->getLightManager();

        auto& tcm = engine->getTransformManager();
        if (!app->meshSet->getRenderables().empty()) {
            auto ti = tcm.getInstance(app->meshSet->getRenderables()[0]);
            tcm.setTransform(ti, mat4f{ mat3f(g_meshScale), float3(0.0f, 0.0f, -4.0f) } *
                                         tcm.getWorldTransform(ti));
        }

        auto& rcm = engine->getRenderableManager();
        for (auto renderable: app->meshSet->getRenderables()) {
            if (rcm.hasComponent(renderable)) {
                auto ri = rcm.getInstance(renderable);
                auto blendingMode =
                        rcm.getMaterialInstanceAt(ri, 0)->getMaterial()->getBlendingMode();
                rcm.setCastShadows(ri, blendingMode == BlendingMode::OPAQUE ||
                                               blendingMode == BlendingMode::MASKED);
                scene->addEntity(renderable);
            }
        }

        auto& em = EntityManager::get();
        app->lights.push_back(em.create());

        LightManager::Builder(LightManager::Type::SUN)
                .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
                .intensity(110000)
                .direction({ 0.7, -1, -0.8 })
                .sunAngularRadius(1.9f)
                .castShadows(true)
                .build(*engine, app->lights.back());
        scene->addEntity(app->lights.back());

        lcm.setShadowOptions(lcm.getInstance(app->lights[0]),
                { .screenSpaceContactShadows = true });

        view->setAmbientOcclusionOptions({ .enabled = true });
        view->setBloomOptions({ .enabled = true });

        if (app->moreLights) {
            app->lights.push_back(em.create());
            LightManager::Builder(LightManager::Type::POINT)
                    .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.92f, 0.89f)))
                    .intensity(1000.0f, LightManager::EFFICIENCY_LED)
                    .position({ 0.0f, -0.2f, -3.0f })
                    .falloff(4.0f)
                    .build(*engine, app->lights.back());

            app->lights.push_back(em.create());
            LightManager::Builder(LightManager::Type::FOCUSED_SPOT)
                    .color(Color::toLinear<ACCURATE>(sRGBColor(0.98f, 0.12f, 0.19f)))
                    .intensity(2000.0f, LightManager::EFFICIENCY_LED)
                    .position({ 0.6f, 0.6f, -3.2f })
                    .direction({ -1.0f, 0.0f, 0.0f })
                    .spotLightCone(static_cast<float>(M_PI / 8),
                            static_cast<float>((M_PI / 8) * 1.1))
                    .falloff(4.0f)
                    .castShadows(true)
                    .build(*engine, app->lights.back());

            app->lights.push_back(em.create());
            LightManager::Builder(LightManager::Type::POINT)
                    .color(Color::toLinear<ACCURATE>(sRGBColor(0.18f, 0.12f, 0.89f)))
                    .intensity(1000.0f, LightManager::EFFICIENCY_LED)
                    .position({ -0.6f, 0.3f, -3.2f })
                    .falloff(2.0f)
                    .build(*engine, app->lights.back());

            app->lights.push_back(em.create());
            LightManager::Builder(LightManager::Type::POINT)
                    .color(Color::toLinear<ACCURATE>(sRGBColor(0.88f, 0.82f, 0.29f)))
                    .intensity(1000.0f, LightManager::EFFICIENCY_LED)
                    .position({ 0.0f, 1.5f, -3.5f })
                    .falloff(2.0f)
                    .build(*engine, app->lights.back());

            app->lights.push_back(em.create());
            LightManager::Builder(LightManager::Type::FOCUSED_SPOT)
                    .color(Color::toLinear<ACCURATE>(sRGBColor(0.12f, 0.98f, 0.19f)))
                    .intensity(2000.0f, LightManager::EFFICIENCY_LED)
                    .position({ 0.0f, 0.6f, -3.2f })
                    .direction({ 1.0f, 0.0f, 0.0f })
                    .spotLightCone(static_cast<float>(M_PI / 8),
                            static_cast<float>((M_PI / 8) * 1.1))
                    .falloff(4.0f)
                    .castShadows(true)
                    .build(*engine, app->lights.back());

            for (const auto& light: app->lights) {
                scene->addEntity(light);
                const LightManager::Instance& instance = lcm.getInstance(light);
                if (!lcm.isDirectional(instance)) {
                    app->spheres.emplace_back(*engine, app->filamentApp->getDefaultMaterial());
                    app->spheres.back().setRadius(0.025f).setPosition(lcm.getPosition(instance));

                    auto mi = app->spheres.back().getMaterialInstance();
                    mi->setParameter("baseColor", RgbaType::LINEAR,
                            LinearColorA{ lcm.getColor(instance), 100.0f });
                    mi->setParameter("roughness", 0.2f);
                    mi->setParameter("metallic", 0.0f);
                }
            }

            for (const auto& sphere: app->spheres) {
                scene->addEntity(sphere.getSolidRenderable());
            }
        }

        if (app->discoBall) {
            IcoSphere sphere(2);
            auto const& vertices = sphere.getVertices();
            auto n = vertices.size();
            app->lights.resize(app->lights.size() + n);
            slog.d << "light count = " << n << io::endl;

            Entity discoBall = em.create();
            tcm.create(discoBall, {}, mat4f::translation(float3{ 0, 3, -14 }));
            auto parent = tcm.getInstance(discoBall);

            for (size_t i = 0, c = n; i < c; i++) {
                app->lights.push_back(em.create());
                LightManager::Builder(LightManager::Type::FOCUSED_SPOT)
                        .color(abs(vertices[i]))
                        .intensity(1000.0f, LightManager::EFFICIENCY_HALOGEN)
                        .direction(vertices[i])
                        .spotLightCone(0.0174f * 0.5f, 0.0174f * 2.0f)
                        .falloff(20.0f)
                        .build(*engine, app->lights.back());

                tcm.create(app->lights.back(), parent);

                scene->addEntity(app->lights.back());
            }
            app->discoBallEntity = discoBall;

            app->spheres.emplace_back(*engine, app->filamentApp->getDefaultMaterial());
            app->spheres.back().setRadius(0.2f);
            auto mi = app->spheres.back().getMaterialInstance();
            mi->setParameter("baseColor", RgbaType::LINEAR, LinearColorA{ 1, 1, 1, 1 });
            mi->setParameter("roughness", 0.01f);
            mi->setParameter("metallic", 1.0f);
            scene->addEntity(app->spheres.back().getSolidRenderable());
            tcm.setParent(tcm.getInstance(app->spheres.back().getSolidRenderable()),
                    tcm.getInstance(discoBall));
        }

        if (!app->meshSet->getRenderables().empty()) {
            app->meshAabb.reset(
                    new Cube(*engine, app->filamentApp->getTransparentMaterial(), { 0, 0, 1 }));

            Entity object = app->meshSet->getRenderables().size() > 1
                                    ? app->meshSet->getRenderables()[1]
                                    : app->meshSet->getRenderables()[0];

            RenderableManager::Instance ri = rcm.getInstance(object);
            if (ri) {
                app->meshAabb->mapAabb(*engine, rcm.getAxisAlignedBoundingBox(ri));
                scene->addEntity(app->meshAabb->getWireFrameRenderable());
                scene->addEntity(app->meshAabb->getSolidRenderable());
            }

            tcm.setParent(tcm.getInstance(app->meshAabb->getSolidRenderable()),
                    tcm.getInstance(object));
            tcm.setParent(tcm.getInstance(app->meshAabb->getWireFrameRenderable()),
                    tcm.getInstance(object));
            rcm.setLayerMask(rcm.getInstance(app->meshAabb->getSolidRenderable()), 0x3, 0x2);
            rcm.setLayerMask(rcm.getInstance(app->meshAabb->getWireFrameRenderable()), 0x3, 0x2);
        }

        if (app->shadowPlane) {
            Material* shadowMaterial =
                    Material::Builder()
                            .package(RESOURCES_GROUNDSHADOW_DATA, RESOURCES_GROUNDSHADOW_SIZE)
                            .build(*engine);
            shadowMaterial->setDefaultParameter("strength", 0.7f);

            static constexpr uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

            static constexpr filament::math::float3 vertices[] = {
                { -10, 0, -10 },
                { -10, 0, 10 },
                { 10, 0, 10 },
                { 10, 0, -10 },
            };

            short4 const tbn = filament::math::packSnorm16(mat3f::packTangentFrame(
                    filament::math::mat3f{ float3{ 1.0f, 0.0f, 0.0f }, float3{ 0.0f, 0.0f, 1.0f },
                        float3{ 0.0f, 1.0f, 0.0f } }).xyzw);

            static const filament::math::short4 normals[]{ tbn, tbn, tbn, tbn };

            VertexBuffer* vertexBuffer = VertexBuffer::Builder()
                                                 .vertexCount(4)
                                                 .bufferCount(2)
                                                 .attribute(VertexAttribute::POSITION, 0,
                                                         VertexBuffer::AttributeType::FLOAT3)
                                                 .attribute(VertexAttribute::TANGENTS, 1,
                                                         VertexBuffer::AttributeType::SHORT4)
                                                 .normalized(VertexAttribute::TANGENTS)
                                                 .build(*engine);

            vertexBuffer->setBufferAt(*engine, 0,
                    VertexBuffer::BufferDescriptor(vertices,
                            vertexBuffer->getVertexCount() * sizeof(vertices[0])));
            vertexBuffer->setBufferAt(*engine, 1,
                    VertexBuffer::BufferDescriptor(normals,
                            vertexBuffer->getVertexCount() * sizeof(normals[0])));

            IndexBuffer* indexBuffer = IndexBuffer::Builder().indexCount(6).build(*engine);

            indexBuffer->setBuffer(*engine,
                    IndexBuffer::BufferDescriptor(indices,
                            indexBuffer->getIndexCount() * sizeof(uint32_t)));

            Entity planeRenderable = em.create();
            RenderableManager::Builder(1)
                    .boundingBox({ { 0, 0, 0 }, { 10, 1e-4f, 10 } })
                    .material(0, shadowMaterial->getDefaultInstance())
                    .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vertexBuffer,
                            indexBuffer, 0, 6)
                    .culling(false)
                    .receiveShadows(true)
                    .castShadows(false)
                    .build(*engine, planeRenderable);

            scene->addEntity(planeRenderable);

            tcm.setTransform(tcm.getInstance(planeRenderable),
                    filament::math::mat4f::translation(float3{ 0, -1, -4 }));

            app->plane = {
                .vb = vertexBuffer,
                .ib = indexBuffer,
                .mat = shadowMaterial,
                .renderable = planeRenderable,
            };
        }
    };

    FilamentApp2::AnimCallback animate = nullptr;
    if (app->discoBall) {
        struct State {
            double lastTime = 0;
        };
        auto state = std::make_shared<State>();
        animate = [app, state](filament::Engine* engine, filament::View*, double now) mutable {
            auto& tcm = engine->getTransformManager();
            tcm.setTransform(tcm.getInstance(app->discoBallEntity),
                    filament::math::mat4f::translation(filament::math::float3{
                        0,
                        2,
                        -4,
                    }) * filament::math::mat4f::rotation(app->discoAngle,
                                 filament::math::float3{ 0, 1, 0 }));
            if (state->lastTime != 0) {
                double dT = now - state->lastTime;
                app->discoAngle += app->discoAngularSpeed * dT;
            }
            state->lastTime = now;
        };
    }

    auto fApp = samples::getBuilder(config, dm, loader)
                        .setup(setup)
                        .cleanup(cleanup)
                        .preRender(preRender)
                        .animation(animate)
                        .build();
    app->filamentApp = fApp.get();

    return fApp;
}

samples::SampleParameters createAppParameters() {
    return {
        samples::Parameter::makeBool("more-lights", 'm', "Enable more point lights", false),
        samples::Parameter::makeBool("disco", 'd', "Enable rotating disco ball", false),
        samples::Parameter::makeBool("shadow-plane", 'p', "Enable shadow-receiving ground plane",
                false),
        samples::Parameter::makeFloat("scale", 's', "Applies uniform scale", 1.0f),
    };
}

#ifndef __ANDROID__
int main(int argc, char* argv[]) {
    SampleConfig config;
    samples::CommandLineSpecification spec = {
        .sampleDescription = "LIGHTBULB is a point light and shadow testing tool for Filament.",
        .positionalArgsDescription = { "mesh files (.obj, .fbx)" },
        .parameters = createAppParameters(),
    };

    samples::handleCommandLineArguments(argc, argv, &config, spec);
    auto dm = samples::getDisplayManager(config);

    for (const auto& fname : config.positionalArgs) {
        utils::Path filename(fname.c_str_safe());
        if (!filename.exists()) {
            std::cerr << "file " << filename << " not found!" << std::endl;
            return 1;
        }
    }

    config.title = "Lightbulb";

    auto app = createSampleApp(config, dm.get(), nullptr);
    app->run();

    return 0;
}
#endif
