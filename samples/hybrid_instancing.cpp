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

// -------------------------------------------------------------------------------------------------
// HYBRID INSTANCING SAMPLE
// -------------------------------------------------------------------------------------------------
// This sample demonstrates hybrid instancing, a technique for rendering a large number of
// objects where some transformations are computed on the CPU (per-instance) and others are
// computed on the GPU (per-vertex). It is particularly well-suited for particle systems.
//
// The scene features multiple particle emitters that move in Lissajous patterns. Each emitter
// manages a cloud of small triangle particles. The sample can be switched between two modes:
// - FIREWORKS: Emitters burst all their particles at once on a timer.
// - CONTINUOUS: Emitters continuously emit new particles as old ones die.
//
// The sample also showcases several other Filament features:
// - Dynamic lighting: Each emitter has a colored point light attached to it, which fades
//   out with the particles in fireworks mode. A global directional light simulates moonlight.
// - UI controls: A robust ImGui-based UI allows for real-time manipulation of gravity,
//   emitter count, particle/emitter freezing, lighting, and emitter modes.
// - Command-line arguments: The camera mode can be set at launch.
// -------------------------------------------------------------------------------------------------

#include "common/arguments.h"

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>

#include <filament/Camera.h>
#include <filament/Color.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/InstanceBuffer.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialEnums.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <camutils/Bookmark.h>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <math/norm.h>

#include <utils/EntityManager.h>
#include <utils/Path.h>

#include <getopt/getopt.h>

#include <algorithm>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstdint>

#include <math.h>

#include <imgui.h>

#include "generated/resources/resources.h"

using namespace filament;
using namespace filament::math;
using utils::Entity;
using utils::EntityManager;

// ------------------------------------------------------------------------------------------------
// App Constants
// ------------------------------------------------------------------------------------------------

// The number of particles in each emitter.
static constexpr uint32_t PARTICLE_COUNT = 64;

// Particle properties
static constexpr float PARTICLE_LIFETIME_MIN = 1.0f;
static constexpr float PARTICLE_LIFETIME_MAX = 3.0f;
static constexpr float PARTICLE_VELOCITY_SCALE = 2.0f;
static constexpr float PARTICLE_ROTATION_SPEED_MIN = M_PI;
static constexpr float PARTICLE_ROTATION_SPEED_MAX = 2.0f * M_PI;

// Emitter properties
static constexpr float EMITTER_FREQUENCY_MIN = 0.5f;
static constexpr float EMITTER_FREQUENCY_MAX = 1.0f;
static constexpr float EMITTER_X_RADIUS_MIN = 2.0f;
static constexpr float EMITTER_X_RADIUS_MAX = 15.0f;
static constexpr float EMITTER_Y_RADIUS_MIN = 2.0f;
static constexpr float EMITTER_Y_RADIUS_MAX = 15.0f;
static constexpr float EMITTER_Z_RADIUS_MIN = 2.0f;
static constexpr float EMITTER_Z_RADIUS_MAX = 10.0f;
static constexpr float EMITTER_PHASE_MAX = 2.0f * M_PI;

// Lighting and Material properties
static constexpr float CONTINUOUS_LIGHT_INTENSITY = 10000.0f;
static constexpr float LIGHT_FALLOFF = 20.0f;
static constexpr float EMISSIVE_FACTOR = 200.0f;
static constexpr float MOONLIGHT_INTENSITY = 100.0f;

// Ground plane properties
static constexpr float GROUND_PLANE_EXTENT = 20.0f;
static constexpr float GROUND_PLANE_Y = -10.0f;

// Camera properties
static constexpr float CAMERA_EXPOSURE_APERTURE = 2.8f;
static constexpr float CAMERA_EXPOSURE_SHUTTER_SPEED = 1.0f / 125.0f;
static constexpr float CAMERA_EXPOSURE_SENSITIVITY = 400.0f;

// UI properties
static constexpr float GRAVITY_STRENGTH_MAX = 2.0f;
static constexpr float FIREWORKS_DELAY_MIN = 0.1f;
static constexpr float FIREWORKS_DELAY_MAX = 10.0f;
static constexpr int EMITTER_COUNT_MIN = 1;
static constexpr int EMITTER_COUNT_MAX = 100;

