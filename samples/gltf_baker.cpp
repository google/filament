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
#define DEBUG_PATHTRACER 0

#include "app/Config.h"
#include "app/FilamentApp.h"
#include "app/IBL.h"

#include <filament/Engine.h>
#include <filament/Scene.h>
#include <filament/View.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/AssetPipeline.h>
#include <gltfio/FilamentAsset.h>
#include <gltfio/ResourceLoader.h>
#include <gltfio/SimpleViewer.h>

#include <image/ImageOps.h>
#include <image/LinearImage.h>

#include <imageio/ImageEncoder.h>

#include <utils/NameComponentManager.h>
#include <utils/JobSystem.h>

#include <math/vec2.h>

#include <getopt/getopt.h>

#include <atomic>
#include <fstream>
#include <string>

#include "generated/resources/gltf.h"
#include "generated/resources/resources.h"

using namespace filament;
using namespace gltfio;
using namespace utils;

enum AppState {
    EMPTY,
    LOADED,
    RENDERING,
    PREPPING,
    PREPPED,
    BAKING,
    BAKED,
    EXPORTED,
};

struct App {
    Engine* engine;
    Camera* camera;
    SimpleViewer* viewer;
    Config config;
    AssetLoader* loader;
    FilamentAsset* asset = nullptr;
    NameComponentManager* names;
    MaterialProvider* materials;
    bool actualSize = false;
    AppState state = EMPTY;
    utils::Path filename;
    image::LinearImage ambientOcclusion;
    image::LinearImage bentNormals;
    image::LinearImage meshNormals;
    image::LinearImage meshPositions;
    bool showOverlay = false;
    bool enablePrepScale = true;
    View* overlayView = nullptr;
    Scene* overlayScene = nullptr;
    VertexBuffer* overlayVb = nullptr;
    IndexBuffer* overlayIb = nullptr;
    Texture* overlayTexture = nullptr;
    MaterialInstance* overlayMaterial = nullptr;
    utils::Entity overlayEntity;
    AppState pushedState;
    gltfio::AssetPipeline* pipeline;
    uint32_t bakeResolution = 1024;
    bool preserveMaterialsForExport = true;

    // Secondary threads might write to the following fields.
    std::shared_ptr<std::string> statusText;
    std::shared_ptr<std::string> messageBoxText;
    std::atomic<bool> requestOverlayUpdate;
    std::atomic<bool> requestStatePop;
};

struct OverlayVertex {
    filament::math::float2 position;
    filament::math::float2 uv;
};

static OverlayVertex OVERLAY_VERTICES[4] = {
    {{0, 0}, {0, 0}},
    {{ 1000, 0}, {1, 0}},
    {{0,  1000}, {0, 1}},
    {{ 1000,  1000}, {1, 1}},
};

static const char* DEFAULT_IBL = "envs/venetian_crossroads";

static const char* INI_FILENAME = "gltf_baker.ini";

static void printUsage(char* name) {
    std::string exec_name(Path(name).getName());
    std::string usage(
        "SHOWCASE can perform AO baking on the specified glTF file. If no file is specified,"
        "it loads the most recently-used glTF file.\n"
        "Usage:\n"
        "    SHOWCASE [options] [gltf path]\n"
        "Options:\n"
        "   --help, -h\n"
        "       Prints this message\n\n"
        "   --actual-size, -s\n"
        "       Do not scale the model to fit into a unit cube\n\n"
    );
    const std::string from("SHOWCASE");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    std::cout << usage;
}

