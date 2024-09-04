#include <fstream>
#include <iostream>

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/LightManager.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Skybox.h>
#include <filament/View.h>
#include <filament/Viewport.h>
#include <filament/SwapChain.h>
#include <filament/Texture.h>
#include <filament/TransformManager.h>
#include <filament/RenderTarget.h>

#include <filamat/MaterialBuilder.h>

#include <utils/EntityManager.h>
#include <utils/Log.h>

#include <math/norm.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/ResourceLoader.h>

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_video.h>

#include <filamentapp/NativeWindowHelper.h>

#include "generated/resources/resources.h"
#include "materials/uberarchive.h"

#include <vector>

using namespace filament;
using namespace math;
using namespace utils;

namespace {

const static uint32_t gIndices[] = {0, 1, 2, 2, 3, 0};

float const QUAD_WIDTH = .5;

const static math::float3 gVertices[] = {
    float3 {0, 0, 0} * QUAD_WIDTH,
    float3 {0, 1, 0} * QUAD_WIDTH,
    float3 {1, 1, 0} * QUAD_WIDTH,
    float3 {1, 0, 0} * QUAD_WIDTH,
};

const static math::float2 gUVs[] = {
    {0, 0},
    {0, 1},
    {1, 1},
    {1, 0},
};

const static short4 tbn = math::packSnorm16(mat3f::packTangentFrame(
        math::mat3f{float3{1.0f, 0.0f, 0.0f}, float3{0.0f, 0.0f, 1.0f},
            float3{0.0f, 1.0f, 0.0f}}).xyzw);

const static math::short4 gNormals[]{tbn, tbn, tbn, tbn};

struct TestPrimitive {
    TestPrimitive(Engine* engine) {
        vertexBuffer =
                VertexBuffer::Builder()
                        .vertexCount(4)
                        .bufferCount(3)
                        .attribute(VertexAttribute::POSITION, 0,
                                VertexBuffer::AttributeType::FLOAT3)
                        .attribute(VertexAttribute::UV0, 1, VertexBuffer::AttributeType::FLOAT2)
                        .attribute(VertexAttribute::TANGENTS, 2, VertexBuffer::AttributeType::FLOAT4)
                        .build(*engine);
        vertexBuffer->setBufferAt(*engine, 0,
                VertexBuffer::BufferDescriptor(gVertices,
                        vertexBuffer->getVertexCount() * sizeof(gVertices[0])));
        vertexBuffer->setBufferAt(*engine, 1,
                VertexBuffer::BufferDescriptor(gUVs,
                        vertexBuffer->getVertexCount() * sizeof(gUVs[0])));
        vertexBuffer->setBufferAt(*engine, 2,
                VertexBuffer::BufferDescriptor(gNormals,
                        vertexBuffer->getVertexCount() * sizeof(gNormals[0])));

        indexBuffer = IndexBuffer::Builder().indexCount(6).build(*engine);

        indexBuffer->setBuffer(*engine, IndexBuffer::BufferDescriptor(gIndices,
                                                indexBuffer->getIndexCount() * sizeof(uint32_t)));
    }

    VertexBuffer* verts() const { return vertexBuffer; }
    IndexBuffer* inds() const { return indexBuffer; }

private:
    VertexBuffer* vertexBuffer;
    IndexBuffer* indexBuffer;
};

struct TestMaterial {
    TestMaterial(Engine* engine) {
        material = Material::Builder()
                .package(RESOURCES_ICONTEST_DATA, RESOURCES_ICONTEST_SIZE)
                .build(*engine);
    }

