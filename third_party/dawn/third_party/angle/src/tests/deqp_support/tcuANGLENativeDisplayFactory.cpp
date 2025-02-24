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

#include "egluNativeDisplay.hpp"

#include "tcuANGLENativeDisplayFactory.h"

#include "deClock.h"
#include "deMemory.h"
#include "egluDefs.hpp"
#include "eglwLibrary.hpp"
#include "tcuTexture.hpp"
#include "util/OSPixmap.h"
#include "util/OSWindow.h"
#include "util/autogen/angle_features_autogen.h"

// clang-format off
#if (DE_OS == DE_OS_WIN32)
    #define ANGLE_EGL_LIBRARY_FULL_NAME ANGLE_EGL_LIBRARY_NAME ".dll"
#elif (DE_OS == DE_OS_UNIX) || (DE_OS == DE_OS_ANDROID)
    #define ANGLE_EGL_LIBRARY_FULL_NAME ANGLE_EGL_LIBRARY_NAME ".so"
#elif (DE_OS == DE_OS_OSX)
    #define ANGLE_EGL_LIBRARY_FULL_NAME ANGLE_EGL_LIBRARY_NAME ".dylib"
#else
    #error "Unsupported platform"
#endif
// clang-format on

#if defined(ANGLE_USE_X11)
#    include <X11/Xlib.h>
#endif

#if defined(ANGLE_USE_WAYLAND)
#    include <wayland-client.h>
#    include <wayland-egl-backend.h>
#endif

#if (DE_OS == DE_OS_ANDROID)
#    define NATIVE_EGL_LIBRARY_FULL_NAME "libEGL.so"
#endif

namespace tcu
{
namespace
{

template <typename destType, typename sourceType>
destType bitCast(sourceType source)
{
    constexpr size_t copySize =
        sizeof(destType) < sizeof(sourceType) ? sizeof(destType) : sizeof(sourceType);
    destType output(0);
    memcpy(&output, &source, copySize);
    return output;
}

enum
{
    DEFAULT_SURFACE_WIDTH  = 400,
    DEFAULT_SURFACE_HEIGHT = 300,
};

constexpr eglu::NativeDisplay::Capability kDisplayCapabilities =
    static_cast<eglu::NativeDisplay::Capability>(
        eglu::NativeDisplay::CAPABILITY_GET_DISPLAY_PLATFORM |
        eglu::NativeDisplay::CAPABILITY_GET_DISPLAY_PLATFORM_EXT);
constexpr eglu::NativePixmap::Capability kBitmapCapabilities =
    eglu::NativePixmap::CAPABILITY_CREATE_SURFACE_LEGACY;
constexpr eglu::NativeWindow::Capability kWindowCapabilities =
    static_cast<eglu::NativeWindow::Capability>(
#if (DE_OS == DE_OS_WIN32)
        eglu::NativeWindow::CAPABILITY_READ_SCREEN_PIXELS |
#endif
        eglu::NativeWindow::CAPABILITY_CREATE_SURFACE_LEGACY |
        eglu::NativeWindow::CAPABILITY_GET_SURFACE_SIZE |
        eglu::NativeWindow::CAPABILITY_GET_SCREEN_SIZE |
        eglu::NativeWindow::CAPABILITY_SET_SURFACE_SIZE |
        eglu::NativeWindow::CAPABILITY_CHANGE_VISIBILITY |
        eglu::NativeWindow::CAPABILITY_CREATE_SURFACE_PLATFORM_EXTENSION);

class ANGLENativeDisplay : public eglu::NativeDisplay
{
  public:
    explicit ANGLENativeDisplay(EGLNativeDisplayType display,
                                std::vector<eglw::EGLAttrib> attribs,
                                const EGLenum platformType,
                                const char *eglLibraryName);
    ~ANGLENativeDisplay() override = default;

    void *getPlatformNative() override
    {
        // On OSX 64bits mDeviceContext is a 32 bit integer, so we can't simply
        // use reinterpret_cast<void*>.
        return bitCast<void *>(mDeviceContext);
    }
    const eglw::EGLAttrib *getPlatformAttributes() const override
    {
        return &mPlatformAttributes[0];
    }
    const eglw::Library &getLibrary() const override { return mLibrary; }

    EGLNativeDisplayType getDeviceContext() const { return mDeviceContext; }

  private:
    EGLNativeDisplayType mDeviceContext;
    eglw::DefaultLibrary mLibrary;
    std::vector<eglw::EGLAttrib> mPlatformAttributes;
};

class NativePixmapFactory : public eglu::NativePixmapFactory
{
  public:
    NativePixmapFactory();
    ~NativePixmapFactory() override = default;

