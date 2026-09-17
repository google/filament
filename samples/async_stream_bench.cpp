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

// A benchmark of resource streaming. While the scene renders, the sample creates objects, each
// an RGBA8 texture on a textured quad, on a frame schedule: --burst of them every --every
// frames, whatever the earlier ones' progress, so every run gets the same work in the same
// frames. They go through the asynchronous API (Builder::async, setImageAsync,
// setBufferAtAsync, setBufferAsync) by default, with the Engine in the asynchronous mode
// --async-mode names (thread, the default, or amortized), or through the synchronous one with
// --sync. With --keep, the sample destroys the burst that many bursts back as a burst starts,
// the way a scene streaming assets evicts what it no longer shows; by default every object
// stays. The run paces itself to --frame-time milliseconds a frame, as a scene streaming assets
// at a fixed frame rate would. It ends --every frames after the last burst once every object is
// visible or canceled, or gives up when no object has completed in a long while, and prints how
// long the objects took to appear and how the frames from the first burst on fared: the driver
// thread's time on each frame, the frames the Renderer skipped, and the interval between the
// rendered frames. The driver thread's time on a frame runs from the later of the frame's
// beginFrame and the previous frame's end on the driver thread to the frame's own end there.
// That span holds whatever sits ahead of the frame's own commands: an upload issued before
// beginFrame, or the asynchronous mode's jobs that tick() drains. Those run on the driver thread
// there, and the backend's own beginFrame-to-endFrame window misses them. A warm-up quad draws
// the material before the schedule starts, so the first burst does not compile its pipeline.

#include "common/arguments.h"

#include "generated/resources/resources.h"

#include <filamentapp/FilamentApp2.h>

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>

#include <backend/DriverEnums.h>

#include <utils/EntityManager.h>

#include <math/mat4.h>
#include <math/vec2.h>
#include <math/vec3.h>

#include <samples/SampleConfig.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <map>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

using namespace filament;
using namespace filament::math;
using utils::Entity;
using utils::EntityManager;
using AsyncCallStatus = backend::AsyncCallStatus;

namespace {

struct Vertex {
    float2 position;
    float2 uv;
};

constexpr Vertex QUAD_VERTICES[4] = {
    {{ -1, -1 }, { 0, 0 }},
    {{  1, -1 }, { 1, 0 }},
    {{ -1,  1 }, { 0, 1 }},
    {{  1,  1 }, { 1, 1 }},
};

constexpr uint16_t QUAD_INDICES[6] = {
    0, 1, 2,
    3, 2, 1,
};

constexpr uint8_t WARMUP_TEXEL[4] = { 255, 255, 255, 255 };

// Frames without an object completing, after the last burst, before the run gives up.
constexpr int STALL_FRAMES = 600;

struct Object {
    Texture* tex = nullptr;
    VertexBuffer* vb = nullptr;
    IndexBuffer* ib = nullptr;
    MaterialInstance* mi = nullptr;
    Entity renderable;
    float3 position;
    int pending = 0;        // asynchronous uploads still to complete
    int startFrame = -1;    // the frame the object was started in
    int visibleFrame = -1;  // the frame its renderable was added to the scene in
};

struct App {
    // The schedule (--objects, --every, --burst, --texture-size), the eviction (--keep) and the
    // pacing (--frame-time).
    int objectCount;
    int every;
    int burst;
    int textureSize;
    int keep;
    float frameTimeMs;
    backend::AsynchronousMode requestedMode;  // the Engine's mode, as asked for

    FilamentApp2* filamentApp = nullptr;
    Engine* engine = nullptr;
    Scene* scene = nullptr;
    Entity camera;
    Camera* cam = nullptr;
    Skybox* skybox = nullptr;
    Material* material = nullptr;
    // Every upload reads this one image. It outlives the Engine, so no upload can read it after
    // it is gone.
    std::vector<uint8_t> image;
    std::vector<Object> objects;
    Object warmup;  // draws the material before the schedule starts

    bool sync = false;  // the synchronous API: the Engine's asynchronous mode is off
    bool shuttingDown = false;
    int frame = 0;  // frames so far, rendered or skipped by the Renderer
    std::chrono::steady_clock::time_point nextFrame;
    int started = 0;
    int visible = 0;
    int canceled = 0;  // objects given up on: a creation or upload canceled, or evicted first
    int lastProgressFrame = 0;  // the last frame an object started, became visible or was given up
    int firstStartFrame = -1;
    int lastStartFrame = -1;
    int renderedSinceStart = 0;  // the frames from the first start on that were rendered
    // Every rendered frame's Renderer::FrameInfo, by frame id, and the time on the FrameInfo
    // clock the first object started at: the frames from then on are the ones the summary reads.
    std::map<uint32_t, Renderer::FrameInfo> frameInfos;
    int64_t firstStartTime = 0;

