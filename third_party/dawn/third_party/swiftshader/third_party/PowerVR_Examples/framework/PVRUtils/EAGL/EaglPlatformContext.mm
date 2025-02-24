/*!
\brief Contains implementation code for the EAGL(iOS) version of the PlatformContext.
\file PVRUtils/EAGL/EaglPlatformContext.mm
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
 #include "PVRUtils/EAGL/EaglPlatformHandles.h"
#import <QuartzCore/CAEAGLLayer.h>
#include "PVRUtils/EGL/EglPlatformContext.h"

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

GLuint framebuffer = 0;
GLuint renderbuffer = 0;
GLuint depthBuffer = 0;

GLuint msaaFrameBuffer = 0;
GLuint msaaColorBuffer = 0;
GLuint msaaDepthBuffer = 0;

namespace pvr {
namespace platform {

void EglContext_::release()
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

void EglContext_::populateMaxApiVersion()
{
    _maxApiVersion = Api::Unspecified;
    Api graphicsapi = Api::OpenGLESMaxVersion;
    while (graphicsapi > Api::Unspecified)
    {
        const char* esversion = (graphicsapi == Api::OpenGLES31 ? "3.1" : graphicsapi == Api::OpenGLES3 ? "3.0" : graphicsapi == Api::OpenGLES2 ? "2.0" : "UNKNOWN_VERSION");

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

bool EglContext_::isApiSupported(Api apiLevel)
{
	return apiLevel <= getMaxApiVersion();
}
void init(EglContext_& platformContext,const NativeDisplay& nativeDisplay,
    const NativeWindow& nativeWindow, DisplayAttributes& attributes, const pvr::Api& apiContextType,
    UIView** outView, EAGLContext** outContext)
{
    pvr::Api apiRequest = apiContextType;
    if(apiContextType == pvr::Api::Unspecified || !platformContext.isApiSupported(apiContextType))
    {
        apiRequest = platformContext.getMaxApiVersion();
        Log(LogLevel::Information, "Unspecified target API. Setting to max API level, which is %s", apiName(apiRequest));
    }
    
    if(!platformContext.isInitialized())
    {
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
            default: throw InvalidOperationError("[EglContext] Unsupported Api");
        }
        
        context = [[EAGLContext alloc] initWithAPI:api];
        
        if(!context)
        {
            throw OperationFailedError("[EglContext] Failed to create EAGL context");
        }
        if(![EAGLContext setCurrentContext:context])
        {
            throw OperationFailedError("[EglContext] Failed to make newly created EAGL Context valid.");
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
            
            throw OperationFailedError("[EglContext] Failed to create RenderBuffer Storage for the Default Framebuffer");
        }
        
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, reinterpret_cast<GLint*>(&attributes.width));
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT,reinterpret_cast<GLint*>(&attributes.height));
        
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
        
        if(attributes.depthBPP || attributes.stencilBPP)
        {
            GLuint format=0;
            
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
            throw OperationFailedError(strings::createFormatted("[EGLContext] Failed to make complete framebuffer object with error  %x", glCheckFramebufferStatus(GL_FRAMEBUFFER)));
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
                    GLuint format=0;
                    
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
                    throw OperationFailedError(strings::createFormatted("[EGLContext] Failed to make complete framebuffer object with error  %x", glCheckFramebufferStatus(GL_FRAMEBUFFER)));
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
    }
}

unsigned int EglContext_::getOnScreenFbo()
{
    return framebuffer;
}

void EglContext_::makeCurrent()
{
    if (_parentContext == nullptr)
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

        if (![EAGLContext setCurrentContext:_platformContextHandles->context])
            throw OperationFailedError("[EglContext_::makeCurrent] Failed to set current context.");
    }
    else
    {
        if(![EAGLContext setCurrentContext:_platformContextHandles->context])
            throw OperationFailedError("[EglContext_::makeCurrent] Failed to set shared current context.");
    }
}

void EglContext_::swapBuffers()
{
    EAGLContext* oldContext = [EAGLContext currentContext];
    
    if(oldContext != _platformContextHandles->context) { [EAGLContext setCurrentContext:_platformContextHandles->context]; }
    
    //MSAA
    if(msaaFrameBuffer)
    {
        glDisable(GL_SCISSOR_TEST);
        glBindFramebuffer(GL_READ_FRAMEBUFFER_APPLE, msaaFrameBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER_APPLE, framebuffer);
        glResolveMultisampleFramebufferAPPLE();
    }
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    
    if ([_platformContextHandles->context presentRenderbuffer:GL_RENDERBUFFER] == NO)
        throw OperationFailedError("[EglContext::swapBuffers] Failed to present renderbuffer.");
    
    if(oldContext != _platformContextHandles->context)
        if (![EAGLContext setCurrentContext:oldContext])
            throw OperationFailedError("[EglContext::swapBuffers] Failed to make current in present renderbuffer.");
}

static inline void preInitialize(NativePlatformHandles& handles)
{
    if (!handles) { handles = std::make_shared<NativePlatformHandles_>(); }
}

void EglContext_::init(OSWindow window, OSDisplay display, DisplayAttributes& attributes, Api minApi, Api maxApi)
{
    if(_initialized) { throw InvalidOperationError("[EglContext::init] Context already initialized"); }
    _attributes = &attributes;
    if(!_preInitialized)
    {
        preInitialize(_platformContextHandles);
        _preInitialized = true;
        populateMaxApiVersion();
    }

	if (maxApi == Api::Unspecified) { maxApi = getMaxApiVersion(); }
	if (minApi == Api::Unspecified) { minApi = Api::OpenGLES2; }
	else
	{
		maxApi = std::min(maxApi, getMaxApiVersion());
	}
    
	if (minApi > maxApi)
	{
		throw InvalidOperationError(pvr::strings::createFormatted("[EglContext::init]: API level requested [%s] was not supported. Max supported API level on this device is [%s]\n"
																  "**** APPLICATION WILL EXIT ****\n",
			apiName(minApi), apiName(getMaxApiVersion())));
	}

	if (minApi == Api::Unspecified)
	{
		_apiType = maxApi;
		Log(LogLevel::Information, "Unspecified target API -- Setting to max API level : %s", apiName(_apiType));
	}
	else
	{
		_apiType = std::max(minApi, maxApi);
		Log(LogLevel::Information, "Requested minimum API level : %s. Will actually create %s since it is supported.", apiName(minApi), apiName(_apiType));
	}

    EAGLContext* context;
    UIView* view;
    pvr::platform::init(*this, reinterpret_cast<const NativeDisplay>(display),
                reinterpret_cast<const NativeWindow>(window),
           attributes, _apiType, &view,&context);
        
    _platformContextHandles->context = context;
    _platformContextHandles->view = (__bridge VoidUIView*)view;
    _initialized = true;
        
    makeCurrent();
}

Api EglContext_::getMaxApiVersion()
{
    if(!_preInitialized)
    {
        preInitialize(_platformContextHandles);
        _preInitialized = true;
        populateMaxApiVersion();
    }
	
    return _maxApiVersion;
}

Api EglContext_::getApiVersion()
{
    return _apiType;
}

std::unique_ptr<EglContext_> EglContext_::createSharedContextFromEGLContext()
{
    std::unique_ptr<EglContext_> retval = std::make_unique<platform::EglContext_>();

    retval->_parentContext = &(*this);
    _platformContextHandles = std::make_unique<NativePlatformHandles_>();
    
    EAGLRenderingAPI api;
    switch (retval->_parentContext->getApiVersion())
    {
        case pvr::Api::OpenGLES2: //GLES2
            Log(LogLevel::Debug, "EAGL context creation: Setting kEAGLRenderingAPIOpenGLES2");
            api = EAGLRenderingAPI(kEAGLRenderingAPIOpenGLES2);
            break;
        case pvr::Api::OpenGLES3: //GLES2
            Log(LogLevel::Debug, "EGL context creation: kEAGLRenderingAPIOpenGLES3");
            api = EAGLRenderingAPI(kEAGLRenderingAPIOpenGLES3);
            break;
        default:
            throw InvalidOperationError("[SharedEglContext constructor] Unrecognised Api version for the parent context");
    }
    _platformContextHandles->context= [[EAGLContext alloc] initWithAPI:api sharegroup:retval->_parentContext->getNativePlatformHandles().context.sharegroup];
    if(!_platformContextHandles->context)
    {
        throw OperationFailedError("[SharedEglContext constructor] Failed to create SharedContext");
    }

	return retval;
}
} // namespace platform
} // namespace pvr

namespace pvr {
// Creates an instance of a platform context
std::unique_ptr<platform::EglContext_> createEglContext() { return std::make_unique<platform::EglContext_>(); }
} // namespace pvr
//!\endcond 
