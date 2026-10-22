/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#if SDL_VIDEO_RENDER_METAL && !SDL_RENDER_DISABLED

#include "SDL_hints.h"
#include "SDL_log.h"
#include "SDL_assert.h"
#include "SDL_syswm.h"
#include "../SDL_sysrender.h"

#ifdef __MACOSX__
#include "../../video/cocoa/SDL_cocoametalview.h"
#else
#include "../../video/uikit/SDL_uikitmetalview.h"
#endif
#include <Availability.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

/* Regenerate these with build-metal-shaders.sh */
#ifdef __MACOSX__
#include "SDL_shaders_metal_osx.h"
#else
#include "SDL_shaders_metal_ios.h"
#endif

/* Apple Metal renderer implementation */

static SDL_Renderer *METAL_CreateRenderer(SDL_Window * window, Uint32 flags);
static void METAL_WindowEvent(SDL_Renderer * renderer,
                           const SDL_WindowEvent *event);
static int METAL_GetOutputSize(SDL_Renderer * renderer, int *w, int *h);
static SDL_bool METAL_SupportsBlendMode(SDL_Renderer * renderer, SDL_BlendMode blendMode);
static int METAL_CreateTexture(SDL_Renderer * renderer, SDL_Texture * texture);
static int METAL_UpdateTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                            const SDL_Rect * rect, const void *pixels,
                            int pitch);
static int METAL_UpdateTextureYUV(SDL_Renderer * renderer, SDL_Texture * texture,
                               const SDL_Rect * rect,
                               const Uint8 *Yplane, int Ypitch,
                               const Uint8 *Uplane, int Upitch,
                               const Uint8 *Vplane, int Vpitch);
static int METAL_LockTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                          const SDL_Rect * rect, void **pixels, int *pitch);
static void METAL_UnlockTexture(SDL_Renderer * renderer, SDL_Texture * texture);
static int METAL_SetRenderTarget(SDL_Renderer * renderer, SDL_Texture * texture);
static int METAL_UpdateViewport(SDL_Renderer * renderer);
static int METAL_UpdateClipRect(SDL_Renderer * renderer);
static int METAL_RenderClear(SDL_Renderer * renderer);
static int METAL_RenderDrawPoints(SDL_Renderer * renderer,
                               const SDL_FPoint * points, int count);
static int METAL_RenderDrawLines(SDL_Renderer * renderer,
                              const SDL_FPoint * points, int count);
static int METAL_RenderFillRects(SDL_Renderer * renderer,
                              const SDL_FRect * rects, int count);
static int METAL_RenderCopy(SDL_Renderer * renderer, SDL_Texture * texture,
                         const SDL_Rect * srcrect, const SDL_FRect * dstrect);
static int METAL_RenderCopyEx(SDL_Renderer * renderer, SDL_Texture * texture,
                         const SDL_Rect * srcrect, const SDL_FRect * dstrect,
                         const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip);
static int METAL_RenderReadPixels(SDL_Renderer * renderer, const SDL_Rect * rect,
                               Uint32 pixel_format, void * pixels, int pitch);
static void METAL_RenderPresent(SDL_Renderer * renderer);
static void METAL_DestroyTexture(SDL_Renderer * renderer, SDL_Texture * texture);
static void METAL_DestroyRenderer(SDL_Renderer * renderer);
static void *METAL_GetMetalLayer(SDL_Renderer * renderer);
static void *METAL_GetMetalCommandEncoder(SDL_Renderer * renderer);

SDL_RenderDriver METAL_RenderDriver = {
    METAL_CreateRenderer,
    {
        "metal",
        (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE),
        6,
        {
            SDL_PIXELFORMAT_ARGB8888,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_PIXELFORMAT_YV12,
            SDL_PIXELFORMAT_IYUV,
            SDL_PIXELFORMAT_NV12,
            SDL_PIXELFORMAT_NV21
        },
    0, 0,
    }
};

/* macOS requires constants in a buffer to have a 256 byte alignment. */
#ifdef __MACOSX__
#define CONSTANT_ALIGN 256
#else
#define CONSTANT_ALIGN 4
#endif

#define ALIGN_CONSTANTS(size) ((size + CONSTANT_ALIGN - 1) & (~(CONSTANT_ALIGN - 1)))

static const size_t CONSTANTS_OFFSET_IDENTITY = 0;
static const size_t CONSTANTS_OFFSET_HALF_PIXEL_TRANSFORM = ALIGN_CONSTANTS(CONSTANTS_OFFSET_IDENTITY + sizeof(float) * 16);
static const size_t CONSTANTS_OFFSET_DECODE_JPEG = ALIGN_CONSTANTS(CONSTANTS_OFFSET_HALF_PIXEL_TRANSFORM + sizeof(float) * 16);
static const size_t CONSTANTS_OFFSET_DECODE_BT601 = ALIGN_CONSTANTS(CONSTANTS_OFFSET_DECODE_JPEG + sizeof(float) * 4 * 4);
static const size_t CONSTANTS_OFFSET_DECODE_BT709 = ALIGN_CONSTANTS(CONSTANTS_OFFSET_DECODE_BT601 + sizeof(float) * 4 * 4);
static const size_t CONSTANTS_OFFSET_CLEAR_VERTS = ALIGN_CONSTANTS(CONSTANTS_OFFSET_DECODE_BT709 + sizeof(float) * 4 * 4);
static const size_t CONSTANTS_LENGTH = CONSTANTS_OFFSET_CLEAR_VERTS + sizeof(float) * 6;

typedef enum SDL_MetalVertexFunction
{
    SDL_METAL_VERTEX_SOLID,
    SDL_METAL_VERTEX_COPY,
} SDL_MetalVertexFunction;

typedef enum SDL_MetalFragmentFunction
{
    SDL_METAL_FRAGMENT_SOLID = 0,
    SDL_METAL_FRAGMENT_COPY,
    SDL_METAL_FRAGMENT_YUV,
    SDL_METAL_FRAGMENT_NV12,
    SDL_METAL_FRAGMENT_NV21,
    SDL_METAL_FRAGMENT_COUNT,
} SDL_MetalFragmentFunction;

typedef struct METAL_PipelineState
{
    SDL_BlendMode blendMode;
    void *pipe;
} METAL_PipelineState;

typedef struct METAL_PipelineCache
{
    METAL_PipelineState *states;
    int count;
    SDL_MetalVertexFunction vertexFunction;
    SDL_MetalFragmentFunction fragmentFunction;
    MTLPixelFormat renderTargetFormat;
    const char *label;
} METAL_PipelineCache;

/* Each shader combination used by drawing functions has a separate pipeline
 * cache, and we have a separate list of caches for each render target pixel
 * format. This is more efficient than iterating over a global cache to find
 * the pipeline based on the specified shader combination and RT pixel format,
 * since we know what the RT pixel format is when we set the render target, and
 * we know what the shader combination is inside each drawing function's code. */
typedef struct METAL_ShaderPipelines
{
    MTLPixelFormat renderTargetFormat;
    METAL_PipelineCache caches[SDL_METAL_FRAGMENT_COUNT];
} METAL_ShaderPipelines;

@interface METAL_RenderData : NSObject
    @property (nonatomic, retain) id<MTLDevice> mtldevice;
    @property (nonatomic, retain) id<MTLCommandQueue> mtlcmdqueue;
    @property (nonatomic, retain) id<MTLCommandBuffer> mtlcmdbuffer;
    @property (nonatomic, retain) id<MTLRenderCommandEncoder> mtlcmdencoder;
    @property (nonatomic, retain) id<MTLLibrary> mtllibrary;
    @property (nonatomic, retain) id<CAMetalDrawable> mtlbackbuffer;
    @property (nonatomic, retain) id<MTLSamplerState> mtlsamplernearest;
    @property (nonatomic, retain) id<MTLSamplerState> mtlsamplerlinear;
    @property (nonatomic, retain) id<MTLBuffer> mtlbufconstants;
    @property (nonatomic, retain) CAMetalLayer *mtllayer;
    @property (nonatomic, retain) MTLRenderPassDescriptor *mtlpassdesc;
    @property (nonatomic, assign) METAL_ShaderPipelines *activepipelines;
    @property (nonatomic, assign) METAL_ShaderPipelines *allpipelines;
    @property (nonatomic, assign) int pipelinescount;
