//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_RENDERER_VULKAN_ANDROID_AHBFUNCTIONS_H_
#define LIBANGLE_RENDERER_VULKAN_ANDROID_AHBFUNCTIONS_H_

#include <android/hardware_buffer.h>

namespace rx
{

class AHBFunctions
{
  public:
    AHBFunctions();
    ~AHBFunctions();

    void acquire(AHardwareBuffer *buffer) const { mAcquireFn(buffer); }
    void describe(const AHardwareBuffer *buffer, AHardwareBuffer_Desc *outDesc) const
    {
        mDescribeFn(buffer, outDesc);
    }
    void release(AHardwareBuffer *buffer) const { mReleaseFn(buffer); }

    bool valid() const { return mAcquireFn && mDescribeFn && mReleaseFn; }

  private:
    using PFN_AHARDWAREBUFFER_acquire  = void (*)(AHardwareBuffer *buffer);
    using PFN_AHARDWAREBUFFER_describe = void (*)(const AHardwareBuffer *buffer,
                                                  AHardwareBuffer_Desc *outDesc);
    using PFN_AHARDWAREBUFFER_release  = void (*)(AHardwareBuffer *buffer);

    PFN_AHARDWAREBUFFER_acquire mAcquireFn   = nullptr;
    PFN_AHARDWAREBUFFER_describe mDescribeFn = nullptr;
    PFN_AHARDWAREBUFFER_release mReleaseFn   = nullptr;

    void getAhbProcAddresses(void *handle);

    void *mLibNativeWindowHandle;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_ANDROID_AHBFUNCTIONS_H_
