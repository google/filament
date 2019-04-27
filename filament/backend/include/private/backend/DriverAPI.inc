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

// turns all arguments of this macro into a single argument
#define SINGLE_ARG(...) __VA_ARGS__

// adds Parameter to the parameter List
#define PARAM_LIST_ADD(Parameter, List) \
        SINGLE_ARG(Parameter, List)

// adds argument Type Parameter to the argument List
#define ARG_LIST_ADD(Type, Parameter, List) \
        SINGLE_ARG(Type Parameter, List)

// parameter lists
#define PARAM_1( P0)                                          P0
#define PARAM_2( P1,  P0)                                     PARAM_LIST_ADD(P1,  PARAM_1( P0))
#define PARAM_3( P2,  P1, P0)                                 PARAM_LIST_ADD(P2,  PARAM_2( P1, P0))
#define PARAM_4( P3,  P2, P1, P0)                             PARAM_LIST_ADD(P3,  PARAM_3( P2, P1, P0))
#define PARAM_5( P4,  P3, P2, P1, P0)                         PARAM_LIST_ADD(P4,  PARAM_4( P3, P2, P1, P0))
#define PARAM_6( P5,  P4, P3, P2, P1, P0)                     PARAM_LIST_ADD(P5,  PARAM_5( P4, P3, P2, P1, P0))
#define PARAM_7( P6,  P5, P4, P3, P2, P1, P0)                 PARAM_LIST_ADD(P6,  PARAM_6( P5, P4, P3, P2, P1, P0))
#define PARAM_8( P7,  P6, P5, P4, P3, P2, P1, P0)             PARAM_LIST_ADD(P7,  PARAM_7( P6, P5, P4, P3, P2, P1, P0))
#define PARAM_9( P8,  P7, P6, P5, P4, P3, P2, P1, P0)         PARAM_LIST_ADD(P8,  PARAM_8( P7, P6, P5, P4, P3, P2, P1, P0))
#define PARAM_10(P9,  P8, P7, P6, P5, P4, P3, P2, P1, P0)     PARAM_LIST_ADD(P9,  PARAM_9( P8, P7, P6, P5, P4, P3, P2, P1, P0))
#define PARAM_11(P10, P9, P8, P7, P6, P5, P4, P3, P2, P1, P0) PARAM_LIST_ADD(P10, PARAM_10(P9, P8, P7, P6, P5, P4, P3, P2, P1, P0))

// argument lists
#define ARG_1( T0,  P0)                                                                                  T0 P0
#define ARG_2( T1,  P1,  T0, P0)                                                                         ARG_LIST_ADD(T1,  P1,  ARG_1( T0, P0))
#define ARG_3( T2,  P2,  T1, P1, T0, P0)                                                                 ARG_LIST_ADD(T2,  P2,  ARG_2( T1, P1, T0, P0))
#define ARG_4( T3,  P3,  T2, P2, T1, P1, T0, P0)                                                         ARG_LIST_ADD(T3,  P3,  ARG_3( T2, P2, T1, P1, T0, P0))
#define ARG_5( T4,  P4,  T3, P3, T2, P2, T1, P1, T0, P0)                                                 ARG_LIST_ADD(T4,  P4,  ARG_4( T3, P3, T2, P2, T1, P1, T0, P0))
#define ARG_6( T5,  P5,  T4, P4, T3, P3, T2, P2, T1, P1, T0, P0)                                         ARG_LIST_ADD(T5,  P5,  ARG_5( T4, P4, T3, P3, T2, P2, T1, P1, T0, P0))
#define ARG_7( T6,  P6,  T5, P5, T4, P4, T3, P3, T2, P2, T1, P1, T0, P0)                                 ARG_LIST_ADD(T6,  P6,  ARG_6( T5, P5, T4, P4, T3, P3, T2, P2, T1, P1, T0, P0))
#define ARG_8( T7,  P7,  T6, P6, T5, P5, T4, P4, T3, P3, T2, P2, T1, P1, T0, P0)                         ARG_LIST_ADD(T7,  P7,  ARG_7( T6, P6, T5, P5, T4, P4, T3, P3, T2, P2, T1, P1, T0, P0))
#define ARG_9( T8,  P8,  T7, P7, T6, P6, T5, P5, T4, P4, T3, P3, T2, P2, T1, P1, T0, P0)                 ARG_LIST_ADD(T8,  P8,  ARG_8( T7, P7, T6, P6, T5, P5, T4, P4, T3, P3, T2, P2, T1, P1, T0, P0))
#define ARG_10(T9,  P9,  T8, P8, T7, P7, T6, P6, T5, P5, T4, P4, T3, P3, T2, P2, T1, P1, T0, P0)         ARG_LIST_ADD(T9,  P9,  ARG_9( T8, P8, T7, P7, T6, P6, T5, P5, T4, P4, T3, P3, T2, P2, T1, P1, T0, P0))
#define ARG_11(T10, P10, T9, P9, T8, P8, T7, P7, T6, P6, T5, P5, T4, P4, T3, P3, T2, P2, T1, P1, T0, P0) ARG_LIST_ADD(T10, P10, ARG_10(T9, P9, T8, P8, T7, P7, T6, P6, T5, P5, T4, P4, T3, P3, T2, P2, T1, P1, T0, P0))


