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

#include "app/Config.h"
#include "app/FilamentApp.h"
#include "app/IBL.h"

#include <filagui/ImGuiMath.h>

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Scene.h>
#include <filament/View.h>
#include <filament/VertexBuffer.h>

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
#include <functional>
#include <fstream>
#include <iostream>
#include <string>

#include "generated/resources/resources.h"

using namespace filament;
using namespace gltfio;
using namespace utils;
using filament::math::ushort2;

enum class Visualization : int {
    MESH_CURRENT,
    MESH_VERTEX_NORMALS,
    MESH_MODIFIED,
    MESH_PREVIEW_AO,
    MESH_PREVIEW_UV,
    MESH_GBUFFER_NORMALS,
    IMAGE_OCCLUSION,
    IMAGE_BENT_NORMALS,
    IMAGE_GBUFFER_NORMALS
};

static const char* DEFAULT_IBL = "venetian_crossroads_2k";
static const char* INI_FILENAME = "gltf_baker.ini";
static const char* TMP_UV_FILENAME = "gltf_baker_tmp_uv.png";
static const char* TMP_AO_FILENAME = "gltf_baker_tmp_ao.png";
static const char* TMP_NORMALS_FILENAME = "gltf_baker_tmp_mn.png";
static constexpr int PATH_SIZE = 256;

struct BakerApp;

using BakerAppTask = std::function<void(BakerApp*)>;

struct BakerApp {
    Config config;
    Engine* engine = nullptr;
    Camera* camera = nullptr;
    SimpleViewer* viewer = nullptr;
    NameComponentManager* names = nullptr;
    MaterialProvider* materials = nullptr;
    AssetLoader* loader = nullptr;
    gltfio::AssetPipeline* pipeline = nullptr;
    bool viewerActualSize = false;
    utils::Path filename;
    bool hasTestRender = false;
    bool isWorking = false;
    std::string statusText;
    ImVec4 statusColor;
    std::string messageBoxText;
    bool requestViewerUpdate = false;
    Visualization visualization = Visualization::MESH_CURRENT;

    // Bundle of Filament entities (renderables, textures, etc.) for the currently displayed mesh.
    FilamentAsset* viewerAsset = nullptr;

    // Available glTF scenes suitable for display, depending on "visualization".
    gltfio::AssetPipeline::AssetHandle flattenedAsset = nullptr;
    gltfio::AssetPipeline::AssetHandle parameterizedAsset = nullptr;
    gltfio::AssetPipeline::AssetHandle modifiedAsset = nullptr;
    gltfio::AssetPipeline::AssetHandle previewAoAsset = nullptr;
    gltfio::AssetPipeline::AssetHandle previewUvAsset = nullptr;
    gltfio::AssetPipeline::AssetHandle normalsAsset = nullptr;

    // Available 2D images suitable for display, depending on "visualization".
    image::LinearImage ambientOcclusion;
    image::LinearImage bentNormals;
    image::LinearImage meshNormals;
    image::LinearImage meshPositions;

    // AssetPipeline callbacks are triggered from outside the UI thread. To keep things simple, we
    // defer their execution until the next iteration of the main loop. We store only one item per
    // callback type, which provides the side benefit of skipping callbacks that occur more than
    // once per frame. We use std::function rather than raw C function pointers to allow simple
    // lambdas with captures.
    std::atomic<BakerAppTask*> onDone;
    std::atomic<BakerAppTask*> onTile;

    struct {
        uint32_t resolution = 1024;
        size_t samplesPerPixel = 256;
        float aoRayNear = std::numeric_limits<float>::epsilon() * 10.0f;
        bool dilateCharts = true;
        bool applyDenoiser = true;
        int maxIterations = 2;
    } bakeOptions;

    struct {
        Visualization selection = Visualization::MESH_MODIFIED;
        char outputFolder[PATH_SIZE];
        char gltfPath[PATH_SIZE];
        char binPath[PATH_SIZE];
        char occlusionPath[PATH_SIZE];
        char bentNormalsPath[PATH_SIZE];
    } exportOptions;

    struct {
        View* view = nullptr;
        Scene* scene = nullptr;
        VertexBuffer* vb = nullptr;
        IndexBuffer* ib = nullptr;
        Texture* texture = nullptr;
        MaterialInstance* material = nullptr;
        utils::Entity entity;
    } overlayQuad;
};

#define makeTileCallback(FN) [](ushort2,  ushort2, void* userData) { \
    BakerApp* app = (BakerApp*) userData; \
    BakerAppTask* previous = app->onTile.exchange(new BakerAppTask(FN)); \
    delete previous; \
}

#define makeDoneCallback(FN) [](void* userData) { \
    BakerApp* app = (BakerApp*) userData; \
    BakerAppTask* previous = app->onDone.exchange(new BakerAppTask(FN)); \
    delete previous; \
}

