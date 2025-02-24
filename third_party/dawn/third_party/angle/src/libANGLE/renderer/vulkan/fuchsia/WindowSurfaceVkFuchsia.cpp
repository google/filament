//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkFuchsia.cpp:
//    Implements methods from WindowSurfaceVkFuchsia.
//

#include "libANGLE/renderer/vulkan/fuchsia/WindowSurfaceVkFuchsia.h"

#include <zircon/syscalls.h>
#include <zircon/syscalls/object.h>
#include "common/fuchsia_egl/fuchsia_egl.h"
#include "common/fuchsia_egl/fuchsia_egl_backend.h"

#include "libANGLE/renderer/vulkan/vk_renderer.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

namespace rx
{

WindowSurfaceVkFuchsia::WindowSurfaceVkFuchsia(const egl::SurfaceState &surfaceState,
                                               EGLNativeWindowType window)
    : WindowSurfaceVk(surfaceState, window)
{}

WindowSurfaceVkFuchsia::~WindowSurfaceVkFuchsia() {}

// static
bool WindowSurfaceVkFuchsia::isValidNativeWindow(EGLNativeWindowType window)
{
    fuchsia_egl_window *egl_window = reinterpret_cast<fuchsia_egl_window *>(window);
    return fuchsia_egl_window_get_width(egl_window) >= 0;
}

angle::Result WindowSurfaceVkFuchsia::createSurfaceVk(vk::ErrorContext *context,
                                                      gl::Extents *extentsOut)
{
#if !defined(ANGLE_SHARED_LIBVULKAN)
    InitImagePipeSurfaceFUCHSIAFunctions(context->getRenderer()->getInstance());
#endif  // !defined(ANGLE_SHARED_LIBVULKAN)
    fuchsia_egl_window *egl_window = reinterpret_cast<fuchsia_egl_window *>(mNativeWindowType);

    VkImagePipeSurfaceCreateInfoFUCHSIA createInfo = {};
    createInfo.sType           = VK_STRUCTURE_TYPE_IMAGEPIPE_SURFACE_CREATE_INFO_FUCHSIA;
    createInfo.imagePipeHandle = fuchsia_egl_window_release_image_pipe(egl_window);
    ANGLE_VK_TRY(context, vkCreateImagePipeSurfaceFUCHSIA(context->getRenderer()->getInstance(),
                                                          &createInfo, nullptr, &mSurface));

    return getCurrentWindowSize(context, extentsOut);
}

angle::Result WindowSurfaceVkFuchsia::getCurrentWindowSize(vk::ErrorContext *context,
                                                           gl::Extents *extentsOut)
{
    fuchsia_egl_window *egl_window = reinterpret_cast<fuchsia_egl_window *>(mNativeWindowType);

    int32_t width  = fuchsia_egl_window_get_width(egl_window);
    int32_t height = fuchsia_egl_window_get_height(egl_window);

    *extentsOut = gl::Extents(width, height, 1);

    return angle::Result::Continue;
}

}  // namespace rx
