//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WaylandWindow.h: Definition of the implementation of OSWindow for Wayland

#ifndef UTIL_WAYLAND_WINDOW_H
#define UTIL_WAYLAND_WINDOW_H

#include <poll.h>
#include <wayland-client.h>
#include <wayland-egl-core.h>

#include "util/OSWindow.h"
#include "util/util_export.h"

bool IsWaylandWindowAvailable();

class ANGLE_UTIL_EXPORT WaylandWindow : public OSWindow
{
  public:
    WaylandWindow();
    ~WaylandWindow() override;

    void disableErrorMessageDialog() override;
    void destroy() override;

    void resetNativeWindow() override;
    EGLNativeWindowType getNativeWindow() const override;

    void setNativeDisplay(EGLNativeDisplayType display) override;
    EGLNativeDisplayType getNativeDisplay() const override;

    void messageLoop() override;

    void setMousePosition(int x, int y) override;
    bool setOrientation(int width, int height) override;
    bool setPosition(int x, int y) override;
    bool resize(int width, int height) override;
    void setVisible(bool isVisible) override;

    void signalTestEvent() override;

  private:
    static void RegistryHandleGlobal(void *data,
                                     struct wl_registry *registry,
                                     uint32_t name,
                                     const char *interface,
                                     uint32_t version);
    static void RegistryHandleGlobalRemove(void *data, struct wl_registry *registry, uint32_t name);

    bool initializeImpl(const std::string &name, int width, int height) override;

    static const struct wl_registry_listener registryListener;

    struct wl_display *mDisplay;
    struct wl_compositor *mCompositor;
    struct wl_surface *mSurface;
    struct wl_egl_window *mWindow;

    struct pollfd fds[1];
};

#endif  // UTIL_WAYLAND_WINDOW_H
