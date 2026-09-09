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

#import "ViewController.h"

#include <backend_test/PlatformRunner.h>

#import <QuartzCore/CAMetalLayer.h>

static test::NativeView nativeView;

namespace test {

NativeView getNativeView() {
    return nativeView;
}

} // namespace test

@interface ViewController ()

@end

@implementation ViewController

- (BOOL)shouldAutorotate
{
    return NO;
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];

    nativeView.ptr = (__bridge void*) self.view.layer;
    // Read the drawable size back off the layer rather than using the screen size: for Metal,
    // FilamentView pins it to the same fixed size the desktop runners use, so that test
    // expectations don't depend on which device the tests run on.
#if FILAMENT_APP_USE_METAL
    CGSize const drawableSize = ((CAMetalLayer*) self.view.layer).drawableSize;
#elif FILAMENT_APP_USE_OPENGL
    CALayer* const glLayer = self.view.layer;
    CGSize const drawableSize = CGSizeMake(glLayer.bounds.size.width * glLayer.contentsScale,
            glLayer.bounds.size.height * glLayer.contentsScale);
#endif
    nativeView.width = static_cast<size_t>(drawableSize.width);
    nativeView.height = static_cast<size_t>(drawableSize.height);

    /*
    OpenGL doesn't have programatic debugger capture on iOS. Instead, the following can be used to initiate
    a frame capture before tests are run.

    NSLog(@"Pausing to allow starting frame capture...");
    sleep(5);
    */

    int result = test::runTests();
    NSLog(@"Tests ran with result: %d", result);
}

@end