    eglu::NativePixmap *createPixmap(eglu::NativeDisplay *nativeDisplay,
                                     int width,
                                     int height) const override;
    eglu::NativePixmap *createPixmap(eglu::NativeDisplay *nativeDisplay,
                                     eglw::EGLDisplay display,
                                     eglw::EGLConfig config,
                                     const eglw::EGLAttrib *attribList,
                                     int width,
                                     int height) const override;
};

class NativePixmap : public eglu::NativePixmap
{
  public:
    NativePixmap(EGLNativeDisplayType display, int width, int height, int bitDepth);
    virtual ~NativePixmap();

    eglw::EGLNativePixmapType getLegacyNative() override;

  private:
    OSPixmap *mPixmap;
};

class NativeWindowFactory : public eglu::NativeWindowFactory
{
  public:
    explicit NativeWindowFactory(EventState *eventState, uint32_t preRotation);
    ~NativeWindowFactory() override = default;

    eglu::NativeWindow *createWindow(eglu::NativeDisplay *nativeDisplay,
                                     const eglu::WindowParams &params) const override;
    eglu::NativeWindow *createWindow(eglu::NativeDisplay *nativeDisplay,
                                     eglw::EGLDisplay display,
                                     eglw::EGLConfig config,
                                     const eglw::EGLAttrib *attribList,
                                     const eglu::WindowParams &params) const override;

  private:
    EventState *mEvents;
    uint32_t mPreRotation;
};

class NativeWindow : public eglu::NativeWindow
{
  public:
    NativeWindow(ANGLENativeDisplay *nativeDisplay,
                 const eglu::WindowParams &params,
                 EventState *eventState,
                 uint32_t preRotation);
    ~NativeWindow() override;

    eglw::EGLNativeWindowType getLegacyNative() override;
    void *getPlatformExtension() override;
    IVec2 getSurfaceSize() const override;
    IVec2 getScreenSize() const override { return getSurfaceSize(); }
    void processEvents() override;
    void setSurfaceSize(IVec2 size) override;
    void setVisibility(eglu::WindowParams::Visibility visibility) override;
    void readScreenPixels(tcu::TextureLevel *dst) const override;

  private:
    OSWindow *mWindow;
    EventState *mEvents;
    uint32_t mPreRotation;
};

// ANGLE NativeDisplay

ANGLENativeDisplay::ANGLENativeDisplay(EGLNativeDisplayType display,
                                       std::vector<EGLAttrib> attribs,
                                       const EGLenum platformType,
                                       const char *eglLibraryName)
    : eglu::NativeDisplay(kDisplayCapabilities, platformType, "EGL_EXT_platform_base"),
      mDeviceContext(display),
      mLibrary(eglLibraryName),
      mPlatformAttributes(std::move(attribs))
{}

// NativePixmap

NativePixmap::NativePixmap(EGLNativeDisplayType display, int width, int height, int bitDepth)
    : eglu::NativePixmap(kBitmapCapabilities), mPixmap(CreateOSPixmap())
{
#if (DE_OS != DE_OS_UNIX)
    throw tcu::NotSupportedError("Pixmap not supported");
#else
    if (!mPixmap)
    {
        throw ResourceError("Failed to create pixmap", DE_NULL, __FILE__, __LINE__);
    }

    if (!mPixmap->initialize(display, width, height, bitDepth))
    {
        throw ResourceError("Failed to initialize pixmap", DE_NULL, __FILE__, __LINE__);
    }
#endif
}

NativePixmap::~NativePixmap()
{
    delete mPixmap;
}

eglw::EGLNativePixmapType NativePixmap::getLegacyNative()
{
    return reinterpret_cast<eglw::EGLNativePixmapType>(mPixmap->getNativePixmap());
}

// NativePixmapFactory

NativePixmapFactory::NativePixmapFactory()
    : eglu::NativePixmapFactory("bitmap", "ANGLE Bitmap", kBitmapCapabilities)
{}

eglu::NativePixmap *NativePixmapFactory::createPixmap(eglu::NativeDisplay *nativeDisplay,
                                                      eglw::EGLDisplay display,
                                                      eglw::EGLConfig config,
                                                      const eglw::EGLAttrib *attribList,
                                                      int width,
                                                      int height) const
{
    const eglw::Library &egl = nativeDisplay->getLibrary();
    int nativeVisual         = 0;

    DE_ASSERT(display != EGL_NO_DISPLAY);

    egl.getConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &nativeVisual);
    EGLU_CHECK_MSG(egl, "eglGetConfigAttrib()");