    MaterialInstance* instance() {
        auto inst = material->createInstance();
        instances.push_back(inst);
        return inst;
    }

private:
    Material* material;
    std::vector<MaterialInstance*> instances;
};

struct TestTexture {
    TestTexture(Engine* engine, size_t width, size_t height, bool attachment=false) {
        auto usage = Texture::Usage::DEFAULT;
        if (attachment) {
            usage = usage | Texture::Usage::COLOR_ATTACHMENT;
        }
        texture = Texture::Builder()
            .width(width).height(height).levels(1)
            .usage(usage)
            .format(Texture::InternalFormat::RGBA8).build(*engine);
        uint8_t* data = (uint8_t*) malloc(width * height * 4);
        std::memset(data, 0xFF, width * height * 4);
        Texture::PixelBufferDescriptor buffer(data, size_t(width * height * 4),
                                              Texture::Format::RGBA, Texture::Type::UBYTE,
                                              [](void* buf, size_t size, void* user) {
                                                  free(buf);
                                              });
        texture->setImage(*engine, 0, 0, 0, 0, width, height, 1, std::move(buffer));
    }
    Texture* get() const  { return texture; }
private:
    Texture* texture;
};

std::pair<SDL_Window*, void*> createSDLwindow() {
    const uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    SDL_Window* win = SDL_CreateWindow("Hello World!", 0, 100, 800, 800, windowFlags);
    if (win == nullptr) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return {NULL, NULL};
    }

    void* nativeWindow = getNativeWindow(win);
#if defined(__APPLE__)
    prepareNativeWindow(win);
    setUpMetalLayer(nativeWindow);
#endif

    return {win, nativeWindow};
}

struct IconScene {

    struct Option {
        float border=1.4;
        bool postProcessing=true;
        bool useSkybox=false;
    };

    IconScene(Engine* engine, Renderer* renderer, uint32_t w, uint32_t h, Option option)
        : engine(engine),
          renderer(renderer),
          material(engine),
          primitives(engine) {
        cameraEntity = EntityManager::get().create();
        camera = engine->createCamera(cameraEntity);
        camera->lookAt(float3(0, 0, -10.f), float3(0, 0, 0), float3(0, 1.0f, 0));
        float halfWidth = (QUAD_WIDTH / 2) * option.border;
        camera->setProjection(Camera::Projection::ORTHO,
                -halfWidth , halfWidth, -halfWidth, halfWidth, -50, 50);

        scene = engine->createScene();

        view = engine->createView();
        view->setViewport({0, 0, w, h});
        view->setScene(scene);
        view->setCamera(camera);
        view->setPostProcessingEnabled(false);

        Entity renderable = EntityManager::get().create();
        matInstance = material.instance();
        RenderableManager::Builder(1)
                .boundingBox({{-10, -10, -10}, {10, 10, 10}})
                .material(0, matInstance)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES,
                        primitives.verts(), primitives.inds(), 0, 6)
                .culling(false)
                .build(*engine, renderable);

        scene->addEntity(renderable);

        auto& tcm = engine->getTransformManager();
        tcm.create(renderable);
        auto inst = tcm.getInstance(renderable);
        mat4f xlate = mat4f::translation(float3(QUAD_WIDTH / -2.0, QUAD_WIDTH / -2.0, 0));
        tcm.setTransform(inst, xlate);
    }

    void render(Texture* texture, Texture* renderTargetTexture, float3 const& multiplier) {
        TextureSampler sampler{TextureSampler::MinFilter::NEAREST,
                TextureSampler::MagFilter::NEAREST};

        matInstance->setParameter("tex", texture, sampler);
        matInstance->setParameter("multiplier", multiplier);

        RenderTarget* target;
        if (auto itr = targets.find(renderTargetTexture); itr != targets.end()) {
            target = itr->second;
        } else {
            target = RenderTarget::Builder()
                    .texture(RenderTarget::AttachmentPoint::COLOR, renderTargetTexture)
                    .build(*engine);
            targets[renderTargetTexture] = target;
        }

        view->setRenderTarget(target);
        renderer->renderStandaloneView(view);
    }

    void renderToSwapChain(SwapChain* swapChain, Texture* texture) {
        TextureSampler sampler{TextureSampler::MinFilter::NEAREST,
                TextureSampler::MagFilter::NEAREST};
        matInstance->setParameter("tex", texture, sampler);
        matInstance->setParameter("multiplier", float3 {1});

        if (renderer->beginFrame(swapChain)) {
            // for each View
            renderer->render(view);
            renderer->endFrame();
        }
    }


