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

#import <UIKit/UIKit.h>

#include <utils/Entity.h>

namespace filament {
class Engine;
class Scene;
class View;
class Renderer;
};

namespace filament::gltfio {
class Animator;
class FilamentAsset;
};

NS_ASSUME_NONNULL_BEGIN

/**
 * FILModelView is a UIView that renders glTF models with an orbit controller.
 *
 * FILModelView owns a Filament engine, renderer, swapchain, view, and scene. It allows clients to
 * access these objects via read-only properties. The view can display only one glTF scene at a
 * time, which can be scaled and translated into the viewing frustum by calling transformToUnitCube.
 * All ECS entities can be accessed and modified via the `asset` property.
 *
 * For GLB files, clients can call loadModelGlb and pass in an NSData* with the contents of the GLB
 * file. For glTF files, clients can call loadModelGltf and pass in an NSData* with the JSON
 * contents, as well as a callback for loading external resources.
 *
 * FILModelView reduces much of the boilerplate required for simple Filament applications, but
 * clients still have the responsibility of adding an IndirectLight and Skybox to the scene.
 * Additionally, clients should: call render and animator->applyAnimation from a CADisplayLink frame
 * callback.
 *
 * See ios/samples/gltf-viewer for a usage example.
 */
@interface FILModelView : UIView

@property(nonatomic, readonly) filament::Engine* engine;
@property(nonatomic, readonly) filament::Scene* scene;
@property(nonatomic, readonly) filament::View* view;
@property(nonatomic, readonly) filament::Renderer* renderer;

@property(nonatomic, readonly) filament::gltfio::FilamentAsset* _Nullable asset;
@property(nonatomic, readonly) filament::gltfio::Animator* _Nullable animator;

@property(nonatomic, readwrite) float cameraFocalLength;

/**
 * Loads a monolithic binary glTF and populates the Filament scene.
 */
- (void)loadModelGlb:(NSData*)buffer;

/**
 * Loads a JSON-style glTF file and populates the Filament scene.
 *
 * The given callback is triggered for each requested resource.
 */
typedef NSData* _Nonnull (^ResourceCallback)(NSString* _Nonnull);
- (void)loadModelGltf:(NSData*)buffer callback:(ResourceCallback)callback;

- (void)destroyModel;

/**
 * Issues a pick query at the given view coordinates.
 * The coordinates should be in UIKit's coordinate system, with the origin at
 * the top-left.
 * The callback is triggered with entity of the picked object.
 */
typedef void (^PickCallback)(utils::Entity);
- (void)issuePickQuery:(CGPoint)point callback:(PickCallback)callback;

- (NSString* _Nullable)getEntityName:(utils::Entity)entity;

/**
 * Sets up a root transform on the current model to make it fit into a unit cube.
 */
- (void)transformToUnitCube;

/**
 * Renders the model and updates the Filament camera.
 */
- (void)render;

@end

NS_ASSUME_NONNULL_END