static int handleCommandLineArguments(int argc, char* argv[], App* app) {
    static constexpr const char* OPTSTR = "ha:i:us";
    static const struct option OPTIONS[] = {
        { "help",       no_argument,       nullptr, 'h' },
        { "actual-size", no_argument,      nullptr, 's' },
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

static void saveIniFile(App& app) {
    std::ofstream out(INI_FILENAME);
    out << "[recent]\n";
    out << "filename=" << app.filename.c_str() << "\n";
}

static void loadIniFile(App& app) {
    utils::Path iniPath(INI_FILENAME);
    if (!app.filename.isEmpty() || !iniPath.isFile()) {
        return;
    }
    std::ifstream infile(INI_FILENAME);
    std::string line;
    while (std::getline(infile, line)) {
        size_t sep = line.find('=');
        if (sep != std::string::npos) {
            std::string lhs = line.substr(0, sep);
            std::string rhs = line.substr(sep + 1);
            if (lhs == "filename") {
                utils::Path gltf = rhs;
                if (gltf.isFile()) {
                    app.filename = rhs;
                }
            }
        }
    }
}

static void updateOverlayVerts(App& app) {
    auto viewportSize = ImGui::GetIO().DisplaySize;
    viewportSize.x -= app.viewer->getSidebarWidth();
    OVERLAY_VERTICES[0].position.x = app.viewer->getSidebarWidth();
    OVERLAY_VERTICES[2].position.x = app.viewer->getSidebarWidth();
    OVERLAY_VERTICES[1].position.x = app.viewer->getSidebarWidth() + viewportSize.x;
    OVERLAY_VERTICES[3].position.x = app.viewer->getSidebarWidth() + viewportSize.x;
    OVERLAY_VERTICES[2].position.y = viewportSize.y;
    OVERLAY_VERTICES[3].position.y = viewportSize.y;
};

static void updateOverlay(App& app) {
    auto& rcm = app.engine->getRenderableManager();
    auto vb = app.overlayVb;
    auto ib = app.overlayIb;
    updateOverlayVerts(app);
    vb->setBufferAt(*app.engine, 0,
            VertexBuffer::BufferDescriptor(OVERLAY_VERTICES, 64, nullptr));
    rcm.destroy(app.overlayEntity);
    RenderableManager::Builder(1)
            .boundingBox({{ 0, 0, 0 }, { 1000, 1000, 1 }})
            .material(0, app.overlayMaterial)
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vb, ib, 0, 6)
            .culling(false)
            .receiveShadows(false)
            .castShadows(false)
            .build(*app.engine, app.overlayEntity);
}

static void updateOverlayTexture(App& app) {
    Engine& engine = *app.engine;
    int w = app.ambientOcclusion.getWidth();
    int h = app.ambientOcclusion.getHeight();
    void* data = app.ambientOcclusion.getPixelRef();
    Texture::PixelBufferDescriptor buffer(data, size_t(w * h * 4),
            Texture::Format::R, Texture::Type::FLOAT);
    app.overlayTexture->setImage(engine, 0, std::move(buffer));
}

static void createOverlayTexture(App& app) {
    Engine& engine = *app.engine;
    using MinFilter = TextureSampler::MinFilter;
    using MagFilter = TextureSampler::MagFilter;

    int w = app.ambientOcclusion.getWidth();
    int h = app.ambientOcclusion.getHeight();
    void* data = app.ambientOcclusion.getPixelRef();
    Texture::PixelBufferDescriptor buffer(data, size_t(w * h * 4),
            Texture::Format::R, Texture::Type::FLOAT);
    auto tex = Texture::Builder()
            .width(uint32_t(w))
            .height(uint32_t(h))
            .levels(1)
            .sampler(Texture::Sampler::SAMPLER_2D)
            .format(Texture::InternalFormat::R8)
            .build(engine);
    tex->setImage(engine, 0, std::move(buffer));

    TextureSampler sampler(MinFilter::LINEAR, MagFilter::LINEAR);
    app.overlayMaterial->setParameter("luma", tex, sampler);

    engine.destroy(app.overlayTexture);
    app.overlayTexture = tex;
}

static void createOverlay(App& app) {
    Engine& engine = *app.engine;

    static constexpr uint16_t OVERLAY_INDICES[6] = { 0, 1, 2, 3, 2, 1 };

    auto vb = VertexBuffer::Builder()
            .vertexCount(4)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 16)
            .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2, 8, 16)
            .build(engine);
    auto ib = IndexBuffer::Builder()
            .indexCount(6)
            .bufferType(IndexBuffer::IndexType::USHORT)
            .build(engine);
    ib->setBuffer(engine,
            IndexBuffer::BufferDescriptor(OVERLAY_INDICES, 12, nullptr));
    auto mat = Material::Builder()
            .package(RESOURCES_AOPREVIEW_DATA, RESOURCES_AOPREVIEW_SIZE)
            .build(engine);
    auto matInstance = mat->createInstance();

    app.overlayVb = vb;
    app.overlayIb = ib;
    app.overlayEntity = EntityManager::get().create();
    app.overlayMaterial = matInstance;
}

