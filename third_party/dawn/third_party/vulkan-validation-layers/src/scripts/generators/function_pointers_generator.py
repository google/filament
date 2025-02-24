#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2025 The Khronos Group Inc.
# Copyright (c) 2015-2025 Valve Corporation
# Copyright (c) 2015-2025 LunarG, Inc.
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
from generators.base_generator import BaseGenerator
from generators.generator_utils import PlatformGuardHelper

class FunctionPointersOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2015-2025 The Khronos Group Inc.
            * Copyright (c) 2015-2025 Valve Corporation
            * Copyright (c) 2015-2025 LunarG, Inc.
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
            ****************************************************************************/\n''')
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore
        self.write('// clang-format off')

        if self.filename == 'vk_function_pointers.h':
            self.generateHeader()
        elif self.filename == 'vk_function_pointers.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// clang-format on')
        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
#pragma once
#include <vulkan/vulkan.h>

#ifdef _WIN32
/* Windows-specific common code: */
// WinBase.h defines CreateSemaphore and synchapi.h defines CreateEvent
//  undefine them to avoid conflicts with VkLayerDispatchTable struct members.
#ifdef CreateSemaphore
#undef CreateSemaphore
#endif
#ifdef CreateEvent
#undef CreateEvent
#endif
#endif

namespace vk {
''')
        guard_helper = PlatformGuardHelper()
        for command in self.vk.commands.values():
            out.extend(guard_helper.add_guard(command.protect))
            out.append(f'extern PFN_{command.name} {command.name[2:]};\n')
        out.extend(guard_helper.add_guard(None))
        out.append('''
void InitCore(const char *api_name);
void InitExtensionFromCore(const char* extension_name);
void InitInstanceExtension(VkInstance instance, const char* extension_name);
void InitDeviceExtension(VkInstance instance, VkDevice device, const char* extension_name);
void ResetAllExtensions();

} // namespace vk''')
        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
#include "vk_function_pointers.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <string>
#include "containers/custom_containers.h"

#ifdef _WIN32
// Dynamic Loading:
typedef HMODULE dl_handle;
static dl_handle open_library(const char *lib_path) {
    // Try loading the library the original way first.
    dl_handle lib_handle = LoadLibrary(lib_path);
    if (lib_handle == NULL && GetLastError() == ERROR_MOD_NOT_FOUND) {
        // If that failed, then try loading it with broader search folders.
        lib_handle = LoadLibraryEx(lib_path, NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
    }
    return lib_handle;
}
static char *open_library_error(const char *libPath) {
    static char errorMsg[164];
    (void)snprintf(errorMsg, 163, "Failed to open dynamic library \\\"%s\\\" with error %lu", libPath, GetLastError());
    return errorMsg;
}
static void *get_proc_address(dl_handle library, const char *name) {
    assert(library);
    assert(name);
    return (void *)GetProcAddress(library, name);
}
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__QNX__) || defined(__GNU__)

#include <dlfcn.h>

typedef void *dl_handle;
static inline dl_handle open_library(const char *libPath) {
    // When loading the library, we use RTLD_LAZY so that not all symbols have to be
    // resolved at this time (which improves performance). Note that if not all symbols
    // can be resolved, this could cause crashes later. Use the LD_BIND_NOW environment
    // variable to force all symbols to be resolved here.
    return dlopen(libPath, RTLD_LAZY | RTLD_LOCAL);
}
static inline const char *open_library_error(const char * /*libPath*/) { return dlerror(); }
static inline void *get_proc_address(dl_handle library, const char *name) {
    assert(library);
    assert(name);
    return dlsym(library, name);
}
#else
#error Dynamic library functions must be defined for this OS.
#endif