#define DECL_DRIVER_API_0(N)                                                            \
    DECL_DRIVER_API(N, ARG_1(int, dummy=0),                                             \
                       PARAM_1(dummy))

#define DECL_DRIVER_API_1(N, T0, P0)                                                    \
    DECL_DRIVER_API(N, ARG_1(T0, P0),                                                   \
                       PARAM_1(P0))

#define DECL_DRIVER_API_2(N, T0, P0, T1, P1)                                            \
    DECL_DRIVER_API(N, ARG_2(T0, P0, T1, P1),                                           \
                       PARAM_2(P0, P1))

#define DECL_DRIVER_API_3(N, T0, P0, T1, P1, T2, P2)                                    \
    DECL_DRIVER_API(N, ARG_3(T0, P0, T1, P1, T2, P2),                                   \
                       PARAM_3(P0, P1, P2))

#define DECL_DRIVER_API_4(N, T0, P0, T1, P1, T2, P2, T3, P3)                            \
    DECL_DRIVER_API(N, ARG_4(T0, P0, T1, P1, T2, P2, T3, P3),                           \
                       PARAM_4(P0, P1, P2, P3))

#define DECL_DRIVER_API_5(N, T0, P0, T1, P1, T2, P2, T3, P3, T4, P4)                    \
    DECL_DRIVER_API(N, ARG_5(T0, P0, T1, P1, T2, P2, T3, P3, T4, P4),                   \
                       PARAM_5(P0, P1, P2, P3, P4))

#define DECL_DRIVER_API_6(N, T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5)            \
    DECL_DRIVER_API(N, ARG_6(T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5),           \
                       PARAM_6(P0, P1, P2, P3, P4, P5))

#define DECL_DRIVER_API_7(N, T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5, T6, P6)    \
    DECL_DRIVER_API(N, ARG_7(T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5, T6, P6),   \
                       PARAM_7(P0, P1, P2, P3, P4, P5, P6))

#define DECL_DRIVER_API_8(N, T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5, T6, P6, T7, P7)  \
    DECL_DRIVER_API(N, ARG_8(T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5, T6, P6, T7, P7), \
                       PARAM_8(P0, P1, P2, P3, P4, P5, P6, P7))

#define DECL_DRIVER_API_9(N, T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5, T6, P6, T7, P7, T8, P8)  \
    DECL_DRIVER_API(N, ARG_9(T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5, T6, P6, T7, P7, T8, P8), \
                       PARAM_9(P0, P1, P2, P3, P4, P5, P6, P7, P8))

#define DECL_DRIVER_API_10(N, T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5, T6, P6, T7, P7, T8, P8, T9, P9)  \
    DECL_DRIVER_API(N, ARG_10(T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5, T6, P6, T7, P7, T8, P8, T9, P9), \
                       PARAM_10(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9))

