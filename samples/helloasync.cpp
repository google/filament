/*
 * Copyright (C) 2026 The Android Open Source Project
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
#include "filament/TransformManager.h"

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <utils/EntityManager.h>

#include <utils/Path.h>

#include <filamentapp/Config.h>
#include <filamentapp/FilamentApp.h>

#include <getopt/getopt.h>

#include <stb_image.h>

#include <iostream> // for cerr
#include <memory>
#include <string>   // for printing usage/help

#include "generated/resources/resources.h"

using namespace filament;
using utils::Entity;
using utils::EntityManager;
using utils::Path;
using MinFilter = TextureSampler::MinFilter;
using MagFilter = TextureSampler::MagFilter;

struct Vertex {
    filament::math::float2 position;
    filament::math::float2 uv;
};

static const Vertex QUAD_VERTICES[4] = {
    {{-1, -1}, {0, 0}},
    {{ 1, -1}, {1, 0}},
    {{-1,  1}, {0, 1}},
    {{ 1,  1}, {1, 1}},
};

static constexpr uint16_t QUAD_INDICES[6] = {
    0, 1, 2,
    3, 2, 1,
};

static void printUsage(char* name) {
    std::string exec_name(utils::Path(name).getName());
    std::string usage("HELLOASYNC creates resources asynchronously\n"
                      "Usage:\n"
                      "    HELLOASYNC [options]\n"
                      "Options:\n"
                      "   --help, -h\n"
                      "       Prints this message\n\n"
                      "API_USAGE");
    const std::string from("HELLOASYNC");
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

static int handleCommandLineArguments(int argc, char* argv[], Config& config) {
    static constexpr const char* OPTSTR = "ha:";
    static const struct option OPTIONS[] = {
        { "help", no_argument, nullptr, 'h' },
        { "api", required_argument, nullptr, 'a' },
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
                config.backend = samples::parseArgumentsForBackend(arg);
                break;
        }
    }
    return optind;
}

struct App {
    // Global data
    Engine* engine = nullptr;
    Entity camera;
    Scene* scene;
    Skybox* skybox = nullptr;
    Camera* cam = nullptr;
    Material* mat = nullptr;


    // --------------------------------------------------------------------------------------------
    // Everything below this point is for demonstrating async logic.

    // The number of objects to be created.
    static constexpr int OBJECT_COUNT = 400;
    static constexpr int OBJECT_COUNT_PER_ROW = 20;
    static constexpr int ROW_COUNT =
            (OBJECT_COUNT + OBJECT_COUNT_PER_ROW - 1) / OBJECT_COUNT_PER_ROW;

    // For demonstration purposes, we load one image and it is shared for every ObjectData instance.
    int imageWidth;
    int imageHeight;
    int imageChannels;

    struct StbImageDeleter {
        void operator()(stbi_uc* p) const {
            // We delay freeing the stb image until after the engine has completely shut down. This
            // ensures the data remains valid while the engine flushing pending tasks (see
            // `updateTexture`).
            // Note: This cleanup is specific to this sample because the image is shared across
            // multiple objects. In a standard application, memory should be released via the
            // cleanup callback in `PixelBufferDescriptor`. (see `updateTexture`)
            stbi_image_free(p);
        }
    };
    std::unique_ptr<stbi_uc, StbImageDeleter> imageData;

    // Object data associated with a single renderable object.
    struct ObjectData {
        Texture* tex = nullptr;
        MaterialInstance* matInstance = nullptr;
        VertexBuffer* vb = nullptr;
        IndexBuffer* ib = nullptr;
        Entity renderable;
        filament::math::mat4f baseTransform;

        bool texReady = false;
        bool vbReady = false;
        bool ibReady = false;
        [[nodiscard]] bool isReadyToCreateRenderable() const {
            return texReady && vbReady && ibReady;
        }
    } objectData[OBJECT_COUNT];

    // The number of objects currently being loaded.
    int loadingObjectIndex = 0;

    // To prevent calling APIs during shutdown. This variable is always referenced in the main(app)
    // thread, so synchronization is unnecessary.
    bool shuttingDown = false;

    // Completion callbacks for chained actions. We store them here instead of directly passing them
    // to async APIs as parameters for better maintainability and legibility.
    using OnLoadImageComplete = Engine::AsyncCompletionCallback;
    using OnCreateTextureComplete = Texture::AsyncCompletionCallback;
    using OnTextureUpdateComplete = Texture::AsyncCompletionCallback;
    using OnCreateVertexBufferComplete = VertexBuffer::AsyncCompletionCallback;
    using OnVertexBufferUpdateComplete = VertexBuffer::AsyncCompletionCallback;
    using OnCreateIndexBufferComplete = IndexBuffer::AsyncCompletionCallback;
    using OnIndexBufferUpdateComplete = IndexBuffer::AsyncCompletionCallback;
    OnLoadImageComplete onLoadImageComplete; // -> Create material & start async renderable creation
    OnCreateTextureComplete onCreateTextureComplete; // -> Update texture
    OnTextureUpdateComplete onTextureUpdateComplete; // -> Create mat instance & mark texture ready!
    OnCreateVertexBufferComplete onCreateVertexBufferComplete; // -> Update vertex buffer
    OnVertexBufferUpdateComplete onVertexBufferUpdateComplete; // -> Mark vertex buffer ready!
    OnCreateIndexBufferComplete onCreateIndexBufferComplete; // -> Update index buffer
    OnIndexBufferUpdateComplete onIndexBufferUpdateComplete; // -> Mark index buffer ready!

    // These methods below handle resource creation and updates. They are intended to support for
    // both standard synchronous flows and asynchronous operations. Note that they must be invoked
    // from the main thread as they call "Filament APIs" in it. In this sample, you see some methods
    // are called directly inside the asynchronous completion callbacks, which is safe because the
    // callbacks are guaranteed to run on the main thread.

    void createMaterial() {
        mat = Material::Builder()
                .package(RESOURCES_BAKEDTEXTURE_DATA, RESOURCES_BAKEDTEXTURE_SIZE)
                .build(*engine);
    }

    void startLoadingOneRenderable() {
        if (loadingObjectIndex >= OBJECT_COUNT) {
            return;
        }

        // `loadingObjectIndex` doesn't have to be an atomic variable because this method is always
        // called from the main thread.
        int index =  loadingObjectIndex++;
        auto* data = &objectData[index];

        // Create required resources for a renderable in parallel
        createTexture(data, onCreateTextureComplete);
        createVertexBuffer(data, onCreateVertexBufferComplete);
        createIndexBuffer(data, onCreateIndexBufferComplete);
    }

    void loadImage(OnLoadImageComplete callback = nullptr) {
        if (shuttingDown) {
            return;
        }

        utils::Invocable<void()> command = [this](){
            Path const path =
                    FilamentApp::getRootAssetsPath() + "textures/Moss_01/Moss_01_Color.png";
            if (!path.exists()) {
                std::cerr << "The texture " << path << " does not exist" << std::endl;
                exit(1);
            }
            imageData.reset(stbi_load(path.c_str(), &imageWidth, &imageHeight,
                    &imageChannels, 4));
            if (!imageData) {
                std::cerr << "The texture " << path << " could not be loaded" << std::endl;
                exit(1);
            }
        };

        if (callback) {
            engine->runCommandAsync(std::move(command), nullptr, std::move(callback));
        } else {
            command();
        }
    }

    void createTexture(void* user, OnCreateTextureComplete callback = nullptr) {
        if (shuttingDown) {
            return;
        }

        auto* data = static_cast<ObjectData*>(user);
        auto builder = Texture::Builder()
                .width(static_cast<uint32_t>(imageWidth))
                .height(static_cast<uint32_t>(imageHeight))
                .levels(1)
                // (For testing purposes) This will add a chained asynchronous operation during the
                // texture creation.
                .swizzle(Texture::Swizzle::SUBSTITUTE_ZERO, Texture::Swizzle::CHANNEL_1,
                        Texture::Swizzle::SUBSTITUTE_ZERO, Texture::Swizzle::SUBSTITUTE_ZERO)
                .sampler(Texture::Sampler::SAMPLER_2D)
                .format(Texture::InternalFormat::RGBA8);
        if (callback) {
            builder.async(nullptr, std::move(callback), user);
        }
        data->tex = builder.build(*engine);
    }

    void updateTexture(void* user, OnTextureUpdateComplete callback = nullptr) {
        if (shuttingDown) {
            return;
        }

        auto* data = static_cast<ObjectData*>(user);
        Texture::PixelBufferDescriptor buffer(imageData.get(),
                static_cast<size_t>(imageWidth * imageHeight * 4),
                Texture::Format::RGBA, Texture::Type::UBYTE
                // Don't destroy the loaded image since it needs to be reused.
                /*, (Texture::PixelBufferDescriptor::Callback)&stbi_image_free*/);
        if (callback) {
            data->tex->setImageAsync(*engine, 0, std::move(buffer),
                    nullptr, std::move(callback), user);
        } else {
            data->tex->setImage(*engine, 0, std::move(buffer));
        }
    }

    void createMaterialInstance(void* user) {
        if (shuttingDown) {
            return;
        }

        auto* data = static_cast<ObjectData*>(user);
        data->matInstance = mat->createInstance();
        TextureSampler sampler(MinFilter::LINEAR, MagFilter::LINEAR);
        data->matInstance->setParameter("albedo", data->tex, sampler);
    }

    void textureReady(void* user) {
        if (shuttingDown) {
            return;
        }

        auto* data = static_cast<ObjectData*>(user);
        data->texReady = true;
        // try creating renderable
        mayCreateRenderable(user);
    }

    void createVertexBuffer(void* user, OnCreateVertexBufferComplete callback = nullptr) {
        if (shuttingDown) {
            return;
        }

        auto* data = static_cast<ObjectData*>(user);
        static_assert(sizeof(Vertex) == 16, "Strange vertex size.");
        auto builder = VertexBuffer::Builder()
                .vertexCount(4)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 16)
                .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2, 8, 16);
        if (callback) {
            builder.async(nullptr, std::move(callback), user);
        }
        data->vb = builder.build(*engine);
    }

    void updateVertexBuffer(void* user, OnVertexBufferUpdateComplete callback = nullptr) {
        if (shuttingDown) {
            return;
        }

        auto* data = static_cast<ObjectData*>(user);
        if (callback) {
            data->vb->setBufferAtAsync(*engine, 0,
                    VertexBuffer::BufferDescriptor(QUAD_VERTICES, 64, nullptr), 0,
                    nullptr, std::move(callback), user);
        } else {
            data->vb->setBufferAt(*engine, 0,
                    VertexBuffer::BufferDescriptor(QUAD_VERTICES, 64, nullptr));
        }
    }

    void vertexBufferReady(void* user) {
        if (shuttingDown) {
            return;
        }

        auto* data = static_cast<ObjectData*>(user);
        data->vbReady = true;
        // try creating renderable
        mayCreateRenderable(user);
    }

    void createIndexBuffer(void* user, OnCreateIndexBufferComplete callback = nullptr) {
        if (shuttingDown) {
            return;
        }

        auto* data = static_cast<ObjectData*>(user);
        auto builder = IndexBuffer::Builder()
                .indexCount(6)
                .bufferType(IndexBuffer::IndexType::USHORT);
        if (callback) {
            builder.async(nullptr, std::move(callback), user);
        }
        data->ib = builder.build(*engine);
    }

    void updateIndexBuffer(void* user, OnIndexBufferUpdateComplete callback = nullptr) {
        if (shuttingDown) {
            return;
        }

        auto* data = static_cast<ObjectData*>(user);
        if (callback) {
            data->ib->setBufferAsync(*engine,
                    IndexBuffer::BufferDescriptor(QUAD_INDICES, 12, nullptr), 0,
                    nullptr, std::move(callback), user);
        } else {
            data->ib->setBuffer(*engine,
                    IndexBuffer::BufferDescriptor(QUAD_INDICES, 12, nullptr));
        }
    }

    void indexBufferReady(void* user) {
        if (shuttingDown) {
            return;
        }

        auto* data = static_cast<ObjectData*>(user);
        data->ibReady = true;
        // try creating renderable
        mayCreateRenderable(user);
    }

    void mayCreateRenderable(void* user) {
        if (shuttingDown) {
            return;
        }

        auto* data = static_cast<ObjectData*>(user);
        if (data->isReadyToCreateRenderable()) {
            createRenderable(user);
            // Done with loading a renderable, load the next one.
            startLoadingOneRenderable();
        }
    }

    void createRenderable(void* user) {
        if (shuttingDown) {
            return;
        }

        auto* data = static_cast<ObjectData*>(user);
        data->renderable = EntityManager::get().create();
        RenderableManager::Builder(1)
                .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
                .material(0, data->matInstance)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, data->vb, data->ib, 0, 6)
                .culling(false)
                .receiveShadows(false)
                .castShadows(false)
                .build(*engine, data->renderable);
        scene->addEntity(data->renderable);
    }
};