// ------------------------------------------------------------------------------------------------
// App Data Structures
// ------------------------------------------------------------------------------------------------

// A simple vertex format for our meshes.
struct Vertex {
    float2 position;
};

// The mesh for a single particle (a triangle).
static constexpr float PARTICLE_TRIANGLE_SIDE = 0.05f;
static constexpr Vertex TRIANGLE_VERTICES[3] = {
    { { -PARTICLE_TRIANGLE_SIDE, -PARTICLE_TRIANGLE_SIDE / 1.732f } },
    { { PARTICLE_TRIANGLE_SIDE, -PARTICLE_TRIANGLE_SIDE / 1.732f } },
    { { 0.0f, PARTICLE_TRIANGLE_SIDE * 2.0f / 1.732f } },
};
static constexpr uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 };

// Holds the state of a single particle.
struct Particle {
    float3 velocity;
    float age = 0.0f;
    float lifetime = 0.0f;
    float3 rotationAxis;
    float rotationSpeed;
    float3 birthPosition; // The emitter's position when the particle was born.
};

// Holds the state of a particle emitter, which is a collection of particles and a renderable.
struct Emitter {
    Entity renderable;
    InstanceBuffer* instanceBuffer = nullptr;
    MaterialInstance* materialInstance = nullptr;
    std::vector<mat4f> localTransforms;
    std::vector<Particle> particles;

    // for the emitter's own movement
    float3 radii;
    float3 frequencies;
    float phase;

    // for fireworks mode
    float timeToBurst = 0.0f;
};

// Holds all the state for this application.
struct App {
    enum class EmitterMode { CONTINUOUS, FIREWORKS };

    struct UiState {
        int emitterCount = 32;
        bool particlesFrozen = false;
        bool freezeEmitters = false;
        EmitterMode emitterMode = EmitterMode::FIREWORKS;
        float fireworksDelay = 3.0f;
        float gravityStrength = 0.0f;
        bool lightsEnabled = true;
        bool moonlightEnabled = true;
    };

    Config config;
    UiState ui;
    int currentEmitterCount = 0;
    bool previousFireworksMode = false;

    Skybox* skybox = nullptr;
    Material* material = nullptr;
    VertexBuffer* vb = nullptr;
    IndexBuffer* ib = nullptr;
    std::vector<Emitter> emitters;
    Scene* scene = nullptr;
    double lastTime = 0.0;

    Material* groundMaterial = nullptr;
    MaterialInstance* groundMi = nullptr;
    VertexBuffer* groundVb = nullptr;
    IndexBuffer* groundIb = nullptr;
    Entity groundPlane;
    Entity moonlight;
};

// ------------------------------------------------------------------------------------------------
// Function Declarations
// ------------------------------------------------------------------------------------------------

static void printUsage(char* name);
static int handleCommandLineArguments(int argc, char* argv[], App* app);
static void doUserInterface(App& app);
static void resetParticle(Particle& particle, std::mt19937& gen, const float3& emitterPosition);
static void setupSkybox(Engine& engine, Scene& scene, App& app);
static void setupGroundPlane(Engine& engine, Scene& scene, App& app);
static void setupMoonlight(Engine& engine, Scene& scene, App& app);
static void createEmitterResources(Engine& engine, App& app);
static void createEmitterInstances(Engine& engine, Scene& scene, App& app);
static void setupFireworks(App& app);
static void animateEmitter(Emitter& emitter, TransformManager& tcm, LightManager& lm, double now,
        double dt, std::mt19937& gen, const App& app);

// ------------------------------------------------------------------------------------------------
// Main
// ------------------------------------------------------------------------------------------------