static void loadAsset(App& app) {
    std::cout << "Loading " << app.filename << "..." << std::endl;

    if (app.filename.getExtension() == "glb") {
        std::cerr << "GLB files are not yet supported." << std::endl;
        exit(1);
    }

    // Peek at the file size to allow pre-allocation.
    long contentSize = static_cast<long>(getFileSize(app.filename.c_str()));
    if (contentSize <= 0) {
        std::cerr << "Unable to open " << app.filename << std::endl;
        exit(1);
    }

    // Consume the glTF file.
    std::ifstream in(app.filename.c_str(), std::ifstream::in);
    std::vector<uint8_t> buffer(static_cast<unsigned long>(contentSize));
    if (!in.read((char*) buffer.data(), contentSize)) {
        std::cerr << "Unable to read " << app.filename << std::endl;
        exit(1);
    }

    // Parse the glTF file and create Filament entities.
    app.asset = app.loader->createAssetFromJson(buffer.data(), buffer.size());
    buffer.clear();
    buffer.shrink_to_fit();

    if (!app.asset) {
        std::cerr << "Unable to parse " << app.filename << std::endl;
        exit(1);
    }

    // Load external textures and buffers.
    gltfio::ResourceLoader({
        .engine = app.engine,
        .gltfPath = app.filename.getAbsolutePath(),
        .normalizeSkinningWeights = true,
        .recomputeBoundingBoxes = false
    }).loadResources(app.asset);

    // Load animation data then free the source hierarchy.
    app.asset->getAnimator();
    app.state = AssetPipeline::isParameterized(app.asset->getSourceAsset()) ? PREPPED : LOADED;

    // Destroy the old asset and add the renderables to the scene.
    app.viewer->setAsset(app.asset, app.names, !app.actualSize);

    app.viewer->setIndirectLight(FilamentApp::get().getIBL()->getIndirectLight());
}

static void prepAsset(App& app) {
    app.state = PREPPING;

    utils::JobSystem* js = utils::JobSystem::getJobSystem();
    utils::JobSystem::Job* parent = js->createJob();
    utils::JobSystem::Job* prep = utils::jobs::createJob(*js, parent, [&app] {
        gltfio::AssetPipeline::AssetHandle asset = app.asset->getSourceAsset();
        uint32_t flags = gltfio::AssetPipeline::FILTER_TRIANGLES;
        if (app.enablePrepScale) {
            flags |= gltfio::AssetPipeline::SCALE_TO_UNIT;
        }
        gltfio::AssetPipeline pipeline;

        {
            app.statusText = std::make_shared<std::string>("Flattening");
            asset = pipeline.flatten(asset, flags);
            app.statusText.reset();
        }

        if (!asset) {
            app.messageBoxText = std::make_shared<std::string>("Unable to flatten model");
            app.pushedState = LOADED;
            app.requestStatePop = true;
            return;
        }

        {
            app.statusText = std::make_shared<std::string>("Parameterizing");
            asset = pipeline.parameterize(asset);
            app.statusText.reset();
        }

        if (!asset) {
            app.messageBoxText = std::make_shared<std::string>(
                    "Unable to parameterize mesh, check terminal output for details.");
            app.pushedState = LOADED;
            app.requestStatePop = true;
            return;
        }

        const utils::Path folder = app.filename.getAbsolutePath().getParent();
        const utils::Path binPath = folder + "prepped.bin";
        const utils::Path outPath = folder + "prepped.gltf";

        pipeline.save(asset, outPath, binPath);
        std::cout << "Generated " << outPath << " and " << binPath << std::endl;

        app.filename = outPath;
        loadAsset(app);

        app.pushedState = PREPPED;
        app.requestStatePop = true;
    });
    js->run(prep);
}

