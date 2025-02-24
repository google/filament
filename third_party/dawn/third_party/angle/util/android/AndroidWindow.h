//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// AndroidWindow.h: Definition of the implementation of OSWindow for Android

#ifndef UTIL_ANDROID_WINDOW_H_
#define UTIL_ANDROID_WINDOW_H_

#include "util/OSWindow.h"

class AndroidWindow : public OSWindow
{
  public:
    AndroidWindow();
    ~AndroidWindow() override;

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

    ANGLE_UTIL_EXPORT static std::string GetExternalStorageDirectory();
    ANGLE_UTIL_EXPORT static std::string GetApplicationDirectory();

  private:
    bool initializeImpl(const std::string &name, int width, int height) override;
};

#endif /* UTIL_ANDROID_WINDOW_H_ */