int main(int const argc, char** argv) {
    App app;
    app.config.title = "Hybrid Instancing";
    handleCommandLineArguments(argc, argv, &app);

    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        app.scene = scene;
        setupSkybox(*engine, *scene, app);
        setupGroundPlane(*engine, *scene, app);
        setupMoonlight(*engine, *scene, app);
        createEmitterResources(*engine, app);
        createEmitterInstances(*engine, *scene, app);
        app.currentEmitterCount = app.ui.emitterCount;

        view->getCamera().setExposure(CAMERA_EXPOSURE_APERTURE, CAMERA_EXPOSURE_SHUTTER_SPEED,
                CAMERA_EXPOSURE_SENSITIVITY);

        view->setPostProcessingEnabled(true);
        view->setBloomOptions({ .enabled = true });
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        // Destroy all the Filament objects that we created.
        for (auto const& emitter: app.emitters) {
            engine->destroy(emitter.renderable);
            engine->destroy(emitter.instanceBuffer);
            engine->destroy(emitter.materialInstance);
        }
        engine->destroy(app.moonlight);
        engine->destroy(app.groundPlane);
        engine->destroy(app.groundMi);
        engine->destroy(app.groundMaterial);
        engine->destroy(app.groundVb);
        engine->destroy(app.groundIb);
        engine->destroy(app.skybox);
        engine->destroy(app.material);
        engine->destroy(app.vb);
        engine->destroy(app.ib);
    };

    auto animate = [&app](Engine* engine, View*, double const now) {
        auto& lm = engine->getLightManager();

        // Toggle moonlight
        auto const moonlightInstance = lm.getInstance(app.moonlight);
        lm.setIntensity(moonlightInstance, app.ui.moonlightEnabled ? MOONLIGHT_INTENSITY : 0.0f);

        // Handle emitter count changes from the UI.
        if (app.ui.emitterCount != app.currentEmitterCount) {
            for (auto const& emitter: app.emitters) {
                engine->destroy(emitter.renderable);
                engine->destroy(emitter.instanceBuffer);
                engine->destroy(emitter.materialInstance);
            }
            app.emitters.clear();
            createEmitterInstances(*engine, *app.scene, app);
            app.currentEmitterCount = app.ui.emitterCount;
            if (app.ui.emitterMode == App::EmitterMode::FIREWORKS) {
                setupFireworks(app);
            }
        }

        // If fireworks mode has just been enabled, set up the emitters for it.
        if (app.ui.emitterMode == App::EmitterMode::FIREWORKS &&
            app.previousFireworksMode == false) {
            setupFireworks(app);
        }
        app.previousFireworksMode = (app.ui.emitterMode == App::EmitterMode::FIREWORKS);

        // Calculate the time delta since the last frame.
        double dt = now - app.lastTime;
        if (app.lastTime == 0.0) {
            dt = 1.0 / 60.0; // First frame
        }
        app.lastTime = now;

        auto& tcm = engine->getTransformManager();
        std::mt19937 gen(static_cast<unsigned int>(now * 1000));

        // Animate each emitter and its particles.
        for (auto& emitter: app.emitters) {
            animateEmitter(emitter, tcm, lm, now, dt, gen, app);
        }
    };

    auto imgui = [&app](Engine*, View*) {
        doUserInterface(app);
    };

    FilamentApp::get().animate(animate);
    FilamentApp::get().run(app.config, setup, cleanup, imgui);

    return 0;
}

// ------------------------------------------------------------------------------------------------
// Helper functions
// ------------------------------------------------------------------------------------------------

// Renders the ImGui UI controls.
void doUserInterface(App& app) {
    ImGui::Begin("Controls");
    ImGui::SliderInt("Emitters", &app.ui.emitterCount, EMITTER_COUNT_MIN, EMITTER_COUNT_MAX);
    ImGui::Checkbox("Freeze Particles", &app.ui.particlesFrozen);
    ImGui::BeginDisabled(app.ui.particlesFrozen);
    ImGui::Checkbox("Freeze Emitters", &app.ui.freezeEmitters);
    ImGui::EndDisabled();
    ImGui::SliderFloat("Gravity", &app.ui.gravityStrength, 0.0f, GRAVITY_STRENGTH_MAX, "%.2f G");
    ImGui::Checkbox("Enable Lights", &app.ui.lightsEnabled);
    ImGui::Checkbox("Moonlight", &app.ui.moonlightEnabled);

    bool fireworks = (app.ui.emitterMode == App::EmitterMode::FIREWORKS);
    if (ImGui::Checkbox("Fireworks Mode", &fireworks)) {
        app.ui.emitterMode = fireworks ? App::EmitterMode::FIREWORKS : App::EmitterMode::CONTINUOUS;
    }

    if (app.ui.emitterMode == App::EmitterMode::FIREWORKS) {
        ImGui::SliderFloat("Delay", &app.ui.fireworksDelay, FIREWORKS_DELAY_MIN,
                FIREWORKS_DELAY_MAX);
    }
    ImGui::End();
}

