/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

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
#include "SDL_syswm.h"
#include "SDL_metal.h"
#include "../SDL_sysrender.h"

#include <Availability.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#ifdef __MACOSX__
#import <AppKit/NSView.h>
#endif

/* Regenerate these with build-metal-shaders.sh */
#ifdef __MACOSX__
#include "SDL_shaders_metal_osx.h"
#elif defined(__TVOS__)
#if TARGET_OS_SIMULATOR
#include "SDL_shaders_metal_tvsimulator.h"
#else
#include "SDL_shaders_metal_tvos.h"
#endif
#else
#if TARGET_OS_SIMULATOR
#include "SDL_shaders_metal_iphonesimulator.h"
#else
#include "SDL_shaders_metal_ios.h"
#endif
#endif

/* Apple Metal renderer implementation */

/* Used to re-create the window with Metal capability */
extern int SDL_RecreateWindow(SDL_Window * window, Uint32 flags);

/* macOS requires constants in a buffer to have a 256 byte alignment. */
/* Use native type alignments from https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf */
#if defined(__MACOSX__) || TARGET_OS_SIMULATOR || TARGET_OS_MACCATALYST
#define CONSTANT_ALIGN(x) (256)
#else
#define CONSTANT_ALIGN(x) (x < 4 ? 4 : x)
#endif

#define DEVICE_ALIGN(x) (x < 4 ? 4 : x)

#define ALIGN_CONSTANTS(align, size) ((size + CONSTANT_ALIGN(align) - 1) & (~(CONSTANT_ALIGN(align) - 1)))

static const size_t CONSTANTS_OFFSET_INVALID = 0xFFFFFFFF;
static const size_t CONSTANTS_OFFSET_IDENTITY = 0;
static const size_t CONSTANTS_OFFSET_HALF_PIXEL_TRANSFORM = ALIGN_CONSTANTS(16, CONSTANTS_OFFSET_IDENTITY + sizeof(float) * 16);
static const size_t CONSTANTS_OFFSET_DECODE_JPEG = ALIGN_CONSTANTS(16, CONSTANTS_OFFSET_HALF_PIXEL_TRANSFORM + sizeof(float) * 16);
static const size_t CONSTANTS_OFFSET_DECODE_BT601 = ALIGN_CONSTANTS(16, CONSTANTS_OFFSET_DECODE_JPEG + sizeof(float) * 4 * 4);
static const size_t CONSTANTS_OFFSET_DECODE_BT709 = ALIGN_CONSTANTS(16, CONSTANTS_OFFSET_DECODE_BT601 + sizeof(float) * 4 * 4);
static const size_t CONSTANTS_LENGTH = CONSTANTS_OFFSET_DECODE_BT709 + sizeof(float) * 4 * 4;

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
    @property (nonatomic, retain) id<MTLBuffer> mtlbufquadindices;
    @property (nonatomic, assign) SDL_MetalView mtlview;
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
    [_mtlbufquadindices release];
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
    @property (nonatomic, assign) BOOL hasdata;

    @property (nonatomic, retain) id<MTLBuffer> lockedbuffer;
    @property (nonatomic, assign) SDL_Rect lockedrect;
@end

@implementation METAL_TextureData
#if !__has_feature(objc_arc)
- (void)dealloc
{
    [_mtltexture release];
    [_mtltexture_uv release];
    [_mtlsampler release];
    [_lockedbuffer release];
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

    MTLVertexDescriptor *vertdesc = [MTLVertexDescriptor vertexDescriptor];

    switch (cache->vertexFunction) {
        case SDL_METAL_VERTEX_SOLID:
            /* position (float2) */
            vertdesc.layouts[0].stride = sizeof(float) * 2;
            vertdesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

            vertdesc.attributes[0].format = MTLVertexFormatFloat2;
            vertdesc.attributes[0].offset = 0;
            vertdesc.attributes[0].bufferIndex = 0;
            break;
        case SDL_METAL_VERTEX_COPY:
            /* position (float2), texcoord (float2) */
            vertdesc.layouts[0].stride = sizeof(float) * 4;
            vertdesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

            vertdesc.attributes[0].format = MTLVertexFormatFloat2;
            vertdesc.attributes[0].offset = 0;
            vertdesc.attributes[0].bufferIndex = 0;

            vertdesc.attributes[1].format = MTLVertexFormatFloat2;
            vertdesc.attributes[1].offset = sizeof(float) * 2;
            vertdesc.attributes[1].bufferIndex = 0;
            break;
    }

    mtlpipedesc.vertexDescriptor = vertdesc;

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
    MakePipelineState(data, cache, @" (blend=mul)", SDL_BLENDMODE_MUL);
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

static void
METAL_ActivateRenderCommandEncoder(SDL_Renderer * renderer, MTLLoadAction load, MTLClearColor *clear_color, id<MTLBuffer> vertex_buffer)
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
            SDL_assert(clear_color != NULL);
            data.mtlpassdesc.colorAttachments[0].clearColor = *clear_color;
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

        /* Set up buffer bindings for positions, texcoords, and color once here,
         * the offsets are adjusted in the code that uses them. */
        if (vertex_buffer != nil) {
            [data.mtlcmdencoder setVertexBuffer:vertex_buffer offset:0 atIndex:0];
            [data.mtlcmdencoder setFragmentBuffer:vertex_buffer offset:0 atIndex:0];
        }

        data.activepipelines = ChooseShaderPipelines(data, mtltexture.pixelFormat);

        // make sure this has a definite place in the queue. This way it will
        //  execute reliably whether the app tries to make its own command buffers
        //  or whatever. This means we can _always_ batch rendering commands!
        [data.mtlcmdbuffer enqueue];
    }
}