static void printUsage(char* name) {
    std::string exec_name(Path(name).getName());
    std::string usage(
        "BAKER can perform AO baking on the specified glTF file. If no file is specified,"
        "it loads the most recently-used glTF file.\n"
        "Usage:\n"
        "    BAKER [options] [gltf path]\n"
        "Options:\n"
        "   --help, -h\n"
        "       Prints this message\n\n"
        "   --actual-size, -s\n"
        "       Do not scale the model to fit into a unit cube in the viewer\n\n"
    );
    const std::string from("BAKER");
    for (size_t pos = usage.find(from); pos != std::string::npos; pos = usage.find(from, pos)) {
        usage.replace(pos, from.length(), exec_name);
    }
    std::cout << usage;
}

static int handleCommandLineArguments(int argc, char* argv[], BakerApp* app) {
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
                app->viewerActualSize = true;
                break;
        }
    }
    return optind;
}

static std::ifstream::pos_type getFileSize(const char* filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

static void saveIniFile(BakerApp& app) {
    std::ofstream out(INI_FILENAME);
    out << "[recent]\n";
    out << "filename=" << app.filename.c_str() << "\n";
}

static void loadIniFile(BakerApp& app) {
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

static void createQuadRenderable(BakerApp& app) {
    auto& rcm = app.engine->getRenderableManager();
    Engine& engine = *app.engine;

    struct OverlayVertex {
        filament::math::float2 position;
        filament::math::float2 uv;
    };
    static OverlayVertex kVertices[4] = {
        {{0, 0}, {0, 0}}, {{ 1000, 0}, {1, 0}}, {{0,  1000}, {0, 1}}, {{ 1000,  1000}, {1, 1}}
    };
    static constexpr uint16_t kIndices[6] = { 0, 1, 2, 3, 2, 1 };

    if (!app.overlayQuad.entity) {
        app.overlayQuad.vb = VertexBuffer::Builder()
                .vertexCount(4)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 16)
                .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2, 8, 16)
                .build(engine);
        app.overlayQuad.ib = IndexBuffer::Builder()
                .indexCount(6)
                .bufferType(IndexBuffer::IndexType::USHORT)
                .build(engine);
        app.overlayQuad.ib->setBuffer(engine,
                IndexBuffer::BufferDescriptor(kIndices, 12, nullptr));
        auto mat = Material::Builder()
                .package(RESOURCES_AOPREVIEW_DATA, RESOURCES_AOPREVIEW_SIZE)
                .build(engine);
        app.overlayQuad.material = mat->createInstance();
        app.overlayQuad.entity = EntityManager::get().create();
    }

    constexpr int margin = 20;
    const int sidebar = app.viewer->getSidebarWidth();
    const auto size = ImGui::GetIO().DisplaySize - ImVec2(sidebar + margin * 2, margin * 2);
    kVertices[0].position.x = sidebar + margin;
    kVertices[1].position.x = sidebar + margin + size.x;
    kVertices[2].position.x = sidebar + margin;
    kVertices[3].position.x = sidebar + margin + size.x;
    kVertices[0].position.y = margin;
    kVertices[1].position.y = margin;
    kVertices[2].position.y = margin + size.y;
    kVertices[3].position.y = margin + size.y;

    auto vb = app.overlayQuad.vb;
    auto ib = app.overlayQuad.ib;
    vb->setBufferAt(*app.engine, 0, VertexBuffer::BufferDescriptor(kVertices, 64, nullptr));
    rcm.destroy(app.overlayQuad.entity);
    RenderableManager::Builder(1)
            .boundingBox({{ 0, 0, 0 }, { 1000, 1000, 1 }})
            .material(0, app.overlayQuad.material)
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vb, ib, 0, 6)
            .culling(false)
            .receiveShadows(false)
            .castShadows(false)
            .build(*app.engine, app.overlayQuad.entity);
}

static void updateViewerMesh(BakerApp& app) {
    gltfio::AssetPipeline::AssetHandle handle;
    switch (app.visualization) {
        case Visualization::MESH_CURRENT: handle = app.flattenedAsset; break;
        case Visualization::MESH_VERTEX_NORMALS: handle = app.flattenedAsset; break;
        case Visualization::MESH_MODIFIED: handle = app.modifiedAsset; break;
        case Visualization::MESH_PREVIEW_AO: handle = app.previewAoAsset; break;
        case Visualization::MESH_PREVIEW_UV: handle = app.previewUvAsset; break;
        case Visualization::MESH_GBUFFER_NORMALS: handle = app.normalsAsset; break;
        default: return;
    }

    if (!app.viewerAsset || app.viewerAsset->getSourceAsset() != handle) {
        auto previousViewerAsset = app.viewerAsset;
        app.viewerAsset = app.loader->createAssetFromHandle(handle);

        // Load external textures and buffers.
        gltfio::ResourceLoader({
            .engine = app.engine,
            .gltfPath = app.filename.getAbsolutePath().c_str(),
            .normalizeSkinningWeights = true,
            .recomputeBoundingBoxes = false
        }).loadResources(app.viewerAsset);

        // Load animation data then free the source hierarchy.
        app.viewerAsset->getAnimator();

        // Remove old renderables and add new renderables to the scene.
        app.viewer->populateScene(app.viewerAsset, !app.viewerActualSize);

        // Destory old Filament entities.
        app.loader->destroyAsset(previousViewerAsset);
    }
}