// Prints command-line usage information.
static void printUsage(char* name) {
    const std::string exec_name(utils::Path(name).getName());
    std::string usage(
            "HYBRID_INSTANCING showcases renderable instancing with instance buffers.\n"
            "Usage:\n"
            "    HYBRID_INSTANCING [options]\n"
            "Options:\n"
            "   --help, -h\n"
            "       Prints this message\n\n"
            "   --emitters=<count>, -e <count>\n"
            "       Sets the number of particle emitters (default: 32)\n\n"
            "   --camera=<mode>, -c <mode>\n"
            "       Sets the camera mode: orbit (default), map, or flight\n\n"
            "API_USAGE");
    const std::string from("HYBRID_INSTANCING");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    const std::string apiUsage("API_USAGE");
    for (size_t pos = usage.find(apiUsage); pos != std::string::npos;
         pos = usage.find(apiUsage, pos)) {
        usage.replace(pos, apiUsage.length(), samples::getBackendAPIArgumentsUsage());
    }
    std::cout << usage;
}

// Parses command-line arguments.
static int handleCommandLineArguments(int const argc, char* argv[], App* app) {
    static constexpr const char* OPTSTR = "he:a:c:";
    static constexpr option OPTIONS[] = {
        { "help", no_argument, nullptr, 'h' },
        { "api", required_argument, nullptr, 'a' },
        { "emitters", required_argument, nullptr, 'e' },
        { "camera", required_argument, nullptr, 'c' },
        { nullptr, 0, nullptr, 0 } };
    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, &option_index)) >= 0) {
        const std::string arg(optarg ? optarg : "");
        switch (opt) {
            default:
            case 'h':
                printUsage(argv[0]);
                exit(0);
            case 'a':
                app->config.backend = samples::parseArgumentsForBackend(arg);
                break;
            case 'e':
                app->ui.emitterCount = atoi(arg.c_str());
                break;
            case 'c':
                if (arg == "flight") {
                    app->config.cameraMode = camutils::Mode::FREE_FLIGHT;
                } else if (arg == "orbit") {
                    app->config.cameraMode = camutils::Mode::ORBIT;
                } else if (arg == "map") {
                    app->config.cameraMode = camutils::Mode::MAP;
                } else {
                    std::cerr << "Unrecognized camera mode. Must be 'flight' | 'orbit' | 'map'."
                            << std::endl;
                }
                break;
        }
    }
    return optind;
}

// Resets a particle to a new random state.
static void resetParticle(Particle& particle, std::mt19937& gen, const float3& emitterPosition) {
    std::uniform_real_distribution vel_dist(-1.0f, 1.0f);
    std::uniform_real_distribution life_dist(PARTICLE_LIFETIME_MIN, PARTICLE_LIFETIME_MAX);
    std::uniform_real_distribution rot_speed_dist(PARTICLE_ROTATION_SPEED_MIN,
            PARTICLE_ROTATION_SPEED_MAX);
    particle.velocity = normalize(float3{ vel_dist(gen), vel_dist(gen), vel_dist(gen) }) *
                        PARTICLE_VELOCITY_SCALE;
    particle.age = 0.0f;
    particle.lifetime = life_dist(gen);
    particle.rotationAxis = normalize(float3{ vel_dist(gen), vel_dist(gen), vel_dist(gen) });
    particle.rotationSpeed = rot_speed_dist(gen);
    particle.birthPosition = emitterPosition;
}

