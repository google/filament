//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FunctionsGLX.cpp: Implements the FunctionsGLX class.

#define ANGLE_SKIP_GLX_DEFINES 1
#include "libANGLE/renderer/gl/glx/FunctionsGLX.h"
#undef ANGLE_SKIP_GLX_DEFINES

// We can only include glx.h in files which do not include ANGLE's GLES
// headers, to avoid doubly-defined GLenum macros, typedefs, etc.
#include <GL/glx.h>

#include <dlfcn.h>
#include <algorithm>

#include "common/string_utils.h"
#include "libANGLE/renderer/gl/glx/functionsglx_typedefs.h"

namespace rx
{

void *FunctionsGLX::sLibHandle = nullptr;

template <typename T>
static bool GetProc(PFNGETPROCPROC getProc, T *member, const char *name)
{
    *member = reinterpret_cast<T>(getProc(name));
    return *member != nullptr;
}

struct FunctionsGLX::GLXFunctionTable
{
    GLXFunctionTable()
        : createContextPtr(nullptr),
          destroyContextPtr(nullptr),
          makeCurrentPtr(nullptr),
          swapBuffersPtr(nullptr),
          queryExtensionPtr(nullptr),
          queryVersionPtr(nullptr),
          getCurrentContextPtr(nullptr),
          getCurrentDrawablePtr(nullptr),
          waitXPtr(nullptr),
          waitGLPtr(nullptr),
          getClientStringPtr(nullptr),
          queryExtensionsStringPtr(nullptr),
          getFBConfigsPtr(nullptr),
          chooseFBConfigPtr(nullptr),
          getFBConfigAttribPtr(nullptr),
          getVisualFromFBConfigPtr(nullptr),
          createWindowPtr(nullptr),
          destroyWindowPtr(nullptr),
          createPbufferPtr(nullptr),
          destroyPbufferPtr(nullptr),
          queryDrawablePtr(nullptr),
          createPixmapPtr(nullptr),
          destroyPixmapPtr(nullptr),
          createContextAttribsARBPtr(nullptr),
          swapIntervalEXTPtr(nullptr),
          swapIntervalMESAPtr(nullptr),
          swapIntervalSGIPtr(nullptr),
          getSyncValuesOMLPtr(nullptr),
          getMscRateOMLPtr(nullptr),
          bindTexImageEXTPtr(nullptr),
          releaseTexImageEXTPtr(nullptr)
    {}

    // GLX 1.0
    PFNGLXCREATECONTEXTPROC createContextPtr;
    PFNGLXDESTROYCONTEXTPROC destroyContextPtr;
    PFNGLXMAKECURRENTPROC makeCurrentPtr;
    PFNGLXSWAPBUFFERSPROC swapBuffersPtr;
    PFNGLXQUERYEXTENSIONPROC queryExtensionPtr;
    PFNGLXQUERYVERSIONPROC queryVersionPtr;
    PFNGLXGETCURRENTCONTEXTPROC getCurrentContextPtr;
    PFNGLXGETCURRENTDRAWABLEPROC getCurrentDrawablePtr;
    PFNGLXWAITXPROC waitXPtr;
    PFNGLXWAITGLPROC waitGLPtr;

    // GLX 1.1
    PFNGLXGETCLIENTSTRINGPROC getClientStringPtr;
    PFNGLXQUERYEXTENSIONSSTRINGPROC queryExtensionsStringPtr;

    // GLX 1.3
    PFNGLXGETFBCONFIGSPROC getFBConfigsPtr;
    PFNGLXCHOOSEFBCONFIGPROC chooseFBConfigPtr;
    PFNGLXGETFBCONFIGATTRIBPROC getFBConfigAttribPtr;
    PFNGLXGETVISUALFROMFBCONFIGPROC getVisualFromFBConfigPtr;
    PFNGLXCREATEWINDOWPROC createWindowPtr;
    PFNGLXDESTROYWINDOWPROC destroyWindowPtr;
    PFNGLXCREATEPBUFFERPROC createPbufferPtr;
    PFNGLXDESTROYPBUFFERPROC destroyPbufferPtr;
    PFNGLXQUERYDRAWABLEPROC queryDrawablePtr;
    PFNGLXCREATEPIXMAPPROC createPixmapPtr;
    PFNGLXDESTROYPIXMAPPROC destroyPixmapPtr;

