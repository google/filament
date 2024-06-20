/*
 * Copyright (C) 2015 The Android Open Source Project
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
 */

// No include guard!!!

/*
 * How to use this header file.
 *
 * The DECL_DRIVER_API() macro must be defined before including this file:
 *     DECL_DRIVER_API(method_name, params_decl, params)
 *
 *     method_name          the name of the method (e.g. beginRenderPass)
 *     params_decl          the list of typed params
 *     params               the parameters, without their type (e.g.: for a function call)
 *
 * The following example adds all the driver methods as virtual
 * members of class:
 *
 * class MyDriver : public Driver {
 * public:
 * #define DRIVER_API(method_name, params_decl, params) \
 *     virtual void method_name(params_decl) override
 * #include "DriverAPI.h"
 * };
 *
 * DECL_DRIVER_API is automatically undefined.
 */

#ifndef DECL_DRIVER_API
#error "The DECL_DRIVER_API() macro must be defined before including this file"
#define DECL_DRIVER_API(M, D, P)
#endif

#ifndef DECL_DRIVER_API_SYNCHRONOUS
#error "The DECL_DRIVER_API_SYNCHRONOUS() macro must be defined before including this file"
#define DECL_DRIVER_API_SYNCHRONOUS(R, M, D, P)
#endif

#ifndef DECL_DRIVER_API_RETURN
#error "The DECL_DRIVER_API_RETURN() macro must be defined before including this file"
#define DECL_DRIVER_API_RETURN(R, M, D, P)
#endif


/*
 * Convenience macros. These are PRIVATE, don't use.
 */

#define EXPAND(x) x

#define APPLY0(M,...)
#define APPLY1(M, A, ...) EXPAND(M(A))
#define APPLY2(M, A, ...) EXPAND(M(A)), EXPAND(APPLY1(M, __VA_ARGS__))
#define APPLY3(M, A, ...) EXPAND(M(A)), EXPAND(APPLY2(M, __VA_ARGS__))
#define APPLY4(M, A, ...) EXPAND(M(A)), EXPAND(APPLY3(M, __VA_ARGS__))
#define APPLY5(M, A, ...) EXPAND(M(A)), EXPAND(APPLY4(M, __VA_ARGS__))
#define APPLY6(M, A, ...) EXPAND(M(A)), EXPAND(APPLY5(M, __VA_ARGS__))
#define APPLY7(M, A, ...) EXPAND(M(A)), EXPAND(APPLY6(M, __VA_ARGS__))
#define APPLY8(M, A, ...) EXPAND(M(A)), EXPAND(APPLY7(M, __VA_ARGS__))
#define APPLY9(M, A, ...) EXPAND(M(A)), EXPAND(APPLY8(M, __VA_ARGS__))
#define APPLY10(M, A, ...) EXPAND(M(A)), EXPAND(APPLY9(M, __VA_ARGS__))
#define APPLY11(M, A, ...) EXPAND(M(A)), EXPAND(APPLY10(M, __VA_ARGS__))
#define APPLY12(M, A, ...) EXPAND(M(A)), EXPAND(APPLY11(M, __VA_ARGS__))
#define APPLY_N__(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, X, ...) APPLY##X
#define APPLY(M, ...) EXPAND(EXPAND(APPLY_N__(M, __VA_ARGS__, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))(M, __VA_ARGS__))


#define PAIR_ARGS_0(M, ...)
#define PAIR_ARGS_1(M, X, Y, ...) M(X, Y)
#define PAIR_ARGS_2(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_1(M, __VA_ARGS__))
#define PAIR_ARGS_3(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_2(M, __VA_ARGS__))
#define PAIR_ARGS_4(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_3(M, __VA_ARGS__))
#define PAIR_ARGS_5(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_4(M, __VA_ARGS__))
#define PAIR_ARGS_6(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_5(M, __VA_ARGS__))
#define PAIR_ARGS_7(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_6(M, __VA_ARGS__))
#define PAIR_ARGS_8(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_7(M, __VA_ARGS__))
#define PAIR_ARGS_9(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_8(M, __VA_ARGS__))
#define PAIR_ARGS_10(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_9(M, __VA_ARGS__))
#define PAIR_ARGS_11(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_10(M, __VA_ARGS__))
#define PAIR_ARGS_12(M, X, Y, ...) M(X, Y), EXPAND(PAIR_ARGS_11(M, __VA_ARGS__))