// Creates a simple skybox with a solid color.
void setupSkybox(Engine& engine, Scene& scene, App& app) {
    app.skybox = Skybox::Builder().color({ 0.1, 0.125, 0.25, 1.0 }).build(engine);
    scene.setSkybox(app.skybox);
}

// Creates a ground plane to receive shadows.
void setupGroundPlane(Engine& engine, Scene& scene, App& app) {
    app.groundMaterial = Material::Builder()
                         .package(RESOURCES_AIDEFAULTMAT_DATA, RESOURCES_AIDEFAULTMAT_SIZE)
                         .build(engine);
    app.groundMi = app.groundMaterial->createInstance();
    app.groundMi->setParameter("baseColor", RgbType::LINEAR, float3{ 0.8f });
    app.groundMi->setParameter("metallic", 0.0f);
    app.groundMi->setParameter("roughness", 0.8f);

    const static float3 GROUND_VERTICES[4] = {
        { -GROUND_PLANE_EXTENT, 0, -GROUND_PLANE_EXTENT },
        { -GROUND_PLANE_EXTENT, 0,  GROUND_PLANE_EXTENT },
        {  GROUND_PLANE_EXTENT, 0,  GROUND_PLANE_EXTENT },
        {  GROUND_PLANE_EXTENT, 0, -GROUND_PLANE_EXTENT },
    };

    short4 const tbn = packSnorm16(
            mat3f::packTangentFrame(
                    mat3f{
                        float3{ 1.0f, 0.0f, 0.0f },
                        float3{ 0.0f, 0.0f, 1.0f },
                        float3{ 0.0f, 1.0f, 0.0f }
                    }
                    ).xyzw);

    const static short4 GROUND_TANGENTS[]{ tbn, tbn, tbn, tbn };

    app.groundVb = VertexBuffer::Builder()
                   .vertexCount(4)
                   .bufferCount(2)
                   .attribute(POSITION, 0, VertexBuffer::AttributeType::FLOAT3, 0, sizeof(float3))
                   .attribute(TANGENTS, 1, VertexBuffer::AttributeType::SHORT4, 0, sizeof(short4))
                   .normalized(TANGENTS)
                   .build(engine);
    app.groundVb->setBufferAt(engine, 0, { GROUND_VERTICES, sizeof(GROUND_VERTICES) });
    app.groundVb->setBufferAt(engine, 1, { GROUND_TANGENTS, sizeof(GROUND_TANGENTS) });

    static constexpr uint16_t GROUND_INDICES[6] = { 0, 1, 2, 2, 3, 0 };
    app.groundIb = IndexBuffer::Builder()
                   .indexCount(6)
                   .bufferType(IndexBuffer::IndexType::USHORT)
                   .build(engine);
    app.groundIb->setBuffer(engine, { GROUND_INDICES, sizeof(GROUND_INDICES) });

    app.groundPlane = EntityManager::get().create();
    RenderableManager::Builder(1)
            .boundingBox({ { 0, 0, 0 },
                           { GROUND_PLANE_EXTENT * 2.0f, 0.01, GROUND_PLANE_EXTENT * 2.0f } })
            .material(0, app.groundMi)
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.groundVb, app.groundIb)
            .culling(false)
            .receiveShadows(true)
            .castShadows(false)
            .build(engine, app.groundPlane);
    scene.addEntity(app.groundPlane);

    auto& tcm = engine.getTransformManager();
    auto const groundInstance = tcm.getInstance(app.groundPlane);
    tcm.setTransform(groundInstance, mat4f::translation(float3{ 0.0f, GROUND_PLANE_Y, 0.0f }));
}

// Creates a directional light to simulate moonlight.
void setupMoonlight(Engine& engine, Scene& scene, App& app) {
    app.moonlight = EntityManager::get().create();
    LightManager::Builder(LightManager::Type::DIRECTIONAL)
            .color(Color::toLinear<ACCURATE>(sRGBColor(0.8f, 0.9f, 1.0f)))
            .intensity(MOONLIGHT_INTENSITY)
            .direction(normalize(float3{ 0.5f, -1.0f, -0.7f }))
            .castShadows(false)
            .build(engine, app.moonlight);
    scene.addEntity(app.moonlight);
}

