//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsEGLDL.cpp: Implements the FunctionsEGLDL class.

#include "libANGLE/renderer/gl/egl/FunctionsEGLDL.h"

#include <dlfcn.h>

namespace rx
{
namespace
{
// In ideal world, we would want this to be a member of FunctionsEGLDL,
// and call dlclose() on it in ~FunctionsEGLDL().
// However, some GL implementations are broken and don't allow multiple
// load/unload cycles, but only static linking with them.
// That's why we dlopen() this handle once and never dlclose() it.
// This is consistent with Chromium's CleanupNativeLibraries() code,
// referencing crbug.com/250813 and http://www.xfree86.org/4.3.0/DRI11.html
void *nativeEGLHandle;
}  // anonymous namespace

FunctionsEGLDL::FunctionsEGLDL() : mGetProcAddressPtr(nullptr) {}

FunctionsEGLDL::~FunctionsEGLDL() {}

egl::Error FunctionsEGLDL::initialize(EGLAttrib platformType,
                                      EGLNativeDisplayType nativeDisplay,
                                      const char *libName,
                                      void *eglHandle)
{

    if (eglHandle)
    {
        // If the handle is provided, use it.
        // Caller has already dlopened the vendor library.
        nativeEGLHandle = eglHandle;
    }

    if (!nativeEGLHandle)
    {
        nativeEGLHandle = dlopen(libName, RTLD_NOW);
        if (!nativeEGLHandle)
        {
            return egl::EglNotInitialized() << "Could not dlopen native EGL: " << dlerror();
        }
    }

    mGetProcAddressPtr =
        reinterpret_cast<PFNEGLGETPROCADDRESSPROC>(dlsym(nativeEGLHandle, "eglGetProcAddress"));
    if (!mGetProcAddressPtr)
    {
        return egl::EglNotInitialized() << "Could not find eglGetProcAddress";
    }

    return FunctionsEGL::initialize(platformType, nativeDisplay);
}

void *FunctionsEGLDL::getProcAddress(const char *name) const
{
    void *f = reinterpret_cast<void *>(mGetProcAddressPtr(name));
    if (f)
    {
        return f;
    }
    return dlsym(nativeEGLHandle, name);
}

}  // namespace rx
