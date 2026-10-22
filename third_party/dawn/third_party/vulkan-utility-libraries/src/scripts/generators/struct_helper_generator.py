#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2023 The Khronos Group Inc.
# Copyright (c) 2015-2023 Valve Corporation
# Copyright (c) 2015-2023 LunarG, Inc.
# Copyright (c) 2015-2023 Google Inc.
# Copyright (c) 2023-2023 RasterGrid Kft.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
from base_generator import BaseGenerator
from generators.generator_utils import PlatformGuardHelper

class StructHelperOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        out = []
        out.append(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See {os.path.basename(__file__)} for modifications
// Copyright 2023 The Khronos Group Inc.
// Copyright 2023 Valve Corporation
// Copyright 2023 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
''')
        out.append('// NOLINTBEGIN') # Wrap for clang-tidy to ignore
        out.append('''
#pragma once
#include <vulkan/vulkan.h>

// clang-format off

namespace vku {

template <typename T>
VkStructureType GetSType() {
    static_assert(sizeof(T) == 0, "GetSType() is being used with an unsupported Type! Is the code-gen up to date?");
    return VK_STRUCTURE_TYPE_APPLICATION_INFO;
}\n''')
        guard_helper = PlatformGuardHelper()

        for struct in [x for x in self.vk.structs.values() if x.sType]:
            out.extend(guard_helper.add_guard(struct.protect))
            out.append(f'template <> inline VkStructureType GetSType<{struct.name}>() {{ return {struct.sType}; }}\n')
        out.extend(guard_helper.add_guard(None))
        out.append('''
// Find an entry of the given type in the const pNext chain
// returns nullptr if the entry is not found
template <typename T> const T *FindStructInPNextChain(const void *next) {
    const VkBaseOutStructure *current = reinterpret_cast<const VkBaseOutStructure *>(next);
    VkStructureType desired_sType = GetSType<T>();
    while (current) {
        if (desired_sType == current->sType) {
            return reinterpret_cast<const T*>(current);
        }
        current = current->pNext;
    }
    return nullptr;
}

// Find an entry of the given type in the non-const pNext chain
// returns nullptr if the entry is not found
template <typename T> T *FindStructInPNextChain(void *next) {
    VkBaseOutStructure *current = reinterpret_cast<VkBaseOutStructure *>(next);
    VkStructureType desired_sType = GetSType<T>();
    while (current) {
        if (desired_sType == current->sType) {
            return reinterpret_cast<T*>(current);
        }
        current = current->pNext;
    }
    return nullptr;
}

// Find last element of pNext chain
inline VkBaseOutStructure *FindLastStructInPNextChain(void *next) {
    auto *current = reinterpret_cast<VkBaseOutStructure *>(next);
    auto *prev = current;
    while (current) {
        prev = current;
        current = reinterpret_cast<VkBaseOutStructure *>(current->pNext);
    }
    return prev;
}

// Init the header of an sType struct
template <typename T>
T InitStruct(void *p_next = nullptr) {
    T out = {};
    out.sType = GetSType<T>();
    out.pNext = p_next;
    return out;
}

// Init the header of an sType struct with pNext and optional fields
template <typename T, typename... StructFields>
T InitStruct(void *p_next, StructFields... fields) {
    T out = {GetSType<T>(), p_next, fields...};
    return out;
}

class InitStructHelper {
    void* p_next = nullptr;
  public:
    InitStructHelper() = default;
    InitStructHelper(void *p_next) : p_next(p_next) {}

    template <typename T>
    operator T() {
        return InitStruct<T>(p_next);
    }
};
#if VK_USE_64_BIT_PTR_DEFINES == 1
template<typename T> VkObjectType GetObjectType() {
    static_assert(sizeof(T) == 0, "GetObjectType() is being used with an unsupported Type! Is the code-gen up to date?");
    return VK_OBJECT_TYPE_UNKNOWN;
}

''')
        for handle in self.vk.handles.values():
            out.extend(guard_helper.add_guard(handle.protect))
            out.append(f'template<> inline VkObjectType GetObjectType<{handle.name}>() {{ return {handle.type}; }}\n')
        out.extend(guard_helper.add_guard(None))
        out.append('''
# else // 32 bit
template<typename T> VkObjectType GetObjectType() {
    static_assert(sizeof(T) == 0, "GetObjectType() does not support 32 bit builds! This is a limitation of the vulkan.h headers");
    return VK_OBJECT_TYPE_UNKNOWN;
}
# endif // VK_USE_64_BIT_PTR_DEFINES == 1
} // namespace vku
\n''')

        out.append('// NOLINTEND') # Wrap for clang-tidy to ignore
        out.append('// clang-format on')
        self.write("".join(out))
