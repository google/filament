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

#import "FilamentArViewController.h"

#import "FilamentApp.h"

#import "MathHelpers.h"

#import <ARKit/ARKit.h>

#define METAL_AVAILABLE __has_include(<QuartzCore/CAMetalLayer.h>)

#if !METAL_AVAILABLE
#error The iOS simulator does not support Metal.
#endif

@interface FilamentArViewController () <ARSessionDelegate> {
    FilamentApp* app;
}

@property (nonatomic, strong) ARSession* session;

// The most recent anchor placed via a tap on the screen.
@property (nonatomic, strong) ARAnchor* anchor;

@end

@implementation FilamentArViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    CGRect nativeBounds = [[UIScreen mainScreen] nativeBounds];
    uint32_t nativeWidth = (uint32_t) nativeBounds.size.width;
    uint32_t nativeHeight = (uint32_t) nativeBounds.size.height;
#if FILAMENT_APP_USE_OPENGL
    // Flip width and height; OpenGL layers are oriented "sideways"
    const uint32_t tmp = nativeWidth;
    nativeWidth = nativeHeight;
    nativeHeight = tmp;
#endif
    app = new FilamentApp((__bridge void*) self.view.layer, nativeWidth, nativeHeight);

    self.session = [ARSession new];
    self.session.delegate = self;

    UITapGestureRecognizer* tapRecognizer =
            [[UITapGestureRecognizer alloc] initWithTarget:self
                                                    action:@selector(handleTap:)];

    tapRecognizer.numberOfTapsRequired = 1;
    [self.view addGestureRecognizer:tapRecognizer];
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];

    ARWorldTrackingConfiguration* configuration = [ARWorldTrackingConfiguration new];
    configuration.planeDetection = ARPlaneDetectionHorizontal;
    [self.session runWithConfiguration:configuration];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];

    [self.session pause];
}

- (void)dealloc
{
    delete app;
}

#pragma mark ARSessionDelegate

- (void)session:(ARSession *)session didUpdateFrame:(ARFrame *)frame
{
    // The height and width are flipped for the viewport because we're requesting transforms in the
    // UIInterfaceOrientationLandscapeRight orientation (landscape, home button on the right-hand
    // side).
    CGRect nativeBounds = [[UIScreen mainScreen] nativeBounds];
    CGSize viewport = CGSizeMake(nativeBounds.size.height, nativeBounds.size.width);

    // This transform gets applied to the UV coordinates of the full-screen triangle used to render
    // the camera feed. We want the inverse because we're applying the transform to the UV
    // coordinates, not the image itself.
    // (See camera_feed.mat and FullScreenTriangle.cpp)
    CGAffineTransform displayTransform =
            [frame displayTransformForOrientation:UIInterfaceOrientationLandscapeRight
                                     viewportSize:viewport];
    CGAffineTransform transformInv = CGAffineTransformInvert(displayTransform);
    mat3f textureTransform(transformInv.a, transformInv.b, 0,
                           transformInv.c, transformInv.d, 0,
                           transformInv.tx, transformInv.ty, 1);

    const auto& projection =
            [frame.camera projectionMatrixForOrientation:UIInterfaceOrientationLandscapeRight
                                            viewportSize:viewport
                                                   zNear: 0.01f
                                                    zFar:10.00f];

    // frame.camera.transform gives a camera transform matrix assuming a landscape-right orientation.
    // For simplicity, the app's orientation is locked to UIInterfaceOrientationLandscapeRight.
    app->render(FilamentApp::FilamentArFrame {
        .cameraImage = (void*) frame.capturedImage,
        .cameraTextureTransform = textureTransform,
        .projection = FILAMENT_MAT4_FROM_SIMD(projection),
        .view = FILAMENT_MAT4F_FROM_SIMD(frame.camera.transform)
    });
}

- (void)handleTap:(UITapGestureRecognizer*)sender
{
    if (self.anchor) {
        [self.session removeAnchor:self.anchor];
    }

    ARFrame* currentFrame = self.session.currentFrame;
    if (!currentFrame) {
        return;
    }

    // Create a transform 0.2 meters in front of the camera.
    mat4f viewTransform = FILAMENT_MAT4F_FROM_SIMD(currentFrame.camera.transform);
    mat4f objectTranslation = mat4f::translation(float3{0.f, 0.f, -.2f});
    mat4f objectTransform = viewTransform * objectTranslation;

    app->setObjectTransform(objectTransform);

    simd_float4x4 simd_transform = SIMD_FLOAT4X4_FROM_FILAMENT(objectTransform);
    self.anchor = [[ARAnchor alloc] initWithName:@"object" transform:simd_transform];
    [self.session addAnchor:self.anchor];
}

- (void)session:(ARSession *)session didUpdateAnchors:(NSArray<ARAnchor*>*)anchors
{
    for (ARAnchor* anchor : anchors) {
        if ([anchor isKindOfClass:[ARPlaneAnchor class]]) {
            ARPlaneAnchor* planeAnchor = (ARPlaneAnchor*) anchor;
            const auto& geometry = planeAnchor.geometry;
            app->updatePlaneGeometry(FilamentApp::FilamentArPlaneGeometry {
                .transform = FILAMENT_MAT4F_FROM_SIMD(planeAnchor.transform),
                // geometry.vertices is an array of simd_float3's, but they're padded to be the
                // same length as a float4.
                .vertices = (float4*) geometry.vertices,
                .indices = (uint16_t*) geometry.triangleIndices,
                .vertexCount = geometry.vertexCount,
                .indexCount = geometry.triangleCount * 3
            });
            continue;
        }

        filament::math::mat4f transform = FILAMENT_MAT4F_FROM_SIMD(anchor.transform);
        app->setObjectTransform(transform);
    }
}

@end