private:
    Engine* engine;
    Renderer* renderer;
    View* view;
    Scene* scene;
    Camera* camera;
    TestMaterial material;
    TestPrimitive primitives;
    Entity cameraEntity;
    Skybox* skybox;
    MaterialInstance* matInstance;
    std::unordered_map<Texture*, RenderTarget*> targets;
};

struct MainScene {
    using TextureMatrix = std::vector<std::vector<Texture*>>;

    MainScene(Engine* engine, Renderer* renderer, uint32_t w, uint32_t h, uint32_t nrows,
            uint32_t ncols)
        : renderer(renderer),
          primitives{engine},
          material(engine),
          engine(engine) {
        cameraEntity = EntityManager::get().create();

        camera = engine->createCamera(cameraEntity);
        camera->lookAt(float3(0, 0, -10.f), float3(0, 0, 0), float3(0, 1.0f, 0));
        camera->setProjection(45.0, double(w) / h, 0.1, 50, Camera::Fov::VERTICAL);

        scene = engine->createScene();

//        skybox = Skybox::Builder().color({0.2, 0.2, 0.2, 1.0}).build(*engine);
//        scene->setSkybox(skybox);

        view = engine->createView();
        view->setViewport({0, 0, w, h});
        view->setScene(scene);
        view->setCamera(camera);
        view->setPostProcessingEnabled(false);

        root = EntityManager::get().create();
        auto& tcm = engine->getTransformManager();
        tcm.create(root);
        auto rootInst = tcm.getInstance(root);
        mat4f xlate = mat4f::translation(float3(-QUAD_WIDTH / 2 * nrows , -QUAD_WIDTH / 2 * ncols, 0));
        tcm.setTransform(rootInst, xlate);

        matInst = material.instance();

        renderable = EntityManager::get().create();
        scene->addEntity(renderable);
        RenderableManager::Builder(1)
                .boundingBox({{-10, -10, -10}, {10, 10, 10}})
                .material(0, matInst)
                .geometry(0, RenderableManager::PrimitiveType::TRIANGLES,
                        primitives.verts(), primitives.inds(), 0, 6)
                .culling(false)
                .build(*engine, renderable);

        auto transformInst = tcm.getInstance(renderable);
        tcm.setParent(transformInst, rootInst);
    }

    void render(Texture* texture, RenderTarget* target, uint32_t r, uint32_t c) {
        auto& tcm = engine->getTransformManager();
        TextureSampler sampler{TextureSampler::MinFilter::NEAREST,
                TextureSampler::MagFilter::NEAREST};
        matInst->setParameter("tex", texture, sampler);
        matInst->setParameter("multiplier", float3 {1});

        auto transformInst = tcm.getInstance(renderable);
        mat4f xlate = mat4f::translation(float3(QUAD_WIDTH * r, QUAD_WIDTH * c, 0));
        tcm.setTransform(transformInst, xlate);

        view->setRenderTarget(target);
        renderer->renderStandaloneView(view);
    }

    void renderToSwapChain(SwapChain* swapChain, Texture* texture, uint32_t r, uint32_t c){
        auto& tcm = engine->getTransformManager();
        TextureSampler sampler{TextureSampler::MinFilter::NEAREST,
                TextureSampler::MagFilter::NEAREST};
        matInst->setParameter("tex", texture, sampler);
        matInst->setParameter("multiplier", float3 {1});

        auto transformInst = tcm.getInstance(renderable);
        mat4f xlate = mat4f::translation(float3(QUAD_WIDTH * r, QUAD_WIDTH * c, 0));
        tcm.setTransform(transformInst, xlate);

        if (renderer->beginFrame(swapChain)) {
            // for each View
            renderer->render(view);
            renderer->endFrame();
        }
    }