static void
METAL_WindowEvent(SDL_Renderer * renderer, const SDL_WindowEvent *event)
{
    if (event->event == SDL_WINDOWEVENT_SHOWN ||
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
    } else if (nv12) {
        mtltexdesc.pixelFormat = MTLPixelFormatRG8Unorm;
        mtltexdesc.width = (texture->w + 1) / 2;
        mtltexdesc.height = (texture->h + 1) / 2;
    }

    if (yuv || nv12) {
        mtltexture_uv = [data.mtldevice newTextureWithDescriptor:mtltexdesc];
        if (mtltexture_uv == nil) {
#if !__has_feature(objc_arc)
            [mtltexture release];
#endif
            return SDL_SetError("Texture allocation failed");
        }
    }

    METAL_TextureData *texturedata = [[METAL_TextureData alloc] init];
    if (texture->scaleMode == SDL_ScaleModeNearest) {
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

static void
METAL_UploadTextureData(id<MTLTexture> texture, SDL_Rect rect, int slice,
                        const void * pixels, int pitch)
{
    [texture replaceRegion:MTLRegionMake2D(rect.x, rect.y, rect.w, rect.h)
               mipmapLevel:0
                     slice:slice
                 withBytes:pixels
               bytesPerRow:pitch
             bytesPerImage:0];
}

static MTLStorageMode
METAL_GetStorageMode(id<MTLResource> resource)
{
    /* iOS 8 does not have this method. */
    if ([resource respondsToSelector:@selector(storageMode)]) {
        return resource.storageMode;
    }
    return MTLStorageModeShared;
}

static int
METAL_UpdateTextureInternal(SDL_Renderer * renderer, METAL_TextureData *texturedata,
                            id<MTLTexture> texture, SDL_Rect rect, int slice,
                            const void * pixels, int pitch)
{
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    SDL_Rect stagingrect = {0, 0, rect.w, rect.h};
    MTLTextureDescriptor *desc;

    /* If the texture is managed or shared and this is the first upload, we can
     * use replaceRegion to upload to it directly. Otherwise we upload the data
     * to a staging texture and copy that over. */
    if (!texturedata.hasdata && METAL_GetStorageMode(texture) != MTLStorageModePrivate) {
        METAL_UploadTextureData(texture, rect, slice, pixels, pitch);
        return 0;
    }

    desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:texture.pixelFormat
                                                              width:rect.w
                                                             height:rect.h
                                                          mipmapped:NO];

    if (desc == nil) {
        return SDL_OutOfMemory();
    }

    /* TODO: We could have a pool of textures or a MTLHeap we allocate from,
     * and release a staging texture back to the pool in the command buffer's
     * completion handler. */
    id<MTLTexture> stagingtex = [data.mtldevice newTextureWithDescriptor:desc];
    if (stagingtex == nil) {
        return SDL_OutOfMemory();
    }

#if !__has_feature(objc_arc)
    [stagingtex autorelease];
#endif

    METAL_UploadTextureData(stagingtex, stagingrect, 0, pixels, pitch);

    if (data.mtlcmdencoder != nil) {
        [data.mtlcmdencoder endEncoding];
        data.mtlcmdencoder = nil;
    }

    if (data.mtlcmdbuffer == nil) {
        data.mtlcmdbuffer = [data.mtlcmdqueue commandBuffer];
    }

    id<MTLBlitCommandEncoder> blitcmd = [data.mtlcmdbuffer blitCommandEncoder];

    [blitcmd copyFromTexture:stagingtex
                 sourceSlice:0
                 sourceLevel:0
                sourceOrigin:MTLOriginMake(0, 0, 0)
                  sourceSize:MTLSizeMake(rect.w, rect.h, 1)
                   toTexture:texture
            destinationSlice:slice
            destinationLevel:0
           destinationOrigin:MTLOriginMake(rect.x, rect.y, 0)];

    [blitcmd endEncoding];

    /* TODO: This isn't very efficient for the YUV formats, which call
     * UpdateTextureInternal multiple times in a row. */
    [data.mtlcmdbuffer commit];
    data.mtlcmdbuffer = nil;

    return 0;
}

static int
METAL_UpdateTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                    const SDL_Rect * rect, const void *pixels, int pitch)
{ @autoreleasepool {
    METAL_TextureData *texturedata = (__bridge METAL_TextureData *)texture->driverdata;

    if (METAL_UpdateTextureInternal(renderer, texturedata, texturedata.mtltexture, *rect, 0, pixels, pitch) < 0) {
        return -1;
    }

    if (texturedata.yuv) {
        int Uslice = texture->format == SDL_PIXELFORMAT_YV12 ? 1 : 0;
        int Vslice = texture->format == SDL_PIXELFORMAT_YV12 ? 0 : 1;
        int UVpitch = (pitch + 1) / 2;
        SDL_Rect UVrect = {rect->x / 2, rect->y / 2, (rect->w + 1) / 2, (rect->h + 1) / 2};

        /* Skip to the correct offset into the next texture */
        pixels = (const void*)((const Uint8*)pixels + rect->h * pitch);
        if (METAL_UpdateTextureInternal(renderer, texturedata, texturedata.mtltexture_uv, UVrect, Uslice, pixels, UVpitch) < 0) {
            return -1;
        }

        /* Skip to the correct offset into the next texture */
        pixels = (const void*)((const Uint8*)pixels + UVrect.h * UVpitch);
        if (METAL_UpdateTextureInternal(renderer, texturedata, texturedata.mtltexture_uv, UVrect, Vslice, pixels, UVpitch) < 0) {
            return -1;
        }
    }

    if (texturedata.nv12) {
        SDL_Rect UVrect = {rect->x / 2, rect->y / 2, (rect->w + 1) / 2, (rect->h + 1) / 2};
        int UVpitch = 2 * ((pitch + 1) / 2);

        /* Skip to the correct offset into the next texture */
        pixels = (const void*)((const Uint8*)pixels + rect->h * pitch);
        if (METAL_UpdateTextureInternal(renderer, texturedata, texturedata.mtltexture_uv, UVrect, 0, pixels, UVpitch) < 0) {
            return -1;
        }
    }

    texturedata.hasdata = YES;

    return 0;
}}

#if SDL_HAVE_YUV
static int
METAL_UpdateTextureYUV(SDL_Renderer * renderer, SDL_Texture * texture,
                    const SDL_Rect * rect,
                    const Uint8 *Yplane, int Ypitch,
                    const Uint8 *Uplane, int Upitch,
                    const Uint8 *Vplane, int Vpitch)
{ @autoreleasepool {
    METAL_TextureData *texturedata = (__bridge METAL_TextureData *)texture->driverdata;
    const int Uslice = 0;
    const int Vslice = 1;
    SDL_Rect UVrect = {rect->x / 2, rect->y / 2, (rect->w + 1) / 2, (rect->h + 1) / 2};

    /* Bail out if we're supposed to update an empty rectangle */
    if (rect->w <= 0 || rect->h <= 0) {
        return 0;
    }

    if (METAL_UpdateTextureInternal(renderer, texturedata, texturedata.mtltexture, *rect, 0, Yplane, Ypitch) < 0) {
        return -1;
    }
    if (METAL_UpdateTextureInternal(renderer, texturedata, texturedata.mtltexture_uv, UVrect, Uslice, Uplane, Upitch)) {
        return -1;
    }
    if (METAL_UpdateTextureInternal(renderer, texturedata, texturedata.mtltexture_uv, UVrect, Vslice, Vplane, Vpitch)) {
        return -1;
    }

    texturedata.hasdata = YES;

    return 0;
}}