static void renderAsset(App& app) {
    app.pushedState = app.state;
    app.state = RENDERING;
    gltfio::AssetPipeline::AssetHandle asset = app.asset->getSourceAsset();

    // Allocate the render target for the path tracer as well as a GPU texture to display it.
    auto viewportSize = ImGui::GetIO().DisplaySize;
    viewportSize.x -= app.viewer->getSidebarWidth();
    app.ambientOcclusion = image::LinearImage(viewportSize.x, viewportSize.y, 1);
    app.showOverlay = true;
    createOverlayTexture(app);

    // Compute the camera paramaeters for the path tracer.
    // ---------------------------------------------------
    // The path tracer does not know about the top-level Filament transform that we use to fit the
    // model into a unit cube (see the -s option), so here we do little trick by temporarily
    // transforming the Filament camera before grabbing its lookAt vectors.
    auto& tcm = app.engine->getTransformManager();
    auto root = tcm.getInstance(app.asset->getRoot());
    auto cam = tcm.getInstance(app.camera->getEntity());
    filament::math::mat4f prev = tcm.getTransform(root);
    tcm.setTransform(root, inverse(prev));
    tcm.setParent(cam, root);
    filament::rays::SimpleCamera camera = {
        .aspectRatio = viewportSize.x / viewportSize.y,
        .eyePosition = app.camera->getPosition(),
        .targetPosition = app.camera->getPosition() + app.camera->getForwardVector(),
        .upVector = app.camera->getUpVector(),
        .vfovDegrees = 45, // NOTE: fov is not queryable, must match with FilamentApp
    };
    tcm.setParent(cam, {});
    tcm.setTransform(root, prev);

    app.pipeline = new gltfio::AssetPipeline();

    // Finally, set up some callbacks and invoke the path tracer.

    using filament::math::ushort2;
    auto onRenderTile = [](ushort2, ushort2, void* userData) {
        App* app = (App*) userData;
        app->requestOverlayUpdate = true;
    };
    auto onRenderDone = [](void* userData) {
        App* app = (App*) userData;
        app->requestStatePop = true;
        app->requestOverlayUpdate = true;
        delete app->pipeline;
    };
    app.pipeline->renderAmbientOcclusion(asset, app.ambientOcclusion, camera, onRenderTile,
            onRenderDone, &app);
}

static void bakeAsset(App& app) {
    app.state = BAKING;
    gltfio::AssetPipeline::AssetHandle asset = app.asset->getSourceAsset();

    // Allocate the render target for the path tracer as well as a GPU texture to display it.
    ImVec2 viewportSize = ImGui::GetIO().DisplaySize;
    const uint32_t res = app.bakeResolution;
    viewportSize.x -= app.viewer->getSidebarWidth();
    app.showOverlay = true;
    app.ambientOcclusion = image::LinearImage(res, res, 1);
    createOverlayTexture(app);

    app.pipeline = new gltfio::AssetPipeline();

    using filament::math::ushort2;
    auto onRenderTile = [](ushort2, ushort2, void* userData) {
        App* app = (App*) userData;
        app->requestOverlayUpdate = true;
    };

#if DEBUG_PATHTRACER
    image::LinearImage outputs[] = {
        app.ambientOcclusion,
        app.bentNormals = image::LinearImage(res, res, 3),
        app.meshNormals = image::LinearImage(res, res, 3),
        app.meshPositions = image::LinearImage(res, res, 3)
    };
    auto onRenderDone = [](void* userData) {
        App* app = (App*) userData;
        using namespace image;
        auto fmt = ImageEncoder::Format::PNG_LINEAR;

        std::ofstream bn("bentNormals.png", std::ios::binary | std::ios::trunc);
        image::LinearImage img = image::verticalFlip(image::vectorsToColors(app->bentNormals));
        ImageEncoder::encode(bn, fmt, img, "", "bentNormals.png");

        std::ofstream mn("meshNormals.png", std::ios::binary | std::ios::trunc);
        img = image::verticalFlip(image::vectorsToColors(app->meshNormals));
        ImageEncoder::encode(mn, fmt, img, "", "meshNormals.png");

        std::ofstream mp("meshPositions.png", std::ios::binary | std::ios::trunc);
        img = image::verticalFlip(image::vectorsToColors(app->meshPositions));
        ImageEncoder::encode(mp, fmt, img, "", "meshPositions.png");

        delete app->pipeline;
        app->state = BAKED;
    };
    app.pipeline->bakeAllOutputs(asset, outputs, onRenderTile, onRenderDone, &app);
#else
    auto onRenderDone = [](void* userData) {
        App* app = (App*) userData;
        delete app->pipeline;
        app->requestOverlayUpdate = true;
        app->state = BAKED;
    };
    app.pipeline->bakeAmbientOcclusion(asset, app.ambientOcclusion, onRenderTile, onRenderDone,
            &app);
#endif
}