    Scene* get() const { return scene; }
    Camera* getCamera() const { return camera; }
    View* getView() const { return view; }

private:
    Renderer* renderer;
    TestMaterial material;
    MaterialInstance* matInst;
    TestPrimitive primitives;
    TextureMatrix textures;
    Scene* scene;
    Entity light;
    Entity root;
    Entity renderable;
    Camera* camera;
    Entity cameraEntity;
    Skybox* skybox;
    View* view;
    std::vector<Entity> renderables;
    Engine* engine;
};

}

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);
    auto const [window, nativeWindow] = createSDLwindow();

    Engine* engine = Engine::create(filament::backend::Backend::VULKAN);
    SwapChain* swapChain =
            engine->createSwapChain(nativeWindow, filament::SwapChain::CONFIG_HAS_STENCIL_BUFFER);
    Renderer* renderer = engine->createRenderer();

    // Determine the current size of the window in physical pixels.
    uint32_t w, h;
    SDL_GL_GetDrawableSize(window, (int*) &w, (int*) &h);

    filamat::MaterialBuilder::init();

    TestTexture matTexture {engine, 27, 27};

    size_t const nrow = 10, ncol = 10;

    uint32_t const iconWidth = 50;
    uint32_t const iconHeight = 50;

    IconScene iconScene {engine, renderer, iconWidth, iconHeight, {.useSkybox=false} };

    auto buildTargets = [nrow, ncol, engine]() {
        std::vector<std::vector<Texture*>> targets;
        for (size_t r = 0; r < nrow; r++) {
            std::vector<Texture*> row;
            for (size_t c = 0; c < ncol; c++) {
                TestTexture ttex {engine, iconWidth, iconHeight, true };
                row.push_back(ttex.get());
            }
            targets.push_back(row);
        }
        return targets;
    };

    std::vector<std::vector<Texture*>> targets0 = buildTargets();
    std::vector<std::vector<Texture*>> targets1 = buildTargets();
    std::vector<std::vector<Texture*>> targets2 = buildTargets();

    MainScene scene(engine, renderer, w, h, nrow, ncol);

    renderer->setClearOptions({.clear = false});
    float3 const a {.7, .3, .3};
    float3 const b {.3, .7, .3};
    std::vector<std::pair<size_t, size_t>> coords;
    for (size_t r = 0; r < nrow; r++) {
        for (size_t c = 0; c < ncol; c++) {
            coords.push_back({r, c});
        }
    }

    IconScene swapChainScene{engine, renderer, w, h, {
            .border = 1.0f, .postProcessing = false}};

    TestTexture mainTex{engine, w, h, true};
    auto mainRT = RenderTarget::Builder()
            .texture(RenderTarget::AttachmentPoint::COLOR, mainTex.get())
            .build(*engine);

    bool closeWindow = false;
    size_t count = 0;
    while (!closeWindow && ++count) {
        constexpr int kMaxEvents = 16;
        SDL_Event events[kMaxEvents];
        int nevents = 0;
        while (nevents < kMaxEvents && SDL_PollEvent(&events[nevents]) != 0) {
            nevents++;
        }

        // Now, loop over the events a second time for app-side processing.
        for (int i = 0; i < nevents; i++) {
            const SDL_Event& event = events[i];
            switch (event.type) {
                case SDL_QUIT:
                    closeWindow = true;
                    break;
                case SDL_WINDOWEVENT:
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_RESIZED:
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
        auto [r, c] = coords[count % coords.size()];
        float3 color0 = (r + c) % 2 == 0 ? a : b;
        float3 color1 = (r + c) % 2 == 0 ? b : a;
        auto rt0 = targets0[r][c];
        auto rt1 = targets1[r][c];
        auto rt2 = targets2[r][c];
        iconScene.render(matTexture.get(), rt0, color0);
        iconScene.render(rt0, rt1, color1);
        iconScene.render(rt1, rt2, float3 {.8});
//        engine->flushAndWait();
        scene.render(rt2, mainRT, r, c);
//        engine->flushAndWait();
//        engine->flush();
//        scene.renderToSwapChain(swapChain, rt1, r, c);


        if (count % 27 == 0) {
            swapChainScene.renderToSwapChain(swapChain, mainTex.get());
        }
        SDL_Delay(16);

        // if (renderer->beginFrame(swapChain)) {
        //     // for each View
        //     renderer->render(scene.getView());
        //     renderer->endFrame();
        // }


//        SDL_Delay(16);

    }
    return 0;
}
