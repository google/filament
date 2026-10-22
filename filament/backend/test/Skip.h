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

#include "BackendTest.h"

#include <gtest/gtest.h>

// skipEnvironment must be a test::SkipEnvironment or something that can be passed to the type's
// constructor
// rationale must be a string
#define SKIP_IF(skipEnvironment, rationale)                                                    \
do {                                                                                           \
    SkipEnvironment skip(skipEnvironment);                                                     \
    if (skip.matches()) {                                                                      \
        GTEST_SKIP() << "Skipping test as the " << skip.describe() << "\n"                     \
                     << " This test can't run there because " << rationale;                    \
    }                                                                                          \
} while (false)

#define SKIP_IF_NOT(skipEnvironment, rationale)                                                \
do {                                                                                           \
    SkipEnvironment skip(skipEnvironment);                                                     \
    if (!skip.matches()) {                                                                     \
        GTEST_SKIP() << "Skipping test as the " << skip.describe() << "\n"                     \
                     << " This test can't run there because " << rationale;                    \
    }                                                                                          \
} while (false)

#define NONFATAL_FAIL_IF(skipEnvironment, rationale)                                           \
do {                                                                                           \
    SkipEnvironment skip(skipEnvironment);                                                     \
    if (skip.matches()) {                                                                      \
        ADD_FAILURE()                                                                          \
                << "Failing test as the " << skip.describe() << "\n"                           \
                << " This test has a known failure where "   \
                << rationale;                                                                  \
    }                                                                                          \
} while (false)

#define FAIL_IF(skipEnvironment, rationale)                                                    \
do {                                                                                           \
    SkipEnvironment skip(skipEnvironment);                                                     \
    if (skip.matches()) {                                                                      \
        GTEST_FAIL()                                                                           \
                << "Failing test as the " << skip.describe() << "\n"                           \
                << " This test should be able to succeed but it needs to fail early because "  \
                << rationale;                                                                  \
    }                                                                                          \
} while (false)

namespace test {

// Tag for a requirement on the swap chain the runner can supply, rather than on the platform. A
// headless run has no native view and therefore no drawable to present, so a test that depends on
// presentation cannot pass there on any backend or operating system. Spelled as a tag type so that
// it reads as SKIP_IF(Headless(), ...) and cannot be confused with the other bool-valued
// attributes below.
struct Headless {};

struct SkipEnvironment {
    SkipEnvironment(const SkipEnvironment&) = default;
    explicit SkipEnvironment(test::Backend backend);
    explicit SkipEnvironment(test::OperatingSystem os);
    explicit SkipEnvironment(Headless);
    SkipEnvironment(Headless, test::Backend backend);
    SkipEnvironment(test::OperatingSystem os, test::Backend backend);

    std::optional<test::Backend> backend;
    std::optional<test::OperatingSystem> os;
    std::optional<bool> isMobile;
    std::optional<bool> headless;

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
