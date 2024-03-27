/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include <utils/Panic.h>

#include <Foundation/Foundation.h>

namespace utils::details {

void throwDarwinException(const Panic& exception) {
    // Turn this exception into an NSException and raise it.
    // This is allowed even if we're built without Objective-C exceptions (-fno-objc-exceptions).
    [[NSException exceptionWithName:@(exception.getType())
                             reason:@(exception.getReason())
                           userInfo:@{
                               @"file" : @(exception.getFile()),
                               @"line" : @(exception.getLine()),
                               @"function" : @(exception.getFunction())
                           }] raise];
}

}
