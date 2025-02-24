//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// OSWindow:
//   Operating system window integration base class.

#ifndef UTIL_OSWINDOW_H_
#define UTIL_OSWINDOW_H_

#include <stdint.h>
#include <list>
#include <string>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "util/Event.h"
#include "util/util_export.h"

class ANGLE_UTIL_EXPORT OSWindow
{
  public:
    static OSWindow *New();
    static void Delete(OSWindow **osWindow);

    bool initialize(const std::string &name, int width, int height);
    virtual void destroy()                   = 0;
    virtual void disableErrorMessageDialog() = 0;

    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;

    // Takes a screenshot of the window, returning the result as a mWidth * mHeight * 4
    // normalized unsigned byte BGRA array. Note that it will be used to test the window
    // manager's behavior so it needs to take an actual screenshot of the screen and not
    // just grab the pixels of the window. Returns if it was successful.
    virtual bool takeScreenshot(uint8_t *pixelData);

    // Re-initializes the native window. This is used on platforms which do not
    // have a reusable EGLNativeWindowType in order to recreate it, and is
    // needed by the test suite because it re-uses the same OSWindow for
    // multiple EGLSurfaces.
    virtual void resetNativeWindow() = 0;

    virtual EGLNativeWindowType getNativeWindow() const = 0;

    // Returns a native pointer that can be used for eglCreatePlatformWindowSurfaceEXT().
    virtual void *getPlatformExtension();

    virtual void setNativeDisplay(EGLNativeDisplayType display) {}
    virtual EGLNativeDisplayType getNativeDisplay() const = 0;

    virtual void messageLoop() = 0;

    bool popEvent(Event *event);
    virtual void pushEvent(Event event);

    virtual void setMousePosition(int x, int y)        = 0;
    virtual bool setOrientation(int width, int height) = 0;
    virtual bool setPosition(int x, int y)             = 0;
    virtual bool resize(int width, int height)         = 0;
    virtual void setVisible(bool isVisible)            = 0;

    virtual void signalTestEvent() = 0;

    // Pops events look for the test event
    bool didTestEventFire();

    // Whether window has been successfully initialized.
    bool valid() const { return mValid; }

    void ignoreSizeEvents() { mIgnoreSizeEvents = true; }

  protected:
    OSWindow();
    virtual ~OSWindow();

    virtual bool initializeImpl(const std::string &name, int width, int height) = 0;

    int mX;
    int mY;
    int mWidth;
    int mHeight;

    std::list<Event> mEvents;

    bool mValid;
    bool mIgnoreSizeEvents;
};

namespace angle
{
// Find a test data file or directory.
ANGLE_UTIL_EXPORT bool FindTestDataPath(const char *searchPath,
                                        char *dataPathOut,
                                        size_t maxDataPathOutLen);
}  // namespace angle

#endif  // UTIL_OSWINDOW_H_
