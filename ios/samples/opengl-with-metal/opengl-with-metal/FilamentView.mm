/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include "FilamentView.h"

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/RenderableManager.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <utils/EntityManager.h>

#include <CoreVideo/CoreVideo.h>

#import "OpenGLRenderer.h"

// These defines are set in the "Preprocessor Macros" build setting for each scheme.
#if !FILAMENT_APP_USE_METAL && !FILAMENT_APP_USE_OPENGL
#error A valid FILAMENT_APP_USE_ backend define must be set.
#endif

using namespace filament;
using utils::Entity;
using utils::EntityManager;

struct App {
    VertexBuffer* vb;
    IndexBuffer* ib;
    Material* mat;
    Texture* tex;
    Entity renderable;
    CVPixelBufferRef img;
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

static constexpr uint16_t TRIANGLE_INDICES[3] = {0, 1, 2};

// This file is compiled via the matc tool. See the "Run Script" build phase.
static constexpr uint8_t SAMPLE_EXTERNAL_TEXTURE_PACKAGE[] = {
#include "SampleExternalTexture.inc"
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

    OpenGLRenderer* openglRenderer;
}

- (instancetype)initWithCoder:(NSCoder*)coder {
    if (self = [super initWithCoder:coder]) {
#if FILAMENT_APP_USE_OPENGL
        [self initializeGLLayer];
#elif FILAMENT_APP_USE_METAL
        [self initializeMetalLayer];
#endif
        openglRenderer = [OpenGLRenderer new];
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

- (void)dealloc {
    engine->destroy(renderer);
    engine->destroy(scene);
    engine->destroy(filaView);
    Entity c = camera->getEntity();
    engine->destroyCameraComponent(c);
    EntityManager::get().destroy(c);
    engine->destroy(swapChain);
    engine->destroy(&engine);
}

- (void)initializeMetalLayer {
#if METAL_AVAILABLE
    CAMetalLayer* metalLayer = (CAMetalLayer*)self.layer;
    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;

    CGRect nativeBounds = [UIScreen mainScreen].nativeBounds;
    metalLayer.drawableSize = nativeBounds.size;
#endif
}

- (void)initializeGLLayer {
    CAEAGLLayer* glLayer = (CAEAGLLayer*)self.layer;
    glLayer.opaque = YES;
}

- (void)initializeFilament {
#if FILAMENT_APP_USE_OPENGL
    engine = Engine::create(filament::Engine::Backend::OPENGL);
#elif FILAMENT_APP_USE_METAL
    engine = Engine::create(filament::Engine::Backend::METAL);
#endif
    swapChain = engine->createSwapChain((__bridge void*)self.layer);
    renderer = engine->createRenderer();
    scene = engine->createScene();
    Entity c = EntityManager::get().create();
    camera = engine->createCamera(c);
    renderer->setClearOptions({.clearColor = {0.1, 0.125, 0.25, 1.0}, .clear = true});

    filaView = engine->createView();
    filaView->setPostProcessingEnabled(false);

    app.vb = VertexBuffer::Builder()
                     .vertexCount(3)
                     .bufferCount(1)
                     .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2,
                             0, 12)
                     .attribute(
                             VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 8, 12)
                     .normalized(VertexAttribute::COLOR)
                     .build(*engine);
    app.vb->setBufferAt(*engine, 0, VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 36, nullptr));

    app.ib = IndexBuffer::Builder()
                     .indexCount(3)
                     .bufferType(IndexBuffer::IndexType::USHORT)
                     .build(*engine);
    app.ib->setBuffer(*engine, IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));

    app.mat = Material::Builder()
                      .package((void*)SAMPLE_EXTERNAL_TEXTURE_PACKAGE,
                              sizeof(SAMPLE_EXTERNAL_TEXTURE_PACKAGE))
                      .build(*engine);

