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

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>
#include <filamentapp/IBL.h>

#include <filament/Camera.h>
#include <filament/ColorGrading.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/TextureProvider.h>

#include <viewer/AutomationEngine.h>
#include <viewer/AutomationSpec.h>
#include <viewer/ViewerGui.h>

#include <camutils/Manipulator.h>

#include <private/filament/EngineEnums.h>

#include <getopt/getopt.h>

#include <utils/NameComponentManager.h>
#include <utils/Log.h>

#include <math/vec3.h>
#include <math/vec4.h>
#include <math/mat3.h>
#include <math/norm.h>

#include <imgui.h>
#include <filagui/ImGuiExtensions.h>

#include <cgltf.h>

#include <algorithm>
#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

#include "generated/resources/gltf_demo.h"
#include "materials/uberarchive.h"

#if FILAMENT_DISABLE_MATOPT
#   define OPTIMIZE_MATERIALS false
#else
#   define OPTIMIZE_MATERIALS true
#endif

using namespace filament;
using namespace filament::math;
using namespace filament::viewer;

using namespace filament::gltfio;
using namespace utils;

enum MaterialSource {
    JITSHADER,
    UBERSHADER,
};

struct App {
    Engine* engine;
    ViewerGui* viewer;
    Config config;
    Camera* mainCamera;
    Entity rootTransformEntity;

    AssetLoader* assetLoader;
    FilamentAsset* asset = nullptr;
    FilamentInstance* instance = nullptr;
    NameComponentManager* names;

    MaterialProvider* materials;
    MaterialSource materialSource = JITSHADER;

    gltfio::ResourceLoader* resourceLoader = nullptr;
    gltfio::TextureProvider* stbDecoder = nullptr;
    gltfio::TextureProvider* ktxDecoder = nullptr;
    bool recomputeAabb = false;

    bool actualSize = false;
    bool originIsFarAway = false;
    float originDistance = 1.0f;

    struct Scene {
        Entity groundPlane;
        VertexBuffer* groundVertexBuffer;
        IndexBuffer* groundIndexBuffer;
        Material* groundMaterial;

        Material* overdrawMaterial;
        // use layer 7 because 0, 1 and 2 are used by FilamentApp
        static constexpr auto OVERDRAW_VISIBILITY_LAYER = 7u;   // overdraw renderables View layer
        static constexpr auto OVERDRAW_LAYERS = 4u;             // unique overdraw colors
        std::array<Entity, OVERDRAW_LAYERS> overdrawVisualizer;
        std::array<MaterialInstance*, OVERDRAW_LAYERS> overdrawMaterialInstances;
        VertexBuffer* fullScreenTriangleVertexBuffer;
        IndexBuffer* fullScreenTriangleIndexBuffer;
    } scene;

    ColorGradingSettings lastColorGradingOptions = { .enabled = false };

    ColorGrading* colorGrading = nullptr;

    std::string notificationText;
    std::string messageBoxText;
    std::string settingsFile;
    std::string batchFile;

    AutomationSpec* automationSpec = nullptr;
    AutomationEngine* automationEngine = nullptr;
    bool screenshot = false;
    uint8_t screenshotSeq = 0;
};

static const char* DEFAULT_IBL = "assets/ibl/lightroom_14b";

static void printUsage(char* name) {
    std::string const exec_name(Path(name).getName());
    std::string usage(
        "SHOWCASE renders the specified glTF file, or a built-in file if none is specified\n"
        "Usage:\n"
        "    SHOWCASE [options] <gltf path>\n"
        "Options:\n"
        "   --help, -h\n"
        "       Prints this message\n\n"
        "   --api, -a\n"
        "       Specify the backend API: "

// Matches logic in filament/backend/src/PlatformFactory.cpp for Backend::DEFAULT
#if defined(IOS) || defined(__APPLE__)
        "opengl, vulkan, or metal (default)"
#elif defined(FILAMENT_DRIVER_SUPPORTS_VULKAN)
        "opengl, vulkan (default), or metal"
#else
        "opengl (default), vulkan, or metal"
#endif
        "\n\n"

        "   --feature-level=<1|2|3>, -f <1|2|3>\n"
        "       Specify the feature level to use. The default is the highest supported feature level.\n\n"
        "   --batch=<path to JSON file or 'default'>, -b\n"
        "       Start automation using the given JSON spec, then quit the app\n\n"
        "   --headless, -e\n"
        "       Use a headless swapchain; ignored if --batch is not present\n\n"
        "   --ibl=<path>, -i <path>\n"
        "       Override the built-in IBL\n"
        "       path can either be a directory containing IBL data files generated by cmgen,\n"
        "       or, a .hdr equiretangular image file\n\n"
        "   --actual-size, -s\n"
        "       Do not scale the model to fit into a unit cube\n\n"
        "   --recompute-aabb, -r\n"
        "       Ignore the min/max attributes in the glTF file\n\n"
        "   --settings=<path to JSON file>, -t\n"
        "       Apply the settings in the given JSON file\n\n"
        "   --ubershader, -u\n"
        "       Enable ubershaders (improves load time, adds shader complexity)\n\n"
        "   --camera=<camera mode>, -c <camera mode>\n"
        "       Set the camera mode: orbit (default) or flight\n"
        "       Flight mode uses the following controls:\n"
        "           Click and drag the mouse to pan the camera\n"
        "           Use the scroll wheel to adjust movement speed\n"
        "           W / S: forward / backward\n"
        "           A / D: left / right\n"
        "           E / Q: up / down\n\n"
        "   --eyes=<stereoscopic eyes>, -y <stereoscopic eyes>\n"
        "       Sets the number of stereoscopic eyes (default: 2) when stereoscopic rendering is\n"
        "       enabled.\n\n"
        "   --split-view, -v\n"
        "       Splits the window into 4 views\n\n"
        "   --vulkan-gpu-hint=<hint>, -g\n"
        "       Vulkan backend allows user to choose their GPU.\n"
        "       You can provide the index of the GPU or\n"
        "       a substring to match against the device name\n\n"
    );
    const std::string from("SHOWCASE");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    std::cout << usage;
}

