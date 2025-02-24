//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Win32Window.h: Definition of the implementation of OSWindow for Win32 (Windows)

#ifndef UTIL_WIN32_WINDOW_H
#define UTIL_WIN32_WINDOW_H

#include <windows.h>
#include <string>

#include "util/OSWindow.h"
#include "util/Timer.h"

class Win32Window : public OSWindow
{
  public:
    Win32Window();
    ~Win32Window() override;

    void destroy() override;
    void disableErrorMessageDialog() override;

    bool takeScreenshot(uint8_t *pixelData) override;

    void resetNativeWindow() override;
    EGLNativeWindowType getNativeWindow() const override;
    EGLNativeDisplayType getNativeDisplay() const override;

    void messageLoop() override;

    void pushEvent(Event event) override;

    void setMousePosition(int x, int y) override;
    bool setOrientation(int width, int height) override;
    bool setPosition(int x, int y) override;
    bool resize(int width, int height) override;
    void setVisible(bool isVisible) override;

    void signalTestEvent() override;

  private:
    bool initializeImpl(const std::string &name, int width, int height) override;
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    std::string mParentClassName;
    std::string mChildClassName;

    bool mIsVisible;
    Timer mSetVisibleTimer;

    bool mIsMouseInWindow;

    EGLNativeWindowType mNativeWindow;
    EGLNativeWindowType mParentWindow;
    EGLNativeDisplayType mNativeDisplay;
};

#endif  // UTIL_WIN32_WINDOW_H