@end

@implementation METAL_RenderData
#if !__has_feature(objc_arc)
- (void)dealloc
{
    [_mtldevice release];
    [_mtlcmdqueue release];
    [_mtlcmdbuffer release];
    [_mtlcmdencoder release];
    [_mtllibrary release];
    [_mtlbackbuffer release];
    [_mtlsamplernearest release];
    [_mtlsamplerlinear release];
    [_mtlbufconstants release];
    [_mtllayer release];
    [_mtlpassdesc release];
    [super dealloc];
}
#endif
@end

@interface METAL_TextureData : NSObject
    @property (nonatomic, retain) id<MTLTexture> mtltexture;
    @property (nonatomic, retain) id<MTLTexture> mtltexture_uv;
    @property (nonatomic, retain) id<MTLSamplerState> mtlsampler;
    @property (nonatomic, assign) SDL_MetalFragmentFunction fragmentFunction;
    @property (nonatomic, assign) BOOL yuv;
    @property (nonatomic, assign) BOOL nv12;
    @property (nonatomic, assign) size_t conversionBufferOffset;
@end

@implementation METAL_TextureData
#if !__has_feature(objc_arc)
- (void)dealloc
{
    [_mtltexture release];
    [_mtltexture_uv release];
    [_mtlsampler release];
    [super dealloc];
}
#endif
@end

static int
IsMetalAvailable(const SDL_SysWMinfo *syswm)
{
    if (syswm->subsystem != SDL_SYSWM_COCOA && syswm->subsystem != SDL_SYSWM_UIKIT) {
        return SDL_SetError("Metal render target only supports Cocoa and UIKit video targets at the moment.");
    }

    // this checks a weak symbol.
#if (defined(__MACOSX__) && (MAC_OS_X_VERSION_MIN_REQUIRED < 101100))
    if (MTLCreateSystemDefaultDevice == NULL) {  // probably on 10.10 or lower.
        return SDL_SetError("Metal framework not available on this system");
    }
#endif

    return 0;
}

static const MTLBlendOperation invalidBlendOperation = (MTLBlendOperation)0xFFFFFFFF;
static const MTLBlendFactor invalidBlendFactor = (MTLBlendFactor)0xFFFFFFFF;

static MTLBlendOperation
GetBlendOperation(SDL_BlendOperation operation)
{
    switch (operation) {
        case SDL_BLENDOPERATION_ADD: return MTLBlendOperationAdd;
        case SDL_BLENDOPERATION_SUBTRACT: return MTLBlendOperationSubtract;
        case SDL_BLENDOPERATION_REV_SUBTRACT: return MTLBlendOperationReverseSubtract;
        case SDL_BLENDOPERATION_MINIMUM: return MTLBlendOperationMin;
        case SDL_BLENDOPERATION_MAXIMUM: return MTLBlendOperationMax;
        default: return invalidBlendOperation;
    }
}

static MTLBlendFactor
GetBlendFactor(SDL_BlendFactor factor)
{
    switch (factor) {
        case SDL_BLENDFACTOR_ZERO: return MTLBlendFactorZero;
        case SDL_BLENDFACTOR_ONE: return MTLBlendFactorOne;
        case SDL_BLENDFACTOR_SRC_COLOR: return MTLBlendFactorSourceColor;
        case SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR: return MTLBlendFactorOneMinusSourceColor;
        case SDL_BLENDFACTOR_SRC_ALPHA: return MTLBlendFactorSourceAlpha;
        case SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA: return MTLBlendFactorOneMinusSourceAlpha;
        case SDL_BLENDFACTOR_DST_COLOR: return MTLBlendFactorDestinationColor;
        case SDL_BLENDFACTOR_ONE_MINUS_DST_COLOR: return MTLBlendFactorOneMinusDestinationColor;
        case SDL_BLENDFACTOR_DST_ALPHA: return MTLBlendFactorDestinationAlpha;
        case SDL_BLENDFACTOR_ONE_MINUS_DST_ALPHA: return MTLBlendFactorOneMinusDestinationAlpha;
        default: return invalidBlendFactor;
    }
}

static NSString *
GetVertexFunctionName(SDL_MetalVertexFunction function)
{
    switch (function) {
        case SDL_METAL_VERTEX_SOLID: return @"SDL_Solid_vertex";
        case SDL_METAL_VERTEX_COPY: return @"SDL_Copy_vertex";
        default: return nil;
    }
}

static NSString *
GetFragmentFunctionName(SDL_MetalFragmentFunction function)
{
    switch (function) {
        case SDL_METAL_FRAGMENT_SOLID: return @"SDL_Solid_fragment";
        case SDL_METAL_FRAGMENT_COPY: return @"SDL_Copy_fragment";
        case SDL_METAL_FRAGMENT_YUV: return @"SDL_YUV_fragment";
        case SDL_METAL_FRAGMENT_NV12: return @"SDL_NV12_fragment";
        case SDL_METAL_FRAGMENT_NV21: return @"SDL_NV21_fragment";
        default: return nil;
    }
}

static id<MTLRenderPipelineState>
MakePipelineState(METAL_RenderData *data, METAL_PipelineCache *cache,
                  NSString *blendlabel, SDL_BlendMode blendmode)
{
    id<MTLFunction> mtlvertfn = [data.mtllibrary newFunctionWithName:GetVertexFunctionName(cache->vertexFunction)];
    id<MTLFunction> mtlfragfn = [data.mtllibrary newFunctionWithName:GetFragmentFunctionName(cache->fragmentFunction)];
    SDL_assert(mtlvertfn != nil);
    SDL_assert(mtlfragfn != nil);

    MTLRenderPipelineDescriptor *mtlpipedesc = [[MTLRenderPipelineDescriptor alloc] init];
    mtlpipedesc.vertexFunction = mtlvertfn;
    mtlpipedesc.fragmentFunction = mtlfragfn;

    MTLRenderPipelineColorAttachmentDescriptor *rtdesc = mtlpipedesc.colorAttachments[0];

    rtdesc.pixelFormat = cache->renderTargetFormat;

    if (blendmode != SDL_BLENDMODE_NONE) {
        rtdesc.blendingEnabled = YES;
        rtdesc.sourceRGBBlendFactor = GetBlendFactor(SDL_GetBlendModeSrcColorFactor(blendmode));
        rtdesc.destinationRGBBlendFactor = GetBlendFactor(SDL_GetBlendModeDstColorFactor(blendmode));
        rtdesc.rgbBlendOperation = GetBlendOperation(SDL_GetBlendModeColorOperation(blendmode));
        rtdesc.sourceAlphaBlendFactor = GetBlendFactor(SDL_GetBlendModeSrcAlphaFactor(blendmode));
        rtdesc.destinationAlphaBlendFactor = GetBlendFactor(SDL_GetBlendModeDstAlphaFactor(blendmode));
        rtdesc.alphaBlendOperation = GetBlendOperation(SDL_GetBlendModeAlphaOperation(blendmode));
    } else {
        rtdesc.blendingEnabled = NO;
    }

    mtlpipedesc.label = [@(cache->label) stringByAppendingString:blendlabel];

    NSError *err = nil;
    id<MTLRenderPipelineState> state = [data.mtldevice newRenderPipelineStateWithDescriptor:mtlpipedesc error:&err];
    SDL_assert(err == nil);

    METAL_PipelineState pipeline;
    pipeline.blendMode = blendmode;
    pipeline.pipe = (void *)CFBridgingRetain(state);

    METAL_PipelineState *states = SDL_realloc(cache->states, (cache->count + 1) * sizeof(pipeline));

#if !__has_feature(objc_arc)
    [mtlpipedesc release];  // !!! FIXME: can these be reused for each creation, or does the pipeline obtain it?
    [mtlvertfn release];
    [mtlfragfn release];
    [state release];
#endif

    if (states) {
        states[cache->count++] = pipeline;
        cache->states = states;
        return (__bridge id<MTLRenderPipelineState>)pipeline.pipe;
    } else {
        CFBridgingRelease(pipeline.pipe);
        SDL_OutOfMemory();
        return NULL;
    }
}