#define DECL_DRIVER_API_11(N, T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5, T6, P6, T7, P7, T8, P8, T9, P9, T10, P10)  \
    DECL_DRIVER_API(N, ARG_11(T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5, T6, P6, T7, P7, T8, P8, T9, P9, T10, P10), \
                       PARAM_11(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10))


#define DECL_DRIVER_API_R_0(R, N)                                                         \
    DECL_DRIVER_API_RETURN(R, N, ARG_1(int, dummy=0),                                     \
                       PARAM_1(dummy))

#define DECL_DRIVER_API_R_1(R, N, T0, P0)                                                 \
    DECL_DRIVER_API_RETURN(R, N, ARG_1(T0, P0),                                           \
                       PARAM_1(P0))

#define DECL_DRIVER_API_R_2(R, N, T0, P0, T1, P1)                                         \
    DECL_DRIVER_API_RETURN(R, N, ARG_2(T0, P0, T1, P1),                                   \
                       PARAM_2(P0, P1))

#define DECL_DRIVER_API_R_3(R, N, T0, P0, T1, P1, T2, P2)                                 \
    DECL_DRIVER_API_RETURN(R, N, ARG_3(T0, P0, T1, P1, T2, P2),                           \
                       PARAM_3(P0, P1, P2))

#define DECL_DRIVER_API_R_4(R, N, T0, P0, T1, P1, T2, P2, T3, P3)                         \
    DECL_DRIVER_API_RETURN(R, N, ARG_4(T0, P0, T1, P1, T2, P2, T3, P3),                   \
                       PARAM_4(P0, P1, P2, P3))

#define DECL_DRIVER_API_R_5(R, N, T0, P0, T1, P1, T2, P2, T3, P3, T4, P4)                 \
    DECL_DRIVER_API_RETURN(R, N, ARG_5(T0, P0, T1, P1, T2, P2, T3, P3, T4, P4),           \
                       PARAM_5(P0, P1, P2, P3, P4))

#define DECL_DRIVER_API_R_6(R, N, T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5)         \
    DECL_DRIVER_API_RETURN(R, N, ARG_6(T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5),   \
                       PARAM_6(P0, P1, P2, P3, P4, P5))

#define DECL_DRIVER_API_R_7(R, N, T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5, T6, P6)       \
    DECL_DRIVER_API_RETURN(R, N, ARG_7(T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5, T6, P6), \
                       PARAM_7(P0, P1, P2, P3, P4, P5, P6))

#define DECL_DRIVER_API_R_8(R, N, T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5, T6, P6, T7, P7)       \
    DECL_DRIVER_API_RETURN(R, N, ARG_8(T0, P0, T1, P1, T2, P2, T3, P3, T4, P4, T5, P5, T6, P6, T7, P7), \
                       PARAM_8(P0, P1, P2, P3, P4, P5, P6, P7))

#define DECL_DRIVER_API_SYNCHRONOUS_0(R, N)                                                    \
    DECL_DRIVER_API_SYNCHRONOUS(R, N,,)

#define DECL_DRIVER_API_SYNCHRONOUS_1(R, N, T0, P0)                                            \
    DECL_DRIVER_API_SYNCHRONOUS(R, N, ARG_1(T0, P0),                                           \
                       PARAM_1(P0))

#define DECL_DRIVER_API_SYNCHRONOUS_2(R, N, T0, P0, T1, P1)                                    \
    DECL_DRIVER_API_SYNCHRONOUS(R, N, ARG_2(T0, P0, T1, P1),                                   \
                       PARAM_2(P0, P1))

#define DECL_DRIVER_API_SYNCHRONOUS_3(R, N, T0, P0, T1, P1, T2, P2)                            \
    DECL_DRIVER_API_SYNCHRONOUS(R, N, ARG_3(T0, P0, T1, P1, T2, P2),                           \
                       PARAM_3(P0, P1, P2))

/*
 * Driver API below...
 */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

DECL_DRIVER_API_2(beginFrame,
        int64_t, monotonic_clock_ns,
        uint32_t, frameId)

DECL_DRIVER_API_1(setPresentationTime,
        int64_t, monotonic_clock_ns)

