/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
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
 */
#pragma once

#include <spirv-tools/libspirv.hpp>
#include "test_common.h"

#include <stdbool.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

// Can be used by tests to record additional details / description of test
#define TEST_DESCRIPTION(desc) RecordProperty("description", desc)

class VkTestFramework : public ::testing::Test {
  public:
    static void InitArgs(int *argc, char *argv[]);
    static void Finish();

    static inline bool m_print_vu = false;
    static inline bool m_syncval_disable_core = false;
    static inline bool m_gpuav_disable_core = false;
    static inline int m_phys_device_index = -1;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    static inline ANativeWindow *window = nullptr;
#endif

  private:
    static inline int m_width = 0;
    static inline int m_height = 0;
};

class TestEnvironment : public ::testing::Environment {
  public:
    void SetUp();

    void TearDown();
};
