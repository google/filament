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

#import "FilamentView.h"

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/SwapChain.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <utils/EntityManager.h>

// These defines are set in the "Preprocessor Macros" build setting for each scheme.
#if !FILAMENT_APP_USE_METAL && \
    !FILAMENT_APP_USE_OPENGL
#error A valid FILAMENT_APP_USE_ backend define must be set.
#endif

using namespace filament;
using utils::Entity;
using utils::EntityManager;

struct App {
    VertexBuffer* vb;
    IndexBuffer* ib;
    Material* mat;
    Entity renderable;
};

struct Vertex {
    filament::math::float2 position;
    uint32_t color;
};

static const Vertex TRIANGLE_VERTICES[3] = {
    {{1, 0}, 0xffff0000u},
    {{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ff00u},
    {{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff0000ffu},
};

static constexpr uint16_t TRIANGLE_INDICES[3] = { 0, 1, 2 };

// This file is compiled via the matc tool. See the "Run Script" build phase.
static constexpr uint8_t BAKED_COLOR_PACKAGE[] = {
#include "bakedColor.inc"
};

@implementation FilamentView {
    Engine* engine;
    Renderer* renderer;
    Scene* scene;
    View* filaView;
    Camera* camera;
    SwapChain* swapChain;
    App app;
    CADisplayLink* displayLink;

    // The amount of rotation to apply to the camera to offset the device's rotation (in radians)
    float deviceRotation;
    float desiredRotation;
}

- (instancetype)initWithCoder:(NSCoder*)coder
{
    if (self = [super initWithCoder:coder]) {
#if FILAMENT_APP_USE_OPENGL
        [self initializeGLLayer];
#elif FILAMENT_APP_USE_METAL
        [self initializeMetalLayer];
#endif
        self.opaque = NO;
        [self initializeFilament];
        self.contentScaleFactor = UIScreen.mainScreen.nativeScale;
    }

    // Call renderloop 60 times a second.
    displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(renderloop)];
    displayLink.preferredFramesPerSecond = 60;
    [displayLink addToRunLoop:NSRunLoop.currentRunLoop forMode:NSDefaultRunLoopMode];

    // Call didRotate when the device orientation changes.
    [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(didRotate:)
                                                 name:UIDeviceOrientationDidChangeNotification
                                               object:nil];

    return self;
}

- (void)dealloc
{
    engine->destroy(renderer);
    engine->destroy(scene);
    engine->destroy(filaView);
    Entity c = camera->getEntity();
    engine->destroyCameraComponent(c);
    EntityManager::get().destroy(c);
    engine->destroy(swapChain);
    engine->destroy(&engine);
}

- (void)initializeMetalLayer
{
#if METAL_AVAILABLE
    CAMetalLayer* metalLayer = (CAMetalLayer*) self.layer;
    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;

    CGRect nativeBounds = [UIScreen mainScreen].nativeBounds;
    metalLayer.drawableSize = nativeBounds.size;

    // This is necessary so iOS composites the Filament view on top of other UIViews.
    metalLayer.opaque = NO;
#endif
}

- (void)initializeGLLayer
{
    CAEAGLLayer* glLayer = (CAEAGLLayer*) self.layer;

    // This is necessary so iOS composites the Filament view on top of other UIViews.
    glLayer.opaque = NO;
}

- (void)initializeFilament
{
#if FILAMENT_APP_USE_OPENGL
    engine = Engine::create(filament::Engine::Backend::OPENGL);
#elif FILAMENT_APP_USE_METAL
    engine = Engine::create(filament::Engine::Backend::METAL);
#endif
    // The SwapChain::CONFIG_TRANSPARENT flag informs Filament that we will be rendering transparent
    // content.
    swapChain = engine->createSwapChain((__bridge void*) self.layer, SwapChain::CONFIG_TRANSPARENT);

    renderer = engine->createRenderer();
    scene = engine->createScene();
    Entity c = EntityManager::get().create();
    camera = engine->createCamera(c);

    filaView = engine->createView();

    // Set a transparent clear color.
    renderer->setClearOptions({.clear = true});

    filaView->setPostProcessingEnabled(false);

    app.vb = VertexBuffer::Builder()
        .vertexCount(3)
        .bufferCount(1)
        .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
        .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
        .normalized(VertexAttribute::COLOR)
        .build(*engine);
    app.vb->setBufferAt(*engine, 0,
                        VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 36, nullptr));

    app.ib = IndexBuffer::Builder()
        .indexCount(3)
        .bufferType(IndexBuffer::IndexType::USHORT)
        .build(*engine);
    app.ib->setBuffer(*engine,
                      IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));

    app.mat = Material::Builder()
        .package((void*) BAKED_COLOR_PACKAGE, sizeof(BAKED_COLOR_PACKAGE))
        .build(*engine);

    app.renderable = EntityManager::get().create();
    RenderableManager::Builder(1)
        .boundingBox({{ -1, -1, -1 }, { 1, 1, 1 }})
        .material(0, app.mat->getDefaultInstance())
        .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib, 0, 3)
        .culling(false)
        .receiveShadows(false)
        .castShadows(false)
        .build(*engine, app.renderable);
    scene->addEntity(app.renderable);

    filaView->setScene(scene);
    filaView->setCamera(camera);
    CGRect nativeBounds = [UIScreen mainScreen].nativeBounds;
    filaView->setViewport(Viewport(0, 0, nativeBounds.size.width, nativeBounds.size.height));
}

- (void)renderloop
{
    [self update];
    if (self->renderer->beginFrame(self->swapChain)) {
        self->renderer->render(self->filaView);
        self->renderer->endFrame();
    }
}

- (void)update
{
    constexpr float ZOOM = 1.5f;
    const uint32_t w = filaView->getViewport().width;
    const uint32_t h = filaView->getViewport().height;
    const float aspect = (float) w / h;
    camera->setProjection(Camera::Projection::ORTHO,
                          -aspect * ZOOM, aspect * ZOOM,
                          -ZOOM, ZOOM, 0, 1);
    auto& tcm = engine->getTransformManager();

    [self updateRotation];

    tcm.setTransform(tcm.getInstance(app.renderable),
                     filament::math::mat4f::rotation(CACurrentMediaTime(), filament::math::float3{0, 0, 1}) *
                     filament::math::mat4f::rotation(deviceRotation, filament::math::float3{0, 0, 1}));
}

- (void)didRotate:(NSNotification*)notification
{
    UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
    desiredRotation = [self rotationForDeviceOrientation:orientation];
}

- (void)updateRotation
{
    static const float ROTATION_SPEED = 0.1;
    float diff = abs(desiredRotation - deviceRotation);
    if (diff > FLT_EPSILON) {
        if (desiredRotation > deviceRotation) {
            deviceRotation += fmin(ROTATION_SPEED, diff);
        }
        if (desiredRotation < deviceRotation) {
            deviceRotation -= fmin(ROTATION_SPEED, diff);
        }
    }
}

+ (Class) layerClass
{
#if FILAMENT_APP_USE_OPENGL
    return [CAEAGLLayer class];
#elif FILAMENT_APP_USE_METAL
    return [CAMetalLayer class];
#endif
}

- (float)rotationForDeviceOrientation:(UIDeviceOrientation)orientation
{
    switch (orientation) {
        default:
        case UIDeviceOrientationPortrait:
            return 0.0f;

        case UIDeviceOrientationLandscapeRight:
            return M_PI_2;

        case UIDeviceOrientationLandscapeLeft:
            return -M_PI_2;

        case UIDeviceOrientationPortraitUpsideDown:
            return M_PI;
    }
}

@end