DECL_DRIVER_API_1(endFrame,
        uint32_t, frameId)

// hint to the driver that we're done with all render targets up to this point. i.e. the driver
// can start rendering. e.g. correspond to glFlush() for a GLES driver.
DECL_DRIVER_API_0(flush)

/*
 * Creating driver objects
 * -----------------------
 */

DECL_DRIVER_API_R_5(backend::VertexBufferHandle, createVertexBuffer,
        uint8_t, bufferCount,
        uint8_t, attributeCount,
        uint32_t, vertexCount,
        backend::AttributeArray, attributes,
        backend::BufferUsage, usage)

DECL_DRIVER_API_R_3(backend::IndexBufferHandle, createIndexBuffer,
        backend::ElementType, elementType,
        uint32_t, indexCount,
        backend::BufferUsage, usage)

DECL_DRIVER_API_R_8(backend::TextureHandle, createTexture,
        backend::SamplerType, target,
        uint8_t, levels,
        backend::TextureFormat, format,
        uint8_t, samples,
        uint32_t, width,
        uint32_t, height,
        uint32_t, depth,
        backend::TextureUsage, usage)

DECL_DRIVER_API_R_1(backend::SamplerGroupHandle, createSamplerGroup,
        size_t, size)

DECL_DRIVER_API_R_2(backend::UniformBufferHandle, createUniformBuffer,
        size_t, size,
        backend::BufferUsage, usage)

DECL_DRIVER_API_R_0(backend::RenderPrimitiveHandle, createRenderPrimitive)

DECL_DRIVER_API_R_1(backend::ProgramHandle, createProgram,
        backend::Program&&, program)

DECL_DRIVER_API_R_0(backend::RenderTargetHandle, createDefaultRenderTarget)

DECL_DRIVER_API_R_7(backend::RenderTargetHandle, createRenderTarget,
        backend::TargetBufferFlags, targetBufferFlags,
        uint32_t, width,
        uint32_t, height,
        uint8_t, samples,
        backend::TargetBufferInfo, color,
        backend::TargetBufferInfo, depth,
        backend::TargetBufferInfo, stencil)

DECL_DRIVER_API_R_0(backend::FenceHandle, createFence)

DECL_DRIVER_API_R_2(backend::SwapChainHandle, createSwapChain, void*, nativeWindow, uint64_t, flags)

DECL_DRIVER_API_R_3(backend::StreamHandle, createStreamFromTextureId, intptr_t, externalTextureId, uint32_t, width, uint32_t, height)

/*
 * Destroying driver objects
 * -------------------------
 */

DECL_DRIVER_API_1(destroyVertexBuffer,    backend::VertexBufferHandle, vbh)
DECL_DRIVER_API_1(destroyIndexBuffer,     backend::IndexBufferHandle, ibh)
DECL_DRIVER_API_1(destroyRenderPrimitive, backend::RenderPrimitiveHandle, rph)
DECL_DRIVER_API_1(destroyProgram,         backend::ProgramHandle, ph)
DECL_DRIVER_API_1(destroySamplerGroup,    backend::SamplerGroupHandle, sbh)
DECL_DRIVER_API_1(destroyUniformBuffer,   backend::UniformBufferHandle, ubh)
DECL_DRIVER_API_1(destroyTexture,         backend::TextureHandle, th)
DECL_DRIVER_API_1(destroyRenderTarget,    backend::RenderTargetHandle, rth)
DECL_DRIVER_API_1(destroySwapChain,       backend::SwapChainHandle, sch)
DECL_DRIVER_API_1(destroyStream,          backend::StreamHandle, sh)

/*
 * Synchronous APIs
 * ----------------
 */

DECL_DRIVER_API_SYNCHRONOUS_0(void, terminate)

DECL_DRIVER_API_SYNCHRONOUS_1(backend::StreamHandle, createStream, void*, stream)

DECL_DRIVER_API_SYNCHRONOUS_3(void, setStreamDimensions, backend::StreamHandle, stream, uint32_t, width, uint32_t, height)

DECL_DRIVER_API_SYNCHRONOUS_1(int64_t, getStreamTimestamp, backend::StreamHandle, stream)

