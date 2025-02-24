//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// libvulkan_loader.h:
//    Helper functions for the loading Vulkan libraries.
//

#include <memory>

#ifndef LIBANGLE_COMMON_VULKAN_LIBVULKAN_LOADER_H_
#    define LIBANGLE_COMMON_VULKAN_LIBVULKAN_LOADER_H_

namespace angle
{
namespace vk
{
void *OpenLibVulkan();
}
}  // namespace angle

#endif  // LIBANGLE_COMMON_VULKAN_LIBVULKAN_LOADER_H_
