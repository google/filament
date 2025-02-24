/*
 * DemoViewController.m
 *
 * Copyright (c) 2014-2018 The Brenwill Workshop Ltd. (http://www.brenwill.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import "DemoViewController.h"
#import <QuartzCore/CAMetalLayer.h>

#include "cube.cpp"

#pragma mark -
#pragma mark DemoViewController

@implementation DemoViewController {
    CVDisplayLinkRef _displayLink;
    struct Demo demo;
    NSTimer* _timer;
}

- (void)dealloc {
    [self quit];
    [super dealloc];
}

- (void)quit {
    CVDisplayLinkRelease(_displayLink);
    demo.cleanup();
}

/** Since this is a single-view app, initialize Vulkan during view loading. */
- (void)viewDidLoad {
    [super viewDidLoad];

    self.view.wantsLayer = YES;  // Back the view with a layer created by the makeBackingLayer method.

    // Convert incoming args to "C" argc/argv strings
    NSArray *args = [[NSProcessInfo processInfo] arguments];
    const char** argv = (const char**) alloca(sizeof(char*) * args.count);
    for(unsigned int i = 0; i < args.count; i++) {
        NSString *s = args[i];
        argv[i] = s.UTF8String;
    }

    demo_main(demo, self.view.layer, args.count, argv);

    // Monitor the rendering loop for a quit condition
    _timer = [NSTimer scheduledTimerWithTimeInterval: 0.2
                                              target: self
                                            selector: @selector(onTick:)
                                            userInfo: self
                                             repeats: YES];

    // Start the rendering loop
    CVDisplayLinkCreateWithActiveCGDisplays(&_displayLink);
    CVDisplayLinkSetOutputCallback(_displayLink, &DisplayLinkCallback, &demo);
    CVDisplayLinkStart(_displayLink);
}

// Close the window if the demo is in a Quit state
-(void)onTick:(NSTimer*)timer {
    if (demo.quit) {
        [[[self view] window] close];
    }
}

#pragma mark Display loop callback function

/** Rendering loop callback function for use with a CVDisplayLink. */
static CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now,
                                    const CVTimeStamp* outputTime,
                                    CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* target) {
    struct Demo* demo = (struct Demo*)target;
    demo->run();
    if (demo->quit) {
        CVDisplayLinkStop(displayLink);
    }
    return kCVReturnSuccess;
}

@end

#pragma mark -
#pragma mark DemoView

@implementation DemoView

/** Indicates that the view wants to draw using the backing layer instead of using drawRect:.  */
- (BOOL)wantsUpdateLayer {
    return YES;
}

/** Returns a Metal-compatible layer. */
+ (Class)layerClass {
    return [CAMetalLayer class];
}

/** If the wantsLayer property is set to YES, this method will be invoked to return a layer instance. */
- (CALayer*)makeBackingLayer {
    CALayer* layer = [self.class.layerClass layer];
    CGSize viewScale = [self convertSizeToBacking:CGSizeMake(1.0, 1.0)];
    layer.contentsScale = MIN(viewScale.width, viewScale.height);
    return layer;
}

@end