static std::ifstream::pos_type getFileSize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

static int handleCommandLineArguments(int argc, char* argv[], App* app) {
    static constexpr const char* OPTSTR = "ha:f:i:usc:rt:b:evg:";
    static const struct option OPTIONS[] = {
        { "help",            no_argument,          nullptr, 'h' },
        { "api",             required_argument,    nullptr, 'a' },
        { "feature-level",   required_argument,    nullptr, 'f' },
        { "batch",           required_argument,    nullptr, 'b' },
        { "headless",        no_argument,          nullptr, 'e' },
        { "ibl",             required_argument,    nullptr, 'i' },
        { "ubershader",      no_argument,          nullptr, 'u' },
        { "actual-size",     no_argument,          nullptr, 's' },
        { "camera",          required_argument,    nullptr, 'c' },
        { "eyes",            required_argument,    nullptr, 'y' },
        { "recompute-aabb",  no_argument,          nullptr, 'r' },
        { "settings",        required_argument,    nullptr, 't' },
        { "split-view",      no_argument,          nullptr, 'v' },
        { "vulkan-gpu-hint", required_argument,    nullptr, 'g' },
        { nullptr, 0, nullptr, 0 }
    };
    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, OPTSTR, OPTIONS, &option_index)) >= 0) {
        std::string const arg(optarg ? optarg : "");
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
            case 'f':
                if (arg == "1") {
                    app->config.featureLevel = backend::FeatureLevel::FEATURE_LEVEL_1;
                } else if (arg == "2") {
                    app->config.featureLevel = backend::FeatureLevel::FEATURE_LEVEL_2;
                } else if (arg == "3") {
                    app->config.featureLevel = backend::FeatureLevel::FEATURE_LEVEL_3;
                } else {
                    std::cerr << "Unrecognized feature level. Must be 1, 2 or 3.\n";
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
            case 'y': {
                int eyeCount = 0;
                try {
                    eyeCount = std::stoi(arg);
                } catch (std::invalid_argument &e) { }
                if (eyeCount >= 1 && eyeCount <= CONFIG_MAX_STEREOSCOPIC_EYES) {
                    app->config.stereoscopicEyeCount = eyeCount;
                } else {
                    std::cerr << "Eye count must be between 1 and CONFIG_MAX_STEREOSCOPIC_EYES ("
                              << (int) CONFIG_MAX_STEREOSCOPIC_EYES << ") (inclusive).\n";
                }
                break;
            }
            case 'e':
                app->config.headless = true;
                break;
            case 'i':
                app->config.iblDirectory = arg;
                break;
            case 'u':
                app->materialSource = UBERSHADER;
                break;
            case 's':
                app->actualSize = true;
                break;
            case 'r':
                app->recomputeAabb = true;
                break;
            case 't':
                app->settingsFile = arg;
                break;
            case 'b': {
                app->batchFile = arg;
                break;
            }
            case 'v': {
                app->config.splitView = true;
                break;
            }
            case 'g': {
                app->config.vulkanGPUHint = arg;
                break;
            }
        }
    }
    if (app->config.headless && app->batchFile.empty()) {
        std::cerr << "--headless is allowed only when --batch is present." << std::endl;
        app->config.headless = false;
    }
    return optind;
}

static bool loadSettings(const char* filename, Settings* out) {
    auto contentSize = getFileSize(filename);
    if (contentSize <= 0) {
        return false;
    }
    std::ifstream in(filename, std::ifstream::binary | std::ifstream::in);
    std::vector<char> json(static_cast<unsigned long>(contentSize));
    if (!in.read(json.data(), contentSize)) {
        return false;
    }
    JsonSerializer serializer;
    return serializer.readJson(json.data(), contentSize, out);
}

