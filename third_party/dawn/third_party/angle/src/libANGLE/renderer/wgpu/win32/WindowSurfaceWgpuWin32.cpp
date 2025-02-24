//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkWin32.cpp:
//    Defines the class interface for WindowSurfaceWgpuWin32, implementing WindowSurfaceWgpu.
//

#include "libANGLE/renderer/wgpu/win32/WindowSurfaceWgpuWin32.h"

#include "libANGLE/Display.h"
#include "libANGLE/renderer/wgpu/DisplayWgpu.h"
#include "libANGLE/renderer/wgpu/wgpu_utils.h"

namespace rx
{
WindowSurfaceWgpuWin32::WindowSurfaceWgpuWin32(const egl::SurfaceState &surfaceState,
                                               EGLNativeWindowType window)
    : WindowSurfaceWgpu(surfaceState, window)
{}

angle::Result WindowSurfaceWgpuWin32::createWgpuSurface(const egl::Display *display,
                                                        wgpu::Surface *outSurface)
{
    DisplayWgpu *displayWgpu = webgpu::GetImpl(display);
    auto &surfaceCache       = displayWgpu->getSurfaceCache();

    EGLNativeWindowType window = getNativeWindow();
    auto cachedSurfaceIter     = surfaceCache.find(window);
    if (cachedSurfaceIter != surfaceCache.end())
    {
        *outSurface = cachedSurfaceIter->second;
        return angle::Result::Continue;
    }

    wgpu::Instance instance = displayWgpu->getInstance();

    wgpu::SurfaceDescriptorFromWindowsHWND hwndDesc;
    hwndDesc.hinstance = GetModuleHandle(nullptr);
    hwndDesc.hwnd      = window;

    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &hwndDesc;

    wgpu::Surface surface = instance.CreateSurface(&surfaceDesc);
    *outSurface           = surface;

    surfaceCache.insert_or_assign(window, surface);
    return angle::Result::Continue;
}

angle::Result WindowSurfaceWgpuWin32::getCurrentWindowSize(const egl::Display *display,
                                                           gl::Extents *outSize)
{
    RECT rect;
    if (!GetClientRect(getNativeWindow(), &rect))
    {
        // TODO: generate a proper error + msg
        return angle::Result::Stop;
    }

    *outSize = gl::Extents(rect.right - rect.left, rect.bottom - rect.top, 1);
    return angle::Result::Continue;
}

WindowSurfaceWgpu *CreateWgpuWindowSurface(const egl::SurfaceState &surfaceState,
                                           EGLNativeWindowType window)
{
    return new WindowSurfaceWgpuWin32(surfaceState, window);
}
}  // namespace rx