    return new NativePixmap(dynamic_cast<ANGLENativeDisplay *>(nativeDisplay)->getDeviceContext(),
                            width, height, nativeVisual);
}

eglu::NativePixmap *NativePixmapFactory::createPixmap(eglu::NativeDisplay *nativeDisplay,
                                                      int width,
                                                      int height) const
{
    const int defaultDepth = 32;
    return new NativePixmap(dynamic_cast<ANGLENativeDisplay *>(nativeDisplay)->getDeviceContext(),
                            width, height, defaultDepth);
}

// NativeWindowFactory

NativeWindowFactory::NativeWindowFactory(EventState *eventState, uint32_t preRotation)
    : eglu::NativeWindowFactory("window", "ANGLE Window", kWindowCapabilities),
      mEvents(eventState),
      mPreRotation(preRotation)
{}

eglu::NativeWindow *NativeWindowFactory::createWindow(eglu::NativeDisplay *nativeDisplay,
                                                      const eglu::WindowParams &params) const
{
    DE_ASSERT(false);
    return nullptr;
}

eglu::NativeWindow *NativeWindowFactory::createWindow(eglu::NativeDisplay *nativeDisplay,
                                                      eglw::EGLDisplay display,
                                                      eglw::EGLConfig config,
                                                      const eglw::EGLAttrib *attribList,
                                                      const eglu::WindowParams &params) const
{
    return new NativeWindow(dynamic_cast<ANGLENativeDisplay *>(nativeDisplay), params, mEvents,
                            mPreRotation);
}

// NativeWindow

NativeWindow::NativeWindow(ANGLENativeDisplay *nativeDisplay,
                           const eglu::WindowParams &params,
                           EventState *eventState,
                           uint32_t preRotation)
    : eglu::NativeWindow(kWindowCapabilities),
      mWindow(OSWindow::New()),
      mEvents(eventState),
      mPreRotation(preRotation)
{
    int osWindowWidth =
        params.width == eglu::WindowParams::SIZE_DONT_CARE ? DEFAULT_SURFACE_WIDTH : params.width;
    int osWindowHeight = params.height == eglu::WindowParams::SIZE_DONT_CARE
                             ? DEFAULT_SURFACE_HEIGHT
                             : params.height;

    if (mPreRotation == 90 || mPreRotation == 270)
    {
        std::swap(osWindowWidth, osWindowHeight);
    }

    mWindow->setNativeDisplay(nativeDisplay->getDeviceContext());
    bool initialized = mWindow->initialize("dEQP ANGLE Tests", osWindowWidth, osWindowHeight);
    TCU_CHECK(initialized);

    if (params.visibility != eglu::WindowParams::VISIBILITY_DONT_CARE)
        NativeWindow::setVisibility(params.visibility);
}

void NativeWindow::setVisibility(eglu::WindowParams::Visibility visibility)
{
    switch (visibility)
    {
        case eglu::WindowParams::VISIBILITY_HIDDEN:
            mWindow->setVisible(false);
            break;

        case eglu::WindowParams::VISIBILITY_VISIBLE:
        case eglu::WindowParams::VISIBILITY_FULLSCREEN:
            mWindow->setVisible(true);
            break;

        default:
            DE_ASSERT(false);
    }
}

NativeWindow::~NativeWindow()
{
    OSWindow::Delete(&mWindow);
}

eglw::EGLNativeWindowType NativeWindow::getLegacyNative()
{
    return reinterpret_cast<eglw::EGLNativeWindowType>(mWindow->getNativeWindow());
}

void *NativeWindow::getPlatformExtension()
{
    return mWindow->getPlatformExtension();
}

IVec2 NativeWindow::getSurfaceSize() const
{
    int width  = mWindow->getWidth();
    int height = mWindow->getHeight();

    if (mPreRotation == 90 || mPreRotation == 270)
    {
        // Return the original dimensions dEQP asked for.  This ensures that the dEQP code is never
        // aware of the window actually being rotated.
        std::swap(width, height);
    }

    return IVec2(width, height);
}

void NativeWindow::processEvents()
{
    mWindow->messageLoop();

    // Look for a quit event to forward to the EventState
    Event event = {};
    while (mWindow->popEvent(&event))
    {
        if (event.Type == Event::EVENT_CLOSED)
        {
            mEvents->signalQuitEvent();
        }
    }
}

void NativeWindow::setSurfaceSize(IVec2 size)
{
    int osWindowWidth  = size.x();
    int osWindowHeight = size.y();

    if (mPreRotation == 90 || mPreRotation == 270)
    {
        std::swap(osWindowWidth, osWindowHeight);
    }

    mWindow->resize(osWindowWidth, osWindowHeight);
}

void NativeWindow::readScreenPixels(tcu::TextureLevel *dst) const
{
    dst->setStorage(TextureFormat(TextureFormat::BGRA, TextureFormat::UNORM_INT8),
                    mWindow->getWidth(), mWindow->getHeight());
    if (!mWindow->takeScreenshot(reinterpret_cast<uint8_t *>(dst->getAccess().getDataPtr())))
    {
        throw InternalError("Failed to read screen pixels", DE_NULL, __FILE__, __LINE__);
    }

    if (mPreRotation != 0)
    {
        throw InternalError("Read screen pixels with prerotation is not supported", DE_NULL,
                            __FILE__, __LINE__);
    }
}

}  // namespace