static int
METAL_UpdateTextureNV(SDL_Renderer * renderer, SDL_Texture * texture,
                    const SDL_Rect * rect,
                    const Uint8 *Yplane, int Ypitch,
                    const Uint8 *UVplane, int UVpitch)
{ @autoreleasepool {
    METAL_TextureData *texturedata = (__bridge METAL_TextureData *)texture->driverdata;
    SDL_Rect UVrect = {rect->x / 2, rect->y / 2, (rect->w + 1) / 2, (rect->h + 1) / 2};

    /* Bail out if we're supposed to update an empty rectangle */
    if (rect->w <= 0 || rect->h <= 0) {
        return 0;
    }

    if (METAL_UpdateTextureInternal(renderer, texturedata, texturedata.mtltexture, *rect, 0, Yplane, Ypitch) < 0) {
        return -1;
    }

    if (METAL_UpdateTextureInternal(renderer, texturedata, texturedata.mtltexture_uv, UVrect, 0, UVplane, UVpitch) < 0) {
        return -1;
    }

    texturedata.hasdata = YES;

    return 0;
}}
#endif

static int
METAL_LockTexture(SDL_Renderer * renderer, SDL_Texture * texture,
               const SDL_Rect * rect, void **pixels, int *pitch)
{ @autoreleasepool {
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    METAL_TextureData *texturedata = (__bridge METAL_TextureData *)texture->driverdata;
    int buffersize = 0;
    id<MTLBuffer> lockedbuffer = nil;

    if (rect->w <= 0 || rect->h <= 0) {
        return SDL_SetError("Invalid rectangle dimensions for LockTexture.");
    }

    *pitch = SDL_BYTESPERPIXEL(texture->format) * rect->w;

    if (texturedata.yuv || texturedata.nv12) {
        buffersize = ((*pitch) * rect->h) + (2 * (*pitch + 1) / 2) * ((rect->h + 1) / 2);
    } else {
        buffersize = (*pitch) * rect->h;
    }

    lockedbuffer = [data.mtldevice newBufferWithLength:buffersize options:MTLResourceStorageModeShared];
    if (lockedbuffer == nil) {
        return SDL_OutOfMemory();
    }

    texturedata.lockedrect = *rect;
    texturedata.lockedbuffer = lockedbuffer;
    *pixels = [lockedbuffer contents];

    /* METAL_TextureData.lockedbuffer retains. */
#if !__has_feature(objc_arc)
    [lockedbuffer release];
#endif

    return 0;
}}

static void
METAL_UnlockTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{ @autoreleasepool {
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    METAL_TextureData *texturedata = (__bridge METAL_TextureData *)texture->driverdata;
    SDL_Rect rect = texturedata.lockedrect;
    int pitch = SDL_BYTESPERPIXEL(texture->format) * rect.w;
    SDL_Rect UVrect = {rect.x / 2, rect.y / 2, (rect.w + 1) / 2, (rect.h + 1) / 2};

    if (texturedata.lockedbuffer == nil) {
        return;
    }

    if (data.mtlcmdencoder != nil) {
        [data.mtlcmdencoder endEncoding];
        data.mtlcmdencoder = nil;
    }

    if (data.mtlcmdbuffer == nil) {
        data.mtlcmdbuffer = [data.mtlcmdqueue commandBuffer];
    }

    id<MTLBlitCommandEncoder> blitcmd = [data.mtlcmdbuffer blitCommandEncoder];

    [blitcmd copyFromBuffer:texturedata.lockedbuffer
               sourceOffset:0
          sourceBytesPerRow:pitch
        sourceBytesPerImage:0
                 sourceSize:MTLSizeMake(rect.w, rect.h, 1)
                  toTexture:texturedata.mtltexture
           destinationSlice:0
           destinationLevel:0
          destinationOrigin:MTLOriginMake(rect.x, rect.y, 0)];

    if (texturedata.yuv) {
        int Uslice = texture->format == SDL_PIXELFORMAT_YV12 ? 1 : 0;
        int Vslice = texture->format == SDL_PIXELFORMAT_YV12 ? 0 : 1;
        int UVpitch = (pitch + 1) / 2;

        [blitcmd copyFromBuffer:texturedata.lockedbuffer
                   sourceOffset:rect.h * pitch
              sourceBytesPerRow:UVpitch
            sourceBytesPerImage:UVpitch * UVrect.h
                     sourceSize:MTLSizeMake(UVrect.w, UVrect.h, 1)
                      toTexture:texturedata.mtltexture_uv
               destinationSlice:Uslice
               destinationLevel:0
              destinationOrigin:MTLOriginMake(UVrect.x, UVrect.y, 0)];

        [blitcmd copyFromBuffer:texturedata.lockedbuffer
                   sourceOffset:(rect.h * pitch) + UVrect.h * UVpitch
              sourceBytesPerRow:UVpitch
            sourceBytesPerImage:UVpitch * UVrect.h
                     sourceSize:MTLSizeMake(UVrect.w, UVrect.h, 1)
                      toTexture:texturedata.mtltexture_uv
               destinationSlice:Vslice
               destinationLevel:0
              destinationOrigin:MTLOriginMake(UVrect.x, UVrect.y, 0)];
    }

    if (texturedata.nv12) {
        int UVpitch = 2 * ((pitch + 1) / 2);

        [blitcmd copyFromBuffer:texturedata.lockedbuffer
                   sourceOffset:rect.h * pitch
              sourceBytesPerRow:UVpitch
            sourceBytesPerImage:0
                     sourceSize:MTLSizeMake(UVrect.w, UVrect.h, 1)
                      toTexture:texturedata.mtltexture_uv
               destinationSlice:0
               destinationLevel:0
              destinationOrigin:MTLOriginMake(UVrect.x, UVrect.y, 0)];
    }

    [blitcmd endEncoding];

    [data.mtlcmdbuffer commit];
    data.mtlcmdbuffer = nil;

    texturedata.lockedbuffer = nil; /* Retained property, so it calls release. */
    texturedata.hasdata = YES;
}}

static void
METAL_SetTextureScaleMode(SDL_Renderer * renderer, SDL_Texture * texture, SDL_ScaleMode scaleMode)
{ @autoreleasepool {
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    METAL_TextureData *texturedata = (__bridge METAL_TextureData *)texture->driverdata;

    if (scaleMode == SDL_ScaleModeNearest) {
        texturedata.mtlsampler = data.mtlsamplernearest;
    } else {
        texturedata.mtlsampler = data.mtlsamplerlinear;
    }
}}

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


