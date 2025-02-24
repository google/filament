//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
//
// clspv_utils:
//     Utilities to map clspv interface variables to OpenCL and Vulkan mappings.
//

#ifndef LIBANGLE_RENDERER_VULKAN_CLSPV_UTILS_H_
#define LIBANGLE_RENDERER_VULKAN_CLSPV_UTILS_H_

#include <string>
#include <vector>

#include <libANGLE/renderer/vulkan/CLDeviceVk.h>

#include "clspv/Compiler.h"
#include "spirv-tools/libspirv.h"

namespace rx
{
struct ClspvPrintfBufferStorage
{
    uint32_t descriptorSet = 0;
    uint32_t binding       = 0;
    uint32_t pcOffset      = 0;
    uint32_t size          = 0;
};

struct ClspvPrintfInfo
{
    uint32_t id = 0;
    std::string formatSpecifier;
    std::vector<uint32_t> argSizes;
};

angle::Result ClspvProcessPrintfBuffer(unsigned char *buffer,
                                       const size_t bufferSize,
                                       const angle::HashMap<uint32_t, ClspvPrintfInfo> *infoMap);

// Populate a list of options that can be supported by clspv based on the features supported by the
// vulkan renderer.
std::string ClspvGetCompilerOptions(const CLDeviceVk *device);

ClspvError ClspvCompileSource(const size_t programCount,
                              const size_t *programSizes,
                              const char **programs,
                              const char *options,
                              char **outputBinary,
                              size_t *outputBinarySize,
                              char **outputLog);

spv_target_env ClspvGetSpirvVersion(const vk::Renderer *renderer);

bool ClspvValidate(vk::Renderer *rendererVk, const angle::spirv::Blob &blob);

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_CLSPV_UTILS_H_
