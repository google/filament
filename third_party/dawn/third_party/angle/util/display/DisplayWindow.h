//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayWindow.h: Definition of the implementation of OSWindow for Linux Display

#ifndef UTIL_DISPLAY_WINDOW_H_
#define UTIL_DISPLAY_WINDOW_H_

#include <string>
#include "util/OSWindow.h"
#include "util/util_export.h"

struct SimpleDisplayWindow
{
    uint16_t width;
    uint16_t height;
};

class ANGLE_UTIL_EXPORT DisplayWindow : public OSWindow
{
  public:
    DisplayWindow();
    DisplayWindow(int visualId);
    ~DisplayWindow() override;

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
    SimpleDisplayWindow mWindow;
};

#endif  // UTIL_DISPLAY_WINDOW_H_
