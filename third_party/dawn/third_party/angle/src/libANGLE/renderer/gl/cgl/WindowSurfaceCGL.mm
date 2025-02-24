//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// WindowSurfaceCGL.cpp: CGL implementation of egl::Surface for windows

#import "libANGLE/renderer/gl/cgl/WindowSurfaceCGL.h"

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <QuartzCore/QuartzCore.h>

#import "common/debug.h"
#import "common/gl/cgl/FunctionsCGL.h"
#import "libANGLE/Context.h"
#import "libANGLE/renderer/gl/FramebufferGL.h"
#import "libANGLE/renderer/gl/RendererGL.h"
#import "libANGLE/renderer/gl/StateManagerGL.h"
#import "libANGLE/renderer/gl/cgl/DisplayCGL.h"

#ifdef ANGLE_OUTSIDE_WEBKIT
#    define SWAP_CGL_LAYER_NAME ANGLESwapCGLLayer
#else
#    define SWAP_CGL_LAYER_NAME WebSwapCGLLayer
#endif

@interface SWAP_CGL_LAYER_NAME : CAOpenGLLayer {
    CGLContextObj mDisplayContext;

    bool initialized;
    rx::SharedSwapState *mSwapState;
    const rx::FunctionsGL *mFunctions;

    GLuint mReadFramebuffer;
}
- (id)initWithSharedState:(rx::SharedSwapState *)swapState
              withContext:(CGLContextObj)displayContext
            withFunctions:(const rx::FunctionsGL *)functions;
@end

@implementation SWAP_CGL_LAYER_NAME
- (id)initWithSharedState:(rx::SharedSwapState *)swapState
              withContext:(CGLContextObj)displayContext
            withFunctions:(const rx::FunctionsGL *)functions
{
    self = [super init];
    if (self != nil)
    {
        self.asynchronous = YES;
        mDisplayContext   = displayContext;

        initialized = false;
        mSwapState  = swapState;
        mFunctions  = functions;

        [self setFrame:CGRectMake(0, 0, mSwapState->textures[0].width,
                                  mSwapState->textures[0].height)];
    }
    return self;
}

- (CGLPixelFormatObj)copyCGLPixelFormatForDisplayMask:(uint32_t)mask
{
    CGLPixelFormatAttribute attribs[] = {
        kCGLPFADisplayMask, static_cast<CGLPixelFormatAttribute>(mask), kCGLPFAOpenGLProfile,
        static_cast<CGLPixelFormatAttribute>(kCGLOGLPVersion_3_2_Core),
        static_cast<CGLPixelFormatAttribute>(0)};

    CGLPixelFormatObj pixelFormat = nullptr;
    GLint numFormats              = 0;
    CGLChoosePixelFormat(attribs, &pixelFormat, &numFormats);

    return pixelFormat;
}

- (CGLContextObj)copyCGLContextForPixelFormat:(CGLPixelFormatObj)pixelFormat
{
    CGLContextObj context = nullptr;
    CGLCreateContext(pixelFormat, mDisplayContext, &context);
    return context;
}

- (BOOL)canDrawInCGLContext:(CGLContextObj)glContext
                pixelFormat:(CGLPixelFormatObj)pixelFormat
               forLayerTime:(CFTimeInterval)timeInterval
                displayTime:(const CVTimeStamp *)timeStamp
{
    BOOL result = NO;

    pthread_mutex_lock(&mSwapState->mutex);
    {
        if (mSwapState->lastRendered->swapId > mSwapState->beingPresented->swapId)
        {
            std::swap(mSwapState->lastRendered, mSwapState->beingPresented);
            result = YES;
        }
    }
    pthread_mutex_unlock(&mSwapState->mutex);

    return result;
}