static void
MakePipelineCache(METAL_RenderData *data, METAL_PipelineCache *cache, const char *label,
                  MTLPixelFormat rtformat, SDL_MetalVertexFunction vertfn, SDL_MetalFragmentFunction fragfn)
{
    SDL_zerop(cache);

    cache->vertexFunction = vertfn;
    cache->fragmentFunction = fragfn;
    cache->renderTargetFormat = rtformat;
    cache->label = label;

    /* Create pipeline states for the default blend modes. Custom blend modes
     * will be added to the cache on-demand. */
    MakePipelineState(data, cache, @" (blend=none)", SDL_BLENDMODE_NONE);
    MakePipelineState(data, cache, @" (blend=blend)", SDL_BLENDMODE_BLEND);
    MakePipelineState(data, cache, @" (blend=add)", SDL_BLENDMODE_ADD);
    MakePipelineState(data, cache, @" (blend=mod)", SDL_BLENDMODE_MOD);
}

static void
DestroyPipelineCache(METAL_PipelineCache *cache)
{
    if (cache != NULL) {
        for (int i = 0; i < cache->count; i++) {
            CFBridgingRelease(cache->states[i].pipe);
        }

        SDL_free(cache->states);
    }
}

void
MakeShaderPipelines(METAL_RenderData *data, METAL_ShaderPipelines *pipelines, MTLPixelFormat rtformat)
{
    SDL_zerop(pipelines);

    pipelines->renderTargetFormat = rtformat;

    MakePipelineCache(data, &pipelines->caches[SDL_METAL_FRAGMENT_SOLID], "SDL primitives pipeline", rtformat, SDL_METAL_VERTEX_SOLID, SDL_METAL_FRAGMENT_SOLID);
    MakePipelineCache(data, &pipelines->caches[SDL_METAL_FRAGMENT_COPY], "SDL copy pipeline", rtformat, SDL_METAL_VERTEX_COPY, SDL_METAL_FRAGMENT_COPY);
    MakePipelineCache(data, &pipelines->caches[SDL_METAL_FRAGMENT_YUV], "SDL YUV pipeline", rtformat, SDL_METAL_VERTEX_COPY, SDL_METAL_FRAGMENT_YUV);
    MakePipelineCache(data, &pipelines->caches[SDL_METAL_FRAGMENT_NV12], "SDL NV12 pipeline", rtformat, SDL_METAL_VERTEX_COPY, SDL_METAL_FRAGMENT_NV12);
    MakePipelineCache(data, &pipelines->caches[SDL_METAL_FRAGMENT_NV21], "SDL NV21 pipeline", rtformat, SDL_METAL_VERTEX_COPY, SDL_METAL_FRAGMENT_NV21);
}

static METAL_ShaderPipelines *
ChooseShaderPipelines(METAL_RenderData *data, MTLPixelFormat rtformat)
{
    METAL_ShaderPipelines *allpipelines = data.allpipelines;
    int count = data.pipelinescount;

    for (int i = 0; i < count; i++) {
        if (allpipelines[i].renderTargetFormat == rtformat) {
            return &allpipelines[i];
        }
    }

    allpipelines = SDL_realloc(allpipelines, (count + 1) * sizeof(METAL_ShaderPipelines));

    if (allpipelines == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }

    MakeShaderPipelines(data, &allpipelines[count], rtformat);

    data.allpipelines = allpipelines;
    data.pipelinescount = count + 1;

    return &data.allpipelines[count];
}

static void
DestroyAllPipelines(METAL_ShaderPipelines *allpipelines, int count)
{
    if (allpipelines != NULL) {
        for (int i = 0; i < count; i++) {
            for (int cache = 0; cache < SDL_METAL_FRAGMENT_COUNT; cache++) {
                DestroyPipelineCache(&allpipelines[i].caches[cache]);
            }
        }

        SDL_free(allpipelines);
    }
}

static inline id<MTLRenderPipelineState>
ChoosePipelineState(METAL_RenderData *data, METAL_ShaderPipelines *pipelines, SDL_MetalFragmentFunction fragfn, SDL_BlendMode blendmode)
{
    METAL_PipelineCache *cache = &pipelines->caches[fragfn];

    for (int i = 0; i < cache->count; i++) {
        if (cache->states[i].blendMode == blendmode) {
            return (__bridge id<MTLRenderPipelineState>)cache->states[i].pipe;
        }
    }

    return MakePipelineState(data, cache, [NSString stringWithFormat:@" (blend=custom 0x%x)", blendmode], blendmode);
}

