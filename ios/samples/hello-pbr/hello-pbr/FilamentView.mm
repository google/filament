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

#import "FilamentView.h"

#import "FilamentApp.h"

// These defines are set in the "Preprocessor Macros" build setting for each scheme.
#if !FILAMENT_APP_USE_METAL && \
    !FILAMENT_APP_USE_OPENGL
#error A valid FILAMENT_APP_USE_ backend define must be set.
#endif

@implementation FilamentView {
    FilamentApp* app;
    CADisplayLink* displayLink;
    CGPoint previousLocation;
    UIPanGestureRecognizer* panRecognizer;
}

- (instancetype)initWithCoder:(NSCoder*)coder
{
    if (self = [super initWithCoder:coder]) {
#if FILAMENT_APP_USE_OPENGL
        [self initializeGLLayer];
#elif FILAMENT_APP_USE_METAL
        [self initializeMetalLayer];
#endif
        app = new FilamentApp((__bridge void*) self.layer);
        self.contentScaleFactor = UIScreen.mainScreen.nativeScale;
        app->initialize();
        app->updateViewportAndCameraProjection(self.bounds.size.width, self.bounds.size.height,
                self.contentScaleFactor);
    }

    // Call renderloop 60 times a second.
    displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(renderloop)];
    displayLink.preferredFramesPerSecond = 60;
    [displayLink addToRunLoop:NSRunLoop.currentRunLoop forMode:NSDefaultRunLoopMode];

    // Set up a pan gesture recognizer, used to orbit the camera.
    panRecognizer = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(didPan:)];
    panRecognizer.minimumNumberOfTouches = 1;
    panRecognizer.maximumNumberOfTouches = 1;
    [self addGestureRecognizer:panRecognizer];

    return self;
}

- (void)dealloc
{
    delete app;
}

- (void)initializeMetalLayer
{
#if METAL_AVAILABLE
    CAMetalLayer* metalLayer = (CAMetalLayer*) self.layer;
    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    metalLayer.opaque = YES;

    CGRect nativeBounds = [UIScreen mainScreen].nativeBounds;
    metalLayer.drawableSize = nativeBounds.size;
#endif
}

// OpenGL ES was deprecated in iOS 12. Ignore deprecation warnings for CAEAGLLayer.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

- (void)initializeGLLayer
{
    CAEAGLLayer* glLayer = (CAEAGLLayer*) self.layer;
    glLayer.opaque = YES;
}

+ (Class) layerClass
{
#if FILAMENT_APP_USE_OPENGL
    return [CAEAGLLayer class];
#elif FILAMENT_APP_USE_METAL
    return [CAMetalLayer class];
#endif
}

#pragma GCC diagnostic pop

- (void)renderloop
{
    app->render();
}

- (void)didPan:(UIPanGestureRecognizer*)sender
{
    CGPoint location = [sender locationInView:self];
    if (sender.state == UIGestureRecognizerStateBegan) {
        previousLocation = location;
        return;
    }
    CGPoint delta = CGPointMake(location.x - previousLocation.x, location.y - previousLocation.y);
    previousLocation = location;
    app->pan(delta.x, delta.y);
}

- (void)updateViewportAndCameraProjection {
    if (!app) {
        return;
    }

#if FILAMENT_APP_USE_METAL
    CAMetalLayer* metalLayer = (CAMetalLayer*)self.layer;
    const uint32_t width = self.bounds.size.width * self.contentScaleFactor;
    const uint32_t height = self.bounds.size.height * self.contentScaleFactor;
    metalLayer.drawableSize = CGSizeMake(width, height);
#endif

    app->updateViewportAndCameraProjection(self.bounds.size.width, self.bounds.size.height,
        self.contentScaleFactor);
}

#pragma mark UIView methods

- (void)layoutSubviews {
    [super layoutSubviews];
    [self updateViewportAndCameraProjection];
}

- (void)setContentScaleFactor:(CGFloat)contentScaleFactor {
    [super setContentScaleFactor:contentScaleFactor];
    [self updateViewportAndCameraProjection];
}

@end
