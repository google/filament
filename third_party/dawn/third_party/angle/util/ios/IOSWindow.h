//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// IOSWindow.h: Definition of the implementation of OSWindow for iOS

#ifndef UTIL_IOS_WINDOW_H_
#define UTIL_IOS_WINDOW_H_

#include "common/debug.h"
#include "util/OSWindow.h"

class IOSWindow : public OSWindow
{
  public:
    IOSWindow() {}
    ~IOSWindow() override {}

    void disableErrorMessageDialog() override {}
    void destroy() override {}

    void resetNativeWindow() override {}
    EGLNativeWindowType getNativeWindow() const override;
    EGLNativeDisplayType getNativeDisplay() const override { return EGL_DEFAULT_DISPLAY; }

    void messageLoop() override {}

    void setMousePosition(int x, int y) override { UNIMPLEMENTED(); }
    bool setOrientation(int width, int height) override;
    bool setPosition(int x, int y) override
    {
        UNIMPLEMENTED();
        return false;
    }
    bool resize(int width, int height) override;
    void setVisible(bool isVisible) override {}

    void signalTestEvent() override { UNIMPLEMENTED(); }

  private:
    bool initializeImpl(const std::string &name, int width, int height) override;
};

#endif  // UTIL_IOS_WINDOW_H_