DECL_DRIVER_API_SYNCHRONOUS_1(void, updateStreams, backend::DriverApi*, driver)

DECL_DRIVER_API_SYNCHRONOUS_1(void, destroyFence, backend::FenceHandle, fh)

DECL_DRIVER_API_SYNCHRONOUS_2(backend::FenceStatus, wait, backend::FenceHandle, fh, uint64_t, timeout)

DECL_DRIVER_API_SYNCHRONOUS_1(bool, isTextureFormatSupported, backend::TextureFormat, format)

DECL_DRIVER_API_SYNCHRONOUS_1(bool, isRenderTargetFormatSupported, backend::TextureFormat, format)

DECL_DRIVER_API_SYNCHRONOUS_0(bool, isFrameTimeSupported)

DECL_DRIVER_API_SYNCHRONOUS_0(bool, canGenerateMipmaps)

DECL_DRIVER_API_SYNCHRONOUS_1(void, setupExternalImage, void*, image)

DECL_DRIVER_API_SYNCHRONOUS_1(void, cancelExternalImage, void*, image)

/*
 * Updating driver objects
 * -----------------------
 */

DECL_DRIVER_API_4(updateVertexBuffer,
        backend::VertexBufferHandle, vbh,
        size_t, index,
        backend::BufferDescriptor&&, data,
        uint32_t, byteOffset)

DECL_DRIVER_API_3(updateIndexBuffer,
        backend::IndexBufferHandle, ibh,
        backend::BufferDescriptor&&, data,
        uint32_t, byteOffset)

DECL_DRIVER_API_2(loadUniformBuffer,
        backend::UniformBufferHandle, ubh,
        backend::BufferDescriptor&&, buffer)

DECL_DRIVER_API_2(updateSamplerGroup,
        backend::SamplerGroupHandle, ubh,
        backend::SamplerGroup&&, samplerGroup)

DECL_DRIVER_API_7(update2DImage,
        backend::TextureHandle, th,
        uint32_t, level,
        uint32_t, xoffset,
        uint32_t, yoffset,
        uint32_t, width,
        uint32_t, height,
        backend::PixelBufferDescriptor&&, data)

DECL_DRIVER_API_4(updateCubeImage,
        backend::TextureHandle, th,
        uint32_t, level,
        backend::PixelBufferDescriptor&&, data,
        backend::FaceOffsets, faceOffsets)

DECL_DRIVER_API_1(generateMipmaps,
        backend::TextureHandle, th)

DECL_DRIVER_API_2(setExternalImage,
        backend::TextureHandle, th,
        void*, image)

DECL_DRIVER_API_2(setExternalStream,
        backend::TextureHandle, th,
        backend::StreamHandle, sh)

DECL_DRIVER_API_2(beginRenderPass,
        backend::RenderTargetHandle, rth,
        const backend::RenderPassParams&, params)

DECL_DRIVER_API_0(endRenderPass)

DECL_DRIVER_API_6(discardSubRenderTargetBuffers,
        backend::RenderTargetHandle, rth,
        backend::TargetBufferFlags, targetBufferFlags,
        uint32_t, left,
        uint32_t, bottom,
        uint32_t, width,
        uint32_t, height)

DECL_DRIVER_API_4(setRenderPrimitiveBuffer,
        backend::RenderPrimitiveHandle, rph,
        backend::VertexBufferHandle, vbh,
        backend::IndexBufferHandle, ibh,
        uint32_t, enabledAttributes)

DECL_DRIVER_API_6(setRenderPrimitiveRange,
        backend::RenderPrimitiveHandle, rph,
        backend::PrimitiveType, pt,
        uint32_t, offset,
        uint32_t, minIndex,
        uint32_t, maxIndex,
        uint32_t, count)

// Sets up a scissor rectangle that automatically gets clipped against the viewport.
DECL_DRIVER_API_4(setViewportScissor,
        int32_t, left,
        int32_t, bottom,
        uint32_t, width,
        uint32_t, height)

/*
 * Swap chain
 */