static void exportAsset(App& app) {
    using namespace image;

    const utils::Path folder = app.filename.getAbsolutePath().getParent();
    const utils::Path binPath = folder + "baked.bin";
    const utils::Path outPath = folder + "baked.gltf";
    const utils::Path texPath = folder + "baked.png";

    std::ofstream out(texPath.c_str(), std::ios::binary | std::ios::trunc);
    ImageEncoder::encode(out, ImageEncoder::Format::PNG_LINEAR, app.ambientOcclusion, "",
            texPath.c_str());

    gltfio::AssetPipeline::AssetHandle asset = app.asset->getSourceAsset();
    gltfio::AssetPipeline pipeline;
    if (app.preserveMaterialsForExport) {
        asset = pipeline.replaceOcclusion(asset, "baked.png");
    } else {
        asset = pipeline.generatePreview(asset, "baked.png");
    }
    pipeline.save(asset, outPath, binPath);

    std::cout << "Generated " << outPath << ", " << binPath << ", and " << texPath << std::endl;
    app.filename = outPath;
    loadAsset(app);

    app.state = EXPORTED;
    app.showOverlay = false;
}

int main(int argc, char** argv) {
    App app;

    app.config.title = "gltf_baker";
    app.config.iblDirectory = FilamentApp::getRootPath() + DEFAULT_IBL;
    app.requestOverlayUpdate = false;
    app.requestStatePop = false;

    int option_index = handleCommandLineArguments(argc, argv, &app);
    int num_args = argc - option_index;
    if (num_args >= 1) {
        app.filename = argv[option_index];
        if (!app.filename.exists()) {
            std::cerr << "file " << app.filename << " not found!" << std::endl;
            return 1;
        }
        if (app.filename.isDirectory()) {
            auto files = app.filename.listContents();
            for (auto file : files) {
                if (file.getExtension() == "gltf") {
                    app.filename = file;
                    break;
                }
            }
            if (app.filename.isDirectory()) {
                std::cerr << "no glTF file found in " << app.filename << std::endl;
                return 1;
            }
        }
    }

    loadIniFile(app);

    auto setup = [&](Engine* engine, View* view, Scene* scene) {
        app.engine = engine;
        app.names = new NameComponentManager(EntityManager::get());
        app.viewer = new SimpleViewer(engine, scene, view, SimpleViewer::FLAG_COLLAPSED);
        app.materials = createMaterialGenerator(engine);
        app.loader = AssetLoader::create({engine, app.materials, app.names });
        app.camera = &view->getCamera();

        if (!app.filename.isEmpty()) {
            loadAsset(app);
            saveIniFile(app);
        }

        createOverlay(app);

        app.viewer->setUiCallback([&app, scene] () {
            const ImVec4 disabled = ImGui::GetStyle().Colors[ImGuiCol_TextDisabled];
            const ImVec4 enabled = ImGui::GetStyle().Colors[ImGuiCol_Text];
            ImGui::GetStyle().FrameRounding = 5;

            // Prep action (flattening and parameterizing).
            const bool canPrep = app.state == LOADED;
            ImGui::PushStyleColor(ImGuiCol_Text, canPrep ? enabled : disabled);
            if (ImGui::Button("Prep", ImVec2(100, 50)) && canPrep) {
                prepAsset(app);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Flattens the asset and generates a new set of UV coordinates.");
            }
            ImGui::PopStyleColor();

            // Render action (invokes path tracer).
            #ifdef FILAMENT_HAS_EMBREE
            const bool canRender = app.state == PREPPED;
            #else
            const bool canRender = false;
            #endif
            ImGui::PushStyleColor(ImGuiCol_Text, canRender ? enabled : disabled);
            if (ImGui::Button("Render", ImVec2(100, 50)) && canRender) {
                renderAsset(app);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Renders the asset using a pathtracer.");
            }
            ImGui::PopStyleColor();

            // Bake action (invokes path tracer).
            #ifdef FILAMENT_HAS_EMBREE
            const bool canBake = app.state == PREPPED;
            #else
            const bool canBake = false;
            #endif
            ImGui::PushStyleColor(ImGuiCol_Text, canBake ? enabled : disabled);
            if (ImGui::Button("Bake", ImVec2(100, 50)) && canBake) {
                bakeAsset(app);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Invokes an embree-based pathtracer.");
            }
            ImGui::PopStyleColor();

            // Export action
            const bool canExport = app.state == BAKED;
            ImGui::PushStyleColor(ImGuiCol_Text, canExport ? enabled : disabled);
            if (ImGui::Button("Export...", ImVec2(100, 50)) && canExport) {
                ImGui::OpenPopup("Export options");
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Saves the baked result to disk.");
            }
            ImGui::PopStyleColor();
            ImGui::GetStyle().FrameRounding = 20;

            // Options
            if (app.ambientOcclusion) {
                ImGui::Checkbox("Show embree result", &app.showOverlay);
            }
            if (canPrep) {
                ImGui::Checkbox("Auto-scale before parameterization", &app.enablePrepScale);
            }
            if (canBake) {
                static const int kFirstOption = std::log2(512);
                int bakeOption = std::log2(app.bakeResolution) - kFirstOption;
                ImGui::Combo("Bake Resolution", &bakeOption,
                        "512 x 512\0"
                        "1024 x 1024\0"
                        "2048 x 2048\0");
                app.bakeResolution = 1 << (bakeOption + kFirstOption);
            }

            if (app.statusText) {
                // Apply a poor man's animation to the ellipsis to indicate that work is being done.
                static const char* suffixes[] = { "...", "......", "........." };
                static float suffixAnim = 0;
                suffixAnim += 0.05f;
                const char* suffix = suffixes[int(suffixAnim) % 3];

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {10, 10} );
                ImGui::Text("%s%s", app.statusText->c_str(), suffix);
                ImGui::PopStyleVar();
            }

            if (app.messageBoxText) {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {10, 10} );
                ImGui::OpenPopup("MessageBox");
                if (ImGui::BeginPopupModal("MessageBox", nullptr,
                        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
                    ImGui::TextUnformatted(app.messageBoxText->c_str());
                    if (ImGui::Button("OK", ImVec2(120,0))) {
                        app.messageBoxText.reset();
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopStyleVar();
            }

            if (ImGui::BeginPopupModal("Export options", nullptr,
                    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
                ImGui::Checkbox("Preserve materials", &app.preserveMaterialsForExport);
                if (ImGui::Button("OK", ImVec2(120,0))) {
                    ImGui::CloseCurrentPopup();
                    exportAsset(app);
                }
                ImGui::EndPopup();
            }
        });

        // Leave FXAA enabled but we also enable MSAA for a nice result. The wireframe looks
        // much better with MSAA enabled.
        view->setSampleCount(4);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        Fence::waitAndDestroy(engine->createFence());
        delete app.viewer;
        app.loader->destroyAsset(app.asset);
        app.materials->destroyMaterials();
        delete app.materials;
        AssetLoader::destroy(&app.loader);
        delete app.names;
    };

    auto animate = [&app](Engine* engine, View* view, double now) {
        // The baker doesn't support animation, just use frame 0.
        if (app.state != EMPTY) {
            app.viewer->applyAnimation(0.0);
        }
        if (!app.overlayScene && app.showOverlay) {
            app.overlayView = FilamentApp::get().getGuiView();
            app.overlayScene = app.overlayView->getScene();
        }
        if (app.overlayScene) {
            app.overlayScene->remove(app.overlayEntity);
            if (app.showOverlay) {
                updateOverlay(app);
                app.overlayScene->addEntity(app.overlayEntity);
            }
        }
        if (app.requestOverlayUpdate) {
            updateOverlayTexture(app);
            app.requestOverlayUpdate = false;
        }
        if (app.requestStatePop) {
            app.state = app.pushedState;
            app.pushedState = EMPTY;
            app.requestStatePop = false;
        }
    };

    auto gui = [&app](filament::Engine* engine, filament::View* view) {
        app.viewer->updateUserInterface();
        FilamentApp::get().setSidebarWidth(app.viewer->getSidebarWidth());
    };

    FilamentApp& filamentApp = FilamentApp::get();
    filamentApp.animate(animate);

    filamentApp.setDropHandler([&] (std::string path) {
        app.viewer->removeAsset();
        app.loader->destroyAsset(app.asset);
        app.filename = path;
        loadAsset(app);
        saveIniFile(app);
    });

    filamentApp.run(app.config, setup, cleanup, gui);

    return 0;
}