static void updateViewerImage(BakerApp& app) {
    Engine& engine = *app.engine;
    using MinFilter = TextureSampler::MinFilter;
    using MagFilter = TextureSampler::MagFilter;

    // Gather information about the displayed image.
    image::LinearImage image;
    switch (app.visualization) {
        case Visualization::IMAGE_OCCLUSION:
            image = app.ambientOcclusion;
            break;
        case Visualization::IMAGE_BENT_NORMALS:
            image = app.bentNormals;
            break;
        case Visualization::IMAGE_GBUFFER_NORMALS:
            image = app.meshNormals;
            break;
        default:
            return;
    }
    const int width = image.getWidth();
    const int height = image.getHeight();
    const int channels = image.getChannels();
    const void* data = image.getPixelRef();
    const Texture::InternalFormat internalFormat = channels == 1 ?
            Texture::InternalFormat::R32F : Texture::InternalFormat::RGB32F;
    const Texture::Format format = channels == 1 ? Texture::Format::R : Texture::Format::RGB;

    // Create a brand new texture object if necessary.
    const Texture* tex = app.overlayQuad.texture;
    if (!tex || tex->getWidth() != width || tex->getHeight() != height ||
            tex->getFormat() != internalFormat) {
        engine.destroy(tex);
        app.overlayQuad.texture = Texture::Builder()
                .width(width)
                .height(height)
                .levels(1)
                .sampler(Texture::Sampler::SAMPLER_2D)
                .format(internalFormat)
                .build(engine);
        TextureSampler sampler(MinFilter::LINEAR, MagFilter::LINEAR);
        app.overlayQuad.material->setParameter("luma", app.overlayQuad.texture, sampler);
        app.overlayQuad.material->setParameter("grayscale", channels == 1);
    }

    // Upload texture data.
    Texture::PixelBufferDescriptor buffer(data, size_t(width * height * channels * sizeof(float)),
            format, Texture::Type::FLOAT);
    app.overlayQuad.texture->setImage(engine, 0, std::move(buffer));
}

static void updateViewer(BakerApp& app) {
    switch (app.visualization) {
        case Visualization::MESH_CURRENT:
        case Visualization::MESH_VERTEX_NORMALS:
        case Visualization::MESH_MODIFIED:
        case Visualization::MESH_PREVIEW_AO:
        case Visualization::MESH_PREVIEW_UV:
        case Visualization::MESH_GBUFFER_NORMALS:
            updateViewerMesh(app);
            break;
        case Visualization::IMAGE_OCCLUSION:
        case Visualization::IMAGE_BENT_NORMALS:
        case Visualization::IMAGE_GBUFFER_NORMALS:
            updateViewerImage(app);
            break;
    }
}

static void loadAssetFromDisk(BakerApp& app) {
    std::cout << "Loading " << app.filename << "..." << std::endl;
    if (app.filename.getExtension() == "glb") {
        std::cerr << "GLB files are not yet supported." << std::endl;
        exit(1);
    }

    auto pipeline = new gltfio::AssetPipeline();
    gltfio::AssetPipeline::AssetHandle handle = pipeline->load(app.filename);
    if (!handle) {
        delete pipeline;
        std::cerr << "Unable to load model" << std::endl;
        exit(1);
    }

    if (!gltfio::AssetPipeline::isFlattened(handle)) {
        handle = pipeline->flatten(handle, AssetPipeline::FILTER_TRIANGLES);
        if (!handle) {
            delete pipeline;
            std::cerr << "Unable to flatten model" << std::endl;
            exit(1);
        }
    }

    // Destroy the previous pipeline to free up resources used by the previous asset.
    delete app.pipeline;
    app.pipeline = pipeline;

    app.viewer->setIndirectLight(FilamentApp::get().getIBL()->getIndirectLight(), nullptr);
    app.flattenedAsset = handle;
    app.requestViewerUpdate = true;

    // Update the window title bar and the default output path.
    const utils::Path defaultFolder = app.filename.getAbsolutePath().getParent();
    strncpy(app.exportOptions.outputFolder, defaultFolder.c_str(), PATH_SIZE);
    FilamentApp::get().setWindowTitle(app.filename.getName().c_str());
}

