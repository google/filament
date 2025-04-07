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

#ifndef TNT_SKIP_H
#define TNT_SKIP_H

#include <gtest/gtest.h>
#include "BackendTest.h"

#define SKIP_IF_BACKEND(backend)                                 \
do {                                                             \
    if (BackendTest::matchesEnvironment(backend)) {              \
        GTEST_SKIP() << "Skipping test because the backend is "  \
                     << static_cast<uint8_t>(backend);           \
    }                                                            \
} while (false)

#define SKIP_IF_OS(operatingSystem)                             \
do {                                                            \
    if (BackendTest::matchesEnvironment(operatingSystem)) {     \
        GTEST_SKIP() << "Skipping test because the OS is "      \
                     << static_cast<uint8_t>(operatingSystem);  \
    }                                                           \
} while (false)

#define SKIP_IF(operatingSystem, backend)                                         \
do {                                                                              \
    if (BackendTest::matchesEnvironment(operatingSystem, backend)) {              \
        GTEST_SKIP() << "Skipping test because the OS is "                        \
                     << static_cast<uint8_t>(operatingSystem)                     \
                     << " and the backend is " << static_cast<uint8_t>(backend);  \
    }                                                                             \
} while (false)

#endif// TNT_SKIP_H