// normalize a value from 0.0f to len into 0.0f to 1.0f.
static inline float
normtex(const float _val, const float len)
{
    return _val / len;
}

static int
METAL_QueueSetViewport(SDL_Renderer * renderer, SDL_RenderCommand *cmd)
{
    float projection[4][4];    /* Prepare an orthographic projection */
    const int w = cmd->data.viewport.rect.w;
    const int h = cmd->data.viewport.rect.h;
    const size_t matrixlen = sizeof (projection);
    float *matrix = (float *) SDL_AllocateRenderVertices(renderer, matrixlen, CONSTANT_ALIGN(16), &cmd->data.viewport.first);
    if (!matrix) {
        return -1;
    }

    SDL_memset(projection, '\0', matrixlen);
    if (w && h) {
        projection[0][0] = 2.0f / w;
        projection[1][1] = -2.0f / h;
        projection[3][0] = -1.0f;
        projection[3][1] = 1.0f;
        projection[3][3] = 1.0f;
    }
    SDL_memcpy(matrix, projection, matrixlen);

    return 0;
}

static int
METAL_QueueSetDrawColor(SDL_Renderer *renderer, SDL_RenderCommand *cmd)
{
    const size_t vertlen = sizeof (float) * 4;
    float *verts = (float *) SDL_AllocateRenderVertices(renderer, vertlen, DEVICE_ALIGN(16), &cmd->data.color.first);
    if (!verts) {
        return -1;
    }
    *(verts++) = ((float)cmd->data.color.r) / 255.0f;
    *(verts++) = ((float)cmd->data.color.g) / 255.0f;
    *(verts++) = ((float)cmd->data.color.b) / 255.0f;
    *(verts++) = ((float)cmd->data.color.a) / 255.0f;
    return 0;
}

static int
METAL_QueueDrawPoints(SDL_Renderer * renderer, SDL_RenderCommand *cmd, const SDL_FPoint * points, int count)
{
    const size_t vertlen = (sizeof (float) * 2) * count;
    float *verts = (float *) SDL_AllocateRenderVertices(renderer, vertlen, DEVICE_ALIGN(8), &cmd->data.draw.first);
    if (!verts) {
        return -1;
    }
    cmd->data.draw.count = count;
    SDL_memcpy(verts, points, vertlen);
    return 0;
}

static int
METAL_QueueDrawLines(SDL_Renderer * renderer, SDL_RenderCommand *cmd, const SDL_FPoint * points, int count)
{
    SDL_assert(count >= 2);  /* should have been checked at the higher level. */

    const size_t vertlen = (sizeof (float) * 2) * count;
    float *verts = (float *) SDL_AllocateRenderVertices(renderer, vertlen, DEVICE_ALIGN(8), &cmd->data.draw.first);
    if (!verts) {
        return -1;
    }
    cmd->data.draw.count = count;
    SDL_memcpy(verts, points, vertlen);

    /* If the line segment is completely horizontal or vertical,
       make it one pixel longer, to satisfy the diamond-exit rule.
       We should probably do this for diagonal lines too, but we'd have to
       do some trigonometry to figure out the correct pixel and generally
       when we have problems with pixel perfection, it's for straight lines
       that are missing a pixel that frames something and not arbitrary
       angles. Maybe !!! FIXME for later, though. */

    points += count - 2;  /* update the last line. */
    verts += (count * 2) - 2;

    const float xstart = points[0].x;
    const float ystart = points[0].y;
    const float xend = points[1].x;
    const float yend = points[1].y;

    if (ystart == yend) {  /* horizontal line */
        verts[0] += (xend > xstart) ? 1.0f : -1.0f;
    } else if (xstart == xend) {  /* vertical line */
        verts[1] += (yend > ystart) ? 1.0f : -1.0f;
    }

    return 0;
}

static int
METAL_QueueFillRects(SDL_Renderer * renderer, SDL_RenderCommand *cmd, const SDL_FRect * rects, int count)
{
    const size_t vertlen = (sizeof (float) * 8) * count;
    float *verts = (float *) SDL_AllocateRenderVertices(renderer, vertlen, DEVICE_ALIGN(8), &cmd->data.draw.first);
    if (!verts) {
        return -1;
    }

    cmd->data.draw.count = count;

    /* Quads in the following vertex order (matches the quad index buffer):
     * 1---3
     * | \ |
     * 0---2
     */
    for (int i = 0; i < count; i++, rects++) {
        if ((rects->w <= 0.0f) || (rects->h <= 0.0f)) {
            cmd->data.draw.count--;
        } else {
            *(verts++) = rects->x;
            *(verts++) = rects->y + rects->h;
            *(verts++) = rects->x;
            *(verts++) = rects->y;
            *(verts++) = rects->x + rects->w;
            *(verts++) = rects->y + rects->h;
            *(verts++) = rects->x + rects->w;
            *(verts++) = rects->y;
        }
    }

    if (cmd->data.draw.count == 0) {
        cmd->command = SDL_RENDERCMD_NO_OP;  // nothing to do, just skip this one later.
    }

    return 0;
}

static int
METAL_QueueCopy(SDL_Renderer * renderer, SDL_RenderCommand *cmd, SDL_Texture * texture,
                const SDL_Rect * srcrect, const SDL_FRect * dstrect)
{
    const float texw = (float) texture->w;
    const float texh = (float) texture->h;
    // !!! FIXME: use an index buffer
    const size_t vertlen = (sizeof (float) * 16);
    float *verts = (float *) SDL_AllocateRenderVertices(renderer, vertlen, DEVICE_ALIGN(8), &cmd->data.draw.first);
    if (!verts) {
        return -1;
    }

    cmd->data.draw.count = 1;

    /* Interleaved positions and texture coordinates */
    *(verts++) = dstrect->x;
    *(verts++) = dstrect->y + dstrect->h;
    *(verts++) = normtex(srcrect->x, texw);
    *(verts++) = normtex(srcrect->y + srcrect->h, texh);

    *(verts++) = dstrect->x;
    *(verts++) = dstrect->y;
    *(verts++) = normtex(srcrect->x, texw);
    *(verts++) = normtex(srcrect->y, texh);

    *(verts++) = dstrect->x + dstrect->w;
    *(verts++) = dstrect->y + dstrect->h;
    *(verts++) = normtex(srcrect->x + srcrect->w, texw);
    *(verts++) = normtex(srcrect->y + srcrect->h, texh);

    *(verts++) = dstrect->x + dstrect->w;
    *(verts++) = dstrect->y;
    *(verts++) = normtex(srcrect->x + srcrect->w, texw);
    *(verts++) = normtex(srcrect->y, texh);

    return 0;
}

