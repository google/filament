//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ScenicWindow.h:
//    Subclasses OSWindow for Fuchsia's Scenic compositor.
//

#ifndef UTIL_FUCHSIA_SCENIC_WINDOW_H
#define UTIL_FUCHSIA_SCENIC_WINDOW_H

#include "common/debug.h"
#include "util/OSWindow.h"
#include "util/util_export.h"

// Disable ANGLE-specific warnings that pop up in fuchsia headers.
ANGLE_DISABLE_DESTRUCTOR_OVERRIDE_WARNING
ANGLE_DISABLE_SUGGEST_OVERRIDE_WARNINGS

#include <fuchsia/element/cpp/fidl.h>
#include <fuchsia_egl.h>
#include <lib/async-loop/cpp/loop.h>
#include <zircon/types.h>
#include <string>

ANGLE_REENABLE_SUGGEST_OVERRIDE_WARNINGS
ANGLE_REENABLE_DESTRUCTOR_OVERRIDE_WARNING

struct FuchsiaEGLWindowDeleter
{
    void operator()(fuchsia_egl_window *eglWindow) { fuchsia_egl_window_destroy(eglWindow); }
};

class ANGLE_UTIL_EXPORT ScenicWindow : public OSWindow
{
  public:
    ScenicWindow();
    ~ScenicWindow() override;

    // OSWindow:
    void disableErrorMessageDialog() override;
    void destroy() override;
    void resetNativeWindow() override;
    EGLNativeWindowType getNativeWindow() const override;
    EGLNativeDisplayType getNativeDisplay() const override;
    void messageLoop() override;
    void setMousePosition(int x, int y) override;
    bool setOrientation(int width, int height) override;
    bool setPosition(int x, int y) override;
    bool resize(int width, int height) override;
    void setVisible(bool isVisible) override;
    void signalTestEvent() override;

    // Presents the window to Scenic.
    //
    // We need to do this once per EGL window surface after adding the
    // surface's image pipe as a child of our window.
    void present();

  private:
    bool initializeImpl(const std::string &name, int width, int height) override;
    void updateViewSize();

    // ScenicWindow async loop.
    async::Loop *const mLoop;

    // System services.
    zx::channel mServiceRoot;
    fuchsia::element::GraphicalPresenterPtr mPresenter;

    // EGL native window.
    std::unique_ptr<fuchsia_egl_window, FuchsiaEGLWindowDeleter> mFuchsiaEGLWindow;
};

#endif  // UTIL_FUCHSIA_SCENIC_WINDOW_H
