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

#include <filamat/MaterialBuilder.h>

#include <utils/EntityManager.h>
#include <utils/Log.h>

#include <math/norm.h>

#include <gltfio/AssetLoader.h>
#include <gltfio/ResourceLoader.h>

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_video.h>


#if defined(__linux__)
void* getNativeWindow(SDL_Window* sdlWindow) {
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    SDL_GetWindowWMInfo(sdlWindow, &wmi);
    if (wmi.subsystem == SDL_SYSWM_X11) {
        Window win = (Window) wmi.info.x11.window;
        return (void*) win;
    } else {
        std::cout << "Unknown SDL subsystem";
    }
    return nullptr;
}
#elif defined(__APPLE__)

#include "helper.h"

#endif

SDL_Window* createSDLwindow() {
    uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
    SDL_Window* win = SDL_CreateWindow("Hello World!", 100, 100, 600, 400, 0);
    if (win == nullptr) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return nullptr;
    }

    return win;
}

using namespace filament;
using namespace math;
using namespace utils;
int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;        
        return 1;
    }
    const static uint32_t indices[] = { 0, 1, 2, };

    const static math::float3 vertices[] = {
        { -10, 0, -10 },
        { -10, 0, 10 },
        { 10, 0, 10 },
    };

    short4 tbn = math::packSnorm16(
            mat3f::packTangentFrame(
                    math::mat3f{
                            float3{ 1.0f, 0.0f, 0.0f },
                            float3{ 0.0f, 0.0f, 1.0f }, float3{ 0.0f, 1.0f, 0.0f } })
                    .xyzw);

    const static math::short4 normals[]{ tbn, tbn, tbn };
    SDL_Window* window = createSDLwindow();
    if (!window) {
        return 1;
    }
    Engine* engine = Engine::create(filament::backend::Backend::VULKAN);
    SwapChain* swapChain = engine->createSwapChain(getNativeWindow(window));
    Renderer* renderer = engine->createRenderer();

    auto cameraEntity = EntityManager::get().create();
    Camera* camera = engine->createCamera(cameraEntity);
    View* view = engine->createView();
    Scene* scene = engine->createScene();

    view->setCamera(camera);
    // Determine the current size of the window in physical pixels.
    uint32_t w, h;
    SDL_GL_GetDrawableSize(window, (int*) &w, (int*) &h);
    camera->lookAt(float3(0, 50.5f, 0), float3(0, 0, 0), float3(1.f, 0, 0));
    camera->setProjection(45.0, double(w) / h, 0.1, 50, Camera::Fov::VERTICAL);
    view->setViewport({ 0, 0, w, h });
    view->setScene(scene);
    view->setPostProcessingEnabled(false);

    VertexBuffer* vertexBuffer =
            VertexBuffer::Builder()
                    .vertexCount(3)
                    .bufferCount(2)
                    .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
                    .attribute(VertexAttribute::TANGENTS, 1, VertexBuffer::AttributeType::SHORT4)
                    .normalized(VertexAttribute::TANGENTS)
                    .build(*engine);

    vertexBuffer->setBufferAt(*engine, 0,
            VertexBuffer::BufferDescriptor(vertices,
                    vertexBuffer->getVertexCount() * sizeof(vertices[0])));
    vertexBuffer->setBufferAt(*engine, 1,
            VertexBuffer::BufferDescriptor(normals,
                    vertexBuffer->getVertexCount() * sizeof(normals[0])));

    IndexBuffer* indexBuffer = IndexBuffer::Builder().indexCount(3).build(*engine);

    indexBuffer->setBuffer(*engine,
            IndexBuffer::BufferDescriptor(indices,
                    indexBuffer->getIndexCount() * sizeof(uint32_t)));

    filamat::MaterialBuilder::init();
    filamat::MaterialBuilder builder;
    builder.name("Material")
            .material("    void material(inout MaterialInputs material) {\n"
                      "        prepareMaterial(material);"
                      "        material.baseColor.rgb = materialParams.baseColor;"
                      "    }")
            .parameter("baseColor", filament::backend::UniformType::FLOAT3)
            .parameter("metallic", filament::backend::UniformType::FLOAT)
            .parameter("roughness", filament::backend::UniformType::FLOAT)
            .parameter("reflectance", filament::backend::UniformType::FLOAT)
            .optimization(filamat::MaterialBuilder::Optimization::NONE)
            .shading(filamat::MaterialBuilder::Shading::UNLIT)
            .targetApi(filamat::MaterialBuilder::TargetApi::ALL)
            .platform(filamat::MaterialBuilder::Platform::ALL);

    filamat::Package package = builder.build(engine->getJobSystem());

    Material* material =
            Material::Builder().package(package.getData(), package.getSize()).build(*engine);
    material->setDefaultParameter("baseColor", RgbType::LINEAR, float3{ 1, 0, 0 });
    material->setDefaultParameter("metallic", 0.0f);
    material->setDefaultParameter("roughness", 0.4f);
    material->setDefaultParameter("reflectance", 0.5f);

    MaterialInstance* materialInstance = material->createInstance();
    Entity renderable = EntityManager::get().create();

    // build a quad
    RenderableManager::Builder(1)
            .boundingBox({ { -1, -1, -1 }, { 1, 1, 1 } })
            .material(0, materialInstance)
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vertexBuffer, indexBuffer, 0,
                    3)
            .culling(false)
            .build(*engine, renderable);
    scene->addEntity(renderable);

    int i = 0;
    while (i++ < 1) {
        // beginFrame() returns false if we need to skip a frame
        if (renderer->beginFrame(swapChain)) {
            // for each View
            renderer->render(view);
            renderer->endFrame();
        }
    }
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            SDL_Delay(16);        
        }
    }
    
    engine->destroy(cameraEntity);
    return 0;
}