    Texture::PixelBufferDescriptor pixels() const {
        return { image.data(), image.size(), Texture::Format::RGBA, Texture::Type::UBYTE };
    }

    static Texture::Builder textureBuilder(int size) {
        return Texture::Builder()
                .width(uint32_t(size))
                .height(uint32_t(size))
                .levels(1)
                .sampler(Texture::Sampler::SAMPLER_2D)
                .format(Texture::InternalFormat::RGBA8);
    }

    static VertexBuffer::Builder quadVertexBufferBuilder() {
        return VertexBuffer::Builder()
                .vertexCount(4)
                .bufferCount(1)
                .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2,
                        0, sizeof(Vertex))
                .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2,
                        sizeof(float2), sizeof(Vertex));
    }

    static IndexBuffer::Builder quadIndexBufferBuilder() {
        return IndexBuffer::Builder()
                .indexCount(6)
                .bufferType(IndexBuffer::IndexType::USHORT);
    }

    // Starts an object: creates its texture and buffers, and uploads them. With the
    // asynchronous API each resource's creation callback issues its upload, and the last upload
    // to complete creates the renderable; the callbacks run on the main thread.
    void startObject(Object& o) {
        o.startFrame = frame;
        lastStartFrame = frame;
        lastProgressFrame = frame;
        if (started++ == 0) {
            firstStartFrame = frame;
            firstStartTime = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count();
        }

        if (sync) {
            createSync(o, textureSize, pixels());
            o.visibleFrame = frame;
            ++visible;
            return;
        }

        o.pending = 3;
        o.tex = textureBuilder(textureSize)
                .async(nullptr, [this](Texture* tex, void* user, AsyncCallStatus status) {
                    if (!proceed(user, status)) return;
                    tex->setImageAsync(*engine, 0, pixels(), nullptr,
                            [this](Texture*, void* user, AsyncCallStatus status) {
                                uploadComplete(user, status);
                            }, user);
                }, &o)
                .build(*engine);
        o.vb = quadVertexBufferBuilder()
                .async(nullptr, [this](VertexBuffer* vb, void* user, AsyncCallStatus status) {
                    if (!proceed(user, status)) return;
                    vb->setBufferAtAsync(*engine, 0, { QUAD_VERTICES, sizeof(QUAD_VERTICES) }, 0,
                            nullptr, [this](VertexBuffer*, void* user, AsyncCallStatus status) {
                                uploadComplete(user, status);
                            }, user);
                }, &o)
                .build(*engine);
        o.ib = quadIndexBufferBuilder()
                .async(nullptr, [this](IndexBuffer* ib, void* user, AsyncCallStatus status) {
                    if (!proceed(user, status)) return;
                    ib->setBufferAsync(*engine, { QUAD_INDICES, sizeof(QUAD_INDICES) }, 0,
                            nullptr, [this](IndexBuffer*, void* user, AsyncCallStatus status) {
                                uploadComplete(user, status);
                            }, user);
                }, &o)
                .build(*engine);
    }

    // Whether a callback's object goes on: not while the Engine shuts down, and not once its
    // creation or an upload was canceled.
    bool proceed(void* user, AsyncCallStatus status) {
        if (shuttingDown) return false;
        auto& o = *static_cast<Object*>(user);
        if (status == AsyncCallStatus::CANCELED) {
            giveUp(o);
            return false;
        }
        // An object given up on has been destroyed: a creation that completes after that must
        // not upload into it.
        return o.pending > 0;
    }

    void uploadComplete(void* user, AsyncCallStatus status) {
        if (!proceed(user, status)) return;
        auto& o = *static_cast<Object*>(user);
        if (o.pending > 0 && --o.pending == 0) {
            createRenderable(o);
            o.visibleFrame = frame;
            ++visible;
            lastProgressFrame = frame;
        }
    }

    // Gives up on an object that will not become visible: the sample ignores its later
    // callbacks, and the run does not wait for it.
    void giveUp(Object& o) {
        if (o.pending > 0) {
            o.pending = 0;
            ++canceled;
            lastProgressFrame = frame;
        }
    }

    // Creates an object through the synchronous API, texels and all.
    void createSync(Object& o, int size, Texture::PixelBufferDescriptor&& texels) {
        o.tex = textureBuilder(size).build(*engine);
        o.tex->setImage(*engine, 0, std::move(texels));
        o.vb = quadVertexBufferBuilder().build(*engine);
        o.vb->setBufferAt(*engine, 0, { QUAD_VERTICES, sizeof(QUAD_VERTICES) });
        o.ib = quadIndexBufferBuilder().build(*engine);
        o.ib->setBuffer(*engine, { QUAD_INDICES, sizeof(QUAD_INDICES) });
        createRenderable(o);
    }

    void createRenderable(Object& o) {
        o.mi = material->createInstance();
        o.mi->setParameter("albedo", o.tex,
                TextureSampler(TextureSampler::MinFilter::LINEAR,
                        TextureSampler::MagFilter::LINEAR));
        o.renderable = EntityManager::get().create();
        RenderableManager::Builder(1)
                .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
                .material(0, o.mi)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, o.vb, o.ib, 0, 6)
                .culling(false)
                .receiveShadows(false)
                .castShadows(false)
                .build(*engine, o.renderable);
        auto& tm = engine->getTransformManager();
        tm.setTransform(tm.getInstance(o.renderable),
                mat4f::translation(o.position) * mat4f::scaling(0.4f));
        scene->addEntity(o.renderable);
    }

    void destroyObject(Object& o) {
        if (o.renderable) {
            scene->remove(o.renderable);
            engine->destroy(o.renderable);
            EntityManager::get().destroy(o.renderable);
            o.renderable = {};
        }
        if (o.mi) engine->destroy(o.mi);
        if (o.ib) engine->destroy(o.ib);
        if (o.vb) engine->destroy(o.vb);
        if (o.tex) engine->destroy(o.tex);
        o.mi = nullptr;
        o.ib = nullptr;
        o.vb = nullptr;
        o.tex = nullptr;
    }

    // Once per frame, before it is rendered. The run does not catch up a frame that overran its
    // period: the next one starts now.
    void beginFrame() {
        if (frameTimeMs > 0.0f) {
            auto const period = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                    std::chrono::duration<float, std::milli>(frameTimeMs));
            nextFrame = std::max(nextFrame + period, std::chrono::steady_clock::now());
            std::this_thread::sleep_until(nextFrame);
        }
        ++frame;
        if (started < objectCount && frame % every == 0) {
            // With --keep, the burst that many bursts back goes as this one starts; every burst
            // before the last is --burst objects. The sample gives up on one not visible yet,
            // its uploads in flight.
            if (keep > 0 && started >= keep * burst) {
                for (int i = started - keep * burst; i < started - (keep - 1) * burst; ++i) {
                    giveUp(objects[i]);
                    destroyObject(objects[i]);
                }
            }
            int const n = std::min(burst, objectCount - started);
            for (int i = 0; i < n; ++i) {
                startObject(objects[started]);
            }
        }
    }

    // Once per rendered frame, after it is rendered; a frame the Renderer skipped gets none.
    // The history holds the last frames whose backend timestamps have arrived; polling every
    // frame sees each frame at least once.
    void endFrame(Renderer* renderer) {
        if (started > 0) ++renderedSinceStart;
        for (auto const& info : renderer->getFrameInfoHistory(renderer->getMaxFrameHistorySize())) {
            frameInfos.insert_or_assign(info.frameId, info);
        }
        if (started < objectCount) return;
        if (visible + canceled == objectCount) {
            if (frame >= lastStartFrame + every) filamentApp->close();
        } else if (frame >= lastProgressFrame + STALL_FRAMES) {
            printf("async_stream_bench: no object completed in %d frames, giving up with %d of "
                   "%d visible\n", STALL_FRAMES, visible, objectCount);
            filamentApp->close();
        }
    }

    char const* requestedModeName() const {
        switch (requestedMode) {
            case backend::AsynchronousMode::NONE: return "asynchronous mode off";
            case backend::AsynchronousMode::THREAD_PREFERRED: return "thread mode requested";
            case backend::AsynchronousMode::AMORTIZATION: return "amortized mode requested";
        }
        return "";
    }

    void printSummary() const {
        auto const backendName = backend::to_string(engine->getBackend());
        printf("async_stream_bench: %d objects of %dx%d RGBA8, %d every %d frames of %.2f ms, "
               "%s API, %s, %.*s backend", objectCount, textureSize, textureSize, burst, every,
                frameTimeMs, sync ? "synchronous" : "asynchronous", requestedModeName(),
                int(backendName.size()), backendName.data());
        if (keep > 0) printf(", the last %d bursts kept", keep);
        printf("\n");

        std::vector<int> latency;
        for (auto const& o : objects) {
            if (o.startFrame >= 0 && o.visibleFrame >= 0) {
                latency.push_back(o.visibleFrame - o.startFrame);
            }
        }
        std::sort(latency.begin(), latency.end());
        printf("  started %d (frames %d-%d), visible %d", started, firstStartFrame,
                lastStartFrame, visible);
        if (!latency.empty()) {
            printf(", frames from start to visible: median %d, p90 %d, max %d",
                    latency[latency.size() / 2], latency[latency.size() * 9 / 10],
                    latency.back());
        }
        printf(", canceled %d\n", canceled);
        if (firstStartFrame < 0) return;

        // The frames from the first start on: the driver thread's time on each rendered one
        // (see the top of the file) and the interval from it to the next rendered one, which
        // spans the frames the Renderer skipped.
        std::vector<double> driverMs;
        std::vector<double> intervalMs;
        int64_t previousEnd = -1;    // the previous rendered frame's backendEndFrame
        int64_t previousBegin = -1;  // its beginFrame
        bool previousInWindow = false;
        for (auto const& [id, info] : frameInfos) {
            bool const inWindow = info.beginFrame >= firstStartTime;
            if (inWindow && info.backendEndFrame >= 0) {
                int64_t const begin = std::max(previousEnd, info.beginFrame);
                driverMs.push_back(double(info.backendEndFrame - begin) / 1e6);
            }
            if (inWindow && previousInWindow) {
                intervalMs.push_back(double(info.beginFrame - previousBegin) / 1e6);
            }
            previousEnd = info.backendEndFrame;
            previousBegin = info.beginFrame;
            previousInWindow = inWindow;
        }
        auto print = [](char const* name, std::vector<double>& v) {
            if (v.empty()) return;
            std::sort(v.begin(), v.end());
            printf("%s ms median %.2f, p99 %.2f, max %.2f", name, v[v.size() / 2],
                    v[v.size() * 99 / 100], v.back());
        };
        int const frames = frame - firstStartFrame + 1;
        printf("  %d frames from the first start, %d skipped: ", frames,
                frames - renderedSinceStart);
        print("driver thread", driverMs);
        printf("; ");
        print("interval between rendered frames", intervalMs);
        printf("\n");
    }
};

} // namespace