namespace vk {
''')
        guard_helper = PlatformGuardHelper()
        for command in self.vk.commands.values():
            out.extend(guard_helper.add_guard(command.protect))
            out.append(f'PFN_{command.name} {command.name[2:]};\n')
        out.extend(guard_helper.add_guard(None))

        out.append('''
void InitCore(const char *api_name) {

#if defined(WIN32)
    std::string filename = std::string(api_name) + "-1.dll";
    auto lib_handle = open_library(filename.c_str());
#elif(__APPLE__)
    std::string filename = std::string("lib") + api_name + ".dylib";
    auto lib_handle = open_library(filename.c_str());
#else
    std::string filename = std::string("lib") + api_name + ".so";
    auto lib_handle = open_library(filename.c_str());
    if (!lib_handle) {
        filename = std::string("lib") + api_name + ".so.1";
        lib_handle = open_library(filename.c_str());
    }
#endif

    if (lib_handle == nullptr) {
        printf("%s\\n", open_library_error(filename.c_str()));
        exit(1);
    }
''')
        out.extend([f'    {x.name[2:]} = reinterpret_cast<PFN_{x.name}>(get_proc_address(lib_handle, "{x.name}"));\n' for x in self.vk.commands.values() if not x.extensions])
        out.append('}')

        out.append('''
void InitExtensionFromCore(const char* extension_name) {
    static const vvl::unordered_map<std::string, std::function<void()>> initializers = {
''')
        for extension in [x for x in self.vk.extensions.values() if x.commands and x.promotedTo]:
            out.extend(guard_helper.add_guard(extension.protect))
            out.append('        {\n')
            out.append(f'            "{extension.name}", []() {{\n')
            for command in [x for x in extension.commands]:
                if command.alias is not None and not self.vk.commands[command.alias].extensions:
                    out.append(f'                {command.name[2:]} = {command.alias[2:]};\n')
            out.append('            }\n')
            out.append('        },\n')
        out.extend(guard_helper.add_guard(None))

        out.append('''
    };

    if (auto it = initializers.find(extension_name); it != initializers.end())
        (it->second)();
}
''')

        out.append('''
void InitInstanceExtension(VkInstance instance, const char* extension_name) {
    assert(instance);
    static const vvl::unordered_map<std::string, std::function<void(VkInstance)>> initializers = {
''')
        for extension in [x for x in self.vk.extensions.values() if x.instance and x.commands]:
            out.extend(guard_helper.add_guard(extension.protect))
            out.append('        {\n')
            out.append(f'            "{extension.name}", [](VkInstance instance) {{\n')
            for command in [x for x in extension.commands]:
                out.append(f'                {command.name[2:]} = reinterpret_cast<PFN_{command.name}>(GetInstanceProcAddr(instance, "{command.name}"));\n')
            out.append('            }\n')
            out.append('        },\n')
        out.extend(guard_helper.add_guard(None))

        out.append('''
    };

    if (auto it = initializers.find(extension_name); it != initializers.end())
        (it->second)(instance);
}
''')

        out.append('''
void InitDeviceExtension(VkInstance instance, VkDevice device, const char* extension_name) {
    static const vvl::unordered_map<std::string, std::function<void(VkInstance, VkDevice)>> initializers = {
''')
        for extension in [x for x in self.vk.extensions.values() if x.device and x.commands]:
            out.extend(guard_helper.add_guard(extension.protect))
            out.append('        {\n')
            instanceCommand = [x for x in extension.commands if x.instance]
            deviceCommand = [x for x in extension.commands if x.device]
            out.append(f'            "{extension.name}", [](VkInstance {"instance" if instanceCommand else ""}, VkDevice {"device" if deviceCommand else ""}) {{\n')
            out.extend([f'                {command.name[2:]} = reinterpret_cast<PFN_{command.name}>(GetDeviceProcAddr(device, "{command.name}"));\n' for command in deviceCommand])
            out.extend([f'                {command.name[2:]} = reinterpret_cast<PFN_{command.name}>(GetInstanceProcAddr(instance, "{command.name}"));\n' for command in instanceCommand])
            out.append('            }\n')
            out.append('        },\n')
        out.extend(guard_helper.add_guard(None))

        out.append('''
    };

    if (auto it = initializers.find(extension_name); it != initializers.end())
        (it->second)(instance, device);
}
''')

        out.append('void ResetAllExtensions() {\n')
        for command in [x for x in self.vk.commands.values() if x.extensions]:
            out.extend(guard_helper.add_guard(command.protect))
            out.append(f'    {command.name[2:]} = nullptr;\n')
        out.extend(guard_helper.add_guard(None))

        out.append('}\n')

        out.append('} // namespace vk')
        self.write(''.join(out))
