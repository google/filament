//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayAndroid.cpp: Android implementation of egl::Display

#include "libANGLE/renderer/gl/egl/android/DisplayAndroid.h"

#include <android/log.h>
#include <android/native_window.h>

#include "libANGLE/Display.h"
#include "libANGLE/renderer/gl/egl/android/NativeBufferImageSiblingAndroid.h"

namespace rx
{

DisplayAndroid::DisplayAndroid(const egl::DisplayState &state) : DisplayEGL(state) {}

DisplayAndroid::~DisplayAndroid() {}

bool DisplayAndroid::isValidNativeWindow(EGLNativeWindowType window) const
{
    return ANativeWindow_getFormat(window) >= 0;
}

egl::Error DisplayAndroid::validateImageClientBuffer(const gl::Context *context,
                                                     EGLenum target,
                                                     EGLClientBuffer clientBuffer,
                                                     const egl::AttributeMap &attribs) const
{
    switch (target)
    {
        case EGL_NATIVE_BUFFER_ANDROID:
            return egl::NoError();

        default:
            return DisplayEGL::validateImageClientBuffer(context, target, clientBuffer, attribs);
    }
}

ExternalImageSiblingImpl *DisplayAndroid::createExternalImageSibling(
    const gl::Context *context,
    EGLenum target,
    EGLClientBuffer buffer,
    const egl::AttributeMap &attribs)
{
    switch (target)
    {
        case EGL_NATIVE_BUFFER_ANDROID:
            return new NativeBufferImageSiblingAndroid(buffer, attribs);

        default:
            return DisplayEGL::createExternalImageSibling(context, target, buffer, attribs);
    }
}

}  // namespace rx