- (void)drawInCGLContext:(CGLContextObj)glContext
             pixelFormat:(CGLPixelFormatObj)pixelFormat
            forLayerTime:(CFTimeInterval)timeInterval
             displayTime:(const CVTimeStamp *)timeStamp
{
    CGLSetCurrentContext(glContext);
    if (!initialized)
    {
        initialized = true;

        mFunctions->genFramebuffers(1, &mReadFramebuffer);
    }

    const auto &texture = *mSwapState->beingPresented;
    if ([self frame].size.width != texture.width || [self frame].size.height != texture.height)
    {
        [self setFrame:CGRectMake(0, 0, texture.width, texture.height)];

        // Without this, the OSX compositor / window system doesn't see the resize.
        [self setNeedsDisplay];
    }

    // TODO(cwallez) support 2.1 contexts too that don't have blitFramebuffer nor the
    // GL_DRAW_FRAMEBUFFER_BINDING query
    GLint drawFBO;
    mFunctions->getIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFBO);

    mFunctions->bindFramebuffer(GL_FRAMEBUFFER, mReadFramebuffer);
    mFunctions->framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                     texture.texture, 0);

    mFunctions->bindFramebuffer(GL_READ_FRAMEBUFFER, mReadFramebuffer);
    mFunctions->bindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFBO);
    mFunctions->blitFramebuffer(0, 0, texture.width, texture.height, 0, 0, texture.width,
                                texture.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Call the super method to flush the context
    [super drawInCGLContext:glContext
                pixelFormat:pixelFormat
               forLayerTime:timeInterval
                displayTime:timeStamp];
}
@end

namespace rx
{

WindowSurfaceCGL::WindowSurfaceCGL(const egl::SurfaceState &state,
                                   RendererGL *renderer,
                                   EGLNativeWindowType layer,
                                   CGLContextObj context)
    : SurfaceGL(state),
      mSwapLayer(nil),
      mCurrentSwapId(0),
      mLayer((__bridge CALayer *)layer),
      mContext(context),
      mFunctions(renderer->getFunctions()),
      mStateManager(renderer->getStateManager()),
      mDSRenderbuffer(0),
      mFramebufferID(0)
{
    pthread_mutex_init(&mSwapState.mutex, nullptr);
}

WindowSurfaceCGL::~WindowSurfaceCGL()
{
    EnsureCGLContextIsCurrent ensureContextCurrent(mContext);

    pthread_mutex_destroy(&mSwapState.mutex);

    if (mFramebufferID != 0)
    {
        mStateManager->deleteFramebuffer(mFramebufferID);
        mFramebufferID = 0;
    }

    if (mDSRenderbuffer != 0)
    {
        mStateManager->deleteRenderbuffer(mDSRenderbuffer);
        mDSRenderbuffer = 0;
    }

    if (mSwapLayer != nil)
    {
        [mSwapLayer removeFromSuperlayer];
        [mSwapLayer release];
        mSwapLayer = nil;
    }

    for (size_t i = 0; i < ArraySize(mSwapState.textures); ++i)
    {
        if (mSwapState.textures[i].texture != 0)
        {
            mStateManager->deleteTexture(mSwapState.textures[i].texture);
            mSwapState.textures[i].texture = 0;
        }
    }
}

egl::Error WindowSurfaceCGL::initialize(const egl::Display *display)
{
    EnsureCGLContextIsCurrent ensureContextCurrent(mContext);

    unsigned width  = getWidth();
    unsigned height = getHeight();

    for (size_t i = 0; i < ArraySize(mSwapState.textures); ++i)
    {
        mFunctions->genTextures(1, &mSwapState.textures[i].texture);
        mStateManager->bindTexture(gl::TextureType::_2D, mSwapState.textures[i].texture);
        mFunctions->texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                               GL_UNSIGNED_BYTE, nullptr);
        mSwapState.textures[i].width  = width;
        mSwapState.textures[i].height = height;
        mSwapState.textures[i].swapId = 0;
    }
    mSwapState.beingRendered  = &mSwapState.textures[0];
    mSwapState.lastRendered   = &mSwapState.textures[1];
    mSwapState.beingPresented = &mSwapState.textures[2];

    mSwapLayer = [[SWAP_CGL_LAYER_NAME alloc] initWithSharedState:&mSwapState
                                                      withContext:mContext
                                                    withFunctions:mFunctions];
    [mLayer addSublayer:mSwapLayer];
    [mSwapLayer setContentsScale:[mLayer contentsScale]];

