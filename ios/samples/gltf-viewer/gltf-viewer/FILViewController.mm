/*
 * Copyright (C) 2021 The Android Open Source Project
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

#import "FILViewController.h"

#import "FILModelView.h"

#include <filament/Scene.h>
#include <filament/Skybox.h>

#include <utils/EntityManager.h>

#include <gltfio/Animator.h>

#include <image/KtxUtility.h>

#include <viewer/AutomationEngine.h>
#include <viewer/RemoteServer.h>

using namespace filament;
using namespace utils;

@interface FILViewController ()

- (void)startDisplayLink;
- (void)stopDisplayLink;

- (void)createRenderables;
- (void)createLights;

@end

@implementation FILViewController {
    CADisplayLink* _displayLink;
    viewer::RemoteServer* _server;
    viewer::AutomationEngine* _automation;

    Texture* _skyboxTexture;
    Skybox* _skybox;
    Texture* _iblTexture;
    IndirectLight* _indirectLight;
    Entity _sun;
}

#pragma mark UIViewController methods

- (void)viewDidLoad {
    [super viewDidLoad];

    self.title = @"https://google.github.io/filament/remote";

    // Arguments:
    // --model <path>
    //     path to glb or gltf file to load from documents directory
    NSString* modelPath = nil;

    NSArray* arguments = [[NSProcessInfo processInfo] arguments];
    for (NSUInteger i = 0; i < arguments.count; i++) {
        NSString* argument = arguments[i];
        NSString* nextArgument = (i + 1) < arguments.count ? arguments[i + 1] : nil;
        if ([argument isEqualToString:@"--model"]) {
            if (!nextArgument) {
                NSLog(@"Warning: --model option requires path argument. None provided.");
            }
            modelPath = nextArgument;
        }
    }

    if (modelPath) {
        [self createRenderablesFromPath:modelPath];
    } else {
        [self createDefaultRenderables];
    }
    [self createLights];

    _server = new viewer::RemoteServer();
    _automation = viewer::AutomationEngine::createDefault();
}

- (void)viewWillAppear:(BOOL)animated {
    [self startDisplayLink];
}

- (void)viewWillDisappear:(BOOL)animated {
    [self stopDisplayLink];
}

- (void)startDisplayLink {
    [self stopDisplayLink];

    // Call our render method 60 times a second.
    _displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(render)];
    _displayLink.preferredFramesPerSecond = 60;
    [_displayLink addToRunLoop:NSRunLoop.currentRunLoop forMode:NSDefaultRunLoopMode];
}

- (void)stopDisplayLink {
    [_displayLink invalidate];
    _displayLink = nil;
}

#pragma mark Private

- (void)createRenderablesFromPath:(NSString*)model {
    // Retrieve the full path to the model in the documents directory.
    NSString* documentPath = [NSSearchPathForDirectoriesInDomains(
            NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
    NSString* path = [documentPath stringByAppendingPathComponent:model];

    if (![[NSFileManager defaultManager] fileExistsAtPath:path]) {
        NSLog(@"Error: no file exists at %@", path);
        return;
    }

    NSData* buffer = [NSData dataWithContentsOfFile:path];
    if ([model hasSuffix:@".glb"]) {
        [self.modelView loadModelGlb:buffer];
    } else if ([model hasSuffix:@".gltf"]) {
        NSString* parentDirectory = [path stringByDeletingLastPathComponent];
        [self.modelView loadModelGltf:buffer
                             callback:^NSData*(NSString* uri) {
                                 NSString* p = [parentDirectory stringByAppendingPathComponent:uri];
                                 return [NSData dataWithContentsOfFile:p];
                             }];
    } else {
        NSLog(@"Error: file %@ must have either a .glb or .gltf extension.", path);
        return;
    }

    self.title = model;
    [self.modelView transformToUnitCube];
}

- (void)createDefaultRenderables {
    NSString* path = [[NSBundle mainBundle] pathForResource:@"scene"
                                                     ofType:@"gltf"
                                                inDirectory:@"BusterDrone"];
    assert(path.length > 0);
    NSData* buffer = [NSData dataWithContentsOfFile:path];
    [self.modelView loadModelGltf:buffer
                         callback:^NSData*(NSString* uri) {
                             NSString* p = [[NSBundle mainBundle] pathForResource:uri
                                                                           ofType:@""
                                                                      inDirectory:@"BusterDrone"];
                             return [NSData dataWithContentsOfFile:p];
                         }];
    [self.modelView transformToUnitCube];
}

- (void)createLights {
    // Load Skybox.
    NSString* skyboxPath = [[NSBundle mainBundle] pathForResource:@"default_env_skybox"
                                                           ofType:@"ktx"];
    assert(skyboxPath.length > 0);
    NSData* skyboxBuffer = [NSData dataWithContentsOfFile:skyboxPath];

    image::KtxBundle* skyboxBundle =
            new image::KtxBundle(static_cast<const uint8_t*>(skyboxBuffer.bytes),
                    static_cast<uint32_t>(skyboxBuffer.length));
    _skyboxTexture = image::ktx::createTexture(self.modelView.engine, skyboxBundle, false);
    _skybox = filament::Skybox::Builder().environment(_skyboxTexture).build(*self.modelView.engine);
    self.modelView.scene->setSkybox(_skybox);

    // Load IBL.
    NSString* iblPath = [[NSBundle mainBundle] pathForResource:@"default_env_ibl" ofType:@"ktx"];
    assert(iblPath.length > 0);
    NSData* iblBuffer = [NSData dataWithContentsOfFile:iblPath];

    image::KtxBundle* iblBundle = new image::KtxBundle(
            static_cast<const uint8_t*>(iblBuffer.bytes), static_cast<uint32_t>(iblBuffer.length));
    math::float3 harmonics[9];
    iblBundle->getSphericalHarmonics(harmonics);
    _iblTexture = image::ktx::createTexture(self.modelView.engine, iblBundle, false);
    _indirectLight = IndirectLight::Builder()
                             .reflections(_iblTexture)
                             .irradiance(3, harmonics)
                             .intensity(30000.0f)
                             .build(*self.modelView.engine);
    self.modelView.scene->setIndirectLight(_indirectLight);

    // Always add a direct light source since it is required for shadowing.
    _sun = EntityManager::get().create();
    LightManager::Builder(LightManager::Type::DIRECTIONAL)
            .color(Color::cct(6500.0f))
            .intensity(100000.0f)
            .direction(math::float3(0.0f, -1.0f, 0.0f))
            .castShadows(true)
            .build(*self.modelView.engine, _sun);
    self.modelView.scene->addEntity(_sun);
}

- (void)loadSettings:(viewer::ReceivedMessage const*)message {
    viewer::AutomationEngine::ViewerContent content = {
        .view = self.modelView.view,
        .renderer = self.modelView.renderer,
        .materials = nullptr,
        .materialCount = 0u,
        .lightManager = &self.modelView.engine->getLightManager(),
        .scene = self.modelView.scene,
        .indirectLight = _indirectLight,
        .sunlight = _sun,
    };
    _automation->applySettings(message->buffer, message->bufferByteCount, content);
    ColorGrading* const colorGrading = _automation->getColorGrading(self.modelView.engine);
    self.modelView.view->setColorGrading(colorGrading);
    self.modelView.cameraFocalLength = _automation->getViewerOptions().cameraFocalLength;
}

- (void)loadGlb:(viewer::ReceivedMessage const*)message {
    [self.modelView destroyModel];
    NSData* buffer = [NSData dataWithBytes:message->buffer length:message->bufferByteCount];
    [self.modelView loadModelGlb:buffer];
    [self.modelView transformToUnitCube];
}

- (void)render {
    auto* animator = self.modelView.animator;
    if (animator) {
        if (animator->getAnimationCount() > 0) {
            animator->applyAnimation(0, CACurrentMediaTime());
        }
        animator->updateBoneMatrices();
    }

    // Check if a new message has been fully received from the client.
    viewer::ReceivedMessage const* message = _server->acquireReceivedMessage();
    if (message && message->label) {
        NSString* label = [NSString stringWithCString:message->label encoding:NSUTF8StringEncoding];
        if ([label hasSuffix:@".json"]) {
            [self loadSettings:message];
        } else if ([label hasSuffix:@".glb"]) {
            self.title = label;
            [self loadGlb:message];
        }

        _server->releaseReceivedMessage(message);
    }

    [self.modelView render];
}

- (void)dealloc {
    delete _server;
    delete _automation;
    self.modelView.engine->destroy(_indirectLight);
    self.modelView.engine->destroy(_iblTexture);
    self.modelView.engine->destroy(_skybox);
    self.modelView.engine->destroy(_skyboxTexture);
    self.modelView.scene->remove(_sun);
    self.modelView.engine->destroy(_sun);
}

@end
