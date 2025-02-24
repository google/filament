//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkAndroid.h:
//    Defines the class interface for DisplayVkAndroid, implementing DisplayVk for Android.
//

#ifndef LIBANGLE_RENDERER_VULKAN_ANDROID_DISPLAYVKANDROID_H_
#define LIBANGLE_RENDERER_VULKAN_ANDROID_DISPLAYVKANDROID_H_

#include "libANGLE/renderer/vulkan/DisplayVk.h"
#include "libANGLE/renderer/vulkan/android/AHBFunctions.h"

namespace rx
{
class DisplayVkAndroid : public DisplayVk
{
  public:
    DisplayVkAndroid(const egl::DisplayState &state);

    egl::Error initialize(egl::Display *display) override;

    bool isValidNativeWindow(EGLNativeWindowType window) const override;

    SurfaceImpl *createWindowSurfaceVk(const egl::SurfaceState &state,
                                       EGLNativeWindowType window) override;

    egl::ConfigSet generateConfigs() override;
    void checkConfigSupport(egl::Config *config) override;

    egl::Error validateImageClientBuffer(const gl::Context *context,
                                         EGLenum target,
                                         EGLClientBuffer clientBuffer,
                                         const egl::AttributeMap &attribs) const override;

    ExternalImageSiblingImpl *createExternalImageSibling(const gl::Context *context,
                                                         EGLenum target,
                                                         EGLClientBuffer buffer,
                                                         const egl::AttributeMap &attribs) override;

    const char *getWSIExtension() const override;

    const AHBFunctions &getAHBFunctions() const { return mAHBFunctions; }

  private:
    void enableRecordableIfSupported(egl::Config *config);

    AHBFunctions mAHBFunctions;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_ANDROID_DISPLAYVKANDROID_H_