int main(int argc, char** argv) {
    Config config;
    config.title = "helloasync";
    config.asynchronousMode = backend::AsynchronousMode::THREAD_PREFERRED;
    handleCommandLineArguments(argc, argv, config);

    App app;

    auto setup = [&app](Engine* engine, View* view, Scene* scene) {
        app.engine = engine;
        app.scene = scene;

        // Set up view (Skybox & Camera)
        app.skybox = Skybox::Builder().color({0.1, 0.125, 0.25, 1.0}).build(*engine);
        scene->setSkybox(app.skybox);

        app.camera = EntityManager::get().create();
        app.cam = engine->createCamera(app.camera);
        const float zoom = 12.0;
        const float aspect =
                static_cast<float>(view->getViewport().width) / view->getViewport().height;
        app.cam->setProjection(Camera::Projection::ORTHO, -zoom, zoom,
                -zoom, zoom, -1, 1);
        view->setCamera(app.cam);
        view->setPostProcessingEnabled(false);

        // Pre-calculate the layout transform for each object in a centered 2D grid arrangement.
        const float rowStart = (App::ROW_COUNT - 1) * 0.5f;
        for (int i = 0; i < App::OBJECT_COUNT; ++i) {
            int row = i / App::OBJECT_COUNT_PER_ROW;
            int col = i % App::OBJECT_COUNT_PER_ROW;

            // Calculate number of items in this row to center it horizontally.
            // Usually equal to OBJECT_COUNT_PER_ROW, except for the last partial row.
            int colCountForThisRow = std::min(
                    App::OBJECT_COUNT - (row * App::OBJECT_COUNT_PER_ROW),
                    App::OBJECT_COUNT_PER_ROW);
            float colStart = (colCountForThisRow - 1) * 0.5f;

            auto s = math::mat4f::scaling(math::float3(0.4f, 0.4f, 0.4f));
            auto t = math::mat4f::translation(
                    math::float3(-colStart + (col * 1.0f), rowStart - (row * 1.0f), 0.0f));
            app.objectData[i].baseTransform = t * s;
        }

        if (engine->isAsynchronousModeEnabled()) {
            // Build a pipeline for asynchronous operations.
            app.onLoadImageComplete = [&app](void* user) {
                // Load this once as it's universal across all objects
                app.createMaterial();
                // Initiate loading multiple renderables at the same time.
                app.startLoadingOneRenderable();
                app.startLoadingOneRenderable();
                app.startLoadingOneRenderable();
                app.startLoadingOneRenderable();
                app.startLoadingOneRenderable();
            };
            app.onCreateTextureComplete = [&app](Texture* tex, void* user) {
                app.updateTexture(user, app.onTextureUpdateComplete);
            };
            app.onTextureUpdateComplete = [&app](Texture* tex, void* user) {
                app.createMaterialInstance(user);
                app.textureReady(user);
            };
            app.onCreateVertexBufferComplete = [&app](VertexBuffer* vb, void* user) {
                app.updateVertexBuffer(user, app.onVertexBufferUpdateComplete);
            };
            app.onVertexBufferUpdateComplete = [&app](VertexBuffer* vb, void* user) {
                app.vertexBufferReady(user);
            };
            app.onCreateIndexBufferComplete = [&app](IndexBuffer* ib, void* user) {
                app.updateIndexBuffer(user, app.onIndexBufferUpdateComplete);
            };
            app.onIndexBufferUpdateComplete = [&app](IndexBuffer* ib, void* user) {
                app.indexBufferReady(user);
            };

            // Start the chain of asynchronous operations.
            app.loadImage(app.onLoadImageComplete);
        } else {
            // Load an image and a material once as they are shared across all objects
            app.loadImage();
            app.createMaterial();
            // Load renderables synchronously
            for (int i = 0; i < App::OBJECT_COUNT; ++i) {
                void* data = &app.objectData[i];
                app.createTexture(data);
                app.updateTexture(data);
                app.createMaterialInstance(data);
                app.createVertexBuffer(data);
                app.updateVertexBuffer(data);
                app.createIndexBuffer(data);
                app.updateIndexBuffer(data);
                app.createRenderable(data);
            }
        }
    };

    auto cleanup = [&app](Engine* engine, View*, Scene*) {
        // We set this flag to guard against accessing resources (textures/buffers) inside
        // completion callbacks after cleanup.
        app.shuttingDown = true;

        for (int i = 0; i < App::OBJECT_COUNT; ++i) {
            auto& data = app.objectData[i];
            if (data.renderable) {
                engine->destroy(data.renderable);
            }
            if (data.matInstance) {
                engine->destroy(data.matInstance);
            }
            if (data.ib) {
                engine->destroy(data.ib);
            }
            if (data.vb) {
                engine->destroy(data.vb);
            }
            if (data.tex) {
                engine->destroy(data.tex);
            }
        }
        if (app.mat) {
            engine->destroy(app.mat);
        }
        if (app.skybox) {
            engine->destroy(app.skybox);
        }
        if (app.camera) {
            engine->destroyCameraComponent(app.camera);
            EntityManager::get().destroy(app.camera);
        }
    };

    FilamentApp::get().animate([&app](Engine* engine, View* view, double now) {
        auto& tm = engine->getTransformManager();
        for (int i = 0; i < App::OBJECT_COUNT; ++i) {
            auto& data = app.objectData[i];
            if (!data.renderable) {
                continue; // Skip updating transform for renderables that are not loaded yet.
            }
            auto r = math::mat4f::rotation(now, math::float3(0, 0, 1));
            tm.setTransform(tm.getInstance(data.renderable), data.baseTransform * r);
        }
    });

    FilamentApp::get().run(config, setup, cleanup);

    return 0;
}