std::unique_ptr<FilamentApp2> createSampleApp(SampleConfig config,
        filament::app::DisplayManager* dm, filament::app::AssetLoader* loader) {
    auto app = std::make_shared<App>();
    app->objectCount = config.getInt("objects", 96);
    app->every = config.getInt("every", 30);
    app->burst = config.getInt("burst", 8);
    app->textureSize = config.getInt("texture-size", 2048);
    app->keep = config.getInt("keep", 0);
    app->frameTimeMs = config.getFloat("frame-time", 16.67f);
    app->requestedMode = config.asynchronousMode;

    auto setup = [app](Engine* engine, View* view, Scene* scene) {
        app->engine = engine;
        app->scene = scene;
        app->sync = !engine->isAsynchronousModeEnabled();
        if (app->sync && app->requestedMode != backend::AsynchronousMode::NONE) {
            auto const backendName = backend::to_string(engine->getBackend());
            fprintf(stderr, "async_stream_bench: the Engine's asynchronous mode is off on %.*s; "
                    "using the synchronous API\n", int(backendName.size()), backendName.data());
        }
        app->nextFrame = std::chrono::steady_clock::now();

        // The objects in a centered grid, seen by an orthographic camera that fits it.
        int const cols = int(std::ceil(std::sqrt(double(app->objectCount))));
        int const rows = (app->objectCount + cols - 1) / cols;
        app->objects.resize(size_t(app->objectCount));
        for (int i = 0; i < app->objectCount; ++i) {
            int const col = i % cols;
            int const row = i / cols;
            app->objects[i].position = { float(col) - 0.5f * float(cols - 1),
                    0.5f * float(rows - 1) - float(row), 0.0f };
        }
        float const aspect = float(view->getViewport().width) / float(view->getViewport().height);
        float const zoom = std::max(0.5f * float(cols) / aspect, 0.5f * float(rows)) + 0.5f;
        app->camera = EntityManager::get().create();
        app->cam = engine->createCamera(app->camera);
        app->cam->setProjection(Camera::Projection::ORTHO,
                -zoom * aspect, zoom * aspect, -zoom, zoom, -1, 1);
        view->setCamera(app->cam);
        view->setPostProcessingEnabled(false);
        app->skybox = Skybox::Builder().color({ 0.1, 0.125, 0.25, 1.0 }).build(*engine);
        scene->setSkybox(app->skybox);

        app->material = Material::Builder()
                .package(RESOURCES_BAKEDTEXTURE_DATA, RESOURCES_BAKEDTEXTURE_SIZE)
                .build(*engine);

        int const size = app->textureSize;
        app->image.resize(size_t(size) * size_t(size) * 4);
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                uint8_t* p = &app->image[(size_t(y) * size_t(size) + size_t(x)) * 4];
                p[0] = uint8_t(x);
                p[1] = uint8_t(y);
                p[2] = uint8_t(x ^ y);
                p[3] = 255;
            }
        }

        // The material's first draw compiles its pipeline; a quad drawn before the schedule
        // starts keeps that out of the first burst. Its 1x1 texture sits behind the grid.
        app->warmup.position = { 0.0f, 0.0f, -0.5f };
        app->createSync(app->warmup, 1, { WARMUP_TEXEL, sizeof(WARMUP_TEXEL),
                Texture::Format::RGBA, Texture::Type::UBYTE });
    };

    auto cleanup = [app](Engine* engine, View*, Scene*) {
        app->printSummary();
        // Guards the completion callbacks that still arrive while the Engine shuts down.
        app->shuttingDown = true;
        for (auto& o : app->objects) app->destroyObject(o);
        app->destroyObject(app->warmup);
        engine->destroy(app->material);
        engine->destroy(app->skybox);
        engine->destroyCameraComponent(app->camera);
        EntityManager::get().destroy(app->camera);
    };

    auto fApp = samples::getBuilder(config, dm, loader)
            .setup(setup)
            .cleanup(cleanup)
            .animation([app](Engine*, View*, double) { app->beginFrame(); })
            .postRender([app](Engine*, View*, Scene*, Renderer* renderer) {
                app->endFrame(renderer);
            })
            .build();
    app->filamentApp = fApp.get();
    return fApp;
}