    mFunctions->genRenderbuffers(1, &mDSRenderbuffer);
    mStateManager->bindRenderbuffer(GL_RENDERBUFFER, mDSRenderbuffer);
    mFunctions->renderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceCGL::makeCurrent(const gl::Context *context)
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceCGL::swap(const gl::Context *context)
{
    const FunctionsGL *functions = GetFunctionsGL(context);
    StateManagerGL *stateManager = GetStateManagerGL(context);

    functions->flush();
    mSwapState.beingRendered->swapId = ++mCurrentSwapId;

    pthread_mutex_lock(&mSwapState.mutex);
    {
        std::swap(mSwapState.beingRendered, mSwapState.lastRendered);
    }
    pthread_mutex_unlock(&mSwapState.mutex);

    unsigned width  = getWidth();
    unsigned height = getHeight();
    auto &texture   = *mSwapState.beingRendered;

    if (texture.width != width || texture.height != height)
    {
        stateManager->bindTexture(gl::TextureType::_2D, texture.texture);
        functions->texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                              GL_UNSIGNED_BYTE, nullptr);

        stateManager->bindRenderbuffer(GL_RENDERBUFFER, mDSRenderbuffer);
        functions->renderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

        texture.width  = width;
        texture.height = height;
    }

    ASSERT(mFramebufferID ==
           GetImplAs<FramebufferGL>(context->getFramebuffer({0}))->getFramebufferID());
    stateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
    functions->framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                    mSwapState.beingRendered->texture, 0);

    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceCGL::postSubBuffer(const gl::Context *context,
                                           EGLint x,
                                           EGLint y,
                                           EGLint width,
                                           EGLint height)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceCGL::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceCGL::bindTexImage(const gl::Context *context,
                                          gl::Texture *texture,
                                          EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error WindowSurfaceCGL::releaseTexImage(const gl::Context *context, EGLint buffer)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

void WindowSurfaceCGL::setSwapInterval(const egl::Display *display, EGLint interval)
{
    // TODO(cwallez) investigate implementing swap intervals other than 0
}

EGLint WindowSurfaceCGL::getWidth() const
{
    return static_cast<EGLint>(CGRectGetWidth([mLayer frame]) * [mLayer contentsScale]);
}

EGLint WindowSurfaceCGL::getHeight() const
{
    return static_cast<EGLint>(CGRectGetHeight([mLayer frame]) * [mLayer contentsScale]);
}

EGLint WindowSurfaceCGL::isPostSubBufferSupported() const
{
    UNIMPLEMENTED();
    return EGL_FALSE;
}

EGLint WindowSurfaceCGL::getSwapBehavior() const
{
    return EGL_BUFFER_DESTROYED;
}

egl::Error WindowSurfaceCGL::attachToFramebuffer(const gl::Context *context,
                                                 gl::Framebuffer *framebuffer)
{
    FramebufferGL *framebufferGL = GetImplAs<FramebufferGL>(framebuffer);
    ASSERT(framebufferGL->getFramebufferID() == 0);

    if (mFramebufferID == 0)
    {
        GLuint framebufferID = 0;
        mFunctions->genFramebuffers(1, &framebufferID);
        mStateManager->bindFramebuffer(GL_FRAMEBUFFER, framebufferID);
        mFunctions->framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                         mSwapState.beingRendered->texture, 0);
        mFunctions->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                            GL_RENDERBUFFER, mDSRenderbuffer);
        mFramebufferID = framebufferID;
    }
    framebufferGL->setFramebufferID(mFramebufferID);
    return egl::NoError();
}

egl::Error WindowSurfaceCGL::detachFromFramebuffer(const gl::Context *context,
                                                   gl::Framebuffer *framebuffer)
{
    FramebufferGL *framebufferGL = GetImplAs<FramebufferGL>(framebuffer);
    ASSERT(framebufferGL->getFramebufferID() == mFramebufferID);
    framebufferGL->setFramebufferID(0);
    return egl::NoError();
}

}  // namespace rx