static int
METAL_QueueCopyEx(SDL_Renderer * renderer, SDL_RenderCommand *cmd, SDL_Texture * texture,
                  const SDL_Rect * srcquad, const SDL_FRect * dstrect,
                  const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip)
{
    const float texw = (float) texture->w;
    const float texh = (float) texture->h;
    const float rads = (float)(M_PI * (float) angle / 180.0f);
    const float c = cosf(rads), s = sinf(rads);
    float minu, maxu, minv, maxv;
    const size_t vertlen = (sizeof (float) * 32);
    float *verts;

    // cheat and store this offset in (count) because it needs to be aligned in ways other fields don't and we aren't using count otherwise.
    verts = (float *) SDL_AllocateRenderVertices(renderer, vertlen, CONSTANT_ALIGN(16), &cmd->data.draw.count);
    if (!verts) {
        return -1;
    }

    // transform matrix
    SDL_memset(verts, '\0', sizeof (*verts) * 16);
    verts[10] = verts[15] = 1.0f;
    // rotation
    verts[0] = c;
    verts[1] = s;
    verts[4] = -s;
    verts[5] = c;

    // translation
    verts[12] = dstrect->x + center->x;
    verts[13] = dstrect->y + center->y;

    // rest of the vertices don't need the aggressive alignment. Pack them in.
    verts = (float *) SDL_AllocateRenderVertices(renderer, vertlen, DEVICE_ALIGN(8), &cmd->data.draw.first);
    if (!verts) {
        return -1;
    }

    minu = normtex(srcquad->x, texw);
    maxu = normtex(srcquad->x + srcquad->w, texw);
    minv = normtex(srcquad->y, texh);
    maxv = normtex(srcquad->y + srcquad->h, texh);

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

    /* Interleaved positions and texture coordinates */
    *(verts++) = -center->x;
    *(verts++) = dstrect->h - center->y;
    *(verts++) = minu;
    *(verts++) = maxv;

    *(verts++) = -center->x;
    *(verts++) = -center->y;
    *(verts++) = minu;
    *(verts++) = minv;

    *(verts++) = dstrect->w - center->x;
    *(verts++) = dstrect->h - center->y;
    *(verts++) = maxu;
    *(verts++) = maxv;

    *(verts++) = dstrect->w - center->x;
    *(verts++) = -center->y;
    *(verts++) = maxu;
    *(verts++) = minv;

    return 0;
}


typedef struct
{
    #if __has_feature(objc_arc)
    __unsafe_unretained id<MTLRenderPipelineState> pipeline;
    __unsafe_unretained id<MTLBuffer> vertex_buffer;
    #else
    id<MTLRenderPipelineState> pipeline;
    id<MTLBuffer> vertex_buffer;
    #endif
    size_t constants_offset;
    SDL_Texture *texture;
    SDL_bool cliprect_dirty;
    SDL_bool cliprect_enabled;
    SDL_Rect cliprect;
    SDL_bool viewport_dirty;
    SDL_Rect viewport;
    size_t projection_offset;
    SDL_bool color_dirty;
    size_t color_offset;
} METAL_DrawStateCache;

static void
SetDrawState(SDL_Renderer *renderer, const SDL_RenderCommand *cmd, const SDL_MetalFragmentFunction shader,
             const size_t constants_offset, id<MTLBuffer> mtlbufvertex, METAL_DrawStateCache *statecache)
{
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    const SDL_BlendMode blend = cmd->data.draw.blend;
    size_t first = cmd->data.draw.first;
    id<MTLRenderPipelineState> newpipeline;

    METAL_ActivateRenderCommandEncoder(renderer, MTLLoadActionLoad, NULL, statecache->vertex_buffer);

    if (statecache->viewport_dirty) {
        MTLViewport viewport;
        viewport.originX = statecache->viewport.x;
        viewport.originY = statecache->viewport.y;
        viewport.width = statecache->viewport.w;
        viewport.height = statecache->viewport.h;
        viewport.znear = 0.0;
        viewport.zfar = 1.0;
        [data.mtlcmdencoder setViewport:viewport];
        [data.mtlcmdencoder setVertexBuffer:mtlbufvertex offset:statecache->projection_offset atIndex:2];  // projection
        statecache->viewport_dirty = SDL_FALSE;
    }

    if (statecache->cliprect_dirty) {
        MTLScissorRect mtlrect;
        if (statecache->cliprect_enabled) {
            const SDL_Rect *rect = &statecache->cliprect;
            mtlrect.x = statecache->viewport.x + rect->x;
            mtlrect.y = statecache->viewport.y + rect->y;
            mtlrect.width = rect->w;
            mtlrect.height = rect->h;
        } else {
            mtlrect.x = statecache->viewport.x;
            mtlrect.y = statecache->viewport.y;
            mtlrect.width = statecache->viewport.w;
            mtlrect.height = statecache->viewport.h;
        }
        if (mtlrect.width > 0 && mtlrect.height > 0) {
            [data.mtlcmdencoder setScissorRect:mtlrect];
        }
        statecache->cliprect_dirty = SDL_FALSE;
    }

    if (statecache->color_dirty) {
        [data.mtlcmdencoder setFragmentBufferOffset:statecache->color_offset atIndex:0];
        statecache->color_dirty = SDL_FALSE;
    }

    newpipeline = ChoosePipelineState(data, data.activepipelines, shader, blend);
    if (newpipeline != statecache->pipeline) {
        [data.mtlcmdencoder setRenderPipelineState:newpipeline];
        statecache->pipeline = newpipeline;
    }

    if (constants_offset != statecache->constants_offset) {
        if (constants_offset != CONSTANTS_OFFSET_INVALID) {
            [data.mtlcmdencoder setVertexBuffer:data.mtlbufconstants offset:constants_offset atIndex:3];
        }
        statecache->constants_offset = constants_offset;
    }

    [data.mtlcmdencoder setVertexBufferOffset:first atIndex:0]; /* position/texcoords */
}