// Creates resources that are shared among all emitters.
void createEmitterResources(Engine& engine, App& app) {
    app.vb = VertexBuffer::Builder()
             .vertexCount(3)
             .bufferCount(1)
             .attribute(POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, sizeof(Vertex))
             .build(engine);
    app.vb->setBufferAt(engine, 0, { TRIANGLE_VERTICES, sizeof(TRIANGLE_VERTICES) });

    app.ib = IndexBuffer::Builder()
             .indexCount(3)
             .bufferType(IndexBuffer::IndexType::USHORT)
             .build(engine);
    app.ib->setBuffer(engine, { TRIANGLE_INDICES, sizeof(TRIANGLE_INDICES) });

    app.material = Material::Builder()
                   .package(RESOURCES_SANDBOXUNLIT_DATA, RESOURCES_SANDBOXUNLIT_SIZE)
                   .build(engine);
}

// Creates the particle emitters and their associated Filament resources.
void createEmitterInstances(Engine& engine, Scene& scene, App& app) {
    std::mt19937 gen(0); // Standard mersenne_twister_engine seeded with 0
    std::uniform_real_distribution fdist(EMITTER_FREQUENCY_MIN, EMITTER_FREQUENCY_MAX);
    std::uniform_real_distribution xdist(EMITTER_X_RADIUS_MIN, EMITTER_X_RADIUS_MAX);
    std::uniform_real_distribution ydist(EMITTER_Y_RADIUS_MIN, EMITTER_Y_RADIUS_MAX);
    std::uniform_real_distribution zdist(EMITTER_Z_RADIUS_MIN, EMITTER_Z_RADIUS_MAX);
    std::uniform_real_distribution pdist(0.0f, EMITTER_PHASE_MAX);
    std::uniform_real_distribution cdist(0.0f, 1.0f);

    for (int i = 0; i < app.ui.emitterCount; ++i) {
        Emitter emitter;
        emitter.instanceBuffer = InstanceBuffer::Builder(PARTICLE_COUNT).build(engine);
        emitter.materialInstance = app.material->createInstance();
        emitter.localTransforms.resize(PARTICLE_COUNT, mat4f(1.0f));
        emitter.particles.resize(PARTICLE_COUNT);

        const LinearColor color{ cdist(gen), cdist(gen), cdist(gen) };
        emitter.materialInstance->setParameter("baseColor", RgbType::LINEAR, color);
        emitter.materialInstance->
                setParameter("emissive", RgbType::LINEAR, color * EMISSIVE_FACTOR);
        emitter.materialInstance->setCullingMode(MaterialInstance::CullingMode::NONE);

        for (auto& p: emitter.particles) {
            resetParticle(p, gen, { 0.0f, 0.0f, 0.0f });
        }

        emitter.renderable = EntityManager::get().create();
        RenderableManager::Builder(1)
                .boundingBox({ { 0, 0, 0 }, { 6, 6, 6 } })
                .material(0, emitter.materialInstance)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib)
                .instances(PARTICLE_COUNT, emitter.instanceBuffer)
                .culling(false)
                .castShadows(false)
                .build(engine, emitter.renderable);

        LightManager::Builder(LightManager::Type::POINT)
                .color(color)
                .intensity(CONTINUOUS_LIGHT_INTENSITY)
                .falloff(LIGHT_FALLOFF)
                .castShadows(false)
                .build(engine, emitter.renderable);

        scene.addEntity(emitter.renderable);

        emitter.radii = { xdist(gen), ydist(gen), zdist(gen) };
        emitter.frequencies = { fdist(gen), fdist(gen), fdist(gen) };
        emitter.phase = pdist(gen);

        app.emitters.push_back(emitter);
    }
}

