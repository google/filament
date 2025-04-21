/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_SKIP_H
#define TNT_SKIP_H

#include <gtest/gtest.h>
#include "BackendTest.h"

// skipEnvironment must be a test::SkipEnvironment
#define SKIP_IF(skipEnvironment)                                                                   \
do {                                                                                               \
    SkipEnvironment skip(skipEnvironment);                                                         \
    if (skip.matches()) {                                                                          \
        GTEST_SKIP() << "Skipping test as the " << skip.describe();                                \
    }                                                                                              \
} while (false)

namespace test {

struct SkipEnvironment {
    SkipEnvironment(const SkipEnvironment&) = default;
    explicit SkipEnvironment(test::Backend backend);
    explicit SkipEnvironment(test::OperatingSystem os);
    SkipEnvironment(test::OperatingSystem os, test::Backend backend);

    std::optional<test::Backend> backend;
    std::optional<test::OperatingSystem> os;
    std::optional<bool> isMobile;

    bool matches();
    // Describes the current state of either matching or mismatching.
    std::string describe();
    // Describe all the non-null requirements.
    std::string describe_requirements();
    // Describes the environment's status for all the attributes that are non-null.
    std::string describe_actual_environment();
};

} // namespace test

#endif// TNT_SKIP_H