static void executeTestRender(BakerApp& app) {
    app.isWorking = true;
    app.hasTestRender = true;
    gltfio::AssetPipeline::AssetHandle currentAsset = app.flattenedAsset;

    // Allocate the render target for the path tracer as well as a GPU texture to display it.
    auto viewportSize = ImGui::GetIO().DisplaySize;
    viewportSize.x -= app.viewer->getSidebarWidth();
    app.ambientOcclusion = image::LinearImage((uint32_t) viewportSize.x,
            (uint32_t) viewportSize.y, 1);
    app.statusText.clear();
    app.visualization = Visualization::IMAGE_OCCLUSION;

    // Compute the camera parameters for the path tracer.
    // ---------------------------------------------------
    // The path tracer does not know about the top-level Filament transform that we use to fit the
    // model into a unit cube (see the -s option), so here we do little trick by temporarily
    // transforming the Filament camera before grabbing its lookAt vectors.
    auto& tcm = app.engine->getTransformManager();
    auto root = tcm.getInstance(app.viewerAsset->getRoot());
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

    // Finally, set up some callbacks and invoke the path tracer.
    auto onRenderTile = makeTileCallback([](BakerApp* app) {
        app->requestViewerUpdate = true;
    });
    auto onRenderDone = makeDoneCallback([](BakerApp* app) {
        app->requestViewerUpdate = true;
        app->isWorking = false;
    });
    app.pipeline->renderAmbientOcclusion(currentAsset, app.ambientOcclusion, camera, {
        .progress = onRenderTile,
        .done = onRenderDone,
        .userData = &app,
        .samplesPerPixel = app.bakeOptions.samplesPerPixel,
        .aoRayNear = app.bakeOptions.aoRayNear,
        .enableDenoise = app.bakeOptions.applyDenoiser
    });
}

static void generateUvVisualization(const utils::Path& pngOutputPath) {
    using namespace image;
    LinearImage uvimage(256, 256, 3);
    for (int y = 0, h = uvimage.getHeight(); y < h; ++y) {
        for (int x = 0, w = uvimage.getWidth(); x < w; ++x) {
            float* dst = uvimage.getPixelRef(x, y);
            dst[0] = float(x) / w;
            dst[1] = float(y) / h;
            dst[2] = 1.0f;
        }
    }
    std::ofstream out(pngOutputPath.c_str(), std::ios::binary | std::ios::trunc);
    ImageEncoder::encode(out, ImageEncoder::Format::PNG_LINEAR, uvimage, "",
            pngOutputPath.c_str());
}

