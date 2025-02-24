//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayImpl.cpp: Implementation methods of egl::Display

#include "libANGLE/renderer/DisplayImpl.h"

#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/DeviceImpl.h"

namespace rx
{
namespace
{
// For back-ends that do not implement EGLDevice.
class MockDevice : public DeviceImpl
{
  public:
    MockDevice() = default;
    egl::Error initialize() override { return egl::NoError(); }
    egl::Error getAttribute(const egl::Display *display, EGLint attribute, void **outValue) override
    {
        UNREACHABLE();
        return egl::EglBadAttribute();
    }
    void generateExtensions(egl::DeviceExtensions *outExtensions) const override
    {
        *outExtensions = egl::DeviceExtensions();
    }
};
}  // anonymous namespace

DisplayImpl::DisplayImpl(const egl::DisplayState &state)
    : mState(state), mExtensionsInitialized(false), mCapsInitialized(false), mBlobCache(nullptr)
{}

DisplayImpl::~DisplayImpl()
{
    ASSERT(mState.surfaceMap.empty());
}

egl::Error DisplayImpl::prepareForCall()
{
    return egl::NoError();
}

egl::Error DisplayImpl::releaseThread()
{
    return egl::NoError();
}

const egl::DisplayExtensions &DisplayImpl::getExtensions() const
{
    if (!mExtensionsInitialized)
    {
        generateExtensions(&mExtensions);
        mExtensionsInitialized = true;
    }

    return mExtensions;
}

egl::Error DisplayImpl::handleGPUSwitch()
{
    return egl::NoError();
}

egl::Error DisplayImpl::forceGPUSwitch(EGLint gpuIDHigh, EGLint gpuIDLow)
{
    return egl::NoError();
}

egl::Error DisplayImpl::waitUntilWorkScheduled()
{
    return egl::NoError();
}

egl::Error DisplayImpl::validateClientBuffer(const egl::Config *configuration,
                                             EGLenum buftype,
                                             EGLClientBuffer clientBuffer,
                                             const egl::AttributeMap &attribs) const
{
    UNREACHABLE();
    return egl::EglBadDisplay() << "DisplayImpl::validateClientBuffer unimplemented.";
}

egl::Error DisplayImpl::validateImageClientBuffer(const gl::Context *context,
                                                  EGLenum target,
                                                  EGLClientBuffer clientBuffer,
                                                  const egl::AttributeMap &attribs) const
{
    UNREACHABLE();
    return egl::EglBadDisplay() << "DisplayImpl::validateImageClientBuffer unimplemented.";
}

egl::Error DisplayImpl::validatePixmap(const egl::Config *config,
                                       EGLNativePixmapType pixmap,
                                       const egl::AttributeMap &attributes) const
{
    UNREACHABLE();
    return egl::EglBadDisplay() << "DisplayImpl::valdiatePixmap unimplemented.";
}

const egl::Caps &DisplayImpl::getCaps() const
{
    if (!mCapsInitialized)
    {
        generateCaps(&mCaps);
        mCapsInitialized = true;
    }

    return mCaps;
}

DeviceImpl *DisplayImpl::createDevice()
{
    return new MockDevice();
}

angle::NativeWindowSystem DisplayImpl::getWindowSystem() const
{
    return angle::NativeWindowSystem::Other;
}

bool DisplayImpl::supportsDmaBufFormat(EGLint format) const
{
    UNREACHABLE();
    return false;
}

egl::Error DisplayImpl::queryDmaBufFormats(EGLint max_formats, EGLint *formats, EGLint *num_formats)
{
    UNREACHABLE();
    return egl::NoError();
}

egl::Error DisplayImpl::queryDmaBufModifiers(EGLint format,
                                             EGLint max_modifiers,
                                             EGLuint64KHR *modifiers,
                                             EGLBoolean *external_only,
                                             EGLint *num_modifiers)
{
    UNREACHABLE();
    return egl::NoError();
}

egl::Error DisplayImpl::querySupportedCompressionRates(const egl::Config *configuration,
                                                       const egl::AttributeMap &attributes,
                                                       EGLint *rates,
                                                       EGLint rate_size,
                                                       EGLint *num_rates) const
{
    UNREACHABLE();
    return egl::NoError();
}
}  // namespace rx