static SDL_Renderer *
METAL_CreateRenderer(SDL_Window * window, Uint32 flags)
{ @autoreleasepool {
    SDL_Renderer *renderer = NULL;
    METAL_RenderData *data = NULL;
    id<MTLDevice> mtldevice = nil;
    SDL_SysWMinfo syswm;

    SDL_VERSION(&syswm.version);
    if (!SDL_GetWindowWMInfo(window, &syswm)) {
        return NULL;
    }

    if (IsMetalAvailable(&syswm) == -1) {
        return NULL;
    }

    renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(*renderer));
    if (!renderer) {
        SDL_OutOfMemory();
        return NULL;
    }

    // !!! FIXME: MTLCopyAllDevices() can find other GPUs on macOS...
    mtldevice = MTLCreateSystemDefaultDevice();

    if (mtldevice == nil) {
        SDL_free(renderer);
        SDL_SetError("Failed to obtain Metal device");
        return NULL;
    }

    // !!! FIXME: error checking on all of this.
    data = [[METAL_RenderData alloc] init];

    renderer->driverdata = (void*)CFBridgingRetain(data);
    renderer->window = window;

#ifdef __MACOSX__
    NSView *view = Cocoa_Mtl_AddMetalView(window);
    CAMetalLayer *layer = (CAMetalLayer *)[view layer];

    layer.device = mtldevice;

    //layer.colorspace = nil;

#else
    UIView *view = UIKit_Mtl_AddMetalView(window);
    CAMetalLayer *layer = (CAMetalLayer *)[view layer];
#endif

    // Necessary for RenderReadPixels.
    layer.framebufferOnly = NO;

    data.mtldevice = layer.device;
    data.mtllayer = layer;
    id<MTLCommandQueue> mtlcmdqueue = [data.mtldevice newCommandQueue];
    data.mtlcmdqueue = mtlcmdqueue;
    data.mtlcmdqueue.label = @"SDL Metal Renderer";
    data.mtlpassdesc = [MTLRenderPassDescriptor renderPassDescriptor];

    NSError *err = nil;

    // The compiled .metallib is embedded in a static array in a header file
    // but the original shader source code is in SDL_shaders_metal.metal.
    dispatch_data_t mtllibdata = dispatch_data_create(sdl_metallib, sdl_metallib_len, dispatch_get_global_queue(0, 0), ^{});
    id<MTLLibrary> mtllibrary = [data.mtldevice newLibraryWithData:mtllibdata error:&err];
    data.mtllibrary = mtllibrary;
    SDL_assert(err == nil);
#if !__has_feature(objc_arc)
    dispatch_release(mtllibdata);
#endif
    data.mtllibrary.label = @"SDL Metal renderer shader library";

    /* Do some shader pipeline state loading up-front rather than on demand. */
    data.pipelinescount = 0;
    data.allpipelines = NULL;
    ChooseShaderPipelines(data, MTLPixelFormatBGRA8Unorm);

    MTLSamplerDescriptor *samplerdesc = [[MTLSamplerDescriptor alloc] init];

    samplerdesc.minFilter = MTLSamplerMinMagFilterNearest;
    samplerdesc.magFilter = MTLSamplerMinMagFilterNearest;
    id<MTLSamplerState> mtlsamplernearest = [data.mtldevice newSamplerStateWithDescriptor:samplerdesc];
    data.mtlsamplernearest = mtlsamplernearest;

    samplerdesc.minFilter = MTLSamplerMinMagFilterLinear;
    samplerdesc.magFilter = MTLSamplerMinMagFilterLinear;
    id<MTLSamplerState> mtlsamplerlinear = [data.mtldevice newSamplerStateWithDescriptor:samplerdesc];
    data.mtlsamplerlinear = mtlsamplerlinear;

    /* Note: matrices are column major. */
    float identitytransform[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };

    float halfpixeltransform[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f,
    };

    /* Metal pads float3s to 16 bytes. */
    float decodetransformJPEG[4*4] = {
        0.0, -0.501960814, -0.501960814, 0.0, /* offset */
        1.0000,  0.0000,  1.4020, 0.0,        /* Rcoeff */
        1.0000, -0.3441, -0.7141, 0.0,        /* Gcoeff */
        1.0000,  1.7720,  0.0000, 0.0,        /* Bcoeff */
    };

    float decodetransformBT601[4*4] = {
        -0.0627451017, -0.501960814, -0.501960814, 0.0, /* offset */
        1.1644,  0.0000,  1.5960, 0.0,                  /* Rcoeff */
        1.1644, -0.3918, -0.8130, 0.0,                  /* Gcoeff */
        1.1644,  2.0172,  0.0000, 0.0,                  /* Bcoeff */
    };

    float decodetransformBT709[4*4] = {
        0.0, -0.501960814, -0.501960814, 0.0, /* offset */
        1.0000,  0.0000,  1.4020, 0.0,        /* Rcoeff */
        1.0000, -0.3441, -0.7141, 0.0,        /* Gcoeff */
        1.0000,  1.7720,  0.0000, 0.0,        /* Bcoeff */
    };

    float clearverts[6] = {0.0f, 0.0f,  0.0f, 2.0f,  2.0f, 0.0f};

    id<MTLBuffer> mtlbufconstantstaging = [data.mtldevice newBufferWithLength:CONSTANTS_LENGTH options:MTLResourceStorageModeShared];
    mtlbufconstantstaging.label = @"SDL constant staging data";

    id<MTLBuffer> mtlbufconstants = [data.mtldevice newBufferWithLength:CONSTANTS_LENGTH options:MTLResourceStorageModePrivate];
    data.mtlbufconstants = mtlbufconstants;
    data.mtlbufconstants.label = @"SDL constant data";

    char *constantdata = [mtlbufconstantstaging contents];
    SDL_memcpy(constantdata + CONSTANTS_OFFSET_IDENTITY, identitytransform, sizeof(identitytransform));
    SDL_memcpy(constantdata + CONSTANTS_OFFSET_HALF_PIXEL_TRANSFORM, halfpixeltransform, sizeof(halfpixeltransform));
    SDL_memcpy(constantdata + CONSTANTS_OFFSET_DECODE_JPEG, decodetransformJPEG, sizeof(decodetransformJPEG));
    SDL_memcpy(constantdata + CONSTANTS_OFFSET_DECODE_BT601, decodetransformBT601, sizeof(decodetransformBT601));
    SDL_memcpy(constantdata + CONSTANTS_OFFSET_DECODE_BT709, decodetransformBT709, sizeof(decodetransformBT709));
    SDL_memcpy(constantdata + CONSTANTS_OFFSET_CLEAR_VERTS, clearverts, sizeof(clearverts));

    id<MTLCommandBuffer> cmdbuffer = [data.mtlcmdqueue commandBuffer];
    id<MTLBlitCommandEncoder> blitcmd = [cmdbuffer blitCommandEncoder];

    [blitcmd copyFromBuffer:mtlbufconstantstaging sourceOffset:0 toBuffer:data.mtlbufconstants destinationOffset:0 size:CONSTANTS_LENGTH];

    [blitcmd endEncoding];
    [cmdbuffer commit];

    // !!! FIXME: force more clears here so all the drawables are sane to start, and our static buffers are definitely flushed.

    renderer->WindowEvent = METAL_WindowEvent;
    renderer->GetOutputSize = METAL_GetOutputSize;
    renderer->SupportsBlendMode = METAL_SupportsBlendMode;
    renderer->CreateTexture = METAL_CreateTexture;
    renderer->UpdateTexture = METAL_UpdateTexture;
    renderer->UpdateTextureYUV = METAL_UpdateTextureYUV;
    renderer->LockTexture = METAL_LockTexture;
    renderer->UnlockTexture = METAL_UnlockTexture;
    renderer->SetRenderTarget = METAL_SetRenderTarget;
    renderer->UpdateViewport = METAL_UpdateViewport;
    renderer->UpdateClipRect = METAL_UpdateClipRect;
    renderer->RenderClear = METAL_RenderClear;
    renderer->RenderDrawPoints = METAL_RenderDrawPoints;
    renderer->RenderDrawLines = METAL_RenderDrawLines;
    renderer->RenderFillRects = METAL_RenderFillRects;
    renderer->RenderCopy = METAL_RenderCopy;
    renderer->RenderCopyEx = METAL_RenderCopyEx;
    renderer->RenderReadPixels = METAL_RenderReadPixels;
    renderer->RenderPresent = METAL_RenderPresent;
    renderer->DestroyTexture = METAL_DestroyTexture;
    renderer->DestroyRenderer = METAL_DestroyRenderer;
    renderer->GetMetalLayer = METAL_GetMetalLayer;
    renderer->GetMetalCommandEncoder = METAL_GetMetalCommandEncoder;

    renderer->info = METAL_RenderDriver.info;
    renderer->info.flags = (SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

#if defined(__MACOSX__) && defined(MAC_OS_X_VERSION_10_13)
    if (@available(macOS 10.13, *)) {
        data.mtllayer.displaySyncEnabled = (flags & SDL_RENDERER_PRESENTVSYNC) != 0;
    } else
#endif
    {
        renderer->info.flags |= SDL_RENDERER_PRESENTVSYNC;
    }

    /* https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf */
    int maxtexsize = 4096;
#if defined(__MACOSX__)
    maxtexsize = 16384;
#elif defined(__TVOS__)
    maxtexsize = 8192;
#ifdef __TVOS_11_0
    if (@available(tvOS 11.0, *)) {
        if ([mtldevice supportsFeatureSet:MTLFeatureSet_tvOS_GPUFamily2_v1]) {
            maxtexsize = 16384;
        }
    }
#endif
#else
#ifdef __IPHONE_11_0
    if ([mtldevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily4_v1]) {
        maxtexsize = 16384;
    } else
#endif
#ifdef __IPHONE_10_0
    if ([mtldevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1]) {
        maxtexsize = 16384;
    } else
#endif
    if ([mtldevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v2] || [mtldevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v2]) {
        maxtexsize = 8192;
    } else {
        maxtexsize = 4096;
    }
#endif

    renderer->info.max_texture_width = maxtexsize;
    renderer->info.max_texture_height = maxtexsize;

#if !__has_feature(objc_arc)
    [mtlcmdqueue release];
    [mtllibrary release];
    [samplerdesc release];
    [mtlsamplernearest release];
    [mtlsamplerlinear release];
    [mtlbufconstants release];
    [view release];
    [data release];
    [mtldevice release];
#endif

    return renderer;
}}