static void executeBakeAo(BakerApp& app) {
    using namespace image;

    auto onRenderTile = makeTileCallback([](BakerApp* app) {
        app->requestViewerUpdate = true;
    });

    auto onRenderDone = makeDoneCallback([](BakerApp* app) {
        gltfio::AssetPipeline* pipeline = app->pipeline;
        gltfio::AssetPipeline::AssetHandle asset = app->parameterizedAsset;
        app->requestViewerUpdate = true;

        // Generate a simple red-green UV visualization texture.
        const utils::Path folder = app->filename.getAbsolutePath().getParent();
        generateUvVisualization(folder + TMP_UV_FILENAME);
        app->previewUvAsset = pipeline->generatePreview(asset, TMP_UV_FILENAME);

        // Export the generated AO texture.
        const utils::Path tmpOcclusionPath = folder + TMP_AO_FILENAME;
        std::ofstream out(tmpOcclusionPath.c_str(), std::ios::binary | std::ios::trunc);
        ImageEncoder::encode(out, ImageEncoder::Format::PNG_LINEAR, app->ambientOcclusion,
                "", tmpOcclusionPath.c_str());

        // Export the mesh normals texture.
        const utils::Path tmpNormalsPath = folder + TMP_NORMALS_FILENAME;
        out = std::ofstream(tmpNormalsPath.c_str(), std::ios::binary | std::ios::trunc);
        ImageEncoder::encode(out, ImageEncoder::Format::PNG_LINEAR, app->meshNormals,
                "", tmpNormalsPath.c_str());

        app->previewAoAsset = pipeline->generatePreview(asset, TMP_AO_FILENAME);
        app->modifiedAsset = pipeline->replaceOcclusion(asset, TMP_AO_FILENAME);
        app->normalsAsset = pipeline->generatePreview(asset, TMP_NORMALS_FILENAME);
        app->isWorking = false;
    });

    auto doRender = [&app, onRenderTile, onRenderDone] {
        const uint32_t res = app.bakeOptions.resolution;
        app.statusText.clear();
        app.hasTestRender = false;
        app.visualization = Visualization::IMAGE_OCCLUSION;
        app.ambientOcclusion = image::LinearImage(res, res, 1);
        app.bentNormals = image::LinearImage(res, res, 3);
        app.meshNormals = image::LinearImage(res, res, 3);
        app.meshPositions = image::LinearImage(res, res, 3);
        image::LinearImage outputs[] = {
            app.ambientOcclusion, app.bentNormals, app.meshNormals, app.meshPositions
        };
        app.pipeline->bakeAllOutputs(app.parameterizedAsset, outputs, {
            .progress = onRenderTile,
            .done = onRenderDone,
            .userData = &app,
            .samplesPerPixel = app.bakeOptions.samplesPerPixel,
            .aoRayNear = app.bakeOptions.aoRayNear,
            .enableDenoise = app.bakeOptions.applyDenoiser,
            .enableDilation = app.bakeOptions.dilateCharts
        });
    };

    app.isWorking = true;
    app.previewAoAsset = nullptr;
    app.modifiedAsset = nullptr;
    app.previewUvAsset = nullptr;
    app.statusColor = ImVec4({0, 1, 0, 1});
    app.statusText = "Parameterizing...";

    utils::JobSystem* js = utils::JobSystem::getJobSystem();
    utils::JobSystem::Job* parent = js->createJob();
    utils::JobSystem::Job* prep = utils::jobs::createJob(*js, parent, [&app, doRender] {
        auto parameterized = app.pipeline->parameterize(app.flattenedAsset,
                app.bakeOptions.maxIterations);
        auto callback = new BakerAppTask([doRender, parameterized](BakerApp* app) {
            if (!parameterized) {
                app->messageBoxText = "Unable to parameterize, check terminal output for details.";
                app->isWorking = false;
                return;
            }
            app->parameterizedAsset = parameterized;
            app->requestViewerUpdate = true;
            doRender();
        });
        BakerAppTask* previous = app.onDone.exchange(callback);
        delete previous;
    });
    js->run(prep);
}

static void executeExport(BakerApp& app) {
    const auto& options = app.exportOptions;
    const utils::Path folder = options.outputFolder;
    const utils::Path binPath = folder + options.binPath;
    const utils::Path gltfPath = folder + options.gltfPath;
    const utils::Path occlusionPath = folder + options.occlusionPath;
    const utils::Path bentNormalsPath = folder + options.bentNormalsPath;

    auto exportOcclusion = [&app, occlusionPath]() {
        using namespace image;
        std::ofstream out(occlusionPath.c_str(), std::ios::binary | std::ios::trunc);
        return ImageEncoder::encode(out, ImageEncoder::Format::PNG_LINEAR, app.ambientOcclusion, "",
                occlusionPath.c_str());
    };

    auto exportBentNormals = [&app, bentNormalsPath]() {
        using namespace image;
        std::ofstream out(bentNormalsPath.c_str(), std::ios::binary | std::ios::trunc);
        return ImageEncoder::encode(out, ImageEncoder::Format::PNG_LINEAR, app.bentNormals, "",
                bentNormalsPath.c_str());
    };

    std::string msg;
    bool error = false;
    const std::string join = ", ";
    switch (options.selection) {
        case Visualization::MESH_CURRENT:
            error = error || !app.pipeline->save(app.flattenedAsset, gltfPath, binPath);
            msg = options.gltfPath + join + options.binPath;
            break;
        case Visualization::MESH_MODIFIED:
            error = error || !exportOcclusion();
            app.pipeline->setOcclusionUri(app.modifiedAsset, options.occlusionPath);
            error = error || !app.pipeline->save(app.modifiedAsset, gltfPath, binPath);
            app.pipeline->setOcclusionUri(app.modifiedAsset, TMP_AO_FILENAME);
            msg = options.gltfPath + join + options.binPath + join + options.occlusionPath;
            break;
        case Visualization::MESH_PREVIEW_AO:
            error = error || !exportOcclusion();
            app.pipeline->setBaseColorUri(app.previewAoAsset, options.occlusionPath);
            error = error || !app.pipeline->save(app.previewAoAsset, gltfPath, binPath);
            app.pipeline->setBaseColorUri(app.previewAoAsset, TMP_AO_FILENAME);
            msg = options.gltfPath + join + options.binPath + join + options.occlusionPath;
            break;
        case Visualization::IMAGE_OCCLUSION:
            error = error || !exportOcclusion();
            msg = options.occlusionPath;
            break;
        case Visualization::IMAGE_BENT_NORMALS:
            error = error || !exportBentNormals();
            msg = options.bentNormalsPath;
            break;
        default:
            return;
    }
    app.statusColor = error ? ImVec4({1, 0, 0, 1}) : ImVec4({0, 1, 0, 1});
    app.statusText = (error ? "Failed export to " : "Exported ") + msg;
}