#define PAIR_ARGS_N__(_0, E1, _1, E2, _2, E3, _3, E4, _4, E5, _5, E6, _6, E7, _7, E8, _8, E9, _9, E10, _10, E11, _11, E12, _12, X, ...) PAIR_ARGS_##X

#define PAIR_ARGS_N(M, ...) \
    EXPAND(EXPAND(PAIR_ARGS_N__(0, ##__VA_ARGS__, 12, E, 11, E, 10, E, 9, E, 8, E, 7, E, 6, E, 5, E, 4, E, 3, E, 2, E, 1, E, 0))(M, __VA_ARGS__))

#define ARG(T, P) T P

#define PARAM(T, P) P

#define DECL_DRIVER_API_N(N, ...) \
    DECL_DRIVER_API(N, PAIR_ARGS_N(ARG, ##__VA_ARGS__), PAIR_ARGS_N(PARAM, ##__VA_ARGS__))

#define DECL_DRIVER_API_R_N(R, N, ...) \
    DECL_DRIVER_API_RETURN(R, N, PAIR_ARGS_N(ARG, ##__VA_ARGS__), PAIR_ARGS_N(PARAM, ##__VA_ARGS__))

#define DECL_DRIVER_API_SYNCHRONOUS_N(R, N, ...) \
    DECL_DRIVER_API_SYNCHRONOUS(R, N, PAIR_ARGS_N(ARG, ##__VA_ARGS__), PAIR_ARGS_N(PARAM, ##__VA_ARGS__))

// on some compilers the ##__VA_ARGS__ hack is not supported, so we can't handle 0-parameter APIs
// with DECL_DRIVER_API_SYNCHRONOUS_N
#define DECL_DRIVER_API_SYNCHRONOUS_0(R, N) \
    DECL_DRIVER_API_SYNCHRONOUS(R, N,,)

// we can't handle empty lists because we don't have __VA_OPT__() in C++14 and the ## hack
// only works for omitted arguments, not empty lists
#define PARAM_1( P, ...) P
#define ARG_1( T, P, ...) T P
#define DECL_DRIVER_API_0(N)      DECL_DRIVER_API(N, ARG_1(int, dummy=0), PARAM_1(dummy))
#define DECL_DRIVER_API_R_0(R, N) DECL_DRIVER_API_RETURN(R, N, ARG_1(int, dummy=0), PARAM_1(dummy))

/*
 * Driver API below...
 */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

DECL_DRIVER_API_0(tick)

DECL_DRIVER_API_N(beginFrame,
        int64_t, monotonic_clock_ns,
        int64_t, refreshIntervalNs,
        uint32_t, frameId)

DECL_DRIVER_API_N(setFrameScheduledCallback,
        backend::SwapChainHandle, sch,
        backend::CallbackHandler*, handler,
        backend::FrameScheduledCallback&&, callback)

DECL_DRIVER_API_N(setFrameCompletedCallback,
        backend::SwapChainHandle, sch,
        backend::CallbackHandler*, handler,
        utils::Invocable<void(void)>&&, callback)

DECL_DRIVER_API_N(setPresentationTime,
        int64_t, monotonic_clock_ns)

DECL_DRIVER_API_N(endFrame,
        uint32_t, frameId)

// hint to the driver that we're done with all render targets up to this point. i.e. the driver
// can start rendering. e.g. correspond to glFlush() for a GLES driver.
DECL_DRIVER_API_0(flush)

// flush and wait for the effects to be done
DECL_DRIVER_API_0(finish)

// reset state tracking, if the driver does any state tracking (e.g. GL)
DECL_DRIVER_API_0(resetState)

/*
 * Creating driver objects
 * -----------------------
 */

DECL_DRIVER_API_R_N(backend::VertexBufferInfoHandle, createVertexBufferInfo,
        uint8_t, bufferCount,
        uint8_t, attributeCount,
        backend::AttributeArray, attributes)

DECL_DRIVER_API_R_N(backend::VertexBufferHandle, createVertexBuffer,
        uint32_t, vertexCount,
        backend::VertexBufferInfoHandle, vbih)

DECL_DRIVER_API_R_N(backend::IndexBufferHandle, createIndexBuffer,
        backend::ElementType, elementType,
        uint32_t, indexCount,
        backend::BufferUsage, usage)

DECL_DRIVER_API_R_N(backend::BufferObjectHandle, createBufferObject,
        uint32_t, byteCount,
        backend::BufferObjectBinding, bindingType,
        backend::BufferUsage, usage)

DECL_DRIVER_API_R_N(backend::TextureHandle, createTexture,
        backend::SamplerType, target,
        uint8_t, levels,
        backend::TextureFormat, format,
        uint8_t, samples,
        uint32_t, width,
        uint32_t, height,
        uint32_t, depth,
        backend::TextureUsage, usage)

DECL_DRIVER_API_R_N(backend::TextureHandle, createTextureView,
        backend::TextureHandle, texture,
        uint8_t, baseLevel,
        uint8_t, levelCount)

DECL_DRIVER_API_R_N(backend::TextureHandle, createTextureSwizzled,
        backend::SamplerType, target,
        uint8_t, levels,
        backend::TextureFormat, format,
        uint8_t, samples,
        uint32_t, width,
        uint32_t, height,
        uint32_t, depth,
        backend::TextureUsage, usage,
        backend::TextureSwizzle, r,
        backend::TextureSwizzle, g,
        backend::TextureSwizzle, b,
        backend::TextureSwizzle, a)

DECL_DRIVER_API_R_N(backend::TextureHandle, importTexture,
        intptr_t, id,
        backend::SamplerType, target,
        uint8_t, levels,
        backend::TextureFormat, format,
        uint8_t, samples,
        uint32_t, width,
        uint32_t, height,
        uint32_t, depth,
        backend::TextureUsage, usage)

DECL_DRIVER_API_R_N(backend::SamplerGroupHandle, createSamplerGroup,
        uint32_t, size, utils::FixedSizeString<32>, debugName)

DECL_DRIVER_API_R_N(backend::RenderPrimitiveHandle, createRenderPrimitive,
        backend::VertexBufferHandle, vbh,
        backend::IndexBufferHandle, ibh,
        backend::PrimitiveType, pt)

DECL_DRIVER_API_R_N(backend::ProgramHandle, createProgram,
        backend::Program&&, program)

DECL_DRIVER_API_R_0(backend::RenderTargetHandle, createDefaultRenderTarget)

DECL_DRIVER_API_R_N(backend::RenderTargetHandle, createRenderTarget,
        backend::TargetBufferFlags, targetBufferFlags,
        uint32_t, width,
        uint32_t, height,
        uint8_t, samples,
        uint8_t, layerCount,
        backend::MRT, color,
        backend::TargetBufferInfo, depth,
        backend::TargetBufferInfo, stencil)

DECL_DRIVER_API_R_0(backend::FenceHandle, createFence)

DECL_DRIVER_API_R_N(backend::SwapChainHandle, createSwapChain,
        void*, nativeWindow,
        uint64_t, flags)

DECL_DRIVER_API_R_N(backend::SwapChainHandle, createSwapChainHeadless,
        uint32_t, width,
        uint32_t, height,
        uint64_t, flags)

DECL_DRIVER_API_R_0(backend::TimerQueryHandle, createTimerQuery)

DECL_DRIVER_API_R_N(backend::DescriptorSetLayoutHandle, createDescriptorSetLayout,
        backend::DescriptorSetLayout&&, info)

DECL_DRIVER_API_R_N(backend::DescriptorSetHandle, createDescriptorSet,
        backend::DescriptorSetLayoutHandle, dslh)

DECL_DRIVER_API_N(updateDescriptorSetBuffer,
        backend::DescriptorSetHandle, dsh,
        backend::descriptor_binding_t, binding,
        backend::BufferObjectHandle, boh,
        uint32_t, offset,
        uint32_t, size
)

DECL_DRIVER_API_N(updateDescriptorSetTexture,
        backend::DescriptorSetHandle, dsh,
        backend::descriptor_binding_t, binding,
        backend::TextureHandle, th,
        SamplerParams, params
)

DECL_DRIVER_API_N(bindDescriptorSet,
        backend::DescriptorSetHandle, dsh,
        backend::descriptor_set_t, set,
        backend::DescriptorSetOffsetArray&&, offsets
)


/*
 * Destroying driver objects
 * -------------------------
 */

DECL_DRIVER_API_N(destroyVertexBuffer,          backend::VertexBufferHandle, vbh)
DECL_DRIVER_API_N(destroyVertexBufferInfo,      backend::VertexBufferInfoHandle, vbih)
DECL_DRIVER_API_N(destroyIndexBuffer,           backend::IndexBufferHandle, ibh)
DECL_DRIVER_API_N(destroyBufferObject,          backend::BufferObjectHandle, ibh)
DECL_DRIVER_API_N(destroyRenderPrimitive,       backend::RenderPrimitiveHandle, rph)
DECL_DRIVER_API_N(destroyProgram,               backend::ProgramHandle, ph)
DECL_DRIVER_API_N(destroySamplerGroup,          backend::SamplerGroupHandle, sbh)
DECL_DRIVER_API_N(destroyTexture,               backend::TextureHandle, th)
DECL_DRIVER_API_N(destroyRenderTarget,          backend::RenderTargetHandle, rth)
DECL_DRIVER_API_N(destroySwapChain,             backend::SwapChainHandle, sch)
DECL_DRIVER_API_N(destroyStream,                backend::StreamHandle, sh)
DECL_DRIVER_API_N(destroyTimerQuery,            backend::TimerQueryHandle, sh)
DECL_DRIVER_API_N(destroyFence,                 backend::FenceHandle, fh)
DECL_DRIVER_API_N(destroyDescriptorSetLayout,   backend::DescriptorSetLayoutHandle, dslh)
DECL_DRIVER_API_N(destroyDescriptorSet,         backend::DescriptorSetHandle, dsh)

/*
 * Synchronous APIs
 * ----------------
 */

DECL_DRIVER_API_SYNCHRONOUS_0(void, terminate)
DECL_DRIVER_API_SYNCHRONOUS_N(backend::StreamHandle, createStreamNative, void*, stream)
DECL_DRIVER_API_SYNCHRONOUS_0(backend::StreamHandle, createStreamAcquired)
DECL_DRIVER_API_SYNCHRONOUS_N(void, setAcquiredImage, backend::StreamHandle, stream, void*, image, backend::CallbackHandler*, handler, backend::StreamCallback, cb, void*, userData)
DECL_DRIVER_API_SYNCHRONOUS_N(void, setStreamDimensions, backend::StreamHandle, stream, uint32_t, width, uint32_t, height)
DECL_DRIVER_API_SYNCHRONOUS_N(int64_t, getStreamTimestamp, backend::StreamHandle, stream)
DECL_DRIVER_API_SYNCHRONOUS_N(void, updateStreams, backend::DriverApi*, driver)
DECL_DRIVER_API_SYNCHRONOUS_N(backend::FenceStatus, getFenceStatus, backend::FenceHandle, fh)
DECL_DRIVER_API_SYNCHRONOUS_N(bool, isTextureFormatSupported, backend::TextureFormat, format)
DECL_DRIVER_API_SYNCHRONOUS_0(bool, isTextureSwizzleSupported)
DECL_DRIVER_API_SYNCHRONOUS_N(bool, isTextureFormatMipmappable, backend::TextureFormat, format)
DECL_DRIVER_API_SYNCHRONOUS_N(bool, isRenderTargetFormatSupported, backend::TextureFormat, format)
DECL_DRIVER_API_SYNCHRONOUS_0(bool, isFrameBufferFetchSupported)
DECL_DRIVER_API_SYNCHRONOUS_0(bool, isFrameBufferFetchMultiSampleSupported)
DECL_DRIVER_API_SYNCHRONOUS_0(bool, isFrameTimeSupported)
DECL_DRIVER_API_SYNCHRONOUS_0(bool, isAutoDepthResolveSupported)
DECL_DRIVER_API_SYNCHRONOUS_0(bool, isSRGBSwapChainSupported)
DECL_DRIVER_API_SYNCHRONOUS_0(bool, isProtectedContentSupported)
DECL_DRIVER_API_SYNCHRONOUS_0(bool, isStereoSupported)
DECL_DRIVER_API_SYNCHRONOUS_0(bool, isParallelShaderCompileSupported)
DECL_DRIVER_API_SYNCHRONOUS_0(bool, isDepthStencilResolveSupported)
DECL_DRIVER_API_SYNCHRONOUS_N(bool, isDepthStencilBlitSupported, backend::TextureFormat, format)
DECL_DRIVER_API_SYNCHRONOUS_0(bool, isProtectedTexturesSupported)
DECL_DRIVER_API_SYNCHRONOUS_0(uint8_t, getMaxDrawBuffers)
DECL_DRIVER_API_SYNCHRONOUS_0(size_t, getMaxUniformBufferSize)
DECL_DRIVER_API_SYNCHRONOUS_0(math::float2, getClipSpaceParams)
DECL_DRIVER_API_SYNCHRONOUS_N(void, setupExternalImage, void*, image)
DECL_DRIVER_API_SYNCHRONOUS_N(backend::TimerQueryResult, getTimerQueryValue, backend::TimerQueryHandle, query, uint64_t*, elapsedTime)
DECL_DRIVER_API_SYNCHRONOUS_N(bool, isWorkaroundNeeded, backend::Workaround, workaround)
DECL_DRIVER_API_SYNCHRONOUS_0(backend::FeatureLevel, getFeatureLevel)

/*
 * Updating driver objects
 * -----------------------
 */

DECL_DRIVER_API_N(setVertexBufferObject,
        backend::VertexBufferHandle, vbh,
        uint32_t, index,
        backend::BufferObjectHandle, bufferObject)

DECL_DRIVER_API_N(updateIndexBuffer,
        backend::IndexBufferHandle, ibh,
        backend::BufferDescriptor&&, data,
        uint32_t, byteOffset)

DECL_DRIVER_API_N(updateBufferObject,
        backend::BufferObjectHandle, ibh,
        backend::BufferDescriptor&&, data,
        uint32_t, byteOffset)

DECL_DRIVER_API_N(updateBufferObjectUnsynchronized,
        backend::BufferObjectHandle, ibh,
        backend::BufferDescriptor&&, data,
        uint32_t, byteOffset)

DECL_DRIVER_API_N(resetBufferObject,
        backend::BufferObjectHandle, ibh)

DECL_DRIVER_API_N(updateSamplerGroup,
        backend::SamplerGroupHandle, ubh,
        backend::BufferDescriptor&&, data)

DECL_DRIVER_API_N(setMinMaxLevels,
        backend::TextureHandle, th,
        uint32_t, minLevel,
        uint32_t, maxLevel)

DECL_DRIVER_API_N(update3DImage,
        backend::TextureHandle, th,
        uint32_t, level,
        uint32_t, xoffset,
        uint32_t, yoffset,
        uint32_t, zoffset,
        uint32_t, width,
        uint32_t, height,
        uint32_t, depth,
        backend::PixelBufferDescriptor&&, data)

DECL_DRIVER_API_N(generateMipmaps,
        backend::TextureHandle, th)

DECL_DRIVER_API_N(setExternalImage,
        backend::TextureHandle, th,
        void*, image)

DECL_DRIVER_API_N(setExternalImagePlane,
        backend::TextureHandle, th,
        void*, image,
        uint32_t, plane)

DECL_DRIVER_API_N(setExternalStream,
        backend::TextureHandle, th,
        backend::StreamHandle, sh)

DECL_DRIVER_API_N(beginRenderPass,
        backend::RenderTargetHandle, rth,
        const backend::RenderPassParams&, params)

DECL_DRIVER_API_0(endRenderPass)

DECL_DRIVER_API_0(nextSubpass)

DECL_DRIVER_API_N(beginTimerQuery,
        backend::TimerQueryHandle, query)

DECL_DRIVER_API_N(endTimerQuery,
        backend::TimerQueryHandle, query)

DECL_DRIVER_API_N(compilePrograms,
        backend::CompilerPriorityQueue, priority,
        backend::CallbackHandler*, handler,
        backend::CallbackHandler::Callback, callback,
        void*, user)

/*
 * Swap chain
 */

DECL_DRIVER_API_N(makeCurrent,
        backend::SwapChainHandle, schDraw,
        backend::SwapChainHandle, schRead)


DECL_DRIVER_API_N(commit,
        backend::SwapChainHandle, sch)

/*
 * Setting rendering state
 * -----------------------
 */

DECL_DRIVER_API_N(bindUniformBuffer,
        uint32_t, index,
        backend::BufferObjectHandle, ubh)

DECL_DRIVER_API_N(bindBufferRange,
        BufferObjectBinding, bindingType,
        uint32_t, index,
        backend::BufferObjectHandle, ubh,
        uint32_t, offset,
        uint32_t, size)

DECL_DRIVER_API_N(bindSamplers,
        uint32_t, index,
        backend::SamplerGroupHandle, sbh)

DECL_DRIVER_API_N(setPushConstant,
        backend::ShaderStage, stage,
        uint8_t, index,
        backend::PushConstantVariant, value)

DECL_DRIVER_API_N(insertEventMarker,
        const char*, string,
        uint32_t, len = 0)

DECL_DRIVER_API_N(pushGroupMarker,
        const char*, string,
        uint32_t, len = 0)

DECL_DRIVER_API_0(popGroupMarker)

DECL_DRIVER_API_0(startCapture)

DECL_DRIVER_API_0(stopCapture)


/*
 * Read-back operations
 * --------------------
 */

DECL_DRIVER_API_N(readPixels,
        backend::RenderTargetHandle, src,
        uint32_t, x,
        uint32_t, y,
        uint32_t, width,
        uint32_t, height,
        backend::PixelBufferDescriptor&&, data)

DECL_DRIVER_API_N(readBufferSubData,
        backend::BufferObjectHandle, src,
        uint32_t, offset,
        uint32_t, size,
        backend::BufferDescriptor&&, data)

/*
 * Rendering operations
 * --------------------
 */

DECL_DRIVER_API_N(blitDEPRECATED,
        backend::TargetBufferFlags, buffers,
        backend::RenderTargetHandle, dst,
        backend::Viewport, dstRect,
        backend::RenderTargetHandle, src,
        backend::Viewport, srcRect,
        backend::SamplerMagFilter, filter)

DECL_DRIVER_API_N(resolve,
        backend::TextureHandle, dst,
        uint8_t, dstLevel, uint8_t, dstLayer,
        backend::TextureHandle, src,
        uint8_t, srcLevel, uint8_t, srcLayer)

DECL_DRIVER_API_N(blit,
        backend::TextureHandle, dst,
        uint8_t, dstLevel, uint8_t, dstLayer,
        math::uint2, dstOrigin,
        backend::TextureHandle, src,
        uint8_t, srcLevel, uint8_t, srcLayer,
        math::uint2, srcOrigin,
        math::uint2, size)

DECL_DRIVER_API_N(bindPipeline,
        backend::PipelineState const&, state)

DECL_DRIVER_API_N(bindRenderPrimitive,
        backend::RenderPrimitiveHandle, rph)

DECL_DRIVER_API_N(draw2,
        uint32_t, indexOffset,
        uint32_t, indexCount,
        uint32_t, instanceCount)

DECL_DRIVER_API_N(draw,
        backend::PipelineState, state,
        backend::RenderPrimitiveHandle, rph,
        uint32_t, indexOffset,
        uint32_t, indexCount,
        uint32_t, instanceCount)


DECL_DRIVER_API_N(dispatchCompute,
        backend::ProgramHandle, program,
        math::uint3, workGroupCount)

DECL_DRIVER_API_N(scissor,
        Viewport, scissor)


#pragma clang diagnostic pop

#undef EXPAND

#undef PARAM_1
#undef ARG_1
#undef DECL_DRIVER_API_0
#undef DECL_DRIVER_API_R_0

#undef PARAM
#undef ARG
#undef DECL_DRIVER_API_N
#undef DECL_DRIVER_API_R_N
#undef DECL_DRIVER_API_SYNCHRONOUS_N
#undef DECL_DRIVER_API_SYNCHRONOUS_0

#undef DECL_DRIVER_API
#undef DECL_DRIVER_API_SYNCHRONOUS
#undef DECL_DRIVER_API_RETURN

#undef PAIR_ARGS_1
#undef PAIR_ARGS_2
#undef PAIR_ARGS_3
#undef PAIR_ARGS_4
#undef PAIR_ARGS_5
#undef PAIR_ARGS_6
#undef PAIR_ARGS_7
#undef PAIR_ARGS_8
#undef PAIR_ARGS_N__
#undef PAIR_ARGS_N

// No include guard!!!