static void
METAL_ActivateRenderCommandEncoder(SDL_Renderer * renderer, MTLLoadAction load)
{
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;

    /* Our SetRenderTarget just signals that the next render operation should
     * set up a new render pass. This is where that work happens. */
    if (data.mtlcmdencoder == nil) {
        id<MTLTexture> mtltexture = nil;

        if (renderer->target != NULL) {
            METAL_TextureData *texdata = (__bridge METAL_TextureData *)renderer->target->driverdata;
            mtltexture = texdata.mtltexture;
        } else {
            if (data.mtlbackbuffer == nil) {
                /* The backbuffer's contents aren't guaranteed to persist after
                 * presenting, so we can leave it undefined when loading it. */
                data.mtlbackbuffer = [data.mtllayer nextDrawable];
                if (load == MTLLoadActionLoad) {
                    load = MTLLoadActionDontCare;
                }
            }
            mtltexture = data.mtlbackbuffer.texture;
        }

        SDL_assert(mtltexture);

        if (load == MTLLoadActionClear) {
            MTLClearColor color = MTLClearColorMake(renderer->r/255.0, renderer->g/255.0, renderer->b/255.0, renderer->a/255.0);
            data.mtlpassdesc.colorAttachments[0].clearColor = color;
        }

        data.mtlpassdesc.colorAttachments[0].loadAction = load;
        data.mtlpassdesc.colorAttachments[0].texture = mtltexture;

        data.mtlcmdbuffer = [data.mtlcmdqueue commandBuffer];
        data.mtlcmdencoder = [data.mtlcmdbuffer renderCommandEncoderWithDescriptor:data.mtlpassdesc];

        if (data.mtlbackbuffer != nil && mtltexture == data.mtlbackbuffer.texture) {
            data.mtlcmdencoder.label = @"SDL metal renderer backbuffer";
        } else {
            data.mtlcmdencoder.label = @"SDL metal renderer render target";
        }

        data.activepipelines = ChooseShaderPipelines(data, mtltexture.pixelFormat);

        /* Make sure the viewport and clip rect are set on the new render pass. */
        METAL_UpdateViewport(renderer);
        METAL_UpdateClipRect(renderer);
    }
}

static void
METAL_WindowEvent(SDL_Renderer * renderer, const SDL_WindowEvent *event)
{
    if (event->event == SDL_WINDOWEVENT_SIZE_CHANGED ||
        event->event == SDL_WINDOWEVENT_SHOWN ||
        event->event == SDL_WINDOWEVENT_HIDDEN) {
        // !!! FIXME: write me
    }
}

static int
METAL_GetOutputSize(SDL_Renderer * renderer, int *w, int *h)
{ @autoreleasepool {
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    if (w) {
        *w = (int)data.mtllayer.drawableSize.width;
    }
    if (h) {
        *h = (int)data.mtllayer.drawableSize.height;
    }
    return 0;
}}

static SDL_bool
METAL_SupportsBlendMode(SDL_Renderer * renderer, SDL_BlendMode blendMode)
{
    SDL_BlendFactor srcColorFactor = SDL_GetBlendModeSrcColorFactor(blendMode);
    SDL_BlendFactor srcAlphaFactor = SDL_GetBlendModeSrcAlphaFactor(blendMode);
    SDL_BlendOperation colorOperation = SDL_GetBlendModeColorOperation(blendMode);
    SDL_BlendFactor dstColorFactor = SDL_GetBlendModeDstColorFactor(blendMode);
    SDL_BlendFactor dstAlphaFactor = SDL_GetBlendModeDstAlphaFactor(blendMode);
    SDL_BlendOperation alphaOperation = SDL_GetBlendModeAlphaOperation(blendMode);

    if (GetBlendFactor(srcColorFactor) == invalidBlendFactor ||
        GetBlendFactor(srcAlphaFactor) == invalidBlendFactor ||
        GetBlendOperation(colorOperation) == invalidBlendOperation ||
        GetBlendFactor(dstColorFactor) == invalidBlendFactor ||
        GetBlendFactor(dstAlphaFactor) == invalidBlendFactor ||
        GetBlendOperation(alphaOperation) == invalidBlendOperation) {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}

static int
METAL_CreateTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{ @autoreleasepool {
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    MTLPixelFormat pixfmt;

    switch (texture->format) {
        case SDL_PIXELFORMAT_ABGR8888:
            pixfmt = MTLPixelFormatRGBA8Unorm;
            break;
        case SDL_PIXELFORMAT_ARGB8888:
            pixfmt = MTLPixelFormatBGRA8Unorm;
            break;
        case SDL_PIXELFORMAT_IYUV:
        case SDL_PIXELFORMAT_YV12:
        case SDL_PIXELFORMAT_NV12:
        case SDL_PIXELFORMAT_NV21:
            pixfmt = MTLPixelFormatR8Unorm;
            break;
        default:
            return SDL_SetError("Texture format %s not supported by Metal", SDL_GetPixelFormatName(texture->format));
    }

    MTLTextureDescriptor *mtltexdesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixfmt
                                            width:(NSUInteger)texture->w height:(NSUInteger)texture->h mipmapped:NO];

    /* Not available in iOS 8. */
    if ([mtltexdesc respondsToSelector:@selector(usage)]) {
        if (texture->access == SDL_TEXTUREACCESS_TARGET) {
            mtltexdesc.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
        } else {
            mtltexdesc.usage = MTLTextureUsageShaderRead;
        }
    }
    
    id<MTLTexture> mtltexture = [data.mtldevice newTextureWithDescriptor:mtltexdesc];
    if (mtltexture == nil) {
        return SDL_SetError("Texture allocation failed");
    }

    id<MTLTexture> mtltexture_uv = nil;

    BOOL yuv = (texture->format == SDL_PIXELFORMAT_IYUV) || (texture->format == SDL_PIXELFORMAT_YV12);
    BOOL nv12 = (texture->format == SDL_PIXELFORMAT_NV12) || (texture->format == SDL_PIXELFORMAT_NV21);

    if (yuv) {
        mtltexdesc.pixelFormat = MTLPixelFormatR8Unorm;
        mtltexdesc.width = (texture->w + 1) / 2;
        mtltexdesc.height = (texture->h + 1) / 2;
        mtltexdesc.textureType = MTLTextureType2DArray;
        mtltexdesc.arrayLength = 2;
        mtltexture_uv = [data.mtldevice newTextureWithDescriptor:mtltexdesc];
    } else if (nv12) {
        mtltexdesc.pixelFormat = MTLPixelFormatRG8Unorm;
        mtltexdesc.width = (texture->w + 1) / 2;
        mtltexdesc.height = (texture->h + 1) / 2;
        mtltexture_uv = [data.mtldevice newTextureWithDescriptor:mtltexdesc];
    }

    METAL_TextureData *texturedata = [[METAL_TextureData alloc] init];
    const char *hint = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);
    if (!hint || *hint == '0' || SDL_strcasecmp(hint, "nearest") == 0) {
        texturedata.mtlsampler = data.mtlsamplernearest;
    } else {
        texturedata.mtlsampler = data.mtlsamplerlinear;
    }
    texturedata.mtltexture = mtltexture;
    texturedata.mtltexture_uv = mtltexture_uv;

    texturedata.yuv = yuv;
    texturedata.nv12 = nv12;

    if (yuv) {
        texturedata.fragmentFunction = SDL_METAL_FRAGMENT_YUV;
    } else if (texture->format == SDL_PIXELFORMAT_NV12) {
        texturedata.fragmentFunction = SDL_METAL_FRAGMENT_NV12;
    } else if (texture->format == SDL_PIXELFORMAT_NV21) {
        texturedata.fragmentFunction = SDL_METAL_FRAGMENT_NV21;
    } else {
        texturedata.fragmentFunction = SDL_METAL_FRAGMENT_COPY;
    }

    if (yuv || nv12) {
        size_t offset = 0;
        SDL_YUV_CONVERSION_MODE mode = SDL_GetYUVConversionModeForResolution(texture->w, texture->h);
        switch (mode) {
            case SDL_YUV_CONVERSION_JPEG: offset = CONSTANTS_OFFSET_DECODE_JPEG; break;
            case SDL_YUV_CONVERSION_BT601: offset = CONSTANTS_OFFSET_DECODE_BT601; break;
            case SDL_YUV_CONVERSION_BT709: offset = CONSTANTS_OFFSET_DECODE_BT709; break;
            default: offset = 0; break;
        }
        texturedata.conversionBufferOffset = offset;
    }

    texture->driverdata = (void*)CFBridgingRetain(texturedata);

#if !__has_feature(objc_arc)
    [texturedata release];
    [mtltexture release];
    [mtltexture_uv release];
#endif

    return 0;
}}