static void
SetCopyState(SDL_Renderer *renderer, const SDL_RenderCommand *cmd, const size_t constants_offset,
             id<MTLBuffer> mtlbufvertex, METAL_DrawStateCache *statecache)
{
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    SDL_Texture *texture = cmd->data.draw.texture;
    METAL_TextureData *texturedata = (__bridge METAL_TextureData *)texture->driverdata;

    SetDrawState(renderer, cmd, texturedata.fragmentFunction, constants_offset, mtlbufvertex, statecache);

    if (texture != statecache->texture) {
        METAL_TextureData *oldtexturedata = NULL;
        if (statecache->texture) {
            oldtexturedata = (__bridge METAL_TextureData *) statecache->texture->driverdata;
        }
        if (!oldtexturedata || (texturedata.mtlsampler != oldtexturedata.mtlsampler)) {
            [data.mtlcmdencoder setFragmentSamplerState:texturedata.mtlsampler atIndex:0];
        }

        [data.mtlcmdencoder setFragmentTexture:texturedata.mtltexture atIndex:0];
        if (texturedata.yuv || texturedata.nv12) {
            [data.mtlcmdencoder setFragmentTexture:texturedata.mtltexture_uv atIndex:1];
            [data.mtlcmdencoder setFragmentBuffer:data.mtlbufconstants offset:texturedata.conversionBufferOffset atIndex:1];
        }
        statecache->texture = texture;
    }
}

