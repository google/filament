/*-------------------------------------------------------------------------
 * drawElements Quality Program Tester Core
 * ----------------------------------------
 *
 * Copyright 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "tcuANGLEPlatform.h"

#include "egluGLContextFactory.hpp"
#include "gluPlatform.hpp"
#include "tcuANGLENativeDisplayFactory.h"
#include "tcuDefs.hpp"
#include "tcuNullContextFactory.hpp"
#include "tcuPlatform.hpp"
#include "util/autogen/angle_features_autogen.h"
#include "util/test_utils.h"

#ifndef _EGLUPLATFORM_HPP
#    include "egluPlatform.hpp"
#endif

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "angle_deqp_libtester.h"

#if (DE_OS == DE_OS_WIN32)
#    include "tcuWGLContextFactory.hpp"
#    include "tcuWin32EGLNativeDisplayFactory.hpp"
#endif  // (DE_OS == DE_OS_WIN32)

#if (DE_OS == DE_OS_UNIX)
#    include "tcuLnxX11EglDisplayFactory.hpp"
#endif  // (DE_OKS == DE_OS_UNIX)

static_assert(EGL_DONT_CARE == -1, "Unexpected value for EGL_DONT_CARE");

namespace tcu
{
class ANGLEPlatform : public tcu::Platform, private glu::Platform, private eglu::Platform
{
  public:
    ANGLEPlatform(angle::LogErrorFunc logErrorFunc,
                  uint32_t preRotation,
                  dEQPDriverOption driverOption);
    ~ANGLEPlatform();

    bool processEvents() override;

    const glu::Platform &getGLPlatform() const override
    {
        return static_cast<const glu::Platform &>(*this);
    }
    const eglu::Platform &getEGLPlatform() const override
    {
        return static_cast<const eglu::Platform &>(*this);
    }

  private:
    // Note: -1 represents EGL_DONT_CARE, but we don't have the EGL headers here.
    std::vector<eglw::EGLAttrib> initAttribs(eglw::EGLAttrib type,
                                             eglw::EGLAttrib deviceType   = -1,
                                             eglw::EGLAttrib majorVersion = -1,
                                             eglw::EGLAttrib minorVersion = -1);

    EventState mEvents;
    angle::PlatformMethods mPlatformMethods;
    std::vector<const char *> mEnableFeatureOverrides;

#if (DE_OS == DE_OS_UNIX)
    lnx::EventState mLnxEventState;
#endif
};

ANGLEPlatform::ANGLEPlatform(angle::LogErrorFunc logErrorFunc,
                             uint32_t preRotation,
                             dEQPDriverOption driverOption)
{
    angle::SetLowPriorityProcess();

    mPlatformMethods.logError = logErrorFunc;

    // Enable non-conformant ES versions and extensions for testing.  Our test expectations would
    // suppress failing tests, but allowing continuous testing of the pieces that are implemented.
    mEnableFeatureOverrides.push_back(
        angle::GetFeatureName(angle::Feature::ExposeNonConformantExtensionsAndVersions));

    // Create pre-rotation attributes.
    switch (preRotation)
    {
        case 90:
            mEnableFeatureOverrides.push_back(
                angle::GetFeatureName(angle::Feature::EmulatedPrerotation90));
            break;
        case 180:
            mEnableFeatureOverrides.push_back(
                angle::GetFeatureName(angle::Feature::EmulatedPrerotation180));
            break;
        case 270:
            mEnableFeatureOverrides.push_back(
                angle::GetFeatureName(angle::Feature::EmulatedPrerotation270));
            break;
        default:
            break;
    }

    mEnableFeatureOverrides.push_back(nullptr);

#if (DE_OS == DE_OS_WIN32)
    {
        std::vector<eglw::EGLAttrib> d3d11Attribs = initAttribs(
            EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE);

        auto *d3d11Factory = new ANGLENativeDisplayFactory("angle-d3d11", "ANGLE D3D11 Display",
                                                           d3d11Attribs, &mEvents);
        m_nativeDisplayFactoryRegistry.registerFactory(d3d11Factory);
    }

    {
        std::vector<eglw::EGLAttrib> d3d11Attribs =
            initAttribs(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
                        EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_REFERENCE_ANGLE);

        auto *d3d11Factory = new ANGLENativeDisplayFactory(
            "angle-d3d11-ref", "ANGLE D3D11 Reference Display", d3d11Attribs, &mEvents);
        m_nativeDisplayFactoryRegistry.registerFactory(d3d11Factory);
    }

    {
        std::vector<eglw::EGLAttrib> d3d9Attribs = initAttribs(
            EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE);

        auto *d3d9Factory = new ANGLENativeDisplayFactory("angle-d3d9", "ANGLE D3D9 Display",
                                                          d3d9Attribs, &mEvents);
        m_nativeDisplayFactoryRegistry.registerFactory(d3d9Factory);
    }

    m_nativeDisplayFactoryRegistry.registerFactory(
        new win32::EGLNativeDisplayFactory(GetModuleHandle(nullptr)));
#endif  // (DE_OS == DE_OS_WIN32)

#if defined(ANGLE_USE_GBM) || (DE_OS == DE_OS_ANDROID) || (DE_OS == DE_OS_WIN32)
    {
        std::vector<eglw::EGLAttrib> glesAttribs =
            initAttribs(EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE);

        auto *glesFactory = new ANGLENativeDisplayFactory("angle-gles", "ANGLE OpenGL ES Display",
                                                          glesAttribs, &mEvents);
        m_nativeDisplayFactoryRegistry.registerFactory(glesFactory);
    }
#endif

    {
        std::vector<eglw::EGLAttrib> glAttribs = initAttribs(EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE);

        auto *glFactory =
            new ANGLENativeDisplayFactory("angle-gl", "ANGLE OpenGL Display", glAttribs, &mEvents);
        m_nativeDisplayFactoryRegistry.registerFactory(glFactory);
    }

#if (DE_OS == DE_OS_ANDROID)
    {
        if (driverOption == dEQPDriverOption::ANGLE)
        {
            std::vector<eglw::EGLAttrib> vkAttribs =
                initAttribs(EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE);
            auto *vkFactory =
                new ANGLENativeDisplayFactory("angle-vulkan", "ANGLE Vulkan Display", vkAttribs,
                                              &mEvents, EGL_PLATFORM_ANGLE_ANGLE);
            m_nativeDisplayFactoryRegistry.registerFactory(vkFactory);
        }
        else
        {
            std::vector<eglw::EGLAttrib> nativeGlesAttribs = {EGL_NONE};
            auto *nativeGLESFactory                        = new ANGLENativeDisplayFactory(
                "native-gles", "Native GLES Display", nativeGlesAttribs, &mEvents,
                EGL_PLATFORM_ANDROID_KHR);
            m_nativeDisplayFactoryRegistry.registerFactory(nativeGLESFactory);
        }
    }
#endif

#if ((DE_OS == DE_OS_WIN32) || (DE_OS == DE_OS_UNIX))
    {
        std::vector<eglw::EGLAttrib> vkAttribs = initAttribs(EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE);

        auto *vkFactory = new ANGLENativeDisplayFactory("angle-vulkan", "ANGLE Vulkan Display",
                                                        vkAttribs, &mEvents);
        m_nativeDisplayFactoryRegistry.registerFactory(vkFactory);
    }
#endif

#if (DE_OS == DE_OS_WIN32) || (DE_OS == DE_OS_UNIX) || (DE_OS == DE_OS_OSX)
    {
        std::vector<eglw::EGLAttrib> swsAttribs = initAttribs(
            EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE);
        m_nativeDisplayFactoryRegistry.registerFactory(new ANGLENativeDisplayFactory(
            "angle-swiftshader", "ANGLE SwiftShader Display", swsAttribs, &mEvents));
    }
#endif

#if (DE_OS == DE_OS_WIN32) || (DE_OS == DE_OS_UNIX) || (DE_OS == DE_OS_OSX)
    {
        std::vector<eglw::EGLAttrib> webgpuAttribs =
            initAttribs(EGL_PLATFORM_ANGLE_TYPE_WEBGPU_ANGLE);

        auto *webgpuFactory = new ANGLENativeDisplayFactory("angle-webgpu", "ANGLE WebGPU Display",
                                                            webgpuAttribs, &mEvents);
        m_nativeDisplayFactoryRegistry.registerFactory(webgpuFactory);
    }
#endif

#if (DE_OS == DE_OS_OSX)
    {
        std::vector<eglw::EGLAttrib> mtlAttribs = initAttribs(EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE);

        auto *mtlFactory = new ANGLENativeDisplayFactory("angle-metal", "ANGLE Metal Display",
                                                         mtlAttribs, &mEvents);
        m_nativeDisplayFactoryRegistry.registerFactory(mtlFactory);
    }
#endif

    {
        std::vector<eglw::EGLAttrib> nullAttribs = initAttribs(EGL_PLATFORM_ANGLE_TYPE_NULL_ANGLE);

        auto *nullFactory = new ANGLENativeDisplayFactory("angle-null", "ANGLE NULL Display",
                                                          nullAttribs, &mEvents);
        m_nativeDisplayFactoryRegistry.registerFactory(nullFactory);
    }

#if (DE_OS == DE_OS_UNIX)
    m_nativeDisplayFactoryRegistry.registerFactory(
        lnx::x11::egl::createDisplayFactory(mLnxEventState));
#endif

    m_contextFactoryRegistry.registerFactory(
        new eglu::GLContextFactory(m_nativeDisplayFactoryRegistry));

    // Add Null context type for use in generating case lists
    m_contextFactoryRegistry.registerFactory(new null::NullGLContextFactory());

#if (DE_OS == DE_OS_WIN32)
    // The wgl::ContextFactory can throw an exception when it fails to load WGL extension functions.
    // Fail gracefully by catching the exception, which prevents adding the factory to the registry.
    try
    {
        m_contextFactoryRegistry.registerFactory(new wgl::ContextFactory(GetModuleHandle(nullptr)));
    }
    catch (tcu::Exception e)
    {}
#endif
}

ANGLEPlatform::~ANGLEPlatform() {}

bool ANGLEPlatform::processEvents()
{
    return !mEvents.quitSignaled();
}

std::vector<eglw::EGLAttrib> ANGLEPlatform::initAttribs(eglw::EGLAttrib type,
                                                        eglw::EGLAttrib deviceType,
                                                        eglw::EGLAttrib majorVersion,
                                                        eglw::EGLAttrib minorVersion)
{
    std::vector<eglw::EGLAttrib> attribs;

    attribs.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
    attribs.push_back(type);

    if (deviceType != EGL_DONT_CARE)
    {
        attribs.push_back(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE);
        attribs.push_back(deviceType);
    }

    if (majorVersion != EGL_DONT_CARE)
    {
        attribs.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE);
        attribs.push_back(majorVersion);
    }

    if (minorVersion != EGL_DONT_CARE)
    {
        attribs.push_back(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE);
        attribs.push_back(minorVersion);
    }

    if (mPlatformMethods.logError)
    {
        static_assert(sizeof(eglw::EGLAttrib) == sizeof(angle::PlatformMethods *),
                      "Unexpected pointer size");
        attribs.push_back(EGL_PLATFORM_ANGLE_PLATFORM_METHODS_ANGLEX);
        attribs.push_back(reinterpret_cast<eglw::EGLAttrib>(&mPlatformMethods));
    }

    if (!mEnableFeatureOverrides.empty())
    {
        attribs.push_back(EGL_FEATURE_OVERRIDES_ENABLED_ANGLE);
        attribs.push_back(reinterpret_cast<EGLAttrib>(mEnableFeatureOverrides.data()));
    }

    attribs.push_back(EGL_NONE);
    return attribs;
}
}  // namespace tcu

// Create platform
tcu::Platform *CreateANGLEPlatform(angle::LogErrorFunc logErrorFunc,
                                   uint32_t preRotation,
                                   dEQPDriverOption driverOption)
{
    return new tcu::ANGLEPlatform(logErrorFunc, preRotation, driverOption);
}

tcu::Platform *createPlatform()
{
    return CreateANGLEPlatform(nullptr, 0, dEQPDriverOption::ANGLE);
}