static int
METAL_UpdateTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                 const SDL_Rect * rect, const void *pixels, int pitch)
{ @autoreleasepool {
    METAL_TextureData *texturedata = (__bridge METAL_TextureData *)texture->driverdata;

    /* !!! FIXME: replaceRegion does not do any synchronization, so it might
     * !!! FIXME: stomp on a previous frame's data that's currently being read
     * !!! FIXME: by the GPU. */
    [texturedata.mtltexture replaceRegion:MTLRegionMake2D(rect->x, rect->y, rect->w, rect->h)
                              mipmapLevel:0
                                withBytes:pixels
                              bytesPerRow:pitch];

    if (texturedata.yuv) {
        int Uslice = texture->format == SDL_PIXELFORMAT_YV12 ? 1 : 0;
        int Vslice = texture->format == SDL_PIXELFORMAT_YV12 ? 0 : 1;

        /* Skip to the correct offset into the next texture */
        pixels = (const void*)((const Uint8*)pixels + rect->h * pitch);
        [texturedata.mtltexture_uv replaceRegion:MTLRegionMake2D(rect->x / 2, rect->y / 2, (rect->w + 1) / 2, (rect->h + 1) / 2)
                                     mipmapLevel:0
                                           slice:Uslice
                                       withBytes:pixels
                                     bytesPerRow:(pitch + 1) / 2
                                   bytesPerImage:0];

        /* Skip to the correct offset into the next texture */
        pixels = (const void*)((const Uint8*)pixels + ((rect->h + 1) / 2) * ((pitch + 1)/2));
        [texturedata.mtltexture_uv replaceRegion:MTLRegionMake2D(rect->x / 2, rect->y / 2, (rect->w + 1) / 2, (rect->h + 1) / 2)
                                     mipmapLevel:0
                                           slice:Vslice
                                       withBytes:pixels
                                     bytesPerRow:(pitch + 1) / 2
                                   bytesPerImage:0];
    }

    if (texturedata.nv12) {
        /* Skip to the correct offset into the next texture */
        pixels = (const void*)((const Uint8*)pixels + rect->h * pitch);
        [texturedata.mtltexture_uv replaceRegion:MTLRegionMake2D(rect->x / 2, rect->y / 2, (rect->w + 1) / 2, (rect->h + 1) / 2)
                                     mipmapLevel:0
                                           slice:0
                                       withBytes:pixels
                                     bytesPerRow:2 * ((pitch + 1) / 2)
                                   bytesPerImage:0];
    }

    return 0;
}}

static int
METAL_UpdateTextureYUV(SDL_Renderer * renderer, SDL_Texture * texture,
                    const SDL_Rect * rect,
                    const Uint8 *Yplane, int Ypitch,
                    const Uint8 *Uplane, int Upitch,
                    const Uint8 *Vplane, int Vpitch)
{ @autoreleasepool {
    METAL_TextureData *texturedata = (__bridge METAL_TextureData *)texture->driverdata;
    int Uslice = texture->format == SDL_PIXELFORMAT_YV12 ? 1 : 0;
    int Vslice = texture->format == SDL_PIXELFORMAT_YV12 ? 0 : 1;

    /* Bail out if we're supposed to update an empty rectangle */
    if (rect->w <= 0 || rect->h <= 0) {
        return 0;
    }

    [texturedata.mtltexture replaceRegion:MTLRegionMake2D(rect->x, rect->y, rect->w, rect->h)
                              mipmapLevel:0
                                withBytes:Yplane
                              bytesPerRow:Ypitch];

    [texturedata.mtltexture_uv replaceRegion:MTLRegionMake2D(rect->x / 2, rect->y / 2, (rect->w + 1) / 2, (rect->h + 1) / 2)
                                 mipmapLevel:0
                                       slice:Uslice
                                   withBytes:Uplane
                                 bytesPerRow:Upitch
                               bytesPerImage:0];

    [texturedata.mtltexture_uv replaceRegion:MTLRegionMake2D(rect->x / 2, rect->y / 2, (rect->w + 1) / 2, (rect->h + 1) / 2)
                                 mipmapLevel:0
                                       slice:Vslice
                                   withBytes:Vplane
                                 bytesPerRow:Vpitch
                               bytesPerImage:0];

    return 0;
}}

static int
METAL_LockTexture(SDL_Renderer * renderer, SDL_Texture * texture,
               const SDL_Rect * rect, void **pixels, int *pitch)
{
    return SDL_Unsupported();   // !!! FIXME: write me
}

static void
METAL_UnlockTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    // !!! FIXME: write me
}

static int
METAL_SetRenderTarget(SDL_Renderer * renderer, SDL_Texture * texture)
{ @autoreleasepool {
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;

    if (data.mtlcmdencoder) {
        /* End encoding for the previous render target so we can set up a new
         * render pass for this one. */
        [data.mtlcmdencoder endEncoding];
        [data.mtlcmdbuffer commit];

        data.mtlcmdencoder = nil;
        data.mtlcmdbuffer = nil;
    }

    /* We don't begin a new render pass right away - we delay it until an actual
     * draw or clear happens. That way we can use hardware clears when possible,
     * which are only available when beginning a new render pass. */
    return 0;
}}

static int
METAL_SetOrthographicProjection(SDL_Renderer *renderer, int w, int h)
{ @autoreleasepool {
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    float projection[4][4];

    if (!w || !h) {
        return 0;
    }

    /* Prepare an orthographic projection */
    projection[0][0] = 2.0f / w;
    projection[0][1] = 0.0f;
    projection[0][2] = 0.0f;
    projection[0][3] = 0.0f;
    projection[1][0] = 0.0f;
    projection[1][1] = -2.0f / h;
    projection[1][2] = 0.0f;
    projection[1][3] = 0.0f;
    projection[2][0] = 0.0f;
    projection[2][1] = 0.0f;
    projection[2][2] = 0.0f;
    projection[2][3] = 0.0f;
    projection[3][0] = -1.0f;
    projection[3][1] = 1.0f;
    projection[3][2] = 0.0f;
    projection[3][3] = 1.0f;

    // !!! FIXME: This should be in a buffer...
    [data.mtlcmdencoder setVertexBytes:projection length:sizeof(float)*16 atIndex:2];
    return 0;
}}

