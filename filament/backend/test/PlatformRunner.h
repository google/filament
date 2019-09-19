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

#ifndef TNT_PLATFORMRUNNER_H
#define TNT_PLATFORMRUNNER_H

#include <stdint.h>
#include <stddef.h>

namespace test {

// To avoid a dependency on filabridge, the Backend enum is replicated here.
enum class Backend : uint8_t {
    OPENGL = 1,
    VULKAN = 2,
    METAL = 3,
    NOOP = 4,
};

struct NativeView {
    void* ptr = nullptr;
    size_t width = 0, height = 0;
};

/**
 * Test runners should implement this method which gets called by backend_test inside runTests.
 *
 * @return a NativeView struct with a pointer to the platform-specific view and dimensions.
 */
NativeView getNativeView();

/**
 * Test runners should call initTests as soon as possible to initialize the test cases.
 * No tests will be run yet.
 *
 * @param backend The backend to run the tests on.
 * @param isMobile True if the platform is a mobile platform (iOS or Android).
 */
void initTests(Backend backend, bool isMobile, int& argc, char* argv[]);

/**
 * Test runners should call runTests when they are ready for tests to be run.
 *
 * @return An int success code to be reported in a platform-specific way.
 */
int runTests();

/**
 * A utility method that can be invoked by test runners to parse arguments.
 * Looks through the provided command-line arguments and finds any -a <backend> arguments.
 */
Backend parseArgumentsForBackend(int argc, char* argv[]);

}

#endif