// Sets up the initial state for fireworks mode.
void setupFireworks(App& app) {
    for (int i = 0; i < app.emitters.size(); ++i) {
        auto& emitter = app.emitters[i];
        emitter.timeToBurst = (float(i) / float(app.emitters.size())) * app.ui.fireworksDelay;
        for (auto& p: emitter.particles) {
            // Make particles "dead" until the first burst.
            p.age = 1e6f;
        }
    }
}

// Animates a single emitter and its particles.
void animateEmitter(Emitter& emitter, TransformManager& tcm, LightManager& lm, double now,
        double dt, std::mt19937& gen, const App& app) {
    // If particles are frozen, skip all updates for this emitter.
    // This also effectively freezes the emitter's movement.
    if (app.ui.particlesFrozen) {
        return;
    }

    // Animate the emitter's transform (a Lissajous curve)
    auto emitterInstance = tcm.getInstance(emitter.renderable);
    float3 emitter_pos;
    if (!app.ui.freezeEmitters) {
        const float t = float(now) * 0.5f + emitter.phase;
        emitter_pos = { emitter.radii.x * std::sin(emitter.frequencies.x * t),
                        emitter.radii.y * std::sin(emitter.frequencies.y * t),
                        emitter.radii.z * std::cos(emitter.frequencies.z * t) };
        tcm.setTransform(emitterInstance, mat4f::translation(emitter_pos));
    } else {
        emitter_pos = tcm.getTransform(emitterInstance)[3].xyz;
    }

    // Update the light's intensity.
    auto lightInstance = lm.getInstance(emitter.renderable);
    if (!app.ui.lightsEnabled) {
        lm.setIntensity(lightInstance, 0.0f);
    } else {
        if (app.ui.emitterMode == App::EmitterMode::FIREWORKS) {
            // Fade the light's intensity with the age of the particles.
            const auto& p = emitter.particles[0];
            const float scale = std::max(0.0f, 1.0f - (p.age / p.lifetime));
            lm.setIntensity(lightInstance, CONTINUOUS_LIGHT_INTENSITY * scale);
        } else {
            // In continuous mode, the light has a constant intensity.
            lm.setIntensity(lightInstance, CONTINUOUS_LIGHT_INTENSITY);
        }
    }

    // In fireworks mode, check if it's time for a burst.
    if (app.ui.emitterMode == App::EmitterMode::FIREWORKS) {
        emitter.timeToBurst -= float(dt);
        if (emitter.timeToBurst <= 0.0f) {
            // Time to burst! Reset all particles.
            for (auto& p: emitter.particles) {
                resetParticle(p, gen, emitter_pos);
            }
            // Reset timer for the next burst.
            emitter.timeToBurst += app.ui.fireworksDelay;
        }
    }

    // Animate particles relative to the emitter
    for (uint32_t i = 0; i < PARTICLE_COUNT; ++i) {
        auto& p = emitter.particles[i];
        p.age += float(dt);

        // In continuous mode, reset particles when their lifetime expires.
        if (app.ui.emitterMode == App::EmitterMode::CONTINUOUS) {
            if (p.age > p.lifetime) {
                resetParticle(p, gen, emitter_pos);
            }
        }

        // If the particle is alive, compute its transform. Otherwise, make it invisible.
        if (p.age <= p.lifetime) {
            const float scale = std::max(0.0f, 1.0f - (p.age / p.lifetime));
            float3 particle_travel = p.velocity * p.age;
            if (app.ui.gravityStrength > 0.0f) {
                constexpr float3 g = { 0.0f, -9.8f, 0.0f };
                particle_travel += 0.5f * g * app.ui.gravityStrength * p.age * p.age;
            }
            const float3 particle_pos = p.birthPosition - emitter_pos + particle_travel;
            const mat4f rotation = mat4f::rotation(p.age * p.rotationSpeed, p.rotationAxis);
            emitter.localTransforms[i] =
                    mat4f::translation(particle_pos) * rotation * mat4f::scaling(scale);
        } else {
            emitter.localTransforms[i] = mat4f::scaling(0.0f);
        }
    }

    emitter.instanceBuffer->setLocalTransforms(emitter.localTransforms.data(), PARTICLE_COUNT);
}