static int
METAL_RunCommandQueue(SDL_Renderer * renderer, SDL_RenderCommand *cmd, void *vertices, size_t vertsize)
{ @autoreleasepool {
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    METAL_DrawStateCache statecache;
    SDL_zero(statecache);

    id<MTLBuffer> mtlbufvertex = nil;

    statecache.pipeline = nil;
    statecache.vertex_buffer = nil;
    statecache.constants_offset = CONSTANTS_OFFSET_INVALID;
    statecache.texture = NULL;
    statecache.color_dirty = SDL_TRUE;
    statecache.cliprect_dirty = SDL_TRUE;
    statecache.viewport_dirty = SDL_TRUE;
    statecache.projection_offset = 0;
    statecache.color_offset = 0;

    // !!! FIXME: have a ring of pre-made MTLBuffers we cycle through? How expensive is creation?
    if (vertsize > 0) {
        /* We can memcpy to a shared buffer from the CPU and read it from the GPU
         * without any extra copying. It's a bit slower on macOS to read shared
         * data from the GPU than to read managed/private data, but we avoid the
         * cost of copying the data and the code's simpler. Apple's best
         * practices guide recommends this approach for streamed vertex data.
         * TODO: this buffer is also used for constants. Is performance still
         * good for those, or should we have a managed buffer for them? */
        mtlbufvertex = [data.mtldevice newBufferWithLength:vertsize options:MTLResourceStorageModeShared];
        #if !__has_feature(objc_arc)
        [mtlbufvertex autorelease];
        #endif
        mtlbufvertex.label = @"SDL vertex data";
        SDL_memcpy([mtlbufvertex contents], vertices, vertsize);

        statecache.vertex_buffer = mtlbufvertex;
    }

    // If there's a command buffer here unexpectedly (app requested one?). Commit it so we can start fresh.
    [data.mtlcmdencoder endEncoding];
    [data.mtlcmdbuffer commit];
    data.mtlcmdencoder = nil;
    data.mtlcmdbuffer = nil;

    while (cmd) {
        switch (cmd->command) {
            case SDL_RENDERCMD_SETVIEWPORT: {
                SDL_memcpy(&statecache.viewport, &cmd->data.viewport.rect, sizeof (statecache.viewport));
                statecache.projection_offset = cmd->data.viewport.first;
                statecache.viewport_dirty = SDL_TRUE;
                statecache.cliprect_dirty = SDL_TRUE;
                break;
            }

            case SDL_RENDERCMD_SETCLIPRECT: {
                SDL_memcpy(&statecache.cliprect, &cmd->data.cliprect.rect, sizeof (statecache.cliprect));
                statecache.cliprect_enabled = cmd->data.cliprect.enabled;
                statecache.cliprect_dirty = SDL_TRUE;
                break;
            }

            case SDL_RENDERCMD_SETDRAWCOLOR: {
                statecache.color_offset = cmd->data.color.first;
                statecache.color_dirty = SDL_TRUE;
                break;
            }

            case SDL_RENDERCMD_CLEAR: {
                /* If we're already encoding a command buffer, dump it without committing it. We'd just
                    clear all its work anyhow, and starting a new encoder will let us use a hardware clear
                    operation via MTLLoadActionClear. */
                if (data.mtlcmdencoder != nil) {
                    [data.mtlcmdencoder endEncoding];

                    // !!! FIXME: have to commit, or an uncommitted but enqueued buffer will prevent the frame from finishing.
                    [data.mtlcmdbuffer commit];
                    data.mtlcmdencoder = nil;
                    data.mtlcmdbuffer = nil;
                }

                // force all this state to be reconfigured on next command buffer.
                statecache.pipeline = nil;
                statecache.constants_offset = CONSTANTS_OFFSET_INVALID;
                statecache.texture = NULL;
                statecache.color_dirty = SDL_TRUE;
                statecache.cliprect_dirty = SDL_TRUE;
                statecache.viewport_dirty = SDL_TRUE;

                const Uint8 r = cmd->data.color.r;
                const Uint8 g = cmd->data.color.g;
                const Uint8 b = cmd->data.color.b;
                const Uint8 a = cmd->data.color.a;
                MTLClearColor color = MTLClearColorMake(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);

                // get new command encoder, set up with an initial clear operation.
                METAL_ActivateRenderCommandEncoder(renderer, MTLLoadActionClear, &color, mtlbufvertex);
                break;
            }

            case SDL_RENDERCMD_DRAW_POINTS:
            case SDL_RENDERCMD_DRAW_LINES: {
                const size_t count = cmd->data.draw.count;
                const MTLPrimitiveType primtype = (cmd->command == SDL_RENDERCMD_DRAW_POINTS) ? MTLPrimitiveTypePoint : MTLPrimitiveTypeLineStrip;
                SetDrawState(renderer, cmd, SDL_METAL_FRAGMENT_SOLID, CONSTANTS_OFFSET_HALF_PIXEL_TRANSFORM, mtlbufvertex, &statecache);
                [data.mtlcmdencoder drawPrimitives:primtype vertexStart:0 vertexCount:count];
                break;
            }

            case SDL_RENDERCMD_FILL_RECTS: {
                const size_t count = cmd->data.draw.count;
                const size_t maxcount = UINT16_MAX / 4;
                SetDrawState(renderer, cmd, SDL_METAL_FRAGMENT_SOLID, CONSTANTS_OFFSET_IDENTITY, mtlbufvertex, &statecache);
                if (count == 1) {
                    [data.mtlcmdencoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
                } else {
                    /* Our index buffer has 16 bit indices, so we can only draw
                     * 65k vertices (16k rects) at a time. */
                    for (size_t i = 0; i < count; i += maxcount) {
                        /* Set the vertex buffer offset for our current positions.
                         * The vertex buffer itself was bound in SetDrawState. */
                        [data.mtlcmdencoder setVertexBufferOffset:cmd->data.draw.first + i*sizeof(float)*8 atIndex:0];
                        [data.mtlcmdencoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                                       indexCount:SDL_min(maxcount, count - i) * 6
                                                        indexType:MTLIndexTypeUInt16
                                                      indexBuffer:data.mtlbufquadindices
                                                indexBufferOffset:0];
                    }
                }
                break;
            }

            case SDL_RENDERCMD_COPY: {
                SetCopyState(renderer, cmd, CONSTANTS_OFFSET_IDENTITY, mtlbufvertex, &statecache);
                [data.mtlcmdencoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
                break;
            }

            case SDL_RENDERCMD_COPY_EX: {
                SetCopyState(renderer, cmd, CONSTANTS_OFFSET_INVALID, mtlbufvertex, &statecache);
                [data.mtlcmdencoder setVertexBuffer:mtlbufvertex offset:cmd->data.draw.count atIndex:3];  // transform
                [data.mtlcmdencoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
                break;
            }

            case SDL_RENDERCMD_NO_OP:
                break;
        }
        cmd = cmd->next;
    }

    return 0;
}}

static int
METAL_RenderReadPixels(SDL_Renderer * renderer, const SDL_Rect * rect,
                    Uint32 pixel_format, void * pixels, int pitch)
{ @autoreleasepool {
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    METAL_ActivateRenderCommandEncoder(renderer, MTLLoadActionLoad, NULL, nil);

    [data.mtlcmdencoder endEncoding];
    id<MTLTexture> mtltexture = data.mtlpassdesc.colorAttachments[0].texture;

#ifdef __MACOSX__
    /* on macOS with managed-storage textures, we need to tell the driver to
     * update the CPU-side copy of the texture data.
     * NOTE: Currently all of our textures are managed on macOS. We'll need some
     * extra copying for any private textures. */
    if (METAL_GetStorageMode(mtltexture) == MTLStorageModeManaged) {
        id<MTLBlitCommandEncoder> blit = [data.mtlcmdbuffer blitCommandEncoder];
        [blit synchronizeResource:mtltexture];
        [blit endEncoding];
    }
#endif

    /* Commit the current command buffer and wait until it's completed, to make
     * sure the GPU has finished rendering to it by the time we read it. */
    [data.mtlcmdbuffer commit];
    [data.mtlcmdbuffer waitUntilCompleted];
    data.mtlcmdencoder = nil;
    data.mtlcmdbuffer = nil;

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

    // If we don't have a command buffer, we can't present, so activate to get one.
    if (data.mtlcmdencoder == nil) {
        // We haven't even gotten a backbuffer yet? Clear it to black. Otherwise, load the existing data.
        if (data.mtlbackbuffer == nil) {
            MTLClearColor color = MTLClearColorMake(0.0f, 0.0f, 0.0f, 1.0f);
            METAL_ActivateRenderCommandEncoder(renderer, MTLLoadActionClear, &color, nil);
        } else {
            METAL_ActivateRenderCommandEncoder(renderer, MTLLoadActionLoad, NULL, nil);
        }
    }

    [data.mtlcmdencoder endEncoding];
    [data.mtlcmdbuffer presentDrawable:data.mtlbackbuffer];
    [data.mtlcmdbuffer commit];

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

        SDL_Metal_DestroyView(data.mtlview);
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
    METAL_ActivateRenderCommandEncoder(renderer, MTLLoadActionLoad, NULL, nil);
    METAL_RenderData *data = (__bridge METAL_RenderData *) renderer->driverdata;
    return (__bridge void*)data.mtlcmdencoder;
}}

static SDL_Renderer *
METAL_CreateRenderer(SDL_Window * window, Uint32 flags)
{ @autoreleasepool {
    SDL_Renderer *renderer = NULL;
    METAL_RenderData *data = NULL;
    id<MTLDevice> mtldevice = nil;
    SDL_MetalView view = NULL;
    CAMetalLayer *layer = nil;
    SDL_SysWMinfo syswm;
    Uint32 window_flags;
    SDL_bool changed_window = SDL_FALSE;

    SDL_VERSION(&syswm.version);
    if (!SDL_GetWindowWMInfo(window, &syswm)) {
        return NULL;
    }

    if (IsMetalAvailable(&syswm) == -1) {
        return NULL;
    }

    window_flags = SDL_GetWindowFlags(window);
    if (!(window_flags & SDL_WINDOW_METAL)) {
        changed_window = SDL_TRUE;
        if (SDL_RecreateWindow(window, (window_flags & ~(SDL_WINDOW_VULKAN | SDL_WINDOW_OPENGL)) | SDL_WINDOW_METAL) < 0) {
            return NULL;
        }
    }

    renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(*renderer));
    if (!renderer) {
        SDL_OutOfMemory();
        if (changed_window) {
            SDL_RecreateWindow(window, window_flags);
        }
        return NULL;
    }

    // !!! FIXME: MTLCopyAllDevices() can find other GPUs on macOS...
    mtldevice = MTLCreateSystemDefaultDevice();

    if (mtldevice == nil) {
        SDL_free(renderer);
        SDL_SetError("Failed to obtain Metal device");
        if (changed_window) {
            SDL_RecreateWindow(window, window_flags);
        }
        return NULL;
    }

    view = SDL_Metal_CreateView(window);

    if (view == NULL) {
#if !__has_feature(objc_arc)
        [mtldevice release];
#endif
        SDL_free(renderer);
        if (changed_window) {
            SDL_RecreateWindow(window, window_flags);
        }
        return NULL;
    }

    // !!! FIXME: error checking on all of this.
    data = [[METAL_RenderData alloc] init];

    if (data == nil) {
#if !__has_feature(objc_arc)
        [mtldevice release];
#endif
        SDL_Metal_DestroyView(view);
        SDL_free(renderer);
        if (changed_window) {
            SDL_RecreateWindow(window, window_flags);
        }
        return NULL;
    }

    renderer->driverdata = (void*)CFBridgingRetain(data);
    renderer->window = window;

    data.mtlview = view;

#ifdef __MACOSX__
    layer = (CAMetalLayer *)[(NSView *)view layer];
#else
    layer = (CAMetalLayer *)[(__bridge UIView *)view layer];
#endif

    layer.device = mtldevice;

    /* Necessary for RenderReadPixels. */
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

    id<MTLBuffer> mtlbufconstantstaging = [data.mtldevice newBufferWithLength:CONSTANTS_LENGTH options:MTLResourceStorageModeShared];
    #if !__has_feature(objc_arc)
    [mtlbufconstantstaging autorelease];
    #endif

    char *constantdata = [mtlbufconstantstaging contents];
    SDL_memcpy(constantdata + CONSTANTS_OFFSET_IDENTITY, identitytransform, sizeof(identitytransform));
    SDL_memcpy(constantdata + CONSTANTS_OFFSET_HALF_PIXEL_TRANSFORM, halfpixeltransform, sizeof(halfpixeltransform));
    SDL_memcpy(constantdata + CONSTANTS_OFFSET_DECODE_JPEG, decodetransformJPEG, sizeof(decodetransformJPEG));
    SDL_memcpy(constantdata + CONSTANTS_OFFSET_DECODE_BT601, decodetransformBT601, sizeof(decodetransformBT601));
    SDL_memcpy(constantdata + CONSTANTS_OFFSET_DECODE_BT709, decodetransformBT709, sizeof(decodetransformBT709));

    int quadcount = UINT16_MAX / 4;
    size_t indicessize = sizeof(UInt16) * quadcount * 6;
    id<MTLBuffer> mtlbufquadindicesstaging = [data.mtldevice newBufferWithLength:indicessize options:MTLResourceStorageModeShared];
#if !__has_feature(objc_arc)
    [mtlbufquadindicesstaging autorelease];
#endif

    /* Quads in the following vertex order (matches the FillRects vertices):
     * 1---3
     * | \ |
     * 0---2
     */
    UInt16 *indexdata = [mtlbufquadindicesstaging contents];
    for (int i = 0; i < quadcount; i++) {
        indexdata[i * 6 + 0] = i * 4 + 0;
        indexdata[i * 6 + 1] = i * 4 + 1;
        indexdata[i * 6 + 2] = i * 4 + 2;

        indexdata[i * 6 + 3] = i * 4 + 2;
        indexdata[i * 6 + 4] = i * 4 + 1;
        indexdata[i * 6 + 5] = i * 4 + 3;
    }

    id<MTLBuffer> mtlbufconstants = [data.mtldevice newBufferWithLength:CONSTANTS_LENGTH options:MTLResourceStorageModePrivate];
    data.mtlbufconstants = mtlbufconstants;
    data.mtlbufconstants.label = @"SDL constant data";

    id<MTLBuffer> mtlbufquadindices = [data.mtldevice newBufferWithLength:indicessize options:MTLResourceStorageModePrivate];
    data.mtlbufquadindices = mtlbufquadindices;
    data.mtlbufquadindices.label = @"SDL quad index buffer";

    id<MTLCommandBuffer> cmdbuffer = [data.mtlcmdqueue commandBuffer];
    id<MTLBlitCommandEncoder> blitcmd = [cmdbuffer blitCommandEncoder];

    [blitcmd copyFromBuffer:mtlbufconstantstaging sourceOffset:0 toBuffer:mtlbufconstants destinationOffset:0 size:CONSTANTS_LENGTH];
    [blitcmd copyFromBuffer:mtlbufquadindicesstaging sourceOffset:0 toBuffer:mtlbufquadindices destinationOffset:0 size:indicessize];

    [blitcmd endEncoding];
    [cmdbuffer commit];

    // !!! FIXME: force more clears here so all the drawables are sane to start, and our static buffers are definitely flushed.

    renderer->WindowEvent = METAL_WindowEvent;
    renderer->GetOutputSize = METAL_GetOutputSize;
    renderer->SupportsBlendMode = METAL_SupportsBlendMode;
    renderer->CreateTexture = METAL_CreateTexture;
    renderer->UpdateTexture = METAL_UpdateTexture;
#if SDL_HAVE_YUV
    renderer->UpdateTextureYUV = METAL_UpdateTextureYUV;
    renderer->UpdateTextureNV = METAL_UpdateTextureNV;
#endif
    renderer->LockTexture = METAL_LockTexture;
    renderer->UnlockTexture = METAL_UnlockTexture;
    renderer->SetTextureScaleMode = METAL_SetTextureScaleMode;
    renderer->SetRenderTarget = METAL_SetRenderTarget;
    renderer->QueueSetViewport = METAL_QueueSetViewport;
    renderer->QueueSetDrawColor = METAL_QueueSetDrawColor;
    renderer->QueueDrawPoints = METAL_QueueDrawPoints;
    renderer->QueueDrawLines = METAL_QueueDrawLines;
    renderer->QueueFillRects = METAL_QueueFillRects;
    renderer->QueueCopy = METAL_QueueCopy;
    renderer->QueueCopyEx = METAL_QueueCopyEx;
    renderer->RunCommandQueue = METAL_RunCommandQueue;
    renderer->RenderReadPixels = METAL_RenderReadPixels;
    renderer->RenderPresent = METAL_RenderPresent;
    renderer->DestroyTexture = METAL_DestroyTexture;
    renderer->DestroyRenderer = METAL_DestroyRenderer;
    renderer->GetMetalLayer = METAL_GetMetalLayer;
    renderer->GetMetalCommandEncoder = METAL_GetMetalCommandEncoder;

    renderer->info = METAL_RenderDriver.info;
    renderer->info.flags = (SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

    renderer->always_batch = SDL_TRUE;

#if (defined(__MACOSX__) && defined(MAC_OS_X_VERSION_10_13)) || TARGET_OS_MACCATALYST
    if (@available(macOS 10.13, *)) {
        data.mtllayer.displaySyncEnabled = (flags & SDL_RENDERER_PRESENTVSYNC) != 0;
        if (data.mtllayer.displaySyncEnabled) {
            renderer->info.flags |= SDL_RENDERER_PRESENTVSYNC;
        }
    } else
#endif
    {
        renderer->info.flags |= SDL_RENDERER_PRESENTVSYNC;
    }

    /* https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf */
    int maxtexsize = 4096;
#if defined(__MACOSX__) || TARGET_OS_MACCATALYST
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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability-new"
    if ([mtldevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily4_v1]) {
        maxtexsize = 16384;
    } else
#pragma clang diagnostic pop
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
    [mtlbufquadindices release];
    [data release];
    [mtldevice release];
#endif

    return renderer;
}}

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

#endif /* SDL_VIDEO_RENDER_METAL && !SDL_RENDER_DISABLED */

/* vi: set ts=4 sw=4 expandtab: */
