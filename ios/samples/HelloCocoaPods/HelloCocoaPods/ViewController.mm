/*
* Copyright 2020 The Android Open Source Project
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

#import "ViewController.h"

#import <MetalKit/MTKView.h>

// TODO: Filament public headers in the 1.8.1 release use DEBUG as a C++ identifier, but Xcode
// defines DEBUG=1. So, we simply undefine it here. This will be fixed in the next release.
#undef DEBUG

// These are all C++ headers, so make sure the type of this file is Objective-C++ source.
#include <filament/Engine.h>
#include <filament/SwapChain.h>
#include <filament/Renderer.h>
#include <filament/View.h>
#include <filament/Camera.h>
#include <filament/Scene.h>
#include <filament/Viewport.h>
#include <filament/VertexBuffer.h>
#include <filament/IndexBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/TransformManager.h>

#include <filamat/MaterialBuilder.h>

#include <utils/Entity.h>
#include <utils/EntityManager.h>

using namespace filament;
using namespace utils;

struct Vertex {
    math::float2 position;
    math::float3 color;
};

@interface ViewController () <MTKViewDelegate>

@end

@implementation ViewController {
    Engine* _engine;
    SwapChain* _swapChain;
    Renderer* _renderer;
    View* _view;
    Scene* _scene;
    Camera* _camera;
    Entity _cameraEntity;

    VertexBuffer* _vertexBuffer;
    IndexBuffer* _indexBuffer;
    Entity _triangle;

    Material* _material;
    MaterialInstance* _materialInstance;
}

- (void)viewDidLoad {
    [super viewDidLoad];

    // Create an Engine, the main entry point into Filament.
    _engine = Engine::create(Engine::Backend::METAL);

    // Create a SwapChain from a CAMetalLayer.
    // This ViewController's view is a MTKView, which is backed by a CAMetalLayer.
    // We must use __bridge here; Filament does not take ownership of the layer.
    MTKView* mtkView = (MTKView*) self.view;
    mtkView.delegate = self;
    _swapChain = _engine->createSwapChain((__bridge void*) mtkView.layer);

    _renderer = _engine->createRenderer();
    _view = _engine->createView();
    _scene = _engine->createScene();

    // Filament uses an entity-component system.
    // We create a camera entity and attach a camera component.
    _cameraEntity = EntityManager::get().create();
    _camera = _engine->createCamera(_cameraEntity);

    // Set the clear color and wire up the Scene and Camera to the View.
    _renderer->setClearOptions({
        .clearColor = {0.25f, 0.5f, 1.0f, 1.0f},
        .clear = true
    });
    _view->setScene(_scene);
    _view->setCamera(_camera);

    // Give our View a starting size based on the drawable size.
    [self resize:mtkView.drawableSize];

    // Create a vertex array and indices for a triangle.
    static const Vertex TRIANGLE_VERTICES[3] = {
        { { 0.867, -0.50}, {1.0, 0.0, 0.0} },
        { { 0.000,  1.00}, {0.0, 1.0, 0.0} },
        { {-0.867, -0.50}, {0.0, 0.0, 1.0} },
    };
    static const uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 };

    // We use BufferDescriptors to communicate data to Filament.
    VertexBuffer::BufferDescriptor vertices(TRIANGLE_VERTICES, sizeof(Vertex) * 3, nullptr);
    IndexBuffer::BufferDescriptor indices(TRIANGLE_INDICES, sizeof(uint16_t) * 3, nullptr);

    using Type = VertexBuffer::AttributeType;

    // Create vertex and index buffers which define the geometry of the triangle.
    const uint8_t stride = sizeof(Vertex);
    _vertexBuffer = VertexBuffer::Builder()
        .vertexCount(3)
        .bufferCount(1)
        .attribute(VertexAttribute::POSITION, 0, Type::FLOAT2, offsetof(Vertex, position), stride)
        .attribute(VertexAttribute::COLOR,    0, Type::FLOAT3, offsetof(Vertex, color),    stride)
        .build(*_engine);

    _indexBuffer = IndexBuffer::Builder()
        .indexCount(3)
        .bufferType(IndexBuffer::IndexType::USHORT)
        .build(*_engine);

    _vertexBuffer->setBufferAt(*_engine, 0, std::move(vertices));
    _indexBuffer->setBuffer(*_engine, std::move(indices));

    // init must be called before we can build any materials.
    filamat::MaterialBuilder::init();

    // Compile a custom material to use on the triangle.
    filamat::Package pkg = filamat::MaterialBuilder()
        // The material name, only used for debugging purposes.
        .name("Triangle material")
        // Use the unlit shading mode, because we don't have any lights in our scene.
        .shading(filamat::MaterialBuilder::Shading::UNLIT)
        .require(VertexAttribute::COLOR)
        // Custom GLSL fragment code.
        .material("void material (inout MaterialInputs material) {"
                  "  prepareMaterial(material);"
                  "  material.baseColor = getColor();"
                  "}")
        // Compile for Metal on mobile platforms.
        .targetApi(filamat::MaterialBuilder::TargetApi::METAL)
        .platform(filamat::MaterialBuilder::Platform::MOBILE)
        .build(_engine->getJobSystem());
    assert(pkg.isValid());

    // We're done building materials.
    filamat::MaterialBuilder::shutdown();

    // Create a Filament material from the Package.
    _material = Material::Builder()
        .package(pkg.getData(), pkg.getSize())
        .build(*_engine);
    _materialInstance = _material->getDefaultInstance();

    // Create a triangle entity that can be added to our scene.
    _triangle = utils::EntityManager::get().create();

    // Add a Renderable component to the triangle using our geometry and material.
    using Primitive = RenderableManager::PrimitiveType;
    RenderableManager::Builder(1)
        .geometry(0, Primitive::TRIANGLES, _vertexBuffer, _indexBuffer, 0, 3)
        // Use the MaterialInstance we just created.
        .material(0, _materialInstance)
        .culling(false)
        .receiveShadows(false)
        .castShadows(false)
        .build(*_engine, _triangle);

    // Add a Transform component to the triangle, so we can animate it.
    _engine->getTransformManager().create(_triangle);

    // Add the triangle to the scene.
    _scene->addEntity(_triangle);
}

- (void)dealloc {
    _engine->destroy(_materialInstance);
    _engine->destroy(_material);
    _engine->destroy(_triangle);
    EntityManager::get().destroy(_triangle);
    _engine->destroy(_indexBuffer);
    _engine->destroy(_vertexBuffer);
    _engine->destroyCameraComponent(_cameraEntity);
    EntityManager::get().destroy(_cameraEntity);
    _engine->destroy(_scene);
    _engine->destroy(_view);
    _engine->destroy(_renderer);
    _engine->destroy(_swapChain);
    _engine->destroy(&_engine);
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size {
    [self resize:size];
}

- (void)drawInMTKView:(nonnull MTKView*)view {
    [self update];
    if (_renderer->beginFrame(_swapChain)) {
        _renderer->render(_view);
        _renderer->endFrame();
    }
}

- (void)update {
    auto& tm = _engine->getTransformManager();
    auto i = tm.getInstance(_triangle);
    const auto time = CACurrentMediaTime();
    tm.setTransform(i, math::mat4f::rotation(time, math::float3 {0.0, 0.0, 1.0}));
}

- (void)resize:(CGSize)size {
    _view->setViewport({0, 0, (uint32_t) size.width, (uint32_t) size.height});
    const double aspect = size.width / size.height;

    const double left   = -2.0 * aspect;
    const double right  =  2.0 * aspect;
    const double bottom = -2.0;
    const double top    =  2.0;
    const double near   =  0.0;
    const double far    =  1.0;
    _camera->setProjection(Camera::Projection::ORTHO, left, right, bottom, top, near, far);
}

@end
