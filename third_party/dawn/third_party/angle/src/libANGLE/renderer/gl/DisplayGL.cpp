//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayGL.h: GL implementation of egl::Display

#include "libANGLE/renderer/gl/DisplayGL.h"

#include "libANGLE/AttributeMap.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/gl/ContextGL.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/SurfaceGL.h"

#include <EGL/eglext.h>

namespace rx
{

// On Linux with the amdgpu driver, the renderer string looks like:
//
// AMD Radeon (TM) <GPU model> Graphics (<GPUgeneration>, DRM <DRMversion>, <kernelversion>,
// LLVM <LLVMversion>) eg. AMD Radeon (TM) RX 460 Graphics (POLARIS11,
// DRM 3.35.0, 5.4.0-65-generic, LLVM 11.0.0)
//
// We also want to handle the case without GPUGeneration:
// AMD Radeon GPU model (DRM DRMversion, kernelversion, LLVM LLVMversion)
//
// Thanks to Kelsey Gilbert of Mozilla for this example
// https://phabricator.services.mozilla.com/D105636
std::string SanitizeRendererString(std::string rendererString)
{
    size_t pos = rendererString.find(", DRM ");
    if (pos != std::string::npos)
    {
        rendererString.resize(pos);
        rendererString.push_back(')');
        return rendererString;
    }
    pos = rendererString.find(" (DRM ");
    if (pos != std::string::npos)
    {
        rendererString.resize(pos);
        return rendererString;
    }
    return rendererString;
}

// OpenGL ES requires a prefix of "OpenGL ES" for the GL_VERSION string.
// We can also add the prefix to desktop OpenGL for consistency.
std::string SanitizeVersionString(std::string versionString, bool isES, bool includeFullVersion)
{
    const std::string GLString = "OpenGL ";
    const std::string ESString = "ES ";
    size_t openGLESPos         = versionString.find(GLString);
    std::ostringstream result;

    if (openGLESPos == std::string::npos)
    {
        openGLESPos = 0;
    }
    else
    {
        openGLESPos += GLString.size() + (isES ? ESString.size() : 0);
    }

    result << GLString << (isES ? ESString : "");
    if (includeFullVersion)
    {
        result << versionString.substr(openGLESPos);
    }
    else
    {
        size_t postVersionSpace = versionString.find(" ", openGLESPos);
        result << versionString.substr(openGLESPos, postVersionSpace - openGLESPos);
    }

    return result.str();
}

DisplayGL::DisplayGL(const egl::DisplayState &state) : DisplayImpl(state) {}

DisplayGL::~DisplayGL() {}

egl::Error DisplayGL::initialize(egl::Display *display)
{
    return egl::NoError();
}

void DisplayGL::terminate() {}

ImageImpl *DisplayGL::createImage(const egl::ImageState &state,
                                  const gl::Context *context,
                                  EGLenum target,
                                  const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

SurfaceImpl *DisplayGL::createPbufferFromClientBuffer(const egl::SurfaceState &state,
                                                      EGLenum buftype,
                                                      EGLClientBuffer clientBuffer,
                                                      const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

StreamProducerImpl *DisplayGL::createStreamProducerD3DTexture(
    egl::Stream::ConsumerType consumerType,
    const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

ShareGroupImpl *DisplayGL::createShareGroup(const egl::ShareGroupState &state)
{
    return new ShareGroupGL(state);
}

egl::Error DisplayGL::makeCurrent(egl::Display *display,
                                  egl::Surface *drawSurface,
                                  egl::Surface *readSurface,
                                  gl::Context *context)
{
    // Ensure that the correct global DebugAnnotator is installed when the end2end tests change
    // the ANGLE back-end (done frequently).
    display->setGlobalDebugAnnotator();

    if (!context)
    {
        return egl::NoError();
    }

    // Pause transform feedback before making a new surface current, to workaround
    // anglebug.com/42260421
    ContextGL *glContext = GetImplAs<ContextGL>(context);
    glContext->getStateManager()->pauseTransformFeedback();

    if (drawSurface == nullptr)
    {
        ANGLE_TRY(makeCurrentSurfaceless(context));
    }

    return egl::NoError();
}

gl::Version DisplayGL::getMaxConformantESVersion() const
{
    // 3.1 support is in progress.
    return std::min(getMaxSupportedESVersion(), gl::Version(3, 0));
}

void DisplayGL::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    // Advertise robust resource initialization on all OpenGL backends for testing even though it is
    // not fully implemented.
    outExtensions->robustResourceInitializationANGLE = true;
}

egl::Error DisplayGL::makeCurrentSurfaceless(gl::Context *context)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

std::string DisplayGL::getRendererDescription()
{
    std::string rendererString        = GetRendererString(getRenderer()->getFunctions());
    const angle::FeaturesGL &features = getRenderer()->getFeatures();

    if (features.sanitizeAMDGPURendererString.enabled)
    {
        return SanitizeRendererString(rendererString);
    }
    return rendererString;
}

std::string DisplayGL::getVendorString()
{
    return GetVendorString(getRenderer()->getFunctions());
}

std::string DisplayGL::getVersionString(bool includeFullVersion)
{
    std::string versionString = GetVersionString(getRenderer()->getFunctions());
    return SanitizeVersionString(versionString,
                                 getRenderer()->getFunctions()->standard == STANDARD_GL_ES,
                                 includeFullVersion);
}

}  // namespace rx