static int
METAL_UpdateViewport(SDL_Renderer * renderer)
{ @autoreleasepool {
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    if (data.mtlcmdencoder) {
        MTLViewport viewport;
        viewport.originX = renderer->viewport.x;
        viewport.originY = renderer->viewport.y;
        viewport.width = renderer->viewport.w;
        viewport.height = renderer->viewport.h;
        viewport.znear = 0.0;
        viewport.zfar = 1.0;
        [data.mtlcmdencoder setViewport:viewport];
        METAL_SetOrthographicProjection(renderer, renderer->viewport.w, renderer->viewport.h);
    }
    return 0;
}}

static int
METAL_UpdateClipRect(SDL_Renderer * renderer)
{ @autoreleasepool {
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    if (data.mtlcmdencoder) {
        MTLScissorRect mtlrect;
        // !!! FIXME: should this care about the viewport?
        if (renderer->clipping_enabled) {
            const SDL_Rect *rect = &renderer->clip_rect;
            mtlrect.x = renderer->viewport.x + rect->x;
            mtlrect.y = renderer->viewport.x + rect->y;
            mtlrect.width = rect->w;
            mtlrect.height = rect->h;
        } else {
            mtlrect.x = renderer->viewport.x;
            mtlrect.y = renderer->viewport.y;
            mtlrect.width = renderer->viewport.w;
            mtlrect.height = renderer->viewport.h;
        }
        if (mtlrect.width > 0 && mtlrect.height > 0) {
            [data.mtlcmdencoder setScissorRect:mtlrect];
        }
    }
    return 0;
}}

static int
METAL_RenderClear(SDL_Renderer * renderer)
{ @autoreleasepool {
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;

    /* Since we set up the render command encoder lazily when a draw is
     * requested, we can do the fast path hardware clear if no draws have
     * happened since the last SetRenderTarget. */
    if (data.mtlcmdencoder == nil) {
        METAL_ActivateRenderCommandEncoder(renderer, MTLLoadActionClear);
    } else {
        // !!! FIXME: render color should live in a dedicated uniform buffer.
        const float color[4] = { ((float)renderer->r) / 255.0f, ((float)renderer->g) / 255.0f, ((float)renderer->b) / 255.0f, ((float)renderer->a) / 255.0f };

        MTLViewport viewport;  // RenderClear ignores the viewport state, though, so reset that.
        viewport.originX = viewport.originY = 0.0;
        viewport.width = data.mtlpassdesc.colorAttachments[0].texture.width;
        viewport.height = data.mtlpassdesc.colorAttachments[0].texture.height;
        viewport.znear = 0.0;
        viewport.zfar = 1.0;

        // Slow path for clearing: draw a filled fullscreen triangle.
        METAL_SetOrthographicProjection(renderer, 1, 1);
        [data.mtlcmdencoder setViewport:viewport];
        [data.mtlcmdencoder setRenderPipelineState:ChoosePipelineState(data, data.activepipelines, SDL_METAL_FRAGMENT_SOLID, SDL_BLENDMODE_NONE)];
        [data.mtlcmdencoder setVertexBuffer:data.mtlbufconstants offset:CONSTANTS_OFFSET_CLEAR_VERTS atIndex:0];
        [data.mtlcmdencoder setVertexBuffer:data.mtlbufconstants offset:CONSTANTS_OFFSET_IDENTITY atIndex:3];
        [data.mtlcmdencoder setFragmentBytes:color length:sizeof(color) atIndex:0];
        [data.mtlcmdencoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];

        // reset the viewport for the rest of our usual drawing work...
        viewport.originX = renderer->viewport.x;
        viewport.originY = renderer->viewport.y;
        viewport.width = renderer->viewport.w;
        viewport.height = renderer->viewport.h;
        viewport.znear = 0.0;
        viewport.zfar = 1.0;
        [data.mtlcmdencoder setViewport:viewport];
        METAL_SetOrthographicProjection(renderer, renderer->viewport.w, renderer->viewport.h);
    }

    return 0;
}}

// normalize a value from 0.0f to len into 0.0f to 1.0f.
static inline float
normtex(const float _val, const float len)
{
    return _val / len;
}

static int
DrawVerts(SDL_Renderer * renderer, const SDL_FPoint * points, int count,
          const MTLPrimitiveType primtype)
{ @autoreleasepool {
    METAL_ActivateRenderCommandEncoder(renderer, MTLLoadActionLoad);

    const size_t vertlen = (sizeof (float) * 2) * count;
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;

    // !!! FIXME: render color should live in a dedicated uniform buffer.
    const float color[4] = { ((float)renderer->r) / 255.0f, ((float)renderer->g) / 255.0f, ((float)renderer->b) / 255.0f, ((float)renderer->a) / 255.0f };

    [data.mtlcmdencoder setRenderPipelineState:ChoosePipelineState(data, data.activepipelines, SDL_METAL_FRAGMENT_SOLID, renderer->blendMode)];
    [data.mtlcmdencoder setFragmentBytes:color length:sizeof(color) atIndex:0];

    [data.mtlcmdencoder setVertexBytes:points length:vertlen atIndex:0];
    [data.mtlcmdencoder setVertexBuffer:data.mtlbufconstants offset:CONSTANTS_OFFSET_HALF_PIXEL_TRANSFORM atIndex:3];
    [data.mtlcmdencoder drawPrimitives:primtype vertexStart:0 vertexCount:count];

    return 0;
}}

static int
METAL_RenderDrawPoints(SDL_Renderer * renderer, const SDL_FPoint * points, int count)
{
    return DrawVerts(renderer, points, count, MTLPrimitiveTypePoint);
}

static int
METAL_RenderDrawLines(SDL_Renderer * renderer, const SDL_FPoint * points, int count)
{
    return DrawVerts(renderer, points, count, MTLPrimitiveTypeLineStrip);
}