samples::SampleParameters createAppParameters() {
    return {
        samples::Parameter::makeInt("objects", '\0', "Objects to stream in", 96, 1),
        samples::Parameter::makeInt("every", '\0', "Frames between two bursts of objects", 30, 1),
        samples::Parameter::makeInt("burst", '\0', "Objects started together in a burst", 8, 1),
        samples::Parameter::makeInt("texture-size", '\0',
                "Side of each object's square RGBA8 texture, in texels", 2048, 1),
        samples::Parameter::makeInt("keep", '\0',
                "Bursts kept: the burst this many bursts back is destroyed as a burst starts "
                "(0: every object stays)", 0, 0),
        samples::Parameter::makeFloat("frame-time", '\0',
                "Milliseconds a frame is paced to, sleeping ahead of the frame (0: unpaced)",
                16.67f, 0.0f),
        samples::Parameter::makeString("async-mode", '\0',
                "The Engine's asynchronous mode: thread (a worker thread, where the backend has "
                "one) or amortized (a few jobs a frame on the driver thread)", "thread"),
        samples::Parameter::makeBool("sync", '\0',
                "Create the objects through the synchronous API, with the Engine's asynchronous "
                "mode off", false),
    };
}

#ifndef __ANDROID__
int main(int argc, char** argv) {
    SampleConfig config;
    config.title = "async_stream_bench";
    config.asynchronousMode = backend::AsynchronousMode::THREAD_PREFERRED;
    samples::handleCommandLineArguments(argc, argv, &config,
            { .parameters = createAppParameters() });
    auto const mode = config.getString("async-mode", "thread");
    if (mode == "amortized") {
        config.asynchronousMode = backend::AsynchronousMode::AMORTIZATION;
    } else if (mode != "thread") {
        fprintf(stderr, "async_stream_bench: --async-mode is thread or amortized, not %s\n",
                mode.c_str());
        return 1;
    }
    if (config.getBool("sync")) {
        config.asynchronousMode = backend::AsynchronousMode::NONE;
    }
    auto dm = samples::getDisplayManager(config);
    auto loader = samples::getAssetLoader(config);
    auto app = createSampleApp(config, dm.get(), loader.get());
    app->run();
    return 0;
}
#endif
