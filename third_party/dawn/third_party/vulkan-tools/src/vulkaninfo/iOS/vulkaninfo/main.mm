/*
 * Copyright (c) 2020 The Khronos Group Inc.
 * Copyright (c) 2020 Valve Corporation
 * Copyright (c) 2020 LunarG, Inc.
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
 *
 * Author: Richard Wright <richard@lunarg.com>
 */

#import <UIKit/UIKit.h>
#import "AppDelegate.h"

int vulkanInfoMain(int argc, char **argv);

int main(int argc, char *argv[]) {
    // First thing we are going to "run" vulkaninfo to create the file output
    // So that we don't have to touch the C++ desktop code, we'll set the current working directory
    // to the shared documents folder where we are allowed to create and store files
    NSArray *docPath = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *myPath = [docPath objectAtIndex:0];
    chdir([myPath UTF8String]);

    // HTML Version
    const char *htmlArgs[2] = {"vulkaninfo", "--html"};
    vulkanInfoMain(2, (char **)htmlArgs);

    // JSON output
    const char *jsonArgs[2] = {"vulkaninfo", "--json"};
    vulkanInfoMain(2, (char **)jsonArgs);

    const char *portArgs[2] = {"vulkaninfo", "--portability"};
    vulkanInfoMain(2, (char **)portArgs);

    NSString *appDelegateClassName;
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        appDelegateClassName = NSStringFromClass([AppDelegate class]);
    }
    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}
