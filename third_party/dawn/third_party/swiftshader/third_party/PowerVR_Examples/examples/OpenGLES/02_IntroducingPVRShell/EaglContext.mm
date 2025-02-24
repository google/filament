/*!
\brief Contains implementation code for the EAGL(iOS) version of the PlatformContext.
\file PVRUtils/EAGL/EaglPlatformContext.mm
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "EglContext.h"
#import <QuartzCore/CAEAGLLayer.h>
using namespace pvr;

@interface APIView : UIView
{
}

@end

// CLASS IMPLEMENTATION
@implementation APIView

+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

@end

// TODO: Don't make these globals
//GLint numDiscardAttachments	= 0;
//GLenum discardAttachments[3];
GLuint framebuffer = 0;
GLuint renderbuffer = 0;
GLuint depthBuffer = 0;

GLuint msaaFrameBuffer = 0;
GLuint msaaColorBuffer = 0;
GLuint msaaDepthBuffer = 0;

//TODO: Lots of error checking.

void EglContext::release()
{
    glDeleteFramebuffers(1,&framebuffer);
    glDeleteRenderbuffers(1,&renderbuffer);
    glDeleteRenderbuffers(1,&depthBuffer);
    glDeleteFramebuffers(1,&msaaFrameBuffer);
    glDeleteRenderbuffers(1, &msaaColorBuffer);
    glDeleteRenderbuffers(1,&msaaDepthBuffer);
    framebuffer = renderbuffer = depthBuffer = msaaFrameBuffer = msaaColorBuffer = msaaDepthBuffer = 0;
    _maxApiVersion = Api::Unspecified;
}

void EglContext::populateMaxApiVersion()
{
    if(_maxApiVersion != Api::Unspecified){ return; }
    _maxApiVersion = Api::Unspecified;
    Api graphicsapi = Api::OpenGLESMaxVersion;
    while (graphicsapi > Api::Unspecified)
    {
        const char* esversion = (graphicsapi == Api::OpenGLES31 ? "3.1" : graphicsapi == Api::OpenGLES3 ? "3.0" : graphicsapi == Api::OpenGLES2 ?
                                                                                                              "2.0" : "UNKNOWN_VERSION");

        EAGLContext* context = NULL;
        // create our context
        switch(graphicsapi)
        {
        case Api::OpenGLES31:
            break;
        case Api::OpenGLES2:
            context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
            break;
        case Api::OpenGLES3:
            context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
            break;
        default:assertion(false, "Unsupported Api");
        }

        if(context != nil)
        {
            _maxApiVersion = graphicsapi;
            Log(LogLevel::Verbose, "Maximum API level detected: OpenGL ES %s", esversion);
            return;
        }
        else
        {
            Log(LogLevel::Verbose, "OpenGL ES %s NOT supported. Trying lower version...", esversion);
        }
        graphicsapi = Api((uint32_t)graphicsapi - 1);
    }
    Log(LogLevel::Critical, "=== FATAL: COULD NOT FIND COMPATIBILITY WITH ANY OPENGL ES VERSION ===");

}

static pvr::Result init(EglContext& platformContext,const NativeDisplay& nativeDisplay,
    const NativeWindow& nativeWindow, DisplayAttributes& attributes, const pvr::Api& apiContextType,
    UIView** outView, EAGLContext** outContext)
{
    pvr::Result result = pvr::Result::Success;
    pvr::Api apiRequest = apiContextType;
    if(apiContextType == pvr::Api::Unspecified || !platformContext.isApiSupported(apiContextType))
    {
        apiRequest = platformContext._maxApiVersion;
        //platformContext.getOsManager().setApiTypeRequired(apiRequest);
        Log(LogLevel::Information, "Unspecified target API. Setting to max API level, which is %s",
            apiName(apiRequest));
    }
    
        UIView* view;
        EAGLContext* context;
        UIWindow* nw = (__bridge UIWindow*)nativeWindow;
        // UIWindow* nw = static_cast<UIWindow*>(nativeWindow);
        // Initialize our UIView surface and add it to our view
        view = [[APIView alloc] initWithFrame:[nw bounds]];
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *) [view layer];
        eaglLayer.opaque = YES;
        
        NSString* pixelFormat;
        
		bool supportSRGB = (&kEAGLColorFormatSRGBA8 != NULL);
		bool requestSRGB = attributes.frameBufferSrgb;
		bool request32bitFbo = (attributes.redBits > 5 || attributes.greenBits > 6 || attributes.blueBits > 5 || attributes.alphaBits > 0);
		if (requestSRGB && !supportSRGB) {
            Log(LogLevel::Warning, "sRGB window backbuffer requested, but an SRGB backbuffer is not supported. Creating linear RGB backbuffer.");
            }
		if (requestSRGB && supportSRGB) {
            pixelFormat = kEAGLColorFormatSRGBA8;
            Log(LogLevel::Information, "sRGB window backbuffer requested. Creating linear RGB backbuffer (kEAGLColorFormatSRGBA8).");
            }
		else if (request32bitFbo) 
        { 
            pixelFormat = kEAGLColorFormatRGBA8; 
            Log(LogLevel::Information, "32-bit window backbuffer requested. Creating linear RGBA backbuffer (kEAGLColorFormatRGBA8).");
        }
		else {
            Log(LogLevel::Information, "Invalid backbuffer requested. Creating linear RGB backbuffer (kEAGLColorFormatRGB565).");
            pixelFormat = kEAGLColorFormatRGB565; 
        }

        [eaglLayer setDrawableProperties:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, pixelFormat, kEAGLDrawablePropertyColorFormat, nil]];
        
        [nw addSubview:view];
        [nw makeKeyAndVisible];
            
        EAGLRenderingAPI api;
        // create our context
        switch(apiRequest)
        {
            case pvr::Api::OpenGLES2:
                Log(LogLevel::Debug, "EGL context creation: Setting EGL_OPENGL_ES2_BIT");
                api = EAGLRenderingAPI(kEAGLRenderingAPIOpenGLES2);
                break;
            case pvr::Api::OpenGLES3:
                Log(LogLevel::Debug, "EGL context creation: EGL_OPENGL_ES3_BIT");
                api = EAGLRenderingAPI(kEAGLRenderingAPIOpenGLES3);
                break;
            default: assertion(0, "Unsupported Api");
        }
        
        context = [[EAGLContext alloc] initWithAPI:api];
        
        if(!context || ![EAGLContext setCurrentContext:context]) // TODO: Save a copy of the current context
        {
            // [context release];
            //  [view release];
            return pvr::Result::UnknownError;
        }
        
        const GLubyte *extensions = glGetString(GL_EXTENSIONS);
        bool hasFramebufferMultisample = (strstr((const char *)extensions, "GL_APPLEframebuffer_multisample") != NULL);
        
        // Create our framebuffers for rendering to
        GLuint oldRenderbuffer;
        GLuint oldFramebuffer;

        glGetIntegerv(GL_RENDERBUFFER_BINDING, (GLint *) &oldRenderbuffer);
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *) &oldFramebuffer);
        
        glGenRenderbuffers(1, &renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
        
        if(![context renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer])
        {
            glDeleteRenderbuffers(1, &renderbuffer);
            glBindRenderbuffer(GL_RENDERBUFFER_BINDING, oldRenderbuffer);
            
            //  [context release];
            //  [view release];
            return pvr::Result::UnknownError;
        }
        
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, reinterpret_cast<GLint*>(&attributes.width));
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT,reinterpret_cast<GLint*>(&attributes.height));
        
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
        
        
     //   if(hasFramebufferDiscard && attributes.discardFrameColor)
        //    discardAttachments[numDiscardAttachments++] = GL_COLOR_ATTACHMENT0;
        
        if(attributes.depthBPP || attributes.stencilBPP)
        {
            GLuint format;
            
            if(attributes.depthBPP && !attributes.stencilBPP){
                format = GL_DEPTH_COMPONENT24_OES;
                Log(LogLevel::Information, "window backbuffer DepthStencil Format: GL_DEPTH_COMPONENT24_OES");
            }
            else if(attributes.depthBPP && attributes.stencilBPP){
                format = GL_DEPTH24_STENCIL8_OES;
                Log(LogLevel::Information, "window backbuffer DepthStencil Format: GL_DEPTH24_STENCIL8_OES");
            }
            else if(!attributes.depthBPP && attributes.stencilBPP){
                format = GL_STENCIL_INDEX8;
                Log(LogLevel::Information, "window backbuffer DepthStencil Format: GL_STENCIL_INDEX8");
            }
            
            glGenRenderbuffers(1, &depthBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, format, attributes.width, attributes.height);
            
            if(attributes.depthBPP)
            {
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
                
            }
            
            if(attributes.stencilBPP)
            {
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
            }
        }
        
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
            return pvr::Result::UnknownError;
        }
        
        //MSAA
        if (hasFramebufferMultisample && attributes.aaSamples > 0)
        {
            GLint maxSamplesAllowed, samplesToUse;
            glGetIntegerv(GL_MAX_SAMPLES_APPLE, &maxSamplesAllowed);
            samplesToUse = attributes.aaSamples < maxSamplesAllowed ? attributes.aaSamples : maxSamplesAllowed;
            
            if(samplesToUse)
            {
                glGenFramebuffers(1, &msaaFrameBuffer);
                glBindFramebuffer(GL_FRAMEBUFFER, msaaFrameBuffer);
                
                glGenRenderbuffers(1, &msaaColorBuffer);
                glBindRenderbuffer(GL_RENDERBUFFER, msaaColorBuffer);
                glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, samplesToUse, GL_RGBA8_OES, attributes.width, attributes.height);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaColorBuffer);
                
                if(attributes.depthBPP || attributes.stencilBPP)
                {
                    GLuint format;
                    
                    if(attributes.depthBPP && !attributes.stencilBPP)
                        format = GL_DEPTH_COMPONENT24_OES;
                    else if(attributes.depthBPP && attributes.stencilBPP)
                        format = GL_DEPTH24_STENCIL8_OES;
                    else if(!attributes.depthBPP && attributes.stencilBPP)
                        format = GL_STENCIL_INDEX8;
                    
                    glGenRenderbuffers(1, &msaaDepthBuffer);
                    glBindRenderbuffer(GL_RENDERBUFFER, msaaDepthBuffer);
                    glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, samplesToUse, format, attributes.width, attributes.height);
                    
                    if(attributes.depthBPP)
                    {
                        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, msaaDepthBuffer);
                    }
                    
                    if(attributes.stencilBPP)
                    {
                        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msaaDepthBuffer);
                    }
                }
                
                if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                {
                    NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
                    return pvr::Result::UnknownError;
                }
            }
        
        }
        glViewport(0, 0, attributes.width, attributes.height);
        glScissor(0, 0, attributes.width, attributes.height);
        
        glBindFramebuffer(GL_FRAMEBUFFER, oldFramebuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, oldRenderbuffer);
        *outContext = context;
#if !defined(TARGET_OS_IPHONE)
        *outView = view;
#endif
    return result;
}

bool EglContext::init(OSWindow window, OSDisplay display, DisplayAttributes& attributes,
    Api minVersion, Api maxVersion)
{
    populateMaxApiVersion();
    if (maxVersion == Api::Unspecified)
    {
        maxVersion = _maxApiVersion;
        
    }
    if(minVersion == Api::Unspecified)
    {
        minVersion = Api::OpenGLES2;
    }
    else
    {
        maxVersion = std::min(maxVersion, _maxApiVersion);
    }
    
    if (minVersion > maxVersion)
    {
        Log(LogLevel::Error, "================================================================================\n"
            "API level requested [%s] was not supported. Max supported API level on this device is [%s]\n"
            "**** APPLICATION WILL EXIT ****\n"
            "================================================================================",
            apiName(_apiType), apiName(_maxApiVersion));
        return false;
    }
    if (minVersion == Api::Unspecified)
    {
        Log(LogLevel::Information, "Unspecified target API -- Setting to max API level : %s", apiName(_apiType));
    }
    
    else
    {
        _apiType = std::max(minVersion, maxVersion);
        Log(LogLevel::Information, "Requested minimum API level : %s. Will actually create %s since it is supported.",
            apiName(minVersion), apiName(_apiType));
    }

    EAGLContext* context;
    UIView* view;
    if(::init(*this, reinterpret_cast<const NativeDisplay>(display),
                reinterpret_cast<const NativeWindow>(window),
            attributes, _apiType, &view,&context) == Result::Success)
    {
        _platformContextHandles.context = context;
        _platformContextHandles.view = (__bridge VoidUIView*)view;
        makeCurrent();
        return true;
    }
    return false;
}

bool EglContext::makeCurrent()
{
    if(msaaFrameBuffer)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFrameBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, msaaColorBuffer);
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    }
    
    if(_platformContextHandles.context != [EAGLContext currentContext])
        [EAGLContext setCurrentContext:_platformContextHandles.context];
    
    return true;
}

bool EglContext::swapBuffers()
{
    bool result;
    EAGLContext* oldContext = [EAGLContext currentContext];
    
    if(oldContext != _platformContextHandles.context)
        [EAGLContext setCurrentContext:_platformContextHandles.context]; // TODO: If we're switching context then we should also be saving the renderbuffer/scissor test state
    
    //MSAA
    if(msaaFrameBuffer)
    {
        glDisable(GL_SCISSOR_TEST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER_APPLE, msaaFrameBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER_APPLE, framebuffer);
        glResolveMultisampleFramebufferAPPLE();
    }
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    
    result = [_platformContextHandles.context presentRenderbuffer:GL_RENDERBUFFER] != NO ? true : false;
    
    if(oldContext != _platformContextHandles.context)
        [EAGLContext setCurrentContext:oldContext];
    
    return result;
}
//!\endcond 