    app.renderable = EntityManager::get().create();
    RenderableManager::Builder(1)
            .boundingBox({{-1, -1, -1}, {1, 1, 1}})
            .material(0, app.mat->getDefaultInstance())
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, app.vb, app.ib, 0, 3)
            .culling(false)
            .receiveShadows(false)
            .castShadows(false)
            .build(*engine, app.renderable);
    scene->addEntity(app.renderable);

    NSDictionary* cvBufferProperties = @{
        (__bridge NSString*)kCVPixelBufferOpenGLCompatibilityKey : @YES,
        (__bridge NSString*)kCVPixelBufferMetalCompatibilityKey : @YES,
    };
    CVReturn cvret = CVPixelBufferCreate(kCFAllocatorDefault, 1024, 1024, kCVPixelFormatType_32BGRA,
            (__bridge CFDictionaryRef)cvBufferProperties, &app.img);
    assert(cvret == kCVReturnSuccess);

    [openglRenderer setPixelBuffer:app.img];

    app.tex = Texture::Builder()
                      .width(1024)
                      .height(1024)
                      .sampler(Texture::Sampler::SAMPLER_EXTERNAL)
                      .usage(Texture::Usage::SAMPLEABLE)
                      .build(*engine);
    app.tex->setExternalImage(*engine, app.img);

    TextureSampler s{};
    app.mat->getDefaultInstance()->setParameter("texture", app.tex, s);

    filaView->setScene(scene);
    filaView->setCamera(camera);
    CGRect nativeBounds = [UIScreen mainScreen].nativeBounds;
    filaView->setViewport(Viewport(0, 0, nativeBounds.size.width, nativeBounds.size.height));
}

- (void)renderloop {
    [self update];

    const Color3 RED{1.0, 0.0, 0.0};
    const Color3 GREEN{0.0, 1.0, 0.0};
    const Color3 BLUE{0.0, 0.0, 1.0};
    const Color3 YELLOW{1.0, 1.0, 0.0};
    const Color3 CYAN{0.0, 1.0, 1.0};
    const Color3 PURPLE{1.0, 0.0, 1.0};
    const Color3 WHITE{1.0, 1.0, 1.0};
    const Color3 BLACK{0.0, 0.0, 0.0};

// WARNING: Fast color strobing!
// Sync mode will clear the OpenGL Scene and Filament Scene to the same color, cycling through a
// list of colors frame-by-frame. This can be used to ensure that OpenGL rendering stays in sync
// with Filament. With this mode enabled, the triangle should "disappear" and the entire screen
// should always be a continuous color.
#define TEST_SYNC 0
#if TEST_SYNC
    const Color3 colors[] = {RED, GREEN, BLUE, YELLOW, CYAN, PURPLE, WHITE, BLACK};

    static size_t frameId = 0;
    const Color3 filamentColor = colors[frameId++ % (sizeof(colors) / sizeof(Color3))];
    const Color3 openGLColor = filamentColor;
#else
    const Color3 filamentColor = BLACK;
    const Color3 openGLColor = GREEN;
#endif

    renderer->setClearOptions(
            {.clearColor = {filamentColor.r, filamentColor.g, filamentColor.b, 1.0},
                    .clear = true});

    if (self->renderer->beginFrame(self->swapChain)) {
        // Render OpenGL
        // TODO: more synchronization might be necessary.
        // Occasionally, when running in TEST_SYNC mode, the triangle is one color "ahead" of
        // Filament.
        [openglRenderer renderWithColor:openGLColor];

        // Render Filament.
        self->renderer->render(self->filaView);
        self->renderer->endFrame();

        // This line fixes the synchronization issues, but is highly inefficient:
        // self->engine->flushAndWait();
    }
}

- (void)update {
    constexpr float ZOOM = 1.5f;
    const uint32_t w = filaView->getViewport().width;
    const uint32_t h = filaView->getViewport().height;
    const float aspect = (float)w / h;
    camera->setProjection(
            Camera::Projection::ORTHO, -aspect * ZOOM, aspect * ZOOM, -ZOOM, ZOOM, 0, 1);
    auto& tcm = engine->getTransformManager();

    [self updateRotation];

    tcm.setTransform(tcm.getInstance(app.renderable),
            filament::math::mat4f::rotation(CACurrentMediaTime(), filament::math::float3{0, 0, 1}) *
                    filament::math::mat4f::rotation(
                            deviceRotation, filament::math::float3{0, 0, 1}));
}

- (void)didRotate:(NSNotification*)notification {
    UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
    desiredRotation = [self rotationForDeviceOrientation:orientation];
}

- (void)updateRotation {
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

+ (Class)layerClass {
#if FILAMENT_APP_USE_OPENGL
    return [CAEAGLLayer class];
#elif FILAMENT_APP_USE_METAL
    return [CAMetalLayer class];
#endif
}

- (float)rotationForDeviceOrientation:(UIDeviceOrientation)orientation {
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
