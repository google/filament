//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// OzoneWindow.h: Definition of the implementation of OSWindow for Ozone

#ifndef UTIL_OZONE_WINDOW_H
#define UTIL_OZONE_WINDOW_H

#include <string>

#include "util/OSWindow.h"

class OzoneWindow : public OSWindow
{
  public:
    OzoneWindow();
    ~OzoneWindow() override;

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

  private:
    bool initializeImpl(const std::string &name, int width, int height) override;

    struct Native
    {
        int32_t x;
        int32_t y;
        int32_t width;
        int32_t height;
        int32_t borderWidth;
        int32_t borderHeight;
        int32_t visible;
        int32_t depth;
    };

    Native mNative;
    static int sLastDepth;
};

#endif  // UTIL_OZONE_WINDOW_H
