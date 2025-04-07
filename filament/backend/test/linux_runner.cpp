/*
 * Copyright (C) 2023 The Android Open Source Project
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


#include "BackendTest.h"
#include "PlatformRunner.h"

#include <array>
#include <iostream>

namespace test {

test::NativeView getNativeView() {
    return {
            .ptr = nullptr,
            .width = WINDOW_WIDTH,
            .height = WINDOW_HEIGHT,
    };
}

}// namespace test

namespace {

std::array<test::Backend, 2> const VALID_BACKENDS{
        test::Backend::OPENGL,
        test::Backend::VULKAN,
};

}// namespace

int main(int argc, char* argv[]) {
    auto backend = test::parseArgumentsForBackend(argc, argv);

    if (!std::any_of(VALID_BACKENDS.begin(), VALID_BACKENDS.end(),
                [backend](test::Backend validBackend) { return backend == validBackend; })) {
        std::cerr << "Specified an invalid backend. Only GL and Vulkan are available" << std::endl;
        return 1;
    }

    test::initTests(backend, test::OperatingSystem::LINUX, false, argc, argv);
    return test::runTests();
}
