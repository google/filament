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

#import "FilamentViewController.h"

#import "App.h"

@interface FilamentViewController () {
    App* app;
    CADisplayLink* displayLink;
    CGPoint previousLocation;
    UIPanGestureRecognizer* panRecognizer;
}

@end

@implementation FilamentViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    CGRect nativeBounds = [[UIScreen mainScreen] nativeBounds];
    NSString* resourcePath = [NSBundle mainBundle].bundlePath;
    app = new App((__bridge void*) self.view.layer, nativeBounds.size.width,
            nativeBounds.size.height, utils::Path([resourcePath cStringUsingEncoding:NSUTF8StringEncoding]));

    // Call render 60 times a second.
    displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(render)];
    displayLink.preferredFramesPerSecond = 60;
    [displayLink addToRunLoop:NSRunLoop.currentRunLoop forMode:NSDefaultRunLoopMode];

    // Set up a pan gesture recognizer, used to orbit the camera.
    panRecognizer = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(didPan:)];
    panRecognizer.minimumNumberOfTouches = 1;
    panRecognizer.maximumNumberOfTouches = 1;
    [self.view addGestureRecognizer:panRecognizer];
}

- (void)render
{
    app->render();
}

- (void)didPan:(UIPanGestureRecognizer*)sender
{
    CGPoint location = [sender locationInView:self.view];
    if (sender.state == UIGestureRecognizerStateBegan) {
        previousLocation = location;
        return;
    }
    CGPoint delta = CGPointMake(location.x - previousLocation.x, location.y - previousLocation.y);
    previousLocation = location;
    app->pan(delta.x, delta.y);
}

- (void)dealloc
{
    delete app;
}

@end
