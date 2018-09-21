/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include "../material_sandbox.h"

#include "filamesh.h"
#include "filaweb.h"

#include <filament/Engine.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/View.h>

#include <math/vec3.h>

#include <utils/Entity.h>

#include <imgui.h>

#include <chrono>

using namespace filament;
using namespace filagui;
using namespace math;
using namespace utils;

using MagFilter = TextureSampler::MagFilter;
using WrapMode = TextureSampler::WrapMode;
using Format = Texture::InternalFormat;

struct SandboxApp {
    Filamesh filamesh;
    Camera* cam;
    SandboxParameters params;
    filaweb::SkyLight skylight;
    Scene* scene;
};

static SandboxApp app;

void setup(Engine* engine, View* view, Scene* scene) {
    app.scene = scene;

    // These initial values seem reasonable with the "pillars_2k" envmap.
    app.params.iblIntensity = 10000.0f;
    app.params.lightDirection.x = 1;
    app.params.lightDirection.y = -1;
    app.params.lightDirection.z = -0.4;
    app.params.iblRotation = 289 * M_PI / 180;

    // These initial values look pretty nice with the shaderball.
    app.params.clearCoat = 0.7f;
    app.params.metallic = 0.0f;
    app.params.reflectance = 0.5f;
    app.params.color.x = 158 / 255.0f;
    app.params.color.y = 118 / 255.0f;
    app.params.color.z = 74 / 255.0f;

    // Create material.
    createInstances(app.params, *engine);

    // Move raw asset data from JavaScript to C++ static storage. Their held data will be freed via
    // BufferDescriptor callbacks after Filament creates the corresponding GPU objects.
    static auto mesh = filaweb::getRawFile("mesh");

    // Create mesh.
    const uint8_t* mdata = mesh.rawData.get();
    const auto destructor = [](void* buffer, size_t size, void* user) {
        auto asset = (filaweb::Asset*) user;
        asset->rawData.reset();
    };
    MaterialInstance* materialInstance = app.params.materialInstance[MATERIAL_LIT];
    app.filamesh = decodeMesh(*engine, mdata, 0, materialInstance, destructor, &mesh);
    scene->addEntity(app.filamesh->renderable);

    // Create the sun.
    scene->addEntity(app.params.light);

    // Create skybox and image-based light source.
    app.skylight = filaweb::getSkyLight(*engine, "pillars_2k");
    scene->setIndirectLight(app.skylight.indirectLight);
    scene->setSkybox(app.skylight.skybox);

    app.cam = engine->createCamera();
    app.cam->setExposure(16.0f, 1 / 125.0f, 100.0f);
    view->setCamera(app.cam);

    // Set up an interesting orientation for the shader ball.
    auto& manip = filaweb::Application::get()->getManipulator();
    manip.setCamera(app.cam);
    manip.lookAt(math::double3({0, 1, 6}), math::double3({0, 1, 0}));
};

void animate(Engine* engine, View* view, double now) {
    static double previous = now;

    // Adjust camera on every frame in case window size changes.
    using Fov = Camera::Fov;
    const uint32_t width = view->getViewport().width;
    const uint32_t height = view->getViewport().height;
    double ratio = double(width) / height;
    app.cam->setProjection(45.0, ratio, 0.1, 50.0, ratio < 1 ? Fov::HORIZONTAL : Fov::VERTICAL);
};

void ui(Engine* engine, View* view) {
    auto& params = app.params;
    // Initial window size and position are somewhat optimized for a Pixel 2 portrait screen.
    ImGui::SetNextWindowSize(ImVec2(300.0f, 150.0f), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(8, 6), ImGuiCond_Once);
    ImGui::Begin("Material", nullptr, ImGuiWindowFlags_NoResize);
    {
        ImGui::ColorEdit3("baseColor", &params.color.r);
        if (params.currentMaterialModel > MATERIAL_MODEL_UNLIT) {
            if (params.currentBlending == BLENDING_TRANSPARENT ||
                    params.currentBlending == BLENDING_FADE) {
                ImGui::SliderFloat("alpha", &params.alpha, 0.0f, 1.0f);
            }
            ImGui::SliderFloat("roughness", &params.roughness, 0.0f, 1.0f);
            if (params.currentMaterialModel != MATERIAL_MODEL_CLOTH) {
                ImGui::SliderFloat("metallic", &params.metallic, 0.0f, 1.0f);
                ImGui::SliderFloat("reflectance", &params.reflectance, 0.0f, 1.0f);
            }
            if (params.currentMaterialModel != MATERIAL_MODEL_CLOTH &&
                    params.currentMaterialModel != MATERIAL_MODEL_SUBSURFACE) {
                ImGui::SliderFloat("clearCoat", &params.clearCoat, 0.0f, 1.0f);
            }
            if (params.currentMaterialModel == MATERIAL_MODEL_SUBSURFACE) {
                ImGui::SliderFloat("thickness", &params.thickness, 0.0f, 1.0f);
                ImGui::SliderFloat("subsurfacePower", &params.subsurfacePower, 1.0f, 24.0f);
                ImGui::ColorEdit3("subsurfaceColor", &params.subsurfaceColor.r);
            }
            if (params.currentMaterialModel == MATERIAL_MODEL_CLOTH) {
                ImGui::ColorEdit3("sheenColor", &params.sheenColor.r);
                ImGui::ColorEdit3("subsurfaceColor", &params.subsurfaceColor.r);
            }
        }
    }
    ImGui::End();

    MaterialInstance* materialInstance = updateInstances(params, *engine);

    auto& rcm = engine->getRenderableManager();
    auto instance = rcm.getInstance(app.filamesh->renderable);
    for (size_t i = 0; i < rcm.getPrimitiveCount(instance); i++) {
        rcm.setMaterialInstanceAt(instance, i, materialInstance);
    }
    rcm.setCastShadows(instance, params.castShadows);

    if (params.directionalLightEnabled && !params.hasDirectionalLight) {
        app.scene->addEntity(params.light);
        params.hasDirectionalLight = true;
    } else if (!params.directionalLightEnabled && params.hasDirectionalLight) {
        app.scene->remove(params.light);
        params.hasDirectionalLight = false;
    }

    app.skylight.indirectLight->setIntensity(params.iblIntensity);
    app.skylight.indirectLight->setRotation(
            mat3f::rotate(params.iblRotation, float3{ 0, 1, 0 }));
}

// This is called only after the JavaScript layer has created a WebGL 2.0 context and all assets
// have been downloaded.
extern "C" void launch() {
    filaweb::Application::get()->run(setup, animate, ui);
}

// The main() entry point is implicitly called after JIT compilation, but potentially before the
// WebGL context has been created or assets have finished loading.
int main() { }