static void createGroundPlane(Engine* engine, Scene* scene, App& app) {
    auto& em = EntityManager::get();
    Material* shadowMaterial = Material::Builder()
            .package(GLTF_DEMO_GROUNDSHADOW_DATA, GLTF_DEMO_GROUNDSHADOW_SIZE)
            .build(*engine);
    auto& viewerOptions = app.viewer->getSettings().viewer;
    shadowMaterial->setDefaultParameter("strength", viewerOptions.groundShadowStrength);

    const static uint32_t indices[] = {
            0, 1, 2, 2, 3, 0
    };

    Aabb aabb = app.asset->getBoundingBox();
    if (!app.actualSize) {
        mat4f const transform = fitIntoUnitCube(aabb, 4);
        aabb = aabb.transform(transform);
    }

    float3 planeExtent{10.0f * aabb.extent().x, 0.0f, 10.0f * aabb.extent().z};

    const static float3 vertices[] = {
            { -planeExtent.x, 0, -planeExtent.z },
            { -planeExtent.x, 0,  planeExtent.z },
            {  planeExtent.x, 0,  planeExtent.z },
            {  planeExtent.x, 0, -planeExtent.z },
    };

    short4 const tbn = packSnorm16(
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

    Entity const groundPlane = em.create();
    RenderableManager::Builder(1)
            .boundingBox({
                    {}, { planeExtent.x, 1e-4f, planeExtent.z }
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

static constexpr float4 sFullScreenTriangleVertices[3] = {
        { -1.0f, -1.0f, 1.0f, 1.0f },
        {  3.0f, -1.0f, 1.0f, 1.0f },
        { -1.0f,  3.0f, 1.0f, 1.0f }
};

static const uint16_t sFullScreenTriangleIndices[3] = { 0, 1, 2 };

static void createOverdrawVisualizerEntities(Engine* engine, Scene* scene, App& app) {
    Material* material = Material::Builder()
            .package(GLTF_DEMO_OVERDRAW_DATA, GLTF_DEMO_OVERDRAW_SIZE)
            .build(*engine);

    const float3 overdrawColors[App::Scene::OVERDRAW_LAYERS] = {
            {0.0f, 0.0f, 1.0f},     // blue         (overdrawn 1 time)
            {0.0f, 1.0f, 0.0f},     // green        (overdrawn 2 times)
            {1.0f, 0.0f, 1.0f},     // magenta      (overdrawn 3 times)
            {1.0f, 0.0f, 0.0f}      // red          (overdrawn 4+ times)
    };

    for (auto i = 0; i < App::Scene::OVERDRAW_LAYERS; i++) {
        MaterialInstance* matInstance = material->createInstance();
        // TODO: move this to the material definition.
        matInstance->setStencilCompareFunction(MaterialInstance::StencilCompareFunc::E);
        // The stencil value represents the number of times the fragment has been written to.
        // We want 0-1 writes to be the regular color. Overdraw visualization starts at 2+ writes,
        // which represents a fragment overdrawn 1 time.
        matInstance->setStencilReferenceValue(i + 2);
        matInstance->setParameter("color", overdrawColors[i]);
        app.scene.overdrawMaterialInstances[i] = matInstance;
    }
    auto& lastMi = app.scene.overdrawMaterialInstances[App::Scene::OVERDRAW_LAYERS - 1];
    // This seems backwards, but it isn't. The comparison function compares:
    // the reference value (left side) <= stored stencil value (right side)
    lastMi->setStencilCompareFunction(MaterialInstance::StencilCompareFunc::LE);

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

    auto& em = EntityManager::get();
    const auto& matInstances = app.scene.overdrawMaterialInstances;
    for (auto i = 0; i < App::Scene::OVERDRAW_LAYERS; i++) {
        Entity overdrawEntity = em.create();
        RenderableManager::Builder(1)
                .boundingBox({{}, {1.0f, 1.0f, 1.0f}})
                .material(0, matInstances[i])
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vertexBuffer, indexBuffer, 0, 3)
                .culling(false)
                .priority(7u)   // ensure the overdraw primitives are drawn last
                .layerMask(0xFF, 1u << App::Scene::OVERDRAW_VISIBILITY_LAYER)
                .build(*engine, overdrawEntity);
        scene->addEntity(overdrawEntity);
        app.scene.overdrawVisualizer[i] = overdrawEntity;
    }

    app.scene.overdrawMaterial = material;
    app.scene.fullScreenTriangleVertexBuffer = vertexBuffer;
    app.scene.fullScreenTriangleIndexBuffer = indexBuffer;
}

static void onClick(App& app, View* view, ImVec2 pos) {
    view->pick(pos.x, pos.y, [&app](View::PickingQueryResult const& result){
        if (const char* name = app.asset->getName(result.renderable); name) {
            app.notificationText = name;
        } else {
            app.notificationText.clear();
        }
    });
}

static utils::Path getPathForIBLAsset(std::string_view string) {
    auto isIBL = [] (utils::Path file) -> bool {
        return file.getExtension() == "ktx" || file.getExtension() == "hdr";
    };

    utils::Path filename{ string };
    if (!filename.exists()) {
        std::cerr << "file " << filename << " not found!" << std::endl;
        return {};
    }

    if (filename.isDirectory()) {
        std::vector<Path> files = filename.listContents();
        if (std::none_of(files.cbegin(), files.cend(), isIBL)) {
            return {};
        }
    } else if (!isIBL(filename)) {
        return {};
    }

    return filename;
}

static utils::Path getPathForGLTFAsset(std::string_view string) {
    auto isGLTF = [] (utils::Path file) -> bool {
        return file.getExtension() == "gltf" || file.getExtension() == "glb";
    };

    utils::Path filename{ string };
    if (!filename.exists()) {
        std::cerr << "file " << filename << " not found!" << std::endl;
        return {};
    }

    if (filename.isDirectory()) {
        std::vector<Path> files = filename.listContents();
        auto it = std::find_if(files.cbegin(), files.cend(), isGLTF);
        if (it == files.end()) {
            return {};
        }
        filename = *it;
    } else if (!isGLTF(filename)) {
        return {};
    }

    return filename;
}

static bool checkGLTFAsset(const utils::Path& filename) {
    // Peek at the file size to allow pre-allocation.
    long const contentSize = static_cast<long>(getFileSize(filename.c_str()));
    if (contentSize <= 0) {
        std::cerr << "Unable to open " << filename << std::endl;
        return false;
    }

    // Consume the glTF file.
    std::ifstream in(filename.c_str(), std::ifstream::binary | std::ifstream::in);
    std::vector<uint8_t> buffer(static_cast<unsigned long>(contentSize));
    if (!in.read((char*) buffer.data(), contentSize)) {
        std::cerr << "Unable to read " << filename << std::endl;
        return false;
    }

    // Try parsing the glTF file to check the validity of the file format.
    cgltf_options options{};
    cgltf_data* sourceAsset = nullptr;
    cgltf_result result = cgltf_parse(&options, buffer.data(), contentSize, &sourceAsset);
    cgltf_free(sourceAsset);
    if (result != cgltf_result_success) {
        slog.e << "Unable to parse glTF file." << io::endl;
        return false;
    }
    return true;
};


int main(int argc, char** argv) {
    App app;

    app.config.title = "Filament";
    app.config.iblDirectory = FilamentApp::getRootAssetsPath() + DEFAULT_IBL;

    int const optionIndex = handleCommandLineArguments(argc, argv, &app);

    utils::Path filename;
    int const num_args = argc - optionIndex;
    if (num_args >= 1) {
        filename = getPathForGLTFAsset(argv[optionIndex]);
        if (filename.isEmpty()) {
            std::cerr << "no glTF file found in " << filename << std::endl;
            return 1;
        }
    }

    auto loadAsset = [&app](const utils::Path& filename) {
        // Peek at the file size to allow pre-allocation.
        long const contentSize = static_cast<long>(getFileSize(filename.c_str()));
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
        app.asset = app.assetLoader->createAsset(buffer.data(), buffer.size());
        if (!app.asset) {
            std::cerr << "Unable to parse " << filename << std::endl;
            exit(1);
        }

        // pre-compile all material variants
        std::set<Material*> materials;
        RenderableManager const& rcm = app.engine->getRenderableManager();
        Slice<Entity> const renderables{
                app.asset->getRenderableEntities(), app.asset->getRenderableEntityCount() };
        for (Entity const e: renderables) {
            auto ri = rcm.getInstance(e);
            size_t const c = rcm.getPrimitiveCount(ri);
            for (size_t i = 0; i < c; i++) {
                MaterialInstance* const mi = rcm.getMaterialInstanceAt(ri, i);
                Material* ma = const_cast<Material *>(mi->getMaterial());
                materials.insert(ma);
            }
        }
        for (Material* ma : materials) {
            // Don't attempt to precompile shaders on WebGL.
            // Chrome already suffers from slow shader compilation:
            // https://github.com/google/filament/issues/6615
            // Precompiling shaders exacerbates the problem.
#if !defined(__EMSCRIPTEN__)
            // First compile high priority variants
            ma->compile(Material::CompilerPriorityQueue::HIGH,
                    UserVariantFilterBit::DIRECTIONAL_LIGHTING |
                    UserVariantFilterBit::DYNAMIC_LIGHTING |
                    UserVariantFilterBit::SHADOW_RECEIVER);

            // and then, everything else at low priority, except STE, which is very uncommon.
            ma->compile(Material::CompilerPriorityQueue::LOW,
                    UserVariantFilterBit::FOG |
                    UserVariantFilterBit::SKINNING |
                    UserVariantFilterBit::SSR |
                    UserVariantFilterBit::VSM);
#endif
        }

        app.instance = app.asset->getInstance();
        buffer.clear();
        buffer.shrink_to_fit();
    };

    auto loadResources = [&app] (const utils::Path& filename) {
        // Load external textures and buffers.
        std::string const gltfPath = filename.getAbsolutePath();
        ResourceConfiguration configuration = {};
        configuration.engine = app.engine;
        configuration.gltfPath = gltfPath.c_str();
        configuration.normalizeSkinningWeights = true;

        if (!app.resourceLoader) {
            app.resourceLoader = new gltfio::ResourceLoader(configuration);
            app.stbDecoder = createStbProvider(app.engine);
            app.ktxDecoder = createKtx2Provider(app.engine);
            app.resourceLoader->addTextureProvider("image/png", app.stbDecoder);
            app.resourceLoader->addTextureProvider("image/jpeg", app.stbDecoder);
            app.resourceLoader->addTextureProvider("image/ktx2", app.ktxDecoder);
        } else {
            app.resourceLoader->setConfiguration(configuration);
        }

        if (!app.resourceLoader->asyncBeginLoad(app.asset)) {
            std::cerr << "Unable to start loading resources for " << filename << std::endl;
            exit(1);
        }

        if (app.recomputeAabb) {
            app.asset->getInstance()->recomputeBoundingBoxes();
        }

        app.asset->releaseSourceData();

        // Enable stencil writes on all material instances.
        const size_t matInstanceCount = app.instance->getMaterialInstanceCount();
        MaterialInstance* const* const instances = app.instance->getMaterialInstances();
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

    auto setup = [&](Engine* engine, View* view, Scene* scene) {
        app.engine = engine;
        app.names = new NameComponentManager(EntityManager::get());
        app.viewer = new ViewerGui(engine, scene, view, 410);
        app.viewer->getSettings().viewer.autoScaleEnabled = !app.actualSize;

        engine->enableAccurateTranslations();
        auto& tcm = engine->getTransformManager();
        app.rootTransformEntity = engine->getEntityManager().create();
        tcm.create(app.rootTransformEntity);
        tcm.create(view->getFogEntity());

        const bool batchMode = !app.batchFile.empty();

        // First check if a custom automation spec has been provided. If it fails to load, the app
        // must be closed since it could be invoked from a script.
        if (batchMode && app.batchFile != "default") {
            auto size = getFileSize(app.batchFile.c_str());
            if (size > 0) {
                std::ifstream in(app.batchFile, std::ifstream::binary | std::ifstream::in);
                std::vector<char> json(static_cast<unsigned long>(size));
                in.read(json.data(), size);
                app.automationSpec = AutomationSpec::generate(json.data(), size);
                if (!app.automationSpec) {
                    std::cerr << "Unable to parse automation spec: " << app.batchFile << std::endl;
                    exit(1);
                }
            } else {
                std::cerr << "Unable to load automation spec: " << app.batchFile << std::endl;
                exit(1);
            }
        }

        // If no custom spec has been provided, or if in interactive mode, load the default spec.
        if (!app.automationSpec) {
            app.automationSpec = AutomationSpec::generateDefaultTestCases();
        }

        app.automationEngine = new AutomationEngine(app.automationSpec, &app.viewer->getSettings());

        if (batchMode) {
            app.automationEngine->startBatchMode();
            auto options = app.automationEngine->getOptions();
            options.sleepDuration = 0.0;
            options.exportScreenshots = true;
            options.exportSettings = true;
            app.automationEngine->setOptions(options);
            app.viewer->stopAnimation();
        }

        if (!app.settingsFile.empty()) {
            bool const success = loadSettings(app.settingsFile.c_str(), &app.viewer->getSettings());
            if (success) {
                std::cout << "Loaded settings from " << app.settingsFile << std::endl;
            } else {
                std::cerr << "Failed to load settings from " << app.settingsFile << std::endl;
            }
        }

        app.materials = (app.materialSource == JITSHADER) ?
                createJitShaderProvider(engine, OPTIMIZE_MATERIALS) :
                createUbershaderProvider(engine, UBERARCHIVE_DEFAULT_DATA, UBERARCHIVE_DEFAULT_SIZE);

        app.assetLoader = AssetLoader::create({engine, app.materials, app.names });
        app.mainCamera = &view->getCamera();
        if (filename.isEmpty()) {
            app.asset = app.assetLoader->createAsset(
                    GLTF_DEMO_DAMAGEDHELMET_DATA,
                    GLTF_DEMO_DAMAGEDHELMET_SIZE);
            app.instance = app.asset->getInstance();
        } else {
            loadAsset(filename);
        }

        loadResources(filename);
        app.viewer->setAsset(app.asset, app.instance);

        createGroundPlane(engine, scene, app);
        createOverdrawVisualizerEntities(engine, scene, app);

        app.viewer->setUiCallback([&app, scene, view, engine] () {
            auto& automation = *app.automationEngine;

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                ImVec2 pos = ImGui::GetMousePos();
                pos.x -= app.viewer->getSidebarWidth();
                pos.x *= ImGui::GetIO().DisplayFramebufferScale.x;
                pos.y *= ImGui::GetIO().DisplayFramebufferScale.y;
                if (pos.x > 0) {
                    pos.y = view->getViewport().height - 1 - pos.y;
                    onClick(app, view, pos);
                }
            }

            const ImVec4 yellow(1.0f,1.0f,0.0f,1.0f);

            if (!app.notificationText.empty()) {
                ImGui::TextColored(yellow, "Picked %s", app.notificationText.c_str());
                ImGui::Spacing();
            }

            float const progress = app.resourceLoader->asyncGetLoadProgress();
            if (progress < 1.0) {
                ImGui::ProgressBar(progress);
            } else {
                // The model is now fully loaded, so let automation know.
                automation.signalBatchMode();
            }

            // The screenshots do not include the UI, but we auto-open the Automation UI group
            // when in batch mode. This is useful when a human is observing progress.
            const int flags = automation.isBatchModeEnabled() ? ImGuiTreeNodeFlags_DefaultOpen : 0;

            if (ImGui::CollapsingHeader("Automation", flags)) {
                ImGui::Indent();

                if (automation.isRunning()) {
                    ImGui::TextColored(yellow, "Test case %zu / %zu",
                            automation.currentTest(), automation.testCount());
                } else {
                    ImGui::TextColored(yellow, "%zu test cases", automation.testCount());
                }

                auto options = automation.getOptions();

                ImGui::PushItemWidth(150);
                ImGui::SliderFloat("Sleep (seconds)", &options.sleepDuration, 0.0, 5.0);
                ImGui::PopItemWidth();

                // Hide the tooltip during automation to avoid photobombing the screenshot.
                if (ImGui::IsItemHovered() && !automation.isRunning()) {
                    ImGui::SetTooltip("Specifies the amount of time to sleep between test cases.");
                }

                ImGui::Checkbox("Export screenshot for each test", &options.exportScreenshots);
                ImGui::Checkbox("Export settings JSON for each test", &options.exportSettings);

                automation.setOptions(options);

                if (automation.isRunning()) {
                    if (ImGui::Button("Stop batch test")) {
                        automation.stopRunning();
                    }
                } else if (ImGui::Button("Run batch test")) {
                    automation.startRunning();
                }

                if (ImGui::Button("Export view settings")) {
                    AutomationEngine::exportSettings(app.viewer->getSettings(), "settings.json");
                    app.messageBoxText = automation.getStatusMessage();
                    ImGui::OpenPopup("MessageBox");
                }
                ImGui::Unindent();
            }

            if (ImGui::CollapsingHeader("Stats")) {
                ImGui::Indent();
                ImGui::Text("%zu entities in the asset", app.asset->getEntityCount());
                ImGui::Text("%zu renderables (excluding UI)", scene->getRenderableCount());
                ImGui::Text("%zu skipped frames", FilamentApp::get().getSkippedFrameCount());
                ImGui::Unindent();
            }

            if (ImGui::CollapsingHeader("Debug")) {
                auto& debug = engine->getDebugRegistry();
                if (engine->getBackend() == Engine::Backend::METAL) {
                    if (ImGui::Button("Capture frame")) {
                        bool* captureFrame =
                                debug.getPropertyAddress<bool>("d.renderer.doFrameCapture");
                        *captureFrame = true;
                    }
                }
                if (ImGui::Button("Screenshot")) {
                    app.screenshot = true;
                }
                ImGui::Checkbox("Disable buffer padding",
                        debug.getPropertyAddress<bool>("d.renderer.disable_buffer_padding"));
                ImGui::Checkbox("Disable sub-passes",
                        debug.getPropertyAddress<bool>("d.renderer.disable_subpasses"));
                ImGui::Checkbox("Camera at origin",
                        debug.getPropertyAddress<bool>("d.view.camera_at_origin"));
                ImGui::Checkbox("Far Origin", &app.originIsFarAway);
                ImGui::SliderFloat("Origin", &app.originDistance, 0, 1);
                ImGui::Checkbox("Far uses shadow casters",
                        debug.getPropertyAddress<bool>("d.shadowmap.far_uses_shadowcasters"));
                ImGui::Checkbox("Focus shadow casters",
                        debug.getPropertyAddress<bool>("d.shadowmap.focus_shadowcasters"));
                ImGui::Checkbox("Disable light frustum alignment",
                        debug.getPropertyAddress<bool>("d.shadowmap.disable_light_frustum_align"));
                ImGui::Checkbox("Depth clamp",
                        debug.getPropertyAddress<bool>("d.shadowmap.depth_clamp"));

                bool debugDirectionalShadowmap;
                if (debug.getProperty("d.shadowmap.debug_directional_shadowmap",
                        &debugDirectionalShadowmap)) {
                    ImGui::Checkbox("Debug DIR shadowmap", &debugDirectionalShadowmap);
                    debug.setProperty("d.shadowmap.debug_directional_shadowmap",
                            debugDirectionalShadowmap);
                }

                ImGui::Checkbox("Display Shadow Texture",
                        debug.getPropertyAddress<bool>("d.shadowmap.display_shadow_texture"));
                if (*debug.getPropertyAddress<bool>("d.shadowmap.display_shadow_texture")) {
                    int layerCount;
                    int levelCount;
                    debug.getProperty("d.shadowmap.display_shadow_texture_layer_count", &layerCount);
                    debug.getProperty("d.shadowmap.display_shadow_texture_level_count", &levelCount);
                    ImGui::Indent();
                    ImGui::SliderFloat("scale", debug.getPropertyAddress<float>(
                                    "d.shadowmap.display_shadow_texture_scale"), 0.0f, 8.0f);
                    ImGui::SliderFloat("contrast", debug.getPropertyAddress<float>(
                                    "d.shadowmap.display_shadow_texture_power"), 0.0f, 2.0f);
                    ImGui::SliderInt("layer", debug.getPropertyAddress<int>(
                                    "d.shadowmap.display_shadow_texture_layer"), 0, layerCount - 1);
                    ImGui::SliderInt("level", debug.getPropertyAddress<int>(
                                    "d.shadowmap.display_shadow_texture_level"), 0, levelCount - 1);
                    ImGui::SliderInt("channel", debug.getPropertyAddress<int>(
                                    "d.shadowmap.display_shadow_texture_channel"), 0, 3);
                    ImGui::Unindent();
                }
                bool debugFroxelVisualization;
                if (debug.getProperty("d.lighting.debug_froxel_visualization",
                        &debugFroxelVisualization)) {
                    ImGui::Checkbox("Froxel Visualization", &debugFroxelVisualization);
                    debug.setProperty("d.lighting.debug_froxel_visualization",
                            debugFroxelVisualization);
                }

                auto dataSource = debug.getDataSource("d.view.frame_info");
                if (dataSource.data) {
                    ImGuiExt::PlotLinesSeries("FrameInfo", 6,
                            [](int series) {
                                const ImVec4 colors[] = {
                                        { 1,    0, 0, 1 }, // target
                                        { 0, 0.5f, 0, 1 }, // frame-time
                                        { 0,    1, 0, 1 }, // frame-time denoised
                                        { 1,    1, 0, 1 }, // i
                                        { 1,    0, 1, 1 }, // d
                                        { 0,    1, 1, 1 }, // e

                                };
                                ImGui::PushStyleColor(ImGuiCol_PlotLines, colors[series]);
                            },
                            [](int series, void* buffer, int i) -> float {
                                auto const* p = (DebugRegistry::FrameHistory const*)buffer + i;
                                switch (series) {
                                    case 0:     return 0.03f * p->target;
                                    case 1:     return 0.03f * p->frameTime;
                                    case 2:     return 0.03f * p->frameTimeDenoised;
                                    case 3:     return p->pid_i * 0.5f / 100.0f + 0.5f;
                                    case 4:     return p->pid_d * 0.5f / 0.100f + 0.5f;
                                    case 5:     return p->pid_e * 0.5f / 1.000f + 0.5f;
                                    default:    return 0.0f;
                                }
                            },
                            [](int series) {
                                if (series < 6) ImGui::PopStyleColor();
                            },
                            const_cast<void*>(dataSource.data), int(dataSource.count), 0,
                            nullptr, 0.0f, 1.0f, { 0, 100 });
                }
#ifndef NDEBUG
                ImGui::SliderFloat("Kp", debug.getPropertyAddress<float>("d.view.pid.kp"), 0, 2);
                ImGui::SliderFloat("Ki", debug.getPropertyAddress<float>("d.view.pid.ki"), 0, 10);
                ImGui::SliderFloat("Kd", debug.getPropertyAddress<float>("d.view.pid.kd"), 0, 10);
#endif
                const auto overdrawVisibilityBit = (1u << App::Scene::OVERDRAW_VISIBILITY_LAYER);
                bool visualizeOverdraw = view->getVisibleLayers() & overdrawVisibilityBit;
                // TODO: enable after stencil buffer supported is added for Vulkan.
                const bool overdrawDisabled = engine->getBackend() == backend::Backend::VULKAN;
                ImGui::BeginDisabled(overdrawDisabled);
                ImGui::Checkbox(!overdrawDisabled ? "Visualize overdraw"
                                                  : "Visualize overdraw (disabled for Vulkan)",
                        &visualizeOverdraw);
                ImGui::EndDisabled();
                view->setVisibleLayers(overdrawVisibilityBit,
                        (uint8_t)visualizeOverdraw << App::Scene::OVERDRAW_VISIBILITY_LAYER);
                view->setStencilBufferEnabled(visualizeOverdraw);
            }

            if (ImGui::BeginPopupModal("MessageBox", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("%s", app.messageBoxText.c_str());
                if (ImGui::Button("OK", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        });
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        app.automationEngine->terminate();
        app.resourceLoader->asyncCancelLoad();
        app.assetLoader->destroyAsset(app.asset);
        app.materials->destroyMaterials();

        engine->destroy(app.scene.groundPlane);
        engine->destroy(app.scene.groundVertexBuffer);
        engine->destroy(app.scene.groundIndexBuffer);
        engine->destroy(app.scene.groundMaterial);
        engine->destroy(app.colorGrading);

        engine->destroy(app.scene.fullScreenTriangleVertexBuffer);
        engine->destroy(app.scene.fullScreenTriangleIndexBuffer);

        auto& em = EntityManager::get();
        for (auto e : app.scene.overdrawVisualizer) {
            engine->destroy(e);
            em.destroy(e);
        }

        for (auto mi : app.scene.overdrawMaterialInstances) {
            engine->destroy(mi);
        }
        engine->destroy(app.scene.overdrawMaterial);

        delete app.viewer;
        delete app.materials;
        delete app.names;
        delete app.resourceLoader;
        delete app.stbDecoder;
        delete app.ktxDecoder;

        AssetLoader::destroy(&app.assetLoader);
    };

    auto animate = [&app](Engine*, View*, double now) {
        app.resourceLoader->asyncUpdateLoad();

        // Optionally fit the model into a unit cube at the origin.
        app.viewer->updateRootTransform();

        // Gradually add renderables to the scene as their textures become ready.
        app.viewer->populateScene();

        app.viewer->applyAnimation(now);
    };

    auto resize = [&app](Engine*, View* view) {
        Camera& camera = view->getCamera();
        if (&camera == app.mainCamera) {
            // Don't adjust the aspect ratio of the main camera, this is done inside of
            // FilamentApp.cpp
            return;
        }
        const Viewport& vp = view->getViewport();
        double const aspectRatio = (double) vp.width / vp.height;
        camera.setScaling({1.0 / aspectRatio, 1.0 });
    };

    auto gui = [&app](Engine*, View*) {
        app.viewer->updateUserInterface();

        FilamentApp::get().setSidebarWidth(app.viewer->getSidebarWidth());
    };

    auto preRender = [&app](Engine* engine, View* view, Scene* scene, Renderer* renderer) {
        auto& rcm = engine->getRenderableManager();
        auto instance = rcm.getInstance(app.scene.groundPlane);
        const auto viewerOptions = app.automationEngine->getViewerOptions();
        rcm.setLayerMask(instance,
                0xff, viewerOptions.groundPlaneEnabled ? 0xff : 0x00);

        engine->setAutomaticInstancingEnabled(viewerOptions.autoInstancingEnabled);

        // Note that this focal length might be different from the slider value because the
        // automation engine applies Camera::computeEffectiveFocalLength when DoF is enabled.
        FilamentApp::get().setCameraFocalLength(viewerOptions.cameraFocalLength);
        FilamentApp::get().setCameraNearFar(viewerOptions.cameraNear, viewerOptions.cameraFar);

        const size_t cameraCount = app.asset->getCameraEntityCount();
        view->setCamera(app.mainCamera);

        const int currentCamera = app.viewer->getCurrentCamera();
        if (currentCamera > 0 && currentCamera <= cameraCount) {
            const utils::Entity* cameras = app.asset->getCameraEntities();
            Camera* camera = engine->getCameraComponent(cameras[currentCamera - 1]);
            assert_invariant(camera);
            view->setCamera(camera);

            // Override the aspect ratio in the glTF file and adjust the aspect ratio of this
            // camera to the viewport.
            const Viewport& vp = view->getViewport();
            double const aspectRatio = (double) vp.width / vp.height;
            camera->setScaling({1.0 / aspectRatio, 1.0});
        }

        static bool stereoscopicEnabled = false;
        if (stereoscopicEnabled != view->getStereoscopicOptions().enabled) {
            // Stereo was turned on/off.
            FilamentApp::get().reconfigureCameras();
            stereoscopicEnabled = view->getStereoscopicOptions().enabled;
        }

        app.scene.groundMaterial->setDefaultParameter(
                "strength", viewerOptions.groundShadowStrength);

        // This applies clear options, the skybox mask, and some camera settings.
        Camera& camera = view->getCamera();
        Skybox* skybox = scene->getSkybox();
        applySettings(engine, app.viewer->getSettings().viewer, &camera, skybox, renderer);

        // technically we don't need to do this each frame
        auto& tcm = engine->getTransformManager();
        TransformManager::Instance const& root = tcm.getInstance(app.rootTransformEntity);
        tcm.setParent(tcm.getInstance(camera.getEntity()), root);
        tcm.setParent(tcm.getInstance(app.asset->getRoot()), root);
        tcm.setParent(tcm.getInstance(view->getFogEntity()), root);

        // these values represent a point somewhere on Earth's surface
        float const d = app.originIsFarAway ? app.originDistance : 0.0f;
//        tcm.setTransform(root, mat4::translation(double3{ 67.0, -6366759.0, -21552.0 } * d));
        tcm.setTransform(root, mat4::translation(
                double3{ 2304097.1410110965, -4688442.9915525438, -3639452.5611694567 } * d));

        // Check if color grading has changed.
        ColorGradingSettings const& options = app.viewer->getSettings().view.colorGrading;
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
    };

    auto postRender = [&app](Engine* engine, View* view, Scene*, Renderer* renderer) {
        if (app.screenshot) {
            std::ostringstream stringStream;
            stringStream << "screenshot" << std::setfill('0') << std::setw(2) << +app.screenshotSeq;
            AutomationEngine::exportScreenshot(
                    view, renderer, stringStream.str() + ".ppm", false, app.automationEngine);
            ++app.screenshotSeq;
            app.screenshot = false;
        }
        if (app.automationEngine->shouldClose()) {
            FilamentApp::get().close();
            return;
        }
        AutomationEngine::ViewerContent const content = {
            .view = view,
            .renderer = renderer,
            .materials = app.instance->getMaterialInstances(),
            .materialCount = app.instance->getMaterialInstanceCount(),
        };
        app.automationEngine->tick(engine, content, ImGui::GetIO().DeltaTime);
    };

    FilamentApp& filamentApp = FilamentApp::get();
    filamentApp.animate(animate);
    filamentApp.resize(resize);

    filamentApp.setDropHandler([&](std::string_view path) {
        utils::Path filename = getPathForGLTFAsset(path);
        if (!filename.isEmpty()) {
            if (checkGLTFAsset(filename)) {
                app.resourceLoader->asyncCancelLoad();
                app.resourceLoader->evictResourceData();
                app.viewer->removeAsset();
                app.assetLoader->destroyAsset(app.asset);
                loadAsset(filename);
                loadResources(filename);
                app.viewer->setAsset(app.asset, app.instance);
            }
            return;
        }

        filename = getPathForIBLAsset(path);
        if (!filename.isEmpty()) {
            FilamentApp::get().loadIBL(path);
            return;
        }
    });

    filamentApp.run(app.config, setup, cleanup, gui, preRender, postRender);

    return 0;
}