static int
METAL_RenderFillRects(SDL_Renderer * renderer, const SDL_FRect * rects, int count)
{ @autoreleasepool {
    METAL_ActivateRenderCommandEncoder(renderer, MTLLoadActionLoad);
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;

    // !!! FIXME: render color should live in a dedicated uniform buffer.
    const float color[4] = { ((float)renderer->r) / 255.0f, ((float)renderer->g) / 255.0f, ((float)renderer->b) / 255.0f, ((float)renderer->a) / 255.0f };

    [data.mtlcmdencoder setRenderPipelineState:ChoosePipelineState(data, data.activepipelines, SDL_METAL_FRAGMENT_SOLID, renderer->blendMode)];
    [data.mtlcmdencoder setFragmentBytes:color length:sizeof(color) atIndex:0];
    [data.mtlcmdencoder setVertexBuffer:data.mtlbufconstants offset:CONSTANTS_OFFSET_IDENTITY atIndex:3];

    for (int i = 0; i < count; i++, rects++) {
        if ((rects->w <= 0.0f) || (rects->h <= 0.0f)) continue;

        const float verts[] = {
            rects->x, rects->y + rects->h,
            rects->x, rects->y,
            rects->x + rects->w, rects->y + rects->h,
            rects->x + rects->w, rects->y
        };

        [data.mtlcmdencoder setVertexBytes:verts length:sizeof(verts) atIndex:0];
        [data.mtlcmdencoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    }

    return 0;
}}

static void
METAL_SetupRenderCopy(METAL_RenderData *data, SDL_Texture *texture, METAL_TextureData *texturedata)
{
    float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    if (texture->modMode) {
        color[0] = ((float)texture->r) / 255.0f;
        color[1] = ((float)texture->g) / 255.0f;
        color[2] = ((float)texture->b) / 255.0f;
        color[3] = ((float)texture->a) / 255.0f;
    }

    [data.mtlcmdencoder setRenderPipelineState:ChoosePipelineState(data, data.activepipelines, texturedata.fragmentFunction, texture->blendMode)];
    [data.mtlcmdencoder setFragmentBytes:color length:sizeof(color) atIndex:0];
    [data.mtlcmdencoder setFragmentSamplerState:texturedata.mtlsampler atIndex:0];

    [data.mtlcmdencoder setFragmentTexture:texturedata.mtltexture atIndex:0];

    if (texturedata.yuv || texturedata.nv12) {
        [data.mtlcmdencoder setFragmentTexture:texturedata.mtltexture_uv atIndex:1];
        [data.mtlcmdencoder setFragmentBuffer:data.mtlbufconstants offset:texturedata.conversionBufferOffset atIndex:1];
    }
}

static int
METAL_RenderCopy(SDL_Renderer * renderer, SDL_Texture * texture,
              const SDL_Rect * srcrect, const SDL_FRect * dstrect)
{ @autoreleasepool {
    METAL_ActivateRenderCommandEncoder(renderer, MTLLoadActionLoad);
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    METAL_TextureData *texturedata = (__bridge METAL_TextureData *)texture->driverdata;
    const float texw = (float) texturedata.mtltexture.width;
    const float texh = (float) texturedata.mtltexture.height;

    METAL_SetupRenderCopy(data, texture, texturedata);

    const float xy[] = {
        dstrect->x, dstrect->y + dstrect->h,
        dstrect->x, dstrect->y,
        dstrect->x + dstrect->w, dstrect->y + dstrect->h,
        dstrect->x + dstrect->w, dstrect->y
    };

    const float uv[] = {
        normtex(srcrect->x, texw), normtex(srcrect->y + srcrect->h, texh),
        normtex(srcrect->x, texw), normtex(srcrect->y, texh),
        normtex(srcrect->x + srcrect->w, texw), normtex(srcrect->y + srcrect->h, texh),
        normtex(srcrect->x + srcrect->w, texw), normtex(srcrect->y, texh)
    };

    [data.mtlcmdencoder setVertexBytes:xy length:sizeof(xy) atIndex:0];
    [data.mtlcmdencoder setVertexBytes:uv length:sizeof(uv) atIndex:1];
    [data.mtlcmdencoder setVertexBuffer:data.mtlbufconstants offset:CONSTANTS_OFFSET_IDENTITY atIndex:3];
    [data.mtlcmdencoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];

    return 0;
}}

static int
METAL_RenderCopyEx(SDL_Renderer * renderer, SDL_Texture * texture,
              const SDL_Rect * srcrect, const SDL_FRect * dstrect,
              const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip)
{ @autoreleasepool {
    METAL_ActivateRenderCommandEncoder(renderer, MTLLoadActionLoad);
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    METAL_TextureData *texturedata = (__bridge METAL_TextureData *)texture->driverdata;
    const float texw = (float) texturedata.mtltexture.width;
    const float texh = (float) texturedata.mtltexture.height;
    float transform[16];
    float minu, maxu, minv, maxv;

    METAL_SetupRenderCopy(data, texture, texturedata);

    minu = normtex(srcrect->x, texw);
    maxu = normtex(srcrect->x + srcrect->w, texw);
    minv = normtex(srcrect->y, texh);
    maxv = normtex(srcrect->y + srcrect->h, texh);

    if (flip & SDL_FLIP_HORIZONTAL) {
        float tmp = maxu;
        maxu = minu;
        minu = tmp;
    }
    if (flip & SDL_FLIP_VERTICAL) {
        float tmp = maxv;
        maxv = minv;
        minv = tmp;
    }

    const float uv[] = {
        minu, maxv,
        minu, minv,
        maxu, maxv,
        maxu, minv
    };

    const float xy[] = {
        -center->x, dstrect->h - center->y,
        -center->x, -center->y,
        dstrect->w - center->x, dstrect->h - center->y,
        dstrect->w - center->x, -center->y
    };

    {
        float rads = (float)(M_PI * (float) angle / 180.0f);
        float c = cosf(rads), s = sinf(rads);
        SDL_memset(transform, 0, sizeof(transform));

        transform[10] = transform[15] = 1.0f;

        /* Rotation */
        transform[0]  = c;
        transform[1]  = s;
        transform[4]  = -s;
        transform[5]  = c;

        /* Translation */
        transform[12] = dstrect->x + center->x;
        transform[13] = dstrect->y + center->y;
    }

    [data.mtlcmdencoder setVertexBytes:xy length:sizeof(xy) atIndex:0];
    [data.mtlcmdencoder setVertexBytes:uv length:sizeof(uv) atIndex:1];
    [data.mtlcmdencoder setVertexBytes:transform length:sizeof(transform) atIndex:3];
    [data.mtlcmdencoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];

    return 0;
}}

static int
METAL_RenderReadPixels(SDL_Renderer * renderer, const SDL_Rect * rect,
                    Uint32 pixel_format, void * pixels, int pitch)
{ @autoreleasepool {
    METAL_ActivateRenderCommandEncoder(renderer, MTLLoadActionLoad);

    // !!! FIXME: this probably needs to commit the current command buffer, and probably waitUntilCompleted
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    id<MTLTexture> mtltexture = data.mtlpassdesc.colorAttachments[0].texture;
    MTLRegion mtlregion = MTLRegionMake2D(rect->x, rect->y, rect->w, rect->h);

    // we only do BGRA8 or RGBA8 at the moment, so 4 will do.
    const int temp_pitch = rect->w * 4;
    void *temp_pixels = SDL_malloc(temp_pitch * rect->h);
    if (!temp_pixels) {
        return SDL_OutOfMemory();
    }

    [mtltexture getBytes:temp_pixels bytesPerRow:temp_pitch fromRegion:mtlregion mipmapLevel:0];

    const Uint32 temp_format = (mtltexture.pixelFormat == MTLPixelFormatBGRA8Unorm) ? SDL_PIXELFORMAT_ARGB8888 : SDL_PIXELFORMAT_ABGR8888;
    const int status = SDL_ConvertPixels(rect->w, rect->h, temp_format, temp_pixels, temp_pitch, pixel_format, pixels, pitch);
    SDL_free(temp_pixels);
    return status;
}}

static void
METAL_RenderPresent(SDL_Renderer * renderer)
{ @autoreleasepool {
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;

    if (data.mtlcmdencoder != nil) {
        [data.mtlcmdencoder endEncoding];
    }
    if (data.mtlbackbuffer != nil) {
        [data.mtlcmdbuffer presentDrawable:data.mtlbackbuffer];
    }
    if (data.mtlcmdbuffer != nil) {
        [data.mtlcmdbuffer commit];
    }
    data.mtlcmdencoder = nil;
    data.mtlcmdbuffer = nil;
    data.mtlbackbuffer = nil;
}}

static void
METAL_DestroyTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{ @autoreleasepool {
    CFBridgingRelease(texture->driverdata);
    texture->driverdata = NULL;
}}

static void
METAL_DestroyRenderer(SDL_Renderer * renderer)
{ @autoreleasepool {
    if (renderer->driverdata) {
        METAL_RenderData *data = CFBridgingRelease(renderer->driverdata);

        if (data.mtlcmdencoder != nil) {
            [data.mtlcmdencoder endEncoding];
        }

        DestroyAllPipelines(data.allpipelines, data.pipelinescount);
    }

    SDL_free(renderer);
}}

static void *
METAL_GetMetalLayer(SDL_Renderer * renderer)
{ @autoreleasepool {
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    return (__bridge void*)data.mtllayer;
}}

static void *
METAL_GetMetalCommandEncoder(SDL_Renderer * renderer)
{ @autoreleasepool {
    METAL_ActivateRenderCommandEncoder(renderer, MTLLoadActionLoad);
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    return (__bridge void*)data.mtlcmdencoder;
}}

#endif /* SDL_VIDEO_RENDER_METAL && !SDL_RENDER_DISABLED */

/* vi: set ts=4 sw=4 expandtab: */