int main(int argc, char** argv) {
    BakerApp app;

    app.onDone.exchange(nullptr);
    app.onTile.exchange(nullptr);

    strncpy(app.exportOptions.gltfPath, "baked.gltf", PATH_SIZE);
    strncpy(app.exportOptions.binPath, "baked.bin", PATH_SIZE);
    strncpy(app.exportOptions.occlusionPath, "occlusion.png", PATH_SIZE);
    strncpy(app.exportOptions.bentNormalsPath, "bentNormals.png", PATH_SIZE);

    app.config.title = "gltf_baker";
    app.config.iblDirectory = FilamentApp::getRootAssetsPath() + DEFAULT_IBL;

    utils::Path filename;
    int option_index = handleCommandLineArguments(argc, argv, &app);
    int num_args = argc - option_index;
    if (num_args >= 1) {
        filename = argv[option_index];
        if (!filename.exists()) {
            std::cerr << "file " << app.filename << " not found!" << std::endl;
            return 1;
        }
        if (filename.isDirectory()) {
            auto files = filename.listContents();
            for (const auto& file : files) {
                if (file.getExtension() == "gltf") {
                    app.filename = file;
                    break;
                }
            }
            if (filename.isDirectory()) {
                std::cerr << "no glTF file found in " << filename << std::endl;
                return 1;
            }
        }
    }

    app.filename = filename;
    loadIniFile(app);

    auto setup = [&](Engine* engine, View* view, Scene* scene) {
        app.engine = engine;
        app.names = new NameComponentManager(EntityManager::get());

        const int kInitialSidebarWidth = 322;
        app.viewer = new SimpleViewer(engine, scene, view, SimpleViewer::FLAG_COLLAPSED,
                kInitialSidebarWidth);
        app.viewer->enableSunlight(false);
        app.viewer->enableSSAO(false);
        app.viewer->setIBLIntensity(50000.0f);

        app.materials = createMaterialGenerator(engine);
        app.loader = AssetLoader::create({engine, app.materials, app.names });
        app.loader->enableDiagnostics();
        app.camera = &view->getCamera();

        if (!app.filename.isEmpty()) {
            loadAssetFromDisk(app);
            saveIniFile(app);
        }

        app.viewer->setUiCallback([&app] () {
            const ImU32 disabledColor = ImColor(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
            const ImU32 hoveredColor = ImColor(ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
            const ImU32 enabledColor = ImColor(0.5f, 0.5f, 0.0f);
            const ImVec2 buttonSize(100, 50);
            const float buttonPositions[] = { 0, 2 + buttonSize.x, 4 + buttonSize.x * 2 };
            ImVec2 pos;
            ImU32 color;
            bool enabled;

            // Begin action buttons
            ImGui::GetStyle().ItemSpacing.x = 1;
            ImGui::GetStyle().FrameRounding = 10;
            ImGui::PushStyleColor(ImGuiCol_Button, enabledColor);
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::BeginGroup();

            using OnClick = void(*)(BakerApp& app);
            auto showActonButton = [&](const char* label, int cornerFlags, OnClick fn) {
                pos = ImGui::GetCursorScreenPos();
                color = enabled ? enabledColor : disabledColor;
                color = ImGui::IsMouseHoveringRect(pos, pos + buttonSize) ? hoveredColor : color;
                ImGui::GetWindowDrawList()->AddRectFilled(pos, pos + buttonSize, color,
                        ImGui::GetStyle().FrameRounding, cornerFlags);
                ImGui::PushStyleColor(ImGuiCol_Button, color);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
                if (ImGui::Button(label, buttonSize) && enabled) {
                    fn(app);
                }
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
            };

            // TEST RENDER
            ImGui::SameLine(buttonPositions[0]);
            enabled = !app.isWorking;
            showActonButton("Test Render", ImDrawCornerFlags_Left, [](BakerApp& app) {
                executeTestRender(app);
            });
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Renders the asset from the current camera using a pathtracer.");
            }

            // BAKE
            ImGui::SameLine(buttonPositions[1]);
            enabled = !app.isWorking;
            showActonButton("Bake AO", 0, [](BakerApp& app) {
                executeBakeAo(app);
            });
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Generates a new set of UVs and invokes a pathtracer.");
            }

            // EXPORT
            ImGui::SameLine(buttonPositions[2]);
            enabled = !app.isWorking && !app.hasTestRender && app.modifiedAsset;
            showActonButton("Export...", ImDrawCornerFlags_Right, [](BakerApp& app) {
                ImGui::OpenPopup("Export options");
            });
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Saves the baked result to disk.");
            }

            // End action buttons
            ImGui::EndGroup();
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::PopStyleColor();
            ImGui::GetStyle().FrameRounding = 20;
            ImGui::GetStyle().ItemSpacing.x = 8;

            // Model stats
            if (app.viewerAsset) {
                filament::Aabb aabb = app.viewerAsset->getBoundingBox();
                ImGui::TextColored({1, 0, 1, 1}, "min (%g, %g, %g)", aabb.min.x, aabb.min.y, aabb.min.z);
                ImGui::TextColored({1, 0, 1, 1}, "max (%g, %g, %g)", aabb.max.x, aabb.max.y, aabb.max.z);
                ImGui::Spacing();
            }

            // Status text
            if (app.statusText.size()) {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {10, 10} );
                ImGui::TextColored(app.statusColor, "%s", app.statusText.c_str());
                ImGui::PopStyleVar();
                if (app.isWorking) {
                    static float fraction = 0;
                    fraction = fmod(fraction + 0.1f, 2.0f * M_PI);
                    ImGui::ProgressBar(sin(fraction) * 0.5f + 0.5f, {buttonSize.x * 3, 5.0f}, "");
                }
                ImGui::Spacing();
            }

            // Results
            auto addOption = [&app](const char* msg, char num, Visualization e) {
                ImGuiIO& io = ImGui::GetIO();
                int* ptr = (int*) &app.visualization;
                if (io.InputCharacters[0] == num) { app.visualization = e; }
                ImGui::RadioButton(msg, ptr, (int) e);
                ImGui::SameLine();
                ImGui::TextColored({1, 1, 0,1 }, "%c", num);
            };
            if (app.ambientOcclusion && ImGui::CollapsingHeader("Results",
                    ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Indent();
                const Visualization previousVisualization = app.visualization;
                using RV = Visualization;
                addOption("3D model with original materials", '1', RV::MESH_CURRENT);
                addOption("3D model with vertex normals", '2', RV::MESH_VERTEX_NORMALS);
                if (app.hasTestRender) {
                    addOption("Rendered AO test image", '3', RV::IMAGE_OCCLUSION);
                } else if (!app.modifiedAsset) {
                    addOption("2D texture with occlusion", '3', RV::IMAGE_OCCLUSION);
                    addOption("2D texture with bent normals", '4', RV::IMAGE_BENT_NORMALS);
                    addOption("2D texture with mesh normals", '5', RV::IMAGE_GBUFFER_NORMALS);
                } else {
                    addOption("3D model with modified materials", '3', RV::MESH_MODIFIED);
                    addOption("3D model with new occlusion only", '4', RV::MESH_PREVIEW_AO);
                    addOption("3D model with UV visualization", '5', RV::MESH_PREVIEW_UV);
                    addOption("3D model with gbuffer normals", '6', RV::MESH_GBUFFER_NORMALS);
                    addOption("2D texture with occlusion", '7', RV::IMAGE_OCCLUSION);
                    addOption("2D texture with bent normals", '8', RV::IMAGE_BENT_NORMALS);
                    addOption("2D texture with gbuffer normals", '9', RV::IMAGE_GBUFFER_NORMALS);
                }
                if (app.visualization != previousVisualization) {
                    app.requestViewerUpdate = true;
                }
                ImGui::Unindent();
                ImGui::Spacing();
            }

            // Options
            if (ImGui::CollapsingHeader("Bake Options")) {
                int spp = app.bakeOptions.samplesPerPixel;
                ImGui::InputInt("Samples per pixel", &spp);
                app.bakeOptions.samplesPerPixel = spp;

                static const int kFirstOption = (int) std::log2(512);
                int bakeOption = (int) std::log2(app.bakeOptions.resolution) - kFirstOption;
                ImGui::Combo("Texture size", &bakeOption,
                        "512 x 512\0"
                        "1024 x 1024\0"
                        "2048 x 2048\0");
                app.bakeOptions.resolution = 1u << uint32_t(bakeOption + kFirstOption);

                ImGui::InputFloat("Secondary ray tmin", &app.bakeOptions.aoRayNear,
                        std::numeric_limits<float>::epsilon(),
                        std::numeric_limits<float>::epsilon() * 10.0f, 10);

                ImGui::InputInt("Max segmentation attempts", &app.bakeOptions.maxIterations);
                ImGui::Checkbox("Dilate charts", &app.bakeOptions.dilateCharts);
                ImGui::Checkbox("Apply denoiser", &app.bakeOptions.applyDenoiser);
            }

            // Modals
            if (app.messageBoxText.size()) {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {10, 10} );
                ImGui::OpenPopup("MessageBox");
                if (ImGui::BeginPopupModal("MessageBox", nullptr,
                        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
                    ImGui::TextUnformatted(app.messageBoxText.c_str());
                    if (ImGui::Button("OK", ImVec2(120,0))) {
                        app.messageBoxText.clear();
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopStyleVar();
            }

            if (ImGui::BeginPopupModal("Export options", nullptr,
                    ImGuiWindowFlags_AlwaysAutoResize)) {
                using RV = Visualization;
                auto& options = app.exportOptions;
                ImGui::InputText("Output folder", options.outputFolder, PATH_SIZE);
                ImGui::InputText("glTF filename", options.gltfPath, PATH_SIZE);
                ImGui::InputText("Buffer data filename", options.binPath, PATH_SIZE);
                ImGui::InputText("Occlusion image", options.occlusionPath, PATH_SIZE);
                ImGui::InputText("Bent normals image", options.bentNormalsPath, PATH_SIZE);

                auto radio = [&app](const char* name, RV value) {
                    int* ptr = (int*) &app.exportOptions.selection;
                    ImGui::RadioButton(name, ptr, (int) value);
                };
                radio("Export flattened glTF with original materials", RV::MESH_CURRENT);
                radio("Export flattened glTF with modified materials", RV::MESH_MODIFIED);
                radio("Export flattened glTF with new occlusion only", RV::MESH_PREVIEW_AO);
                radio("Export occlusion image only", RV::IMAGE_OCCLUSION);
                radio("Export bent normals image only", RV::IMAGE_BENT_NORMALS);
                if (ImGui::Button("OK", ImVec2(120,0))) {
                    ImGui::CloseCurrentPopup();
                    executeExport(app);
                }
                ImGui::EndPopup();
            }
        });

        // Leave FXAA enabled but we also enable MSAA for a nice result. The wireframe looks
        // much better with MSAA enabled.
        view->setSampleCount(4);
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        std::cout << "Destroying viewer..." << std::endl;
        app.viewer->removeAsset();
        delete app.viewer;
        std::cout << "Destroying viewer asset..." << std::endl;
        app.loader->destroyAsset(app.viewerAsset);
        app.viewerAsset = nullptr;
        std::cout << "Destroying pipeline..." << std::endl;
        delete app.pipeline;
        std::cout << "Destroying AssetLoader materials..." << std::endl;
        app.materials->destroyMaterials();
        delete app.materials;
        std::cout << "Destroying AssetLoader..." << std::endl;
        AssetLoader::destroy(&app.loader);
        std::cout << "Destroying NameComponentManager..." << std::endl;
        delete app.names;
    };

    auto animate = [&app](Engine* engine, View* view, double now) {
        if (app.viewerAsset) {

            // The baker doesn't support animation, just use frame 0.
            app.viewer->applyAnimation(0.0);

            const bool enableDiagnostics = app.visualization == Visualization::MESH_VERTEX_NORMALS;
            auto begin = app.viewerAsset->getMaterialInstances();
            auto end = begin + app.viewerAsset->getMaterialInstanceCount();
            for (auto iter = begin; iter != end; ++iter) {
                (*iter)->setParameter("enableDiagnostics", enableDiagnostics);
            }
        }

        // Perform pending work.
        BakerAppTask* tile = app.onTile.exchange(nullptr);
        BakerAppTask* done = app.onDone.exchange(nullptr);
        if (tile) { (*tile)(&app); delete tile; }
        if (done) { (*done)(&app); delete done; }

        // Update the overlay quad geometry just in case the window size changed.
        app.overlayQuad.view = FilamentApp::get().getGuiView();
        app.overlayQuad.scene = app.overlayQuad.view->getScene();
        app.overlayQuad.scene->remove(app.overlayQuad.entity);
        const bool showOverlay = app.visualization == Visualization::IMAGE_OCCLUSION
                || app.visualization == Visualization::IMAGE_BENT_NORMALS
                || app.visualization == Visualization::IMAGE_GBUFFER_NORMALS;
        if (showOverlay) {
            createQuadRenderable(app);
            app.overlayQuad.scene->addEntity(app.overlayQuad.entity);
        }

        // If requested update the overlay quad texture or 3D mesh data.
        if (app.requestViewerUpdate) {
            updateViewer(app);
            app.requestViewerUpdate = false;
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
        app.loader->destroyAsset(app.viewerAsset);
        app.viewerAsset = nullptr;
        app.filename = path;
        app.hasTestRender = false;
        app.ambientOcclusion = image::LinearImage();
        app.bentNormals = image::LinearImage();
        app.meshNormals = image::LinearImage();
        app.meshPositions = image::LinearImage();
        app.visualization = Visualization::MESH_CURRENT;
        loadAssetFromDisk(app);
        saveIniFile(app);
    });

    filamentApp.run(app.config, setup, cleanup, gui);

    return 0;
}
