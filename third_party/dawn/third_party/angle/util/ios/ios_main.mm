//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ios_main.mm: Alternative entry point for iOS executables that initializes UIKit before calling
// the default entry point.

#import <UIKit/UIKit.h>

#include <stdio.h>

static int original_argc;
static char **original_argv;

int main(int argc, char **argv);

@interface AngleUtilAppDelegate : UIResponder <UIApplicationDelegate>

@property(nullable, nonatomic, strong) UIWindow *window;

@end

@implementation AngleUtilAppDelegate

@synthesize window;

- (void)runMain
{
    exit(main(original_argc, original_argv));
}

- (BOOL)application:(UIApplication *)application
    didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    self.window                    = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    self.window.rootViewController = [[UIViewController alloc] initWithNibName:nil bundle:nil];
    [self.window makeKeyAndVisible];
    // We need to return from this function before the app finishes launching, so call main in a
    // timer callback afterward.
    [NSTimer scheduledTimerWithTimeInterval:0
                                     target:self
                                   selector:@selector(runMain)
                                   userInfo:nil
                                    repeats:NO];
    return YES;
}

@end

extern "C" int ios_main(int argc, char **argv)
{
    original_argc = argc;
    original_argv = argv;
    return UIApplicationMain(argc, argv, nullptr, NSStringFromClass([AngleUtilAppDelegate class]));
}