    // GLX_ARB_create_context
    PFNGLXCREATECONTEXTATTRIBSARBPROC createContextAttribsARBPtr;

    // GLX_EXT_swap_control
    PFNGLXSWAPINTERVALEXTPROC swapIntervalEXTPtr;

    // GLX_MESA_swap_control
    PFNGLXSWAPINTERVALMESAPROC swapIntervalMESAPtr;

    // GLX_SGI_swap_control
    PFNGLXSWAPINTERVALSGIPROC swapIntervalSGIPtr;

    // GLX_OML_sync_control
    PFNGLXGETSYNCVALUESOMLPROC getSyncValuesOMLPtr;
    PFNGLXGETMSCRATEOMLPROC getMscRateOMLPtr;

    // GLX_EXT_texture_from_pixmap
    PFNGLXBINDTEXIMAGEEXTPROC bindTexImageEXTPtr;
    PFNGLXRELEASETEXIMAGEEXTPROC releaseTexImageEXTPtr;
};

FunctionsGLX::FunctionsGLX()
    : majorVersion(0),
      minorVersion(0),
      mXDisplay(nullptr),
      mXScreen(-1),
      mFnPtrs(new GLXFunctionTable())
{}

FunctionsGLX::~FunctionsGLX()
{
    delete mFnPtrs;
    terminate();
}

bool FunctionsGLX::initialize(Display *xDisplay, int screen, std::string *errorString)
{
    terminate();
    mXDisplay = xDisplay;
    mXScreen  = screen;

#if !defined(ANGLE_LINK_GLX)
    // Some OpenGL implementations can't handle having this library
    // handle closed while there's any X window still open against
    // which a GLXWindow was ever created.
    if (!sLibHandle)
    {
        sLibHandle = dlopen("libGL.so.1", RTLD_NOW);
        if (!sLibHandle)
        {
            *errorString = std::string("Could not dlopen libGL.so.1: ") + dlerror();
            return false;
        }
    }

    getProc = reinterpret_cast<PFNGETPROCPROC>(dlsym(sLibHandle, "glXGetProcAddress"));
    if (!getProc)
    {
        getProc = reinterpret_cast<PFNGETPROCPROC>(dlsym(sLibHandle, "glXGetProcAddressARB"));
    }
    if (!getProc)
    {
        *errorString = "Could not retrieve glXGetProcAddress";
        return false;
    }
#else
    getProc = reinterpret_cast<PFNGETPROCPROC>(glXGetProcAddress);
#endif

#define GET_PROC_OR_ERROR(MEMBER, NAME)                             \
    do                                                              \
    {                                                               \
        if (!GetProc(getProc, MEMBER, #NAME))                       \
        {                                                           \
            *errorString = "Could not load GLX entry point " #NAME; \
            return false;                                           \
        }                                                           \
    } while (0)
#if !defined(ANGLE_LINK_GLX)
#    define GET_FNPTR_OR_ERROR(MEMBER, NAME) GET_PROC_OR_ERROR(MEMBER, NAME)
#else
#    define GET_FNPTR_OR_ERROR(MEMBER, NAME) *MEMBER = NAME
#endif

    // GLX 1.0
    GET_FNPTR_OR_ERROR(&mFnPtrs->createContextPtr, glXCreateContext);
    GET_FNPTR_OR_ERROR(&mFnPtrs->destroyContextPtr, glXDestroyContext);
    GET_FNPTR_OR_ERROR(&mFnPtrs->makeCurrentPtr, glXMakeCurrent);
    GET_FNPTR_OR_ERROR(&mFnPtrs->swapBuffersPtr, glXSwapBuffers);
    GET_FNPTR_OR_ERROR(&mFnPtrs->queryExtensionPtr, glXQueryExtension);
    GET_FNPTR_OR_ERROR(&mFnPtrs->queryVersionPtr, glXQueryVersion);
    GET_FNPTR_OR_ERROR(&mFnPtrs->getCurrentContextPtr, glXGetCurrentContext);
    GET_FNPTR_OR_ERROR(&mFnPtrs->getCurrentDrawablePtr, glXGetCurrentDrawable);
    GET_FNPTR_OR_ERROR(&mFnPtrs->waitXPtr, glXWaitX);
    GET_FNPTR_OR_ERROR(&mFnPtrs->waitGLPtr, glXWaitGL);

    // GLX 1.1
    GET_FNPTR_OR_ERROR(&mFnPtrs->getClientStringPtr, glXGetClientString);
    GET_FNPTR_OR_ERROR(&mFnPtrs->queryExtensionsStringPtr, glXQueryExtensionsString);

    // Check we have a working GLX
    {
        int errorBase;
        int eventBase;
        if (!queryExtension(&errorBase, &eventBase))
        {
            *errorString = "GLX is not present.";
            return false;
        }
    }

    // Check we have a supported version of GLX
    if (!queryVersion(&majorVersion, &minorVersion))
    {
        *errorString = "Could not query the GLX version.";
        return false;
    }
    if (majorVersion != 1 || minorVersion < 3)
    {
        *errorString = "Unsupported GLX version (requires at least 1.3).";
        return false;
    }

    const char *extensions = queryExtensionsString();
    if (!extensions)
    {
        *errorString = "glXQueryExtensionsString returned NULL";
        return false;
    }
    angle::SplitStringAlongWhitespace(extensions, &mExtensions);

    // GLX 1.3
    GET_FNPTR_OR_ERROR(&mFnPtrs->getFBConfigsPtr, glXGetFBConfigs);
    GET_FNPTR_OR_ERROR(&mFnPtrs->chooseFBConfigPtr, glXChooseFBConfig);
    GET_FNPTR_OR_ERROR(&mFnPtrs->getFBConfigAttribPtr, glXGetFBConfigAttrib);
    GET_FNPTR_OR_ERROR(&mFnPtrs->getVisualFromFBConfigPtr, glXGetVisualFromFBConfig);
    GET_FNPTR_OR_ERROR(&mFnPtrs->createWindowPtr, glXCreateWindow);
    GET_FNPTR_OR_ERROR(&mFnPtrs->destroyWindowPtr, glXDestroyWindow);
    GET_FNPTR_OR_ERROR(&mFnPtrs->createPbufferPtr, glXCreatePbuffer);
    GET_FNPTR_OR_ERROR(&mFnPtrs->destroyPbufferPtr, glXDestroyPbuffer);
    GET_FNPTR_OR_ERROR(&mFnPtrs->queryDrawablePtr, glXQueryDrawable);
    GET_FNPTR_OR_ERROR(&mFnPtrs->createPixmapPtr, glXCreatePixmap);
    GET_FNPTR_OR_ERROR(&mFnPtrs->destroyPixmapPtr, glXDestroyPixmap);

    // Extensions
    if (hasExtension("GLX_ARB_create_context"))
    {
        GET_PROC_OR_ERROR(&mFnPtrs->createContextAttribsARBPtr, glXCreateContextAttribsARB);
    }
    if (hasExtension("GLX_EXT_swap_control"))
    {
        GET_PROC_OR_ERROR(&mFnPtrs->swapIntervalEXTPtr, glXSwapIntervalEXT);
    }
    if (hasExtension("GLX_MESA_swap_control"))
    {
        GET_PROC_OR_ERROR(&mFnPtrs->swapIntervalMESAPtr, glXSwapIntervalMESA);
    }
    if (hasExtension("GLX_SGI_swap_control"))
    {
        GET_PROC_OR_ERROR(&mFnPtrs->swapIntervalSGIPtr, glXSwapIntervalSGI);
    }
    if (hasExtension("GLX_OML_sync_control"))
    {
        GET_PROC_OR_ERROR(&mFnPtrs->getSyncValuesOMLPtr, glXGetSyncValuesOML);
        GET_PROC_OR_ERROR(&mFnPtrs->getMscRateOMLPtr, glXGetMscRateOML);
    }
    if (hasExtension("GLX_EXT_texture_from_pixmap"))
    {
        GET_PROC_OR_ERROR(&mFnPtrs->bindTexImageEXTPtr, glXBindTexImageEXT);
        GET_PROC_OR_ERROR(&mFnPtrs->releaseTexImageEXTPtr, glXReleaseTexImageEXT);
    }

#undef GET_FNPTR_OR_ERROR
#undef GET_PROC_OR_ERROR

    *errorString = "";
    return true;
}

void FunctionsGLX::terminate() {}

bool FunctionsGLX::hasExtension(const char *extension) const
{
    return std::find(mExtensions.begin(), mExtensions.end(), extension) != mExtensions.end();
}

Display *FunctionsGLX::getDisplay() const
{
    return mXDisplay;
}

int FunctionsGLX::getScreen() const
{
    return mXScreen;
}

// GLX functions

// GLX 1.0
glx::Context FunctionsGLX::createContext(XVisualInfo *visual, glx::Context share, bool direct) const
{
    GLXContext shareCtx = reinterpret_cast<GLXContext>(share);
    GLXContext context  = mFnPtrs->createContextPtr(mXDisplay, visual, shareCtx, direct);
    return reinterpret_cast<glx::Context>(context);
}
void FunctionsGLX::destroyContext(glx::Context context) const
{
    GLXContext ctx = reinterpret_cast<GLXContext>(context);
    mFnPtrs->destroyContextPtr(mXDisplay, ctx);
}
Bool FunctionsGLX::makeCurrent(glx::Drawable drawable, glx::Context context) const
{
    GLXContext ctx = reinterpret_cast<GLXContext>(context);
    return mFnPtrs->makeCurrentPtr(mXDisplay, drawable, ctx);
}
void FunctionsGLX::swapBuffers(glx::Drawable drawable) const
{
    mFnPtrs->swapBuffersPtr(mXDisplay, drawable);
}
Bool FunctionsGLX::queryExtension(int *errorBase, int *event) const
{
    return mFnPtrs->queryExtensionPtr(mXDisplay, errorBase, event);
}
Bool FunctionsGLX::queryVersion(int *major, int *minor) const
{
    return mFnPtrs->queryVersionPtr(mXDisplay, major, minor);
}
glx::Context FunctionsGLX::getCurrentContext() const
{
    GLXContext context = mFnPtrs->getCurrentContextPtr();
    return reinterpret_cast<glx::Context>(context);
}
glx::Drawable FunctionsGLX::getCurrentDrawable() const
{
    GLXDrawable drawable = mFnPtrs->getCurrentDrawablePtr();
    return reinterpret_cast<glx::Drawable>(drawable);
}
void FunctionsGLX::waitX() const
{
    mFnPtrs->waitXPtr();
}
void FunctionsGLX::waitGL() const
{
    mFnPtrs->waitGLPtr();
}

// GLX 1.1
const char *FunctionsGLX::getClientString(int name) const
{
    return mFnPtrs->getClientStringPtr(mXDisplay, name);
}

const char *FunctionsGLX::queryExtensionsString() const
{
    return mFnPtrs->queryExtensionsStringPtr(mXDisplay, mXScreen);
}

// GLX 1.4
glx::FBConfig *FunctionsGLX::getFBConfigs(int *nElements) const
{
    GLXFBConfig *configs = mFnPtrs->getFBConfigsPtr(mXDisplay, mXScreen, nElements);
    return reinterpret_cast<glx::FBConfig *>(configs);
}
glx::FBConfig *FunctionsGLX::chooseFBConfig(const int *attribList, int *nElements) const
{
    GLXFBConfig *configs = mFnPtrs->chooseFBConfigPtr(mXDisplay, mXScreen, attribList, nElements);
    return reinterpret_cast<glx::FBConfig *>(configs);
}
int FunctionsGLX::getFBConfigAttrib(glx::FBConfig config, int attribute, int *value) const
{
    GLXFBConfig cfg = reinterpret_cast<GLXFBConfig>(config);
    return mFnPtrs->getFBConfigAttribPtr(mXDisplay, cfg, attribute, value);
}
XVisualInfo *FunctionsGLX::getVisualFromFBConfig(glx::FBConfig config) const
{
    GLXFBConfig cfg = reinterpret_cast<GLXFBConfig>(config);
    return mFnPtrs->getVisualFromFBConfigPtr(mXDisplay, cfg);
}
GLXWindow FunctionsGLX::createWindow(glx::FBConfig config,
                                     Window window,
                                     const int *attribList) const
{
    GLXFBConfig cfg = reinterpret_cast<GLXFBConfig>(config);
    return mFnPtrs->createWindowPtr(mXDisplay, cfg, window, attribList);
}
void FunctionsGLX::destroyWindow(glx::Window window) const
{
    mFnPtrs->destroyWindowPtr(mXDisplay, window);
}
glx::Pbuffer FunctionsGLX::createPbuffer(glx::FBConfig config, const int *attribList) const
{
    GLXFBConfig cfg = reinterpret_cast<GLXFBConfig>(config);
    return mFnPtrs->createPbufferPtr(mXDisplay, cfg, attribList);
}
void FunctionsGLX::destroyPbuffer(glx::Pbuffer pbuffer) const
{
    mFnPtrs->destroyPbufferPtr(mXDisplay, pbuffer);
}
void FunctionsGLX::queryDrawable(glx::Drawable drawable, int attribute, unsigned int *value) const
{
    mFnPtrs->queryDrawablePtr(mXDisplay, drawable, attribute, value);
}

glx::Pixmap FunctionsGLX::createPixmap(glx::FBConfig config,
                                       Pixmap pixmap,
                                       const int *attribList) const
{
    GLXFBConfig cfg = reinterpret_cast<GLXFBConfig>(config);
    return mFnPtrs->createPixmapPtr(mXDisplay, cfg, pixmap, attribList);
}
void FunctionsGLX::destroyPixmap(Pixmap pixmap) const
{
    mFnPtrs->destroyPixmapPtr(mXDisplay, pixmap);
}

// GLX_ARB_create_context
glx::Context FunctionsGLX::createContextAttribsARB(glx::FBConfig config,
                                                   glx::Context shareContext,
                                                   Bool direct,
                                                   const int *attribList) const
{
    GLXContext shareCtx = reinterpret_cast<GLXContext>(shareContext);
    GLXFBConfig cfg     = reinterpret_cast<GLXFBConfig>(config);
    GLXContext ctx =
        mFnPtrs->createContextAttribsARBPtr(mXDisplay, cfg, shareCtx, direct, attribList);
    return reinterpret_cast<glx::Context>(ctx);
}

void FunctionsGLX::swapIntervalEXT(glx::Drawable drawable, int intervals) const
{
    mFnPtrs->swapIntervalEXTPtr(mXDisplay, drawable, intervals);
}

int FunctionsGLX::swapIntervalMESA(int intervals) const
{
    return mFnPtrs->swapIntervalMESAPtr(intervals);
}

int FunctionsGLX::swapIntervalSGI(int intervals) const
{
    return mFnPtrs->swapIntervalSGIPtr(intervals);
}

bool FunctionsGLX::getSyncValuesOML(glx::Drawable drawable,
                                    int64_t *ust,
                                    int64_t *msc,
                                    int64_t *sbc) const
{
    return mFnPtrs->getSyncValuesOMLPtr(mXDisplay, drawable, ust, msc, sbc);
}

bool FunctionsGLX::getMscRateOML(glx::Drawable drawable,
                                 int32_t *numerator,
                                 int32_t *denominator) const
{
    return mFnPtrs->getMscRateOMLPtr(mXDisplay, drawable, numerator, denominator);
}

void FunctionsGLX::bindTexImageEXT(glx::Drawable drawable, int buffer, const int *attribList) const
{
    mFnPtrs->bindTexImageEXTPtr(mXDisplay, drawable, buffer, attribList);
}
void FunctionsGLX::releaseTexImageEXT(glx::Drawable drawable, int buffer) const
{
    mFnPtrs->releaseTexImageEXTPtr(mXDisplay, drawable, buffer);
}
}  // namespace rx
