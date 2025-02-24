//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WaylandWindow.cpp: Implementation of OSWindow for Wayland

#include "util/linux/wayland/WaylandWindow.h"
#include <cerrno>
#include <cstring>

WaylandWindow::WaylandWindow()
    : mDisplay{nullptr}, mCompositor{nullptr}, mSurface{nullptr}, mWindow{nullptr}
{}

WaylandWindow::~WaylandWindow()
{
    destroy();
}

void WaylandWindow::RegistryHandleGlobal(void *data,
                                         struct wl_registry *registry,
                                         uint32_t name,
                                         const char *interface,
                                         uint32_t version)
{
    WaylandWindow *vc = reinterpret_cast<WaylandWindow *>(data);

    if (strcmp(interface, "wl_compositor") == 0)
    {
        void *compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 1);
        vc->mCompositor  = reinterpret_cast<wl_compositor *>(compositor);
    }
}

void WaylandWindow::RegistryHandleGlobalRemove(void *data,
                                               struct wl_registry *registry,
                                               uint32_t name)
{}

const struct wl_registry_listener WaylandWindow::registryListener = {
    WaylandWindow::RegistryHandleGlobal, WaylandWindow::RegistryHandleGlobalRemove};

bool WaylandWindow::initializeImpl(const std::string &name, int width, int height)
{
    destroy();

    if (!mDisplay)
    {
        mDisplay = wl_display_connect(nullptr);
        if (!mDisplay)
        {
            return false;
        }
    }

    // Not get a window
    struct wl_registry *registry = wl_display_get_registry(mDisplay);
    wl_registry_add_listener(registry, &registryListener, this);

    // Round-trip to get globals
    wl_display_roundtrip(mDisplay);
    if (!mCompositor)
    {
        return false;
    }

    // We don't need this anymore
    wl_registry_destroy(registry);

    mSurface = wl_compositor_create_surface(mCompositor);
    if (!mSurface)
    {
        return false;
    }

    mWindow = wl_egl_window_create(mSurface, width, height);
    if (!mWindow)
    {
        return false;
    }

    fds[0] = {wl_display_get_fd(mDisplay), POLLIN, 0};

    mY      = 0;
    mX      = 0;
    mWidth  = width;
    mHeight = height;

    return true;
}

void WaylandWindow::disableErrorMessageDialog() {}

void WaylandWindow::destroy()
{
    if (mWindow)
    {
        wl_egl_window_destroy(mWindow);
        mWindow = nullptr;
    }

    if (mSurface)
    {
        wl_surface_destroy(mSurface);
        mSurface = nullptr;
    }

    if (mCompositor)
    {
        wl_compositor_destroy(mCompositor);
        mCompositor = nullptr;
    }
}

void WaylandWindow::resetNativeWindow() {}

void WaylandWindow::setNativeDisplay(EGLNativeDisplayType display)
{
    mDisplay = reinterpret_cast<wl_display *>(display);
}

EGLNativeWindowType WaylandWindow::getNativeWindow() const
{
    return reinterpret_cast<EGLNativeWindowType>(mWindow);
}

EGLNativeDisplayType WaylandWindow::getNativeDisplay() const
{
    return reinterpret_cast<EGLNativeDisplayType>(mDisplay);
}

void WaylandWindow::messageLoop()
{
    while (wl_display_prepare_read(mDisplay) != 0)
        wl_display_dispatch_pending(mDisplay);
    if (wl_display_flush(mDisplay) < 0 && errno != EAGAIN)
    {
        wl_display_cancel_read(mDisplay);
        return;
    }
    if (poll(fds, 1, 0) > 0)
    {
        wl_display_read_events(mDisplay);
        wl_display_dispatch_pending(mDisplay);
    }
    else
    {
        wl_display_cancel_read(mDisplay);
    }
}

void WaylandWindow::setMousePosition(int x, int y) {}

bool WaylandWindow::setOrientation(int width, int height)
{
    return true;
}

bool WaylandWindow::setPosition(int x, int y)
{
    return true;
}

bool WaylandWindow::resize(int width, int height)
{
    wl_egl_window_resize(mWindow, width, height, 0, 0);

    mWidth  = width;
    mHeight = height;

    return true;
}

void WaylandWindow::setVisible(bool isVisible) {}

void WaylandWindow::signalTestEvent() {}

bool IsWaylandWindowAvailable()
{
    wl_display *display = wl_display_connect(nullptr);
    if (!display)
    {
        return false;
    }
    wl_display_disconnect(display);
    return true;
}