ANGLENativeDisplayFactory::ANGLENativeDisplayFactory(
    const std::string &name,
    const std::string &description,
    std::vector<eglw::EGLAttrib> platformAttributes,
    EventState *eventState,
    const EGLenum platformType)
    : eglu::NativeDisplayFactory(name,
                                 description,
                                 kDisplayCapabilities,
                                 platformType,
                                 "EGL_EXT_platform_base"),
      mNativeDisplay(bitCast<eglw::EGLNativeDisplayType>(EGL_DEFAULT_DISPLAY)),
      mPlatformAttributes(std::move(platformAttributes)),
      mPlatformType(platformType)
{
#if (DE_OS == DE_OS_UNIX)
#    if defined(ANGLE_USE_X11)
    // Make sure to only open the X display once so that it can be used by the EGL display as well
    // as pixmaps
    mNativeDisplay = bitCast<eglw::EGLNativeDisplayType>(XOpenDisplay(nullptr));
#    endif  // ANGLE_USE_X11

#    if defined(ANGLE_USE_WAYLAND)
    if (mNativeDisplay == 0)
    {
        mNativeDisplay = bitCast<eglw::EGLNativeDisplayType>(wl_display_connect(nullptr));
    }
#    endif  // ANGLE_USE_WAYLAND
#endif      // (DE_OS == DE_OS_UNIX)

    // If pre-rotating, let NativeWindowFactory know.
    uint32_t preRotation = 0;
    for (size_t attrIndex = 0;
         attrIndex < mPlatformAttributes.size() && mPlatformAttributes[attrIndex] != EGL_NONE;
         attrIndex += 2)
    {
        if (mPlatformAttributes[attrIndex] != EGL_FEATURE_OVERRIDES_ENABLED_ANGLE)
        {
            continue;
        }

        const char **enabledFeatures =
            reinterpret_cast<const char **>(mPlatformAttributes[attrIndex + 1]);
        DE_ASSERT(enabledFeatures != nullptr && *enabledFeatures != nullptr);

        for (; *enabledFeatures; ++enabledFeatures)
        {
            if (strcmp(enabledFeatures[0],
                       angle::GetFeatureName(angle::Feature::EmulatedPrerotation90)) == 0)
            {
                preRotation = 90;
            }
            else if (strcmp(enabledFeatures[0],
                            angle::GetFeatureName(angle::Feature::EmulatedPrerotation180)) == 0)
            {
                preRotation = 180;
            }
            else if (strcmp(enabledFeatures[0],
                            angle::GetFeatureName(angle::Feature::EmulatedPrerotation270)) == 0)
            {
                preRotation = 270;
            }
        }
        break;
    }

    m_nativeWindowRegistry.registerFactory(new NativeWindowFactory(eventState, preRotation));
    m_nativePixmapRegistry.registerFactory(new NativePixmapFactory());
}

ANGLENativeDisplayFactory::~ANGLENativeDisplayFactory() = default;

eglu::NativeDisplay *ANGLENativeDisplayFactory::createDisplay(
    const eglw::EGLAttrib *attribList) const
{
    DE_UNREF(attribList);
    if (mPlatformType == EGL_PLATFORM_ANGLE_ANGLE)
    {
        return new ANGLENativeDisplay(bitCast<EGLNativeDisplayType>(mNativeDisplay),
                                      mPlatformAttributes, mPlatformType,
                                      ANGLE_EGL_LIBRARY_FULL_NAME);
    }
#if (DE_OS == DE_OS_ANDROID)
    else if (mPlatformType == EGL_PLATFORM_ANDROID_KHR)
    {
        return new ANGLENativeDisplay(bitCast<EGLNativeDisplayType>(mNativeDisplay),
                                      mPlatformAttributes, mPlatformType,
                                      NATIVE_EGL_LIBRARY_FULL_NAME);
    }
#endif
    else
    {
        throw InternalError("unsupported platform type", DE_NULL, __FILE__, __LINE__);
    }
}

}  // namespace tcu
