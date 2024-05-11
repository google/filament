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
    CGRect nativeBounds = [UIScreen mainScreen].nativeBounds;
    nativeView.height = nativeBounds.size.height;
    nativeView.width = nativeBounds.size.width;

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