DECL_DRIVER_API_2(makeCurrent,
        backend::SwapChainHandle, schDraw,
        backend::SwapChainHandle, schRead)


DECL_DRIVER_API_1(commit,
        backend::SwapChainHandle, sch)

/*
 * Setting rendering state
 * -----------------------
 */

DECL_DRIVER_API_2(bindUniformBuffer,
        size_t, index,
        backend::UniformBufferHandle, ubh)

DECL_DRIVER_API_4(bindUniformBufferRange,
        size_t, index,
        backend::UniformBufferHandle, ubh,
        size_t, offset,
        size_t, size)

DECL_DRIVER_API_2(bindSamplers,
        size_t, index,
        backend::SamplerGroupHandle, sbh)

DECL_DRIVER_API_2(insertEventMarker,
        const char*, string,
        size_t, len = 0)

DECL_DRIVER_API_2(pushGroupMarker,
        const char*, string,
        size_t, len = 0)

DECL_DRIVER_API_0(popGroupMarker)


/*
 * Read-back operations
 * --------------------
 */

DECL_DRIVER_API_6(readPixels,
        backend::RenderTargetHandle, src,
        uint32_t, x,
        uint32_t, y,
        uint32_t, width,
        uint32_t, height,
        backend::PixelBufferDescriptor&&, data)

DECL_DRIVER_API_6(readStreamPixels,
        backend::StreamHandle, sh,
        uint32_t, x,
        uint32_t, y,
        uint32_t, width,
        uint32_t, height,
        backend::PixelBufferDescriptor&&, data)

/*
 * Rendering operations
 * --------------------
 */

DECL_DRIVER_API_6(blit,
        backend::TargetBufferFlags, buffers,
        backend::RenderTargetHandle, dst,
        backend::Viewport, dstRect,
        backend::RenderTargetHandle, src,
        backend::Viewport, srcRect,
        backend::SamplerMagFilter, filter)

DECL_DRIVER_API_2(draw,
        backend::PipelineState, state,
        backend::RenderPrimitiveHandle, rph)

#pragma clang diagnostic pop

#undef SINGLE_ARG
#undef PARAM_LIST_ADD
#undef ARG_LIST_ADD

#undef PARAM_1
#undef PARAM_2
#undef PARAM_3
#undef PARAM_4
#undef PARAM_5
#undef PARAM_6
#undef PARAM_7
#undef PARAM_8
#undef PARAM_9
#undef PARAM_10
#undef PARAM_11

#undef ARG_1
#undef ARG_2
#undef ARG_3
#undef ARG_4
#undef ARG_5
#undef ARG_6
#undef ARG_7
#undef ARG_8
#undef ARG_9
#undef ARG_10
#undef ARG_11

#undef DECL_DRIVER_API_0
#undef DECL_DRIVER_API_1
#undef DECL_DRIVER_API_2
#undef DECL_DRIVER_API_3
#undef DECL_DRIVER_API_4
#undef DECL_DRIVER_API_5
#undef DECL_DRIVER_API_6
#undef DECL_DRIVER_API_7
#undef DECL_DRIVER_API_8
#undef DECL_DRIVER_API_9
#undef DECL_DRIVER_API_10
#undef DECL_DRIVER_API_11

#undef DECL_DRIVER_API_R_0
#undef DECL_DRIVER_API_R_1
#undef DECL_DRIVER_API_R_2
#undef DECL_DRIVER_API_R_3
#undef DECL_DRIVER_API_R_4
#undef DECL_DRIVER_API_R_5
#undef DECL_DRIVER_API_R_6
#undef DECL_DRIVER_API_R_7
#undef DECL_DRIVER_API_R_8

#undef DECL_DRIVER_API_SYNCHRONOUS_0
#undef DECL_DRIVER_API_SYNCHRONOUS_1
#undef DECL_DRIVER_API_SYNCHRONOUS_2
#undef DECL_DRIVER_API_SYNCHRONOUS_3

#undef DECL_DRIVER_API
#undef DECL_DRIVER_API_SYNCHRONOUS
#undef DECL_DRIVER_API_RETURN

// No include guard!!!
