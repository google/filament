/**
 * Copyright 2019-2023 WebGPU-Native developers
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/** @file */

/**
 * \mainpage
 *
 * **Important:** *This documentation is a Work In Progress.*
 *
 * This is the home of WebGPU C API specification. We define here the standard
 * `webgpu.h` header that all implementations should provide.
 *
 * For all details where behavior is not otherwise specified, `webgpu.h` has
 * the same behavior as the WebGPU specification for JavaScript on the Web.
 * The WebIDL-based Web specification is mapped into C as faithfully (and
 * bidirectionally) as practical/possible.
 * The working draft of WebGPU can be found at <https://www.w3.org/TR/webgpu/>.
 *
 * The standard include directive for this header is `#include <webgpu/webgpu.h>`
 * (if it is provided in a system-wide or toolchain-wide include directory).
 */

#ifndef WEBGPU_H_
#define WEBGPU_H_

#if defined(WGPU_SHARED_LIBRARY)
#    if defined(_WIN32)
#        if defined(WGPU_IMPLEMENTATION)
#            define WGPU_EXPORT __declspec(dllexport)
#        else
#            define WGPU_EXPORT __declspec(dllimport)
#        endif
#    else  // defined(_WIN32)
#        if defined(WGPU_IMPLEMENTATION)
#            define WGPU_EXPORT __attribute__((visibility("default")))
#        else
#            define WGPU_EXPORT
#        endif
#    endif  // defined(_WIN32)
#else       // defined(WGPU_SHARED_LIBRARY)
#    define WGPU_EXPORT
#endif  // defined(WGPU_SHARED_LIBRARY)

#if !defined(WGPU_OBJECT_ATTRIBUTE)
#define WGPU_OBJECT_ATTRIBUTE
#endif
#if !defined(WGPU_ENUM_ATTRIBUTE)
#define WGPU_ENUM_ATTRIBUTE
#endif
#if !defined(WGPU_STRUCTURE_ATTRIBUTE)
#define WGPU_STRUCTURE_ATTRIBUTE
#endif
#if !defined(WGPU_FUNCTION_ATTRIBUTE)
#define WGPU_FUNCTION_ATTRIBUTE
#endif
#if !defined(WGPU_NULLABLE)
#define WGPU_NULLABLE
#endif

#include <stdint.h>
#include <stddef.h>
#include <math.h>

#define _wgpu_COMMA ,
#if defined(__cplusplus)
#  define _wgpu_ENUM_ZERO_INIT(type) type(0)
#  define _wgpu_STRUCT_ZERO_INIT {}
#  if __cplusplus >= 201103L
#    define _wgpu_MAKE_INIT_STRUCT(type, value) (type value)
#  else
#    define _wgpu_MAKE_INIT_STRUCT(type, value) value
#  endif
#else
#  define _wgpu_ENUM_ZERO_INIT(type) (type)0
#  define _wgpu_STRUCT_ZERO_INIT {0}
#  if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#    define _wgpu_MAKE_INIT_STRUCT(type, value) ((type) value)
#  else
#    define _wgpu_MAKE_INIT_STRUCT(type, value) value
#  endif
#endif

/**
 * \defgroup Constants Constants
 * \brief Constants.
 *
 * @{
 */

/**
 * 'True' value of @ref WGPUBool.
 *
 * @remark It's not usually necessary to use this, as `true` (from
 * `stdbool.h` or C++) casts to the same value.
 */
#define WGPU_TRUE (UINT32_C(1))
/**
 * 'False' value of @ref WGPUBool.
 *
 * @remark It's not usually necessary to use this, as `false` (from
 * `stdbool.h` or C++) casts to the same value.
 */
#define WGPU_FALSE (UINT32_C(0))
/**
 * Indicates no array layer count is specified. For more info,
 * see @ref SentinelValues and the places that use this sentinel value.
 */
#define WGPU_ARRAY_LAYER_COUNT_UNDEFINED (UINT32_MAX)
/**
 * Indicates no copy stride is specified. For more info,
 * see @ref SentinelValues and the places that use this sentinel value.
 */
#define WGPU_COPY_STRIDE_UNDEFINED (UINT32_MAX)
/**
 * Indicates no depth clear value is specified. For more info,
 * see @ref SentinelValues and the places that use this sentinel value.
 */
#define WGPU_DEPTH_CLEAR_VALUE_UNDEFINED (NAN)
/**
 * Indicates no depth slice is specified. For more info,
 * see @ref SentinelValues and the places that use this sentinel value.
 */
#define WGPU_DEPTH_SLICE_UNDEFINED (UINT32_MAX)
/**
 * For `uint32_t` limits, indicates no limit value is specified. For more info,
 * see @ref SentinelValues and the places that use this sentinel value.
 */
#define WGPU_LIMIT_U32_UNDEFINED (UINT32_MAX)
/**
 * For `uint64_t` limits, indicates no limit value is specified. For more info,
 * see @ref SentinelValues and the places that use this sentinel value.
 */
#define WGPU_LIMIT_U64_UNDEFINED (UINT64_MAX)
/**
 * Indicates no mip level count is specified. For more info,
 * see @ref SentinelValues and the places that use this sentinel value.
 */
#define WGPU_MIP_LEVEL_COUNT_UNDEFINED (UINT32_MAX)
/**
 * Indicates no query set index is specified. For more info,
 * see @ref SentinelValues and the places that use this sentinel value.
 */
#define WGPU_QUERY_SET_INDEX_UNDEFINED (UINT32_MAX)
/**
 * Sentinel value used in @ref WGPUStringView to indicate that the pointer
 * is to a null-terminated string, rather than an explicitly-sized string.
 */
#define WGPU_STRLEN (SIZE_MAX)
/**
 * Indicates a size extending to the end of the buffer. For more info,
 * see @ref SentinelValues and the places that use this sentinel value.
 */
#define WGPU_WHOLE_MAP_SIZE (SIZE_MAX)
/**
 * Indicates a size extending to the end of the buffer. For more info,
 * see @ref SentinelValues and the places that use this sentinel value.
 */
#define WGPU_WHOLE_SIZE (UINT64_MAX)

/** @} */

/**
 * \defgroup UtilityTypes Utility Types
 *
 * @{
 */

/**
 * Nullable value defining a pointer+length view into a UTF-8 encoded string.
 *
 * Values passed into the API may use the special length value @ref WGPU_STRLEN
 * to indicate a null-terminated string.
 * Non-null values passed out of the API (for example as callback arguments)
 * always provide an explicit length and **may or may not be null-terminated**.
 *
 * Some inputs to the API accept null values. Those which do not accept null
 * values "default" to the empty string when null values are passed.
 *
 * Values are encoded as follows:
 * - `{NULL, WGPU_STRLEN}`: the null value.
 * - `{non_null_pointer, WGPU_STRLEN}`: a null-terminated string view.
 * - `{any, 0}`: the empty string.
 * - `{NULL, non_zero_length}`: not allowed (null dereference).
 * - `{non_null_pointer, non_zero_length}`: an explictly-sized string view with
 *   size `non_zero_length` (in bytes).
 *
 * For info on how this is used in various places, see \ref Strings.
 */
typedef struct WGPUStringView {
    WGPU_NULLABLE char const * data;
    size_t length;
} WGPUStringView WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUStringView.
 */
#define WGPU_STRING_VIEW_INIT _wgpu_MAKE_INIT_STRUCT(WGPUStringView, { \
    /*.data=*/NULL _wgpu_COMMA \
    /*.length=*/WGPU_STRLEN _wgpu_COMMA \
})

typedef uint64_t WGPUFlags;
typedef uint32_t WGPUBool;

/** @} */

/**
 * \defgroup Objects Objects
 * \brief Opaque, non-dispatchable handles to WebGPU objects.
 *
 * @{
 */
typedef struct WGPUAdapterImpl* WGPUAdapter WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPUBindGroupImpl* WGPUBindGroup WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPUBindGroupLayoutImpl* WGPUBindGroupLayout WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPUBufferImpl* WGPUBuffer WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPUCommandBufferImpl* WGPUCommandBuffer WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPUCommandEncoderImpl* WGPUCommandEncoder WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPUComputePassEncoderImpl* WGPUComputePassEncoder WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPUComputePipelineImpl* WGPUComputePipeline WGPU_OBJECT_ATTRIBUTE;
/**
 * TODO
 *
 * Releasing the last ref to a `WGPUDevice` also calls @ref wgpuDeviceDestroy.
 * For more info, see @ref DeviceRelease.
 */
typedef struct WGPUDeviceImpl* WGPUDevice WGPU_OBJECT_ATTRIBUTE;
/**
 * A sampleable 2D texture that may perform 0-copy YUV sampling internally. Creation of @ref WGPUExternalTexture is extremely implementation-dependent and not defined in this header.
 */
typedef struct WGPUExternalTextureImpl* WGPUExternalTexture WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPUInstanceImpl* WGPUInstance WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPUPipelineLayoutImpl* WGPUPipelineLayout WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPUQuerySetImpl* WGPUQuerySet WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPUQueueImpl* WGPUQueue WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPURenderBundleImpl* WGPURenderBundle WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPURenderBundleEncoderImpl* WGPURenderBundleEncoder WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPURenderPassEncoderImpl* WGPURenderPassEncoder WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPURenderPipelineImpl* WGPURenderPipeline WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPUSamplerImpl* WGPUSampler WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPUShaderModuleImpl* WGPUShaderModule WGPU_OBJECT_ATTRIBUTE;
/**
 * An object used to continuously present image data to the user, see @ref Surfaces for more details.
 */
typedef struct WGPUSurfaceImpl* WGPUSurface WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPUTextureImpl* WGPUTexture WGPU_OBJECT_ATTRIBUTE;
typedef struct WGPUTextureViewImpl* WGPUTextureView WGPU_OBJECT_ATTRIBUTE;

/** @} */

// Structure forward declarations
struct WGPUAdapterInfo;
struct WGPUBlendComponent;
struct WGPUBufferBindingLayout;
struct WGPUBufferDescriptor;
struct WGPUColor;
struct WGPUCommandBufferDescriptor;
struct WGPUCommandEncoderDescriptor;
struct WGPUCompatibilityModeLimits;
struct WGPUCompilationMessage;
struct WGPUConstantEntry;
struct WGPUExtent3D;
struct WGPUExternalTextureBindingEntry;
struct WGPUExternalTextureBindingLayout;
struct WGPUFuture;
struct WGPUInstanceLimits;
struct WGPUMultisampleState;
struct WGPUOrigin3D;
struct WGPUPassTimestampWrites;
struct WGPUPipelineLayoutDescriptor;
struct WGPUPrimitiveState;
struct WGPUQuerySetDescriptor;
struct WGPUQueueDescriptor;
struct WGPURenderBundleDescriptor;
struct WGPURenderBundleEncoderDescriptor;
struct WGPURenderPassDepthStencilAttachment;
struct WGPURenderPassMaxDrawCount;
struct WGPURequestAdapterWebXROptions;
struct WGPUSamplerBindingLayout;
struct WGPUSamplerDescriptor;
struct WGPUShaderSourceSPIRV;
struct WGPUShaderSourceWGSL;
struct WGPUStencilFaceState;
struct WGPUStorageTextureBindingLayout;
struct WGPUSupportedFeatures;
struct WGPUSupportedInstanceFeatures;
struct WGPUSupportedWGSLLanguageFeatures;
struct WGPUSurfaceCapabilities;
struct WGPUSurfaceColorManagement;
struct WGPUSurfaceConfiguration;
struct WGPUSurfaceSourceAndroidNativeWindow;
struct WGPUSurfaceSourceMetalLayer;
struct WGPUSurfaceSourceWaylandSurface;
struct WGPUSurfaceSourceWindowsHWND;
struct WGPUSurfaceSourceXCBWindow;
struct WGPUSurfaceSourceXlibWindow;
struct WGPUSurfaceTexture;
struct WGPUTexelCopyBufferLayout;
struct WGPUTextureBindingLayout;
struct WGPUTextureBindingViewDimension;
struct WGPUTextureComponentSwizzle;
struct WGPUVertexAttribute;
struct WGPUBindGroupEntry;
struct WGPUBindGroupLayoutEntry;
struct WGPUBlendState;
struct WGPUCompilationInfo;
struct WGPUComputePassDescriptor;
struct WGPUComputeState;
struct WGPUDepthStencilState;
struct WGPUFutureWaitInfo;
struct WGPUInstanceDescriptor;
struct WGPULimits;
struct WGPURenderPassColorAttachment;
struct WGPURequestAdapterOptions;
struct WGPUShaderModuleDescriptor;
struct WGPUSurfaceDescriptor;
struct WGPUTexelCopyBufferInfo;
struct WGPUTexelCopyTextureInfo;
struct WGPUTextureComponentSwizzleDescriptor;
struct WGPUTextureDescriptor;
struct WGPUVertexBufferLayout;
struct WGPUBindGroupDescriptor;
struct WGPUBindGroupLayoutDescriptor;
struct WGPUColorTargetState;
struct WGPUComputePipelineDescriptor;
struct WGPUDeviceDescriptor;
struct WGPURenderPassDescriptor;
struct WGPUTextureViewDescriptor;
struct WGPUVertexState;
struct WGPUFragmentState;
struct WGPURenderPipelineDescriptor;

// Callback info structure forward declarations
struct WGPUBufferMapCallbackInfo;
struct WGPUCompilationInfoCallbackInfo;
struct WGPUCreateComputePipelineAsyncCallbackInfo;
struct WGPUCreateRenderPipelineAsyncCallbackInfo;
struct WGPUDeviceLostCallbackInfo;
struct WGPUPopErrorScopeCallbackInfo;
struct WGPUQueueWorkDoneCallbackInfo;
struct WGPURequestAdapterCallbackInfo;
struct WGPURequestDeviceCallbackInfo;
struct WGPUUncapturedErrorCallbackInfo;

/**
 * \defgroup Enumerations Enumerations
 * \brief Enums.
 *
 * @{
 */

typedef enum WGPUAdapterType {
    WGPUAdapterType_DiscreteGPU = 0x00000001,
    WGPUAdapterType_IntegratedGPU = 0x00000002,
    WGPUAdapterType_CPU = 0x00000003,
    WGPUAdapterType_Unknown = 0x00000004,
    WGPUAdapterType_Force32 = 0x7FFFFFFF
} WGPUAdapterType WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUAddressMode {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUAddressMode_Undefined = 0x00000000,
    WGPUAddressMode_ClampToEdge = 0x00000001,
    WGPUAddressMode_Repeat = 0x00000002,
    WGPUAddressMode_MirrorRepeat = 0x00000003,
    WGPUAddressMode_Force32 = 0x7FFFFFFF
} WGPUAddressMode WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUBackendType {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUBackendType_Undefined = 0x00000000,
    WGPUBackendType_Null = 0x00000001,
    WGPUBackendType_WebGPU = 0x00000002,
    WGPUBackendType_D3D11 = 0x00000003,
    WGPUBackendType_D3D12 = 0x00000004,
    WGPUBackendType_Metal = 0x00000005,
    WGPUBackendType_Vulkan = 0x00000006,
    WGPUBackendType_OpenGL = 0x00000007,
    WGPUBackendType_OpenGLES = 0x00000008,
    WGPUBackendType_Force32 = 0x7FFFFFFF
} WGPUBackendType WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUBlendFactor {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUBlendFactor_Undefined = 0x00000000,
    WGPUBlendFactor_Zero = 0x00000001,
    WGPUBlendFactor_One = 0x00000002,
    WGPUBlendFactor_Src = 0x00000003,
    WGPUBlendFactor_OneMinusSrc = 0x00000004,
    WGPUBlendFactor_SrcAlpha = 0x00000005,
    WGPUBlendFactor_OneMinusSrcAlpha = 0x00000006,
    WGPUBlendFactor_Dst = 0x00000007,
    WGPUBlendFactor_OneMinusDst = 0x00000008,
    WGPUBlendFactor_DstAlpha = 0x00000009,
    WGPUBlendFactor_OneMinusDstAlpha = 0x0000000A,
    WGPUBlendFactor_SrcAlphaSaturated = 0x0000000B,
    WGPUBlendFactor_Constant = 0x0000000C,
    WGPUBlendFactor_OneMinusConstant = 0x0000000D,
    WGPUBlendFactor_Src1 = 0x0000000E,
    WGPUBlendFactor_OneMinusSrc1 = 0x0000000F,
    WGPUBlendFactor_Src1Alpha = 0x00000010,
    WGPUBlendFactor_OneMinusSrc1Alpha = 0x00000011,
    WGPUBlendFactor_Force32 = 0x7FFFFFFF
} WGPUBlendFactor WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUBlendOperation {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUBlendOperation_Undefined = 0x00000000,
    WGPUBlendOperation_Add = 0x00000001,
    WGPUBlendOperation_Subtract = 0x00000002,
    WGPUBlendOperation_ReverseSubtract = 0x00000003,
    WGPUBlendOperation_Min = 0x00000004,
    WGPUBlendOperation_Max = 0x00000005,
    WGPUBlendOperation_Force32 = 0x7FFFFFFF
} WGPUBlendOperation WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUBufferBindingType {
    /**
     * `0`. Indicates that this @ref WGPUBufferBindingLayout member of
     * its parent @ref WGPUBindGroupLayoutEntry is not used.
     * (See also @ref SentinelValues.)
     */
    WGPUBufferBindingType_BindingNotUsed = 0x00000000,
    /**
     * `1`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUBufferBindingType_Undefined = 0x00000001,
    WGPUBufferBindingType_Uniform = 0x00000002,
    WGPUBufferBindingType_Storage = 0x00000003,
    WGPUBufferBindingType_ReadOnlyStorage = 0x00000004,
    WGPUBufferBindingType_Force32 = 0x7FFFFFFF
} WGPUBufferBindingType WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUBufferMapState {
    WGPUBufferMapState_Unmapped = 0x00000001,
    WGPUBufferMapState_Pending = 0x00000002,
    WGPUBufferMapState_Mapped = 0x00000003,
    WGPUBufferMapState_Force32 = 0x7FFFFFFF
} WGPUBufferMapState WGPU_ENUM_ATTRIBUTE;

/**
 * The callback mode controls how a callback for an asynchronous operation may be fired. See @ref Asynchronous-Operations for how these are used.
 */
typedef enum WGPUCallbackMode {
    /**
     * Callbacks created with `WGPUCallbackMode_WaitAnyOnly`:
     * - fire when the asynchronous operation's future is passed to a call to @ref wgpuInstanceWaitAny
     *   AND the operation has already completed or it completes inside the call to @ref wgpuInstanceWaitAny.
     */
    WGPUCallbackMode_WaitAnyOnly = 0x00000001,
    /**
     * Callbacks created with `WGPUCallbackMode_AllowProcessEvents`:
     * - fire for the same reasons as callbacks created with `WGPUCallbackMode_WaitAnyOnly`
     * - fire inside a call to @ref wgpuInstanceProcessEvents if the asynchronous operation is complete.
     */
    WGPUCallbackMode_AllowProcessEvents = 0x00000002,
    /**
     * Callbacks created with `WGPUCallbackMode_AllowSpontaneous`:
     * - fire for the same reasons as callbacks created with `WGPUCallbackMode_AllowProcessEvents`
     * - **may** fire spontaneously on an arbitrary or application thread, when the WebGPU implementations discovers that the asynchronous operation is complete.
     *
     *   Implementations _should_ fire spontaneous callbacks as soon as possible.
     *
     * @note Because spontaneous callbacks may fire at an arbitrary time on an arbitrary thread, applications should take extra care when acquiring locks or mutating state inside the callback. It undefined behavior to re-entrantly call into the webgpu.h API if the callback fires while inside the callstack of another webgpu.h function that is not `wgpuInstanceWaitAny` or `wgpuInstanceProcessEvents`.
     */
    WGPUCallbackMode_AllowSpontaneous = 0x00000003,
    WGPUCallbackMode_Force32 = 0x7FFFFFFF
} WGPUCallbackMode WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUCompareFunction {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUCompareFunction_Undefined = 0x00000000,
    WGPUCompareFunction_Never = 0x00000001,
    WGPUCompareFunction_Less = 0x00000002,
    WGPUCompareFunction_Equal = 0x00000003,
    WGPUCompareFunction_LessEqual = 0x00000004,
    WGPUCompareFunction_Greater = 0x00000005,
    WGPUCompareFunction_NotEqual = 0x00000006,
    WGPUCompareFunction_GreaterEqual = 0x00000007,
    WGPUCompareFunction_Always = 0x00000008,
    WGPUCompareFunction_Force32 = 0x7FFFFFFF
} WGPUCompareFunction WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUCompilationInfoRequestStatus {
    WGPUCompilationInfoRequestStatus_Success = 0x00000001,
    /**
     * See @ref CallbackStatuses.
     */
    WGPUCompilationInfoRequestStatus_CallbackCancelled = 0x00000002,
    WGPUCompilationInfoRequestStatus_Force32 = 0x7FFFFFFF
} WGPUCompilationInfoRequestStatus WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUCompilationMessageType {
    WGPUCompilationMessageType_Error = 0x00000001,
    WGPUCompilationMessageType_Warning = 0x00000002,
    WGPUCompilationMessageType_Info = 0x00000003,
    WGPUCompilationMessageType_Force32 = 0x7FFFFFFF
} WGPUCompilationMessageType WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUComponentSwizzle {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUComponentSwizzle_Undefined = 0x00000000,
    /**
     * Force its value to 0.
     */
    WGPUComponentSwizzle_Zero = 0x00000001,
    /**
     * Force its value to 1.
     */
    WGPUComponentSwizzle_One = 0x00000002,
    /**
     * Take its value from the red channel of the texture.
     */
    WGPUComponentSwizzle_R = 0x00000003,
    /**
     * Take its value from the green channel of the texture.
     */
    WGPUComponentSwizzle_G = 0x00000004,
    /**
     * Take its value from the blue channel of the texture.
     */
    WGPUComponentSwizzle_B = 0x00000005,
    /**
     * Take its value from the alpha channel of the texture.
     */
    WGPUComponentSwizzle_A = 0x00000006,
    WGPUComponentSwizzle_Force32 = 0x7FFFFFFF
} WGPUComponentSwizzle WGPU_ENUM_ATTRIBUTE;

/**
 * Describes how frames are composited with other contents on the screen when @ref wgpuSurfacePresent is called.
 */
typedef enum WGPUCompositeAlphaMode {
    /**
     * `0`. Lets the WebGPU implementation choose the best mode (supported, and with the best performance) between @ref WGPUCompositeAlphaMode_Opaque or @ref WGPUCompositeAlphaMode_Inherit.
     */
    WGPUCompositeAlphaMode_Auto = 0x00000000,
    /**
     * The alpha component of the image is ignored and teated as if it is always 1.0.
     */
    WGPUCompositeAlphaMode_Opaque = 0x00000001,
    /**
     * The alpha component is respected and non-alpha components are assumed to be already multiplied with the alpha component. For example, (0.5, 0, 0, 0.5) is semi-transparent bright red.
     */
    WGPUCompositeAlphaMode_Premultiplied = 0x00000002,
    /**
     * The alpha component is respected and non-alpha components are assumed to NOT be already multiplied with the alpha component. For example, (1.0, 0, 0, 0.5) is semi-transparent bright red.
     */
    WGPUCompositeAlphaMode_Unpremultiplied = 0x00000003,
    /**
     * The handling of the alpha component is unknown to WebGPU and should be handled by the application using system-specific APIs. This mode may be unavailable (for example on Wasm).
     */
    WGPUCompositeAlphaMode_Inherit = 0x00000004,
    WGPUCompositeAlphaMode_Force32 = 0x7FFFFFFF
} WGPUCompositeAlphaMode WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUCreatePipelineAsyncStatus {
    WGPUCreatePipelineAsyncStatus_Success = 0x00000001,
    /**
     * See @ref CallbackStatuses.
     */
    WGPUCreatePipelineAsyncStatus_CallbackCancelled = 0x00000002,
    WGPUCreatePipelineAsyncStatus_ValidationError = 0x00000003,
    WGPUCreatePipelineAsyncStatus_InternalError = 0x00000004,
    WGPUCreatePipelineAsyncStatus_Force32 = 0x7FFFFFFF
} WGPUCreatePipelineAsyncStatus WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUCullMode {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUCullMode_Undefined = 0x00000000,
    WGPUCullMode_None = 0x00000001,
    WGPUCullMode_Front = 0x00000002,
    WGPUCullMode_Back = 0x00000003,
    WGPUCullMode_Force32 = 0x7FFFFFFF
} WGPUCullMode WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUDeviceLostReason {
    WGPUDeviceLostReason_Unknown = 0x00000001,
    WGPUDeviceLostReason_Destroyed = 0x00000002,
    /**
     * See @ref CallbackStatuses.
     */
    WGPUDeviceLostReason_CallbackCancelled = 0x00000003,
    WGPUDeviceLostReason_FailedCreation = 0x00000004,
    WGPUDeviceLostReason_Force32 = 0x7FFFFFFF
} WGPUDeviceLostReason WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUErrorFilter {
    WGPUErrorFilter_Validation = 0x00000001,
    WGPUErrorFilter_OutOfMemory = 0x00000002,
    WGPUErrorFilter_Internal = 0x00000003,
    WGPUErrorFilter_Force32 = 0x7FFFFFFF
} WGPUErrorFilter WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUErrorType {
    WGPUErrorType_NoError = 0x00000001,
    WGPUErrorType_Validation = 0x00000002,
    WGPUErrorType_OutOfMemory = 0x00000003,
    WGPUErrorType_Internal = 0x00000004,
    WGPUErrorType_Unknown = 0x00000005,
    WGPUErrorType_Force32 = 0x7FFFFFFF
} WGPUErrorType WGPU_ENUM_ATTRIBUTE;

/**
 * See @ref WGPURequestAdapterOptions::featureLevel.
 */
typedef enum WGPUFeatureLevel {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUFeatureLevel_Undefined = 0x00000000,
    /**
     * "Compatibility" profile which can be supported on OpenGL ES 3.1 and D3D11.
     */
    WGPUFeatureLevel_Compatibility = 0x00000001,
    /**
     * "Core" profile which can be supported on Vulkan/Metal/D3D12 (at least).
     */
    WGPUFeatureLevel_Core = 0x00000002,
    WGPUFeatureLevel_Force32 = 0x7FFFFFFF
} WGPUFeatureLevel WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUFeatureName {
    WGPUFeatureName_CoreFeaturesAndLimits = 0x00000001,
    WGPUFeatureName_DepthClipControl = 0x00000002,
    WGPUFeatureName_Depth32FloatStencil8 = 0x00000003,
    WGPUFeatureName_TextureCompressionBC = 0x00000004,
    WGPUFeatureName_TextureCompressionBCSliced3D = 0x00000005,
    WGPUFeatureName_TextureCompressionETC2 = 0x00000006,
    WGPUFeatureName_TextureCompressionASTC = 0x00000007,
    WGPUFeatureName_TextureCompressionASTCSliced3D = 0x00000008,
    WGPUFeatureName_TimestampQuery = 0x00000009,
    WGPUFeatureName_IndirectFirstInstance = 0x0000000A,
    WGPUFeatureName_ShaderF16 = 0x0000000B,
    WGPUFeatureName_RG11B10UfloatRenderable = 0x0000000C,
    WGPUFeatureName_BGRA8UnormStorage = 0x0000000D,
    WGPUFeatureName_Float32Filterable = 0x0000000E,
    WGPUFeatureName_Float32Blendable = 0x0000000F,
    WGPUFeatureName_ClipDistances = 0x00000010,
    WGPUFeatureName_DualSourceBlending = 0x00000011,
    WGPUFeatureName_Subgroups = 0x00000012,
    WGPUFeatureName_TextureFormatsTier1 = 0x00000013,
    WGPUFeatureName_TextureFormatsTier2 = 0x00000014,
    WGPUFeatureName_PrimitiveIndex = 0x00000015,
    WGPUFeatureName_TextureComponentSwizzle = 0x00000016,
    WGPUFeatureName_Force32 = 0x7FFFFFFF
} WGPUFeatureName WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUFilterMode {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUFilterMode_Undefined = 0x00000000,
    WGPUFilterMode_Nearest = 0x00000001,
    WGPUFilterMode_Linear = 0x00000002,
    WGPUFilterMode_Force32 = 0x7FFFFFFF
} WGPUFilterMode WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUFrontFace {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUFrontFace_Undefined = 0x00000000,
    WGPUFrontFace_CCW = 0x00000001,
    WGPUFrontFace_CW = 0x00000002,
    WGPUFrontFace_Force32 = 0x7FFFFFFF
} WGPUFrontFace WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUIndexFormat {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUIndexFormat_Undefined = 0x00000000,
    WGPUIndexFormat_Uint16 = 0x00000001,
    WGPUIndexFormat_Uint32 = 0x00000002,
    WGPUIndexFormat_Force32 = 0x7FFFFFFF
} WGPUIndexFormat WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUInstanceFeatureName {
    /**
     * Enable use of ::wgpuInstanceWaitAny with `timeoutNS > 0`.
     */
    WGPUInstanceFeatureName_TimedWaitAny = 0x00000001,
    /**
     * Enable passing SPIR-V shaders to @ref wgpuDeviceCreateShaderModule,
     * via @ref WGPUShaderSourceSPIRV.
     */
    WGPUInstanceFeatureName_ShaderSourceSPIRV = 0x00000002,
    /**
     * Normally, a @ref WGPUAdapter can only create a single device. If this is
     * available and enabled, then adapters won't immediately expire when they
     * create a device, so can be reused to make multiple devices. They may
     * still expire for other reasons.
     */
    WGPUInstanceFeatureName_MultipleDevicesPerAdapter = 0x00000003,
    WGPUInstanceFeatureName_Force32 = 0x7FFFFFFF
} WGPUInstanceFeatureName WGPU_ENUM_ATTRIBUTE;

typedef enum WGPULoadOp {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPULoadOp_Undefined = 0x00000000,
    WGPULoadOp_Load = 0x00000001,
    WGPULoadOp_Clear = 0x00000002,
    WGPULoadOp_Force32 = 0x7FFFFFFF
} WGPULoadOp WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUMapAsyncStatus {
    WGPUMapAsyncStatus_Success = 0x00000001,
    /**
     * See @ref CallbackStatuses.
     */
    WGPUMapAsyncStatus_CallbackCancelled = 0x00000002,
    WGPUMapAsyncStatus_Error = 0x00000003,
    WGPUMapAsyncStatus_Aborted = 0x00000004,
    WGPUMapAsyncStatus_Force32 = 0x7FFFFFFF
} WGPUMapAsyncStatus WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUMipmapFilterMode {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUMipmapFilterMode_Undefined = 0x00000000,
    WGPUMipmapFilterMode_Nearest = 0x00000001,
    WGPUMipmapFilterMode_Linear = 0x00000002,
    WGPUMipmapFilterMode_Force32 = 0x7FFFFFFF
} WGPUMipmapFilterMode WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUOptionalBool {
    /**
     * `0`.
     */
    WGPUOptionalBool_False = 0x00000000,
    WGPUOptionalBool_True = 0x00000001,
    WGPUOptionalBool_Undefined = 0x00000002,
    WGPUOptionalBool_Force32 = 0x7FFFFFFF
} WGPUOptionalBool WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUPopErrorScopeStatus {
    /**
     * The error scope stack was successfully popped and a result was reported.
     */
    WGPUPopErrorScopeStatus_Success = 0x00000001,
    /**
     * See @ref CallbackStatuses.
     */
    WGPUPopErrorScopeStatus_CallbackCancelled = 0x00000002,
    /**
     * The error scope stack could not be popped, because it was empty.
     */
    WGPUPopErrorScopeStatus_Error = 0x00000003,
    WGPUPopErrorScopeStatus_Force32 = 0x7FFFFFFF
} WGPUPopErrorScopeStatus WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUPowerPreference {
    /**
     * `0`. No preference. (See also @ref SentinelValues.)
     */
    WGPUPowerPreference_Undefined = 0x00000000,
    WGPUPowerPreference_LowPower = 0x00000001,
    WGPUPowerPreference_HighPerformance = 0x00000002,
    WGPUPowerPreference_Force32 = 0x7FFFFFFF
} WGPUPowerPreference WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUPredefinedColorSpace {
    WGPUPredefinedColorSpace_SRGB = 0x00000001,
    WGPUPredefinedColorSpace_DisplayP3 = 0x00000002,
    WGPUPredefinedColorSpace_Force32 = 0x7FFFFFFF
} WGPUPredefinedColorSpace WGPU_ENUM_ATTRIBUTE;

/**
 * Describes when and in which order frames are presented on the screen when @ref wgpuSurfacePresent is called.
 */
typedef enum WGPUPresentMode {
    /**
     * `0`. Present mode is not specified. Use the default.
     */
    WGPUPresentMode_Undefined = 0x00000000,
    /**
     * The presentation of the image to the user waits for the next vertical blanking period to update in a first-in, first-out manner.
     * Tearing cannot be observed and frame-loop will be limited to the display's refresh rate.
     * This is the only mode that's always available.
     */
    WGPUPresentMode_Fifo = 0x00000001,
    /**
     * The presentation of the image to the user tries to wait for the next vertical blanking period but may decide to not wait if a frame is presented late.
     * Tearing can sometimes be observed but late-frame don't produce a full-frame stutter in the presentation.
     * This is still a first-in, first-out mechanism so a frame-loop will be limited to the display's refresh rate.
     */
    WGPUPresentMode_FifoRelaxed = 0x00000002,
    /**
     * The presentation of the image to the user is updated immediately without waiting for a vertical blank.
     * Tearing can be observed but latency is minimized.
     */
    WGPUPresentMode_Immediate = 0x00000003,
    /**
     * The presentation of the image to the user waits for the next vertical blanking period to update to the latest provided image.
     * Tearing cannot be observed and a frame-loop is not limited to the display's refresh rate.
     */
    WGPUPresentMode_Mailbox = 0x00000004,
    WGPUPresentMode_Force32 = 0x7FFFFFFF
} WGPUPresentMode WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUPrimitiveTopology {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUPrimitiveTopology_Undefined = 0x00000000,
    WGPUPrimitiveTopology_PointList = 0x00000001,
    WGPUPrimitiveTopology_LineList = 0x00000002,
    WGPUPrimitiveTopology_LineStrip = 0x00000003,
    WGPUPrimitiveTopology_TriangleList = 0x00000004,
    WGPUPrimitiveTopology_TriangleStrip = 0x00000005,
    WGPUPrimitiveTopology_Force32 = 0x7FFFFFFF
} WGPUPrimitiveTopology WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUQueryType {
    WGPUQueryType_Occlusion = 0x00000001,
    WGPUQueryType_Timestamp = 0x00000002,
    WGPUQueryType_Force32 = 0x7FFFFFFF
} WGPUQueryType WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUQueueWorkDoneStatus {
    WGPUQueueWorkDoneStatus_Success = 0x00000001,
    /**
     * See @ref CallbackStatuses.
     */
    WGPUQueueWorkDoneStatus_CallbackCancelled = 0x00000002,
    /**
     * There was some deterministic error. (Note this is currently never used,
     * but it will be relevant when it's possible to create a queue object.)
     */
    WGPUQueueWorkDoneStatus_Error = 0x00000003,
    WGPUQueueWorkDoneStatus_Force32 = 0x7FFFFFFF
} WGPUQueueWorkDoneStatus WGPU_ENUM_ATTRIBUTE;

typedef enum WGPURequestAdapterStatus {
    WGPURequestAdapterStatus_Success = 0x00000001,
    /**
     * See @ref CallbackStatuses.
     */
    WGPURequestAdapterStatus_CallbackCancelled = 0x00000002,
    WGPURequestAdapterStatus_Unavailable = 0x00000003,
    WGPURequestAdapterStatus_Error = 0x00000004,
    WGPURequestAdapterStatus_Force32 = 0x7FFFFFFF
} WGPURequestAdapterStatus WGPU_ENUM_ATTRIBUTE;

typedef enum WGPURequestDeviceStatus {
    WGPURequestDeviceStatus_Success = 0x00000001,
    /**
     * See @ref CallbackStatuses.
     */
    WGPURequestDeviceStatus_CallbackCancelled = 0x00000002,
    WGPURequestDeviceStatus_Error = 0x00000003,
    WGPURequestDeviceStatus_Force32 = 0x7FFFFFFF
} WGPURequestDeviceStatus WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUSamplerBindingType {
    /**
     * `0`. Indicates that this @ref WGPUSamplerBindingLayout member of
     * its parent @ref WGPUBindGroupLayoutEntry is not used.
     * (See also @ref SentinelValues.)
     */
    WGPUSamplerBindingType_BindingNotUsed = 0x00000000,
    /**
     * `1`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUSamplerBindingType_Undefined = 0x00000001,
    WGPUSamplerBindingType_Filtering = 0x00000002,
    WGPUSamplerBindingType_NonFiltering = 0x00000003,
    WGPUSamplerBindingType_Comparison = 0x00000004,
    WGPUSamplerBindingType_Force32 = 0x7FFFFFFF
} WGPUSamplerBindingType WGPU_ENUM_ATTRIBUTE;

/**
 * Status code returned (synchronously) from many operations. Generally
 * indicates an invalid input like an unknown enum value or @ref OutStructChainError.
 * Read the function's documentation for specific error conditions.
 */
typedef enum WGPUStatus {
    WGPUStatus_Success = 0x00000001,
    WGPUStatus_Error = 0x00000002,
    WGPUStatus_Force32 = 0x7FFFFFFF
} WGPUStatus WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUStencilOperation {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUStencilOperation_Undefined = 0x00000000,
    WGPUStencilOperation_Keep = 0x00000001,
    WGPUStencilOperation_Zero = 0x00000002,
    WGPUStencilOperation_Replace = 0x00000003,
    WGPUStencilOperation_Invert = 0x00000004,
    WGPUStencilOperation_IncrementClamp = 0x00000005,
    WGPUStencilOperation_DecrementClamp = 0x00000006,
    WGPUStencilOperation_IncrementWrap = 0x00000007,
    WGPUStencilOperation_DecrementWrap = 0x00000008,
    WGPUStencilOperation_Force32 = 0x7FFFFFFF
} WGPUStencilOperation WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUStorageTextureAccess {
    /**
     * `0`. Indicates that this @ref WGPUStorageTextureBindingLayout member of
     * its parent @ref WGPUBindGroupLayoutEntry is not used.
     * (See also @ref SentinelValues.)
     */
    WGPUStorageTextureAccess_BindingNotUsed = 0x00000000,
    /**
     * `1`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUStorageTextureAccess_Undefined = 0x00000001,
    WGPUStorageTextureAccess_WriteOnly = 0x00000002,
    WGPUStorageTextureAccess_ReadOnly = 0x00000003,
    WGPUStorageTextureAccess_ReadWrite = 0x00000004,
    WGPUStorageTextureAccess_Force32 = 0x7FFFFFFF
} WGPUStorageTextureAccess WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUStoreOp {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUStoreOp_Undefined = 0x00000000,
    WGPUStoreOp_Store = 0x00000001,
    WGPUStoreOp_Discard = 0x00000002,
    WGPUStoreOp_Force32 = 0x7FFFFFFF
} WGPUStoreOp WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUSType {
    WGPUSType_ShaderSourceSPIRV = 0x00000001,
    WGPUSType_ShaderSourceWGSL = 0x00000002,
    WGPUSType_RenderPassMaxDrawCount = 0x00000003,
    WGPUSType_SurfaceSourceMetalLayer = 0x00000004,
    WGPUSType_SurfaceSourceWindowsHWND = 0x00000005,
    WGPUSType_SurfaceSourceXlibWindow = 0x00000006,
    WGPUSType_SurfaceSourceWaylandSurface = 0x00000007,
    WGPUSType_SurfaceSourceAndroidNativeWindow = 0x00000008,
    WGPUSType_SurfaceSourceXCBWindow = 0x00000009,
    WGPUSType_SurfaceColorManagement = 0x0000000A,
    WGPUSType_RequestAdapterWebXROptions = 0x0000000B,
    WGPUSType_TextureComponentSwizzleDescriptor = 0x0000000C,
    WGPUSType_ExternalTextureBindingLayout = 0x0000000D,
    WGPUSType_ExternalTextureBindingEntry = 0x0000000E,
    WGPUSType_CompatibilityModeLimits = 0x0000000F,
    WGPUSType_TextureBindingViewDimension = 0x00000010,
    WGPUSType_Force32 = 0x7FFFFFFF
} WGPUSType WGPU_ENUM_ATTRIBUTE;

/**
 * The status enum for @ref wgpuSurfaceGetCurrentTexture.
 */
typedef enum WGPUSurfaceGetCurrentTextureStatus {
    /**
     * Yay! Everything is good and we can render this frame.
     */
    WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal = 0x00000001,
    /**
     * Still OK - the surface can present the frame, but in a suboptimal way. The surface may need reconfiguration.
     */
    WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal = 0x00000002,
    /**
     * Some operation timed out while trying to acquire the frame.
     */
    WGPUSurfaceGetCurrentTextureStatus_Timeout = 0x00000003,
    /**
     * The surface is too different to be used, compared to when it was originally created.
     */
    WGPUSurfaceGetCurrentTextureStatus_Outdated = 0x00000004,
    /**
     * The connection to whatever owns the surface was lost, or generally needs to be fully reinitialized.
     */
    WGPUSurfaceGetCurrentTextureStatus_Lost = 0x00000005,
    /**
     * There was some deterministic error (for example, the surface is not configured, or there was an @ref OutStructChainError). Should produce @ref ImplementationDefinedLogging containing details.
     */
    WGPUSurfaceGetCurrentTextureStatus_Error = 0x00000006,
    WGPUSurfaceGetCurrentTextureStatus_Force32 = 0x7FFFFFFF
} WGPUSurfaceGetCurrentTextureStatus WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUTextureAspect {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUTextureAspect_Undefined = 0x00000000,
    WGPUTextureAspect_All = 0x00000001,
    WGPUTextureAspect_StencilOnly = 0x00000002,
    WGPUTextureAspect_DepthOnly = 0x00000003,
    WGPUTextureAspect_Force32 = 0x7FFFFFFF
} WGPUTextureAspect WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUTextureDimension {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUTextureDimension_Undefined = 0x00000000,
    WGPUTextureDimension_1D = 0x00000001,
    WGPUTextureDimension_2D = 0x00000002,
    WGPUTextureDimension_3D = 0x00000003,
    WGPUTextureDimension_Force32 = 0x7FFFFFFF
} WGPUTextureDimension WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUTextureFormat {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUTextureFormat_Undefined = 0x00000000,
    WGPUTextureFormat_R8Unorm = 0x00000001,
    WGPUTextureFormat_R8Snorm = 0x00000002,
    WGPUTextureFormat_R8Uint = 0x00000003,
    WGPUTextureFormat_R8Sint = 0x00000004,
    WGPUTextureFormat_R16Unorm = 0x00000005,
    WGPUTextureFormat_R16Snorm = 0x00000006,
    WGPUTextureFormat_R16Uint = 0x00000007,
    WGPUTextureFormat_R16Sint = 0x00000008,
    WGPUTextureFormat_R16Float = 0x00000009,
    WGPUTextureFormat_RG8Unorm = 0x0000000A,
    WGPUTextureFormat_RG8Snorm = 0x0000000B,
    WGPUTextureFormat_RG8Uint = 0x0000000C,
    WGPUTextureFormat_RG8Sint = 0x0000000D,
    WGPUTextureFormat_R32Float = 0x0000000E,
    WGPUTextureFormat_R32Uint = 0x0000000F,
    WGPUTextureFormat_R32Sint = 0x00000010,
    WGPUTextureFormat_RG16Unorm = 0x00000011,
    WGPUTextureFormat_RG16Snorm = 0x00000012,
    WGPUTextureFormat_RG16Uint = 0x00000013,
    WGPUTextureFormat_RG16Sint = 0x00000014,
    WGPUTextureFormat_RG16Float = 0x00000015,
    WGPUTextureFormat_RGBA8Unorm = 0x00000016,
    WGPUTextureFormat_RGBA8UnormSrgb = 0x00000017,
    WGPUTextureFormat_RGBA8Snorm = 0x00000018,
    WGPUTextureFormat_RGBA8Uint = 0x00000019,
    WGPUTextureFormat_RGBA8Sint = 0x0000001A,
    WGPUTextureFormat_BGRA8Unorm = 0x0000001B,
    WGPUTextureFormat_BGRA8UnormSrgb = 0x0000001C,
    WGPUTextureFormat_RGB10A2Uint = 0x0000001D,
    WGPUTextureFormat_RGB10A2Unorm = 0x0000001E,
    WGPUTextureFormat_RG11B10Ufloat = 0x0000001F,
    WGPUTextureFormat_RGB9E5Ufloat = 0x00000020,
    WGPUTextureFormat_RG32Float = 0x00000021,
    WGPUTextureFormat_RG32Uint = 0x00000022,
    WGPUTextureFormat_RG32Sint = 0x00000023,
    WGPUTextureFormat_RGBA16Unorm = 0x00000024,
    WGPUTextureFormat_RGBA16Snorm = 0x00000025,
    WGPUTextureFormat_RGBA16Uint = 0x00000026,
    WGPUTextureFormat_RGBA16Sint = 0x00000027,
    WGPUTextureFormat_RGBA16Float = 0x00000028,
    WGPUTextureFormat_RGBA32Float = 0x00000029,
    WGPUTextureFormat_RGBA32Uint = 0x0000002A,
    WGPUTextureFormat_RGBA32Sint = 0x0000002B,
    WGPUTextureFormat_Stencil8 = 0x0000002C,
    WGPUTextureFormat_Depth16Unorm = 0x0000002D,
    WGPUTextureFormat_Depth24Plus = 0x0000002E,
    WGPUTextureFormat_Depth24PlusStencil8 = 0x0000002F,
    WGPUTextureFormat_Depth32Float = 0x00000030,
    WGPUTextureFormat_Depth32FloatStencil8 = 0x00000031,
    WGPUTextureFormat_BC1RGBAUnorm = 0x00000032,
    WGPUTextureFormat_BC1RGBAUnormSrgb = 0x00000033,
    WGPUTextureFormat_BC2RGBAUnorm = 0x00000034,
    WGPUTextureFormat_BC2RGBAUnormSrgb = 0x00000035,
    WGPUTextureFormat_BC3RGBAUnorm = 0x00000036,
    WGPUTextureFormat_BC3RGBAUnormSrgb = 0x00000037,
    WGPUTextureFormat_BC4RUnorm = 0x00000038,
    WGPUTextureFormat_BC4RSnorm = 0x00000039,
    WGPUTextureFormat_BC5RGUnorm = 0x0000003A,
    WGPUTextureFormat_BC5RGSnorm = 0x0000003B,
    WGPUTextureFormat_BC6HRGBUfloat = 0x0000003C,
    WGPUTextureFormat_BC6HRGBFloat = 0x0000003D,
    WGPUTextureFormat_BC7RGBAUnorm = 0x0000003E,
    WGPUTextureFormat_BC7RGBAUnormSrgb = 0x0000003F,
    WGPUTextureFormat_ETC2RGB8Unorm = 0x00000040,
    WGPUTextureFormat_ETC2RGB8UnormSrgb = 0x00000041,
    WGPUTextureFormat_ETC2RGB8A1Unorm = 0x00000042,
    WGPUTextureFormat_ETC2RGB8A1UnormSrgb = 0x00000043,
    WGPUTextureFormat_ETC2RGBA8Unorm = 0x00000044,
    WGPUTextureFormat_ETC2RGBA8UnormSrgb = 0x00000045,
    WGPUTextureFormat_EACR11Unorm = 0x00000046,
    WGPUTextureFormat_EACR11Snorm = 0x00000047,
    WGPUTextureFormat_EACRG11Unorm = 0x00000048,
    WGPUTextureFormat_EACRG11Snorm = 0x00000049,
    WGPUTextureFormat_ASTC4x4Unorm = 0x0000004A,
    WGPUTextureFormat_ASTC4x4UnormSrgb = 0x0000004B,
    WGPUTextureFormat_ASTC5x4Unorm = 0x0000004C,
    WGPUTextureFormat_ASTC5x4UnormSrgb = 0x0000004D,
    WGPUTextureFormat_ASTC5x5Unorm = 0x0000004E,
    WGPUTextureFormat_ASTC5x5UnormSrgb = 0x0000004F,
    WGPUTextureFormat_ASTC6x5Unorm = 0x00000050,
    WGPUTextureFormat_ASTC6x5UnormSrgb = 0x00000051,
    WGPUTextureFormat_ASTC6x6Unorm = 0x00000052,
    WGPUTextureFormat_ASTC6x6UnormSrgb = 0x00000053,
    WGPUTextureFormat_ASTC8x5Unorm = 0x00000054,
    WGPUTextureFormat_ASTC8x5UnormSrgb = 0x00000055,
    WGPUTextureFormat_ASTC8x6Unorm = 0x00000056,
    WGPUTextureFormat_ASTC8x6UnormSrgb = 0x00000057,
    WGPUTextureFormat_ASTC8x8Unorm = 0x00000058,
    WGPUTextureFormat_ASTC8x8UnormSrgb = 0x00000059,
    WGPUTextureFormat_ASTC10x5Unorm = 0x0000005A,
    WGPUTextureFormat_ASTC10x5UnormSrgb = 0x0000005B,
    WGPUTextureFormat_ASTC10x6Unorm = 0x0000005C,
    WGPUTextureFormat_ASTC10x6UnormSrgb = 0x0000005D,
    WGPUTextureFormat_ASTC10x8Unorm = 0x0000005E,
    WGPUTextureFormat_ASTC10x8UnormSrgb = 0x0000005F,
    WGPUTextureFormat_ASTC10x10Unorm = 0x00000060,
    WGPUTextureFormat_ASTC10x10UnormSrgb = 0x00000061,
    WGPUTextureFormat_ASTC12x10Unorm = 0x00000062,
    WGPUTextureFormat_ASTC12x10UnormSrgb = 0x00000063,
    WGPUTextureFormat_ASTC12x12Unorm = 0x00000064,
    WGPUTextureFormat_ASTC12x12UnormSrgb = 0x00000065,
    WGPUTextureFormat_Force32 = 0x7FFFFFFF
} WGPUTextureFormat WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUTextureSampleType {
    /**
     * `0`. Indicates that this @ref WGPUTextureBindingLayout member of
     * its parent @ref WGPUBindGroupLayoutEntry is not used.
     * (See also @ref SentinelValues.)
     */
    WGPUTextureSampleType_BindingNotUsed = 0x00000000,
    /**
     * `1`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUTextureSampleType_Undefined = 0x00000001,
    WGPUTextureSampleType_Float = 0x00000002,
    WGPUTextureSampleType_UnfilterableFloat = 0x00000003,
    WGPUTextureSampleType_Depth = 0x00000004,
    WGPUTextureSampleType_Sint = 0x00000005,
    WGPUTextureSampleType_Uint = 0x00000006,
    WGPUTextureSampleType_Force32 = 0x7FFFFFFF
} WGPUTextureSampleType WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUTextureViewDimension {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUTextureViewDimension_Undefined = 0x00000000,
    WGPUTextureViewDimension_1D = 0x00000001,
    WGPUTextureViewDimension_2D = 0x00000002,
    WGPUTextureViewDimension_2DArray = 0x00000003,
    WGPUTextureViewDimension_Cube = 0x00000004,
    WGPUTextureViewDimension_CubeArray = 0x00000005,
    WGPUTextureViewDimension_3D = 0x00000006,
    WGPUTextureViewDimension_Force32 = 0x7FFFFFFF
} WGPUTextureViewDimension WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUToneMappingMode {
    WGPUToneMappingMode_Standard = 0x00000001,
    WGPUToneMappingMode_Extended = 0x00000002,
    WGPUToneMappingMode_Force32 = 0x7FFFFFFF
} WGPUToneMappingMode WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUVertexFormat {
    WGPUVertexFormat_Uint8 = 0x00000001,
    WGPUVertexFormat_Uint8x2 = 0x00000002,
    WGPUVertexFormat_Uint8x4 = 0x00000003,
    WGPUVertexFormat_Sint8 = 0x00000004,
    WGPUVertexFormat_Sint8x2 = 0x00000005,
    WGPUVertexFormat_Sint8x4 = 0x00000006,
    WGPUVertexFormat_Unorm8 = 0x00000007,
    WGPUVertexFormat_Unorm8x2 = 0x00000008,
    WGPUVertexFormat_Unorm8x4 = 0x00000009,
    WGPUVertexFormat_Snorm8 = 0x0000000A,
    WGPUVertexFormat_Snorm8x2 = 0x0000000B,
    WGPUVertexFormat_Snorm8x4 = 0x0000000C,
    WGPUVertexFormat_Uint16 = 0x0000000D,
    WGPUVertexFormat_Uint16x2 = 0x0000000E,
    WGPUVertexFormat_Uint16x4 = 0x0000000F,
    WGPUVertexFormat_Sint16 = 0x00000010,
    WGPUVertexFormat_Sint16x2 = 0x00000011,
    WGPUVertexFormat_Sint16x4 = 0x00000012,
    WGPUVertexFormat_Unorm16 = 0x00000013,
    WGPUVertexFormat_Unorm16x2 = 0x00000014,
    WGPUVertexFormat_Unorm16x4 = 0x00000015,
    WGPUVertexFormat_Snorm16 = 0x00000016,
    WGPUVertexFormat_Snorm16x2 = 0x00000017,
    WGPUVertexFormat_Snorm16x4 = 0x00000018,
    WGPUVertexFormat_Float16 = 0x00000019,
    WGPUVertexFormat_Float16x2 = 0x0000001A,
    WGPUVertexFormat_Float16x4 = 0x0000001B,
    WGPUVertexFormat_Float32 = 0x0000001C,
    WGPUVertexFormat_Float32x2 = 0x0000001D,
    WGPUVertexFormat_Float32x3 = 0x0000001E,
    WGPUVertexFormat_Float32x4 = 0x0000001F,
    WGPUVertexFormat_Uint32 = 0x00000020,
    WGPUVertexFormat_Uint32x2 = 0x00000021,
    WGPUVertexFormat_Uint32x3 = 0x00000022,
    WGPUVertexFormat_Uint32x4 = 0x00000023,
    WGPUVertexFormat_Sint32 = 0x00000024,
    WGPUVertexFormat_Sint32x2 = 0x00000025,
    WGPUVertexFormat_Sint32x3 = 0x00000026,
    WGPUVertexFormat_Sint32x4 = 0x00000027,
    WGPUVertexFormat_Unorm10_10_10_2 = 0x00000028,
    WGPUVertexFormat_Unorm8x4BGRA = 0x00000029,
    WGPUVertexFormat_Force32 = 0x7FFFFFFF
} WGPUVertexFormat WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUVertexStepMode {
    /**
     * `0`. Indicates no value is passed for this argument. See @ref SentinelValues.
     */
    WGPUVertexStepMode_Undefined = 0x00000000,
    WGPUVertexStepMode_Vertex = 0x00000001,
    WGPUVertexStepMode_Instance = 0x00000002,
    WGPUVertexStepMode_Force32 = 0x7FFFFFFF
} WGPUVertexStepMode WGPU_ENUM_ATTRIBUTE;

/**
 * Status returned from a call to ::wgpuInstanceWaitAny.
 */
typedef enum WGPUWaitStatus {
    /**
     * At least one WGPUFuture completed successfully.
     */
    WGPUWaitStatus_Success = 0x00000001,
    /**
     * The wait operation succeeded, but no WGPUFutures completed within the timeout.
     */
    WGPUWaitStatus_TimedOut = 0x00000002,
    /**
     * The call was invalid for some reason (see @ref Wait-Any).
     * Should produce @ref ImplementationDefinedLogging containing details.
     */
    WGPUWaitStatus_Error = 0x00000003,
    WGPUWaitStatus_Force32 = 0x7FFFFFFF
} WGPUWaitStatus WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUWGSLLanguageFeatureName {
    WGPUWGSLLanguageFeatureName_ReadonlyAndReadwriteStorageTextures = 0x00000001,
    WGPUWGSLLanguageFeatureName_Packed4x8IntegerDotProduct = 0x00000002,
    WGPUWGSLLanguageFeatureName_UnrestrictedPointerParameters = 0x00000003,
    WGPUWGSLLanguageFeatureName_PointerCompositeAccess = 0x00000004,
    WGPUWGSLLanguageFeatureName_UniformBufferStandardLayout = 0x00000005,
    WGPUWGSLLanguageFeatureName_SubgroupId = 0x00000006,
    WGPUWGSLLanguageFeatureName_TextureAndSamplerLet = 0x00000007,
    WGPUWGSLLanguageFeatureName_SubgroupUniformity = 0x00000008,
    WGPUWGSLLanguageFeatureName_TextureFormatsTier1 = 0x00000009,
    WGPUWGSLLanguageFeatureName_LinearIndexing = 0x0000000A,
    WGPUWGSLLanguageFeatureName_Force32 = 0x7FFFFFFF
} WGPUWGSLLanguageFeatureName WGPU_ENUM_ATTRIBUTE;

/** @} */

/**
 * \defgroup Bitflags Bitflags
 * \brief Type and constant definitions for bitflag types.
 *
 * @{
 */

/**
 * For reserved non-standard bitflag values, see @ref BitflagRegistry.
 */
typedef WGPUFlags WGPUBufferUsage;
/**
 * `0`.
 */
static const WGPUBufferUsage WGPUBufferUsage_None = 0x0000000000000000;
/**
 * The buffer can be *mapped* on the CPU side in *read* mode (using @ref WGPUMapMode_Read).
 */
static const WGPUBufferUsage WGPUBufferUsage_MapRead = 0x0000000000000001;
/**
 * The buffer can be *mapped* on the CPU side in *write* mode (using @ref WGPUMapMode_Write).
 *
 * @note This usage is **not** required to set `mappedAtCreation` to `true` in @ref WGPUBufferDescriptor.
 */
static const WGPUBufferUsage WGPUBufferUsage_MapWrite = 0x0000000000000002;
/**
 * The buffer can be used as the *source* of a GPU-side copy operation.
 */
static const WGPUBufferUsage WGPUBufferUsage_CopySrc = 0x0000000000000004;
/**
 * The buffer can be used as the *destination* of a GPU-side copy operation.
 */
static const WGPUBufferUsage WGPUBufferUsage_CopyDst = 0x0000000000000008;
/**
 * The buffer can be used as an Index buffer when doing indexed drawing in a render pipeline.
 */
static const WGPUBufferUsage WGPUBufferUsage_Index = 0x0000000000000010;
/**
 * The buffer can be used as a Vertex buffer when using a render pipeline.
 */
static const WGPUBufferUsage WGPUBufferUsage_Vertex = 0x0000000000000020;
/**
 * The buffer can be bound to a shader as a uniform buffer.
 */
static const WGPUBufferUsage WGPUBufferUsage_Uniform = 0x0000000000000040;
/**
 * The buffer can be bound to a shader as a storage buffer.
 */
static const WGPUBufferUsage WGPUBufferUsage_Storage = 0x0000000000000080;
/**
 * The buffer can store arguments for an indirect draw call.
 */
static const WGPUBufferUsage WGPUBufferUsage_Indirect = 0x0000000000000100;
/**
 * The buffer can store the result of a timestamp or occlusion query.
 */
static const WGPUBufferUsage WGPUBufferUsage_QueryResolve = 0x0000000000000200;

/**
 * For reserved non-standard bitflag values, see @ref BitflagRegistry.
 */
typedef WGPUFlags WGPUColorWriteMask;
/**
 * `0`.
 */
static const WGPUColorWriteMask WGPUColorWriteMask_None = 0x0000000000000000;
static const WGPUColorWriteMask WGPUColorWriteMask_Red = 0x0000000000000001;
static const WGPUColorWriteMask WGPUColorWriteMask_Green = 0x0000000000000002;
static const WGPUColorWriteMask WGPUColorWriteMask_Blue = 0x0000000000000004;
static const WGPUColorWriteMask WGPUColorWriteMask_Alpha = 0x0000000000000008;
/**
 * `Red | Green | Blue | Alpha`.
 */
static const WGPUColorWriteMask WGPUColorWriteMask_All = 0x000000000000000F;

/**
 * For reserved non-standard bitflag values, see @ref BitflagRegistry.
 */
typedef WGPUFlags WGPUMapMode;
/**
 * `0`.
 */
static const WGPUMapMode WGPUMapMode_None = 0x0000000000000000;
static const WGPUMapMode WGPUMapMode_Read = 0x0000000000000001;
static const WGPUMapMode WGPUMapMode_Write = 0x0000000000000002;

/**
 * For reserved non-standard bitflag values, see @ref BitflagRegistry.
 */
typedef WGPUFlags WGPUShaderStage;
/**
 * `0`.
 */
static const WGPUShaderStage WGPUShaderStage_None = 0x0000000000000000;
static const WGPUShaderStage WGPUShaderStage_Vertex = 0x0000000000000001;
static const WGPUShaderStage WGPUShaderStage_Fragment = 0x0000000000000002;
static const WGPUShaderStage WGPUShaderStage_Compute = 0x0000000000000004;

/**
 * For reserved non-standard bitflag values, see @ref BitflagRegistry.
 */
typedef WGPUFlags WGPUTextureUsage;
/**
 * `0`.
 */
static const WGPUTextureUsage WGPUTextureUsage_None = 0x0000000000000000;
static const WGPUTextureUsage WGPUTextureUsage_CopySrc = 0x0000000000000001;
static const WGPUTextureUsage WGPUTextureUsage_CopyDst = 0x0000000000000002;
static const WGPUTextureUsage WGPUTextureUsage_TextureBinding = 0x0000000000000004;
static const WGPUTextureUsage WGPUTextureUsage_StorageBinding = 0x0000000000000008;
static const WGPUTextureUsage WGPUTextureUsage_RenderAttachment = 0x0000000000000010;
static const WGPUTextureUsage WGPUTextureUsage_TransientAttachment = 0x0000000000000020;

/** @} */

typedef void (*WGPUProc)(void) WGPU_FUNCTION_ATTRIBUTE;

/**
 * \defgroup Callbacks Callbacks
 * \brief Callbacks through which asynchronous functions return.
 *
 * @{
 */

/**
 * See also @ref CallbackError.
 *
 * @param message
 * This parameter is @ref PassedWithoutOwnership.
 */
typedef void (*WGPUBufferMapCallback)(WGPUMapAsyncStatus status, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) WGPU_FUNCTION_ATTRIBUTE;

/**
 * See also @ref CallbackError.
 *
 * @param compilationInfo
 * This argument contains multiple @ref ImplementationAllocatedStructChain roots.
 * Arbitrary chains must be handled gracefully by the application!
 * This parameter is @ref PassedWithoutOwnership.
 */
typedef void (*WGPUCompilationInfoCallback)(WGPUCompilationInfoRequestStatus status, struct WGPUCompilationInfo const * compilationInfo, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) WGPU_FUNCTION_ATTRIBUTE;

/**
 * See also @ref CallbackError.
 *
 * @param pipeline
 * This parameter is @ref PassedWithOwnership.
 */
typedef void (*WGPUCreateComputePipelineAsyncCallback)(WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline pipeline, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) WGPU_FUNCTION_ATTRIBUTE;

/**
 * See also @ref CallbackError.
 *
 * @param pipeline
 * This parameter is @ref PassedWithOwnership.
 */
typedef void (*WGPUCreateRenderPipelineAsyncCallback)(WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline pipeline, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) WGPU_FUNCTION_ATTRIBUTE;

/**
 * See also @ref CallbackError.
 *
 * @param device
 * Pointer to the device which was lost. This is always a non-null pointer.
 * The pointed-to @ref WGPUDevice will be null if, and only if, either:
 * (1) The `reason` is @ref WGPUDeviceLostReason_FailedCreation.
 * (2) The last ref of the device has been (or is being) released: see @ref DeviceRelease.
 * This parameter is @ref PassedWithoutOwnership.
 *
 * @param reason
 * An error code explaining why the device was lost.
 *
 * @param message
 * A @ref LocalizableHumanReadableMessageString describing why the device was lost.
 * This parameter is @ref PassedWithoutOwnership.
 */
typedef void (*WGPUDeviceLostCallback)(WGPUDevice const * device, WGPUDeviceLostReason reason, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) WGPU_FUNCTION_ATTRIBUTE;

/**
 * See also @ref CallbackError.
 *
 * @param status
 * See @ref WGPUPopErrorScopeStatus.
 *
 * @param type
 * The type of the error caught by the scope, or @ref WGPUErrorType_NoError if there was none.
 * If the `status` is not @ref WGPUPopErrorScopeStatus_Success, this is @ref WGPUErrorType_NoError.
 *
 * @param message
 * If the `status` is not @ref WGPUPopErrorScopeStatus_Success **or**
 * the `type` is not @ref WGPUErrorType_NoError, this is a non-empty
 * @ref LocalizableHumanReadableMessageString;
 * otherwise, this is an empty string.
 * This parameter is @ref PassedWithoutOwnership.
 */
typedef void (*WGPUPopErrorScopeCallback)(WGPUPopErrorScopeStatus status, WGPUErrorType type, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) WGPU_FUNCTION_ATTRIBUTE;

/**
 * See also @ref CallbackError.
 *
 * @param status
 * See @ref WGPUQueueWorkDoneStatus.
 *
 * @param message
 * If the `status` is not @ref WGPUQueueWorkDoneStatus_Success,
 * this is a non-empty @ref LocalizableHumanReadableMessageString;
 * otherwise, this is an empty string.
 * This parameter is @ref PassedWithoutOwnership.
 */
typedef void (*WGPUQueueWorkDoneCallback)(WGPUQueueWorkDoneStatus status, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) WGPU_FUNCTION_ATTRIBUTE;

/**
 * See also @ref CallbackError.
 *
 * @param adapter
 * This parameter is @ref PassedWithOwnership.
 *
 * @param message
 * This parameter is @ref PassedWithoutOwnership.
 */
typedef void (*WGPURequestAdapterCallback)(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) WGPU_FUNCTION_ATTRIBUTE;

/**
 * See also @ref CallbackError.
 *
 * @param device
 * This parameter is @ref PassedWithOwnership.
 *
 * @param message
 * This parameter is @ref PassedWithoutOwnership.
 */
typedef void (*WGPURequestDeviceCallback)(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) WGPU_FUNCTION_ATTRIBUTE;

/**
 * See also @ref CallbackError.
 *
 * @param device
 * This parameter is @ref PassedWithoutOwnership.
 *
 * @param message
 * This parameter is @ref PassedWithoutOwnership.
 */
typedef void (*WGPUUncapturedErrorCallback)(WGPUDevice const * device, WGPUErrorType type, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2) WGPU_FUNCTION_ATTRIBUTE;

/** @} */
/**
 * \defgroup ChainedStructures Chained Structures
 * \brief Structures used to extend descriptors.
 *
 * @{
 */

typedef struct WGPUChainedStruct {
    struct WGPUChainedStruct * next;
    WGPUSType sType;
} WGPUChainedStruct WGPU_STRUCTURE_ATTRIBUTE;

/** @} */


/**
 * \defgroup Structures Structures
 * \brief Descriptors and other transparent structures.
 *
 * @{
 */

/**
 * \defgroup CallbackInfoStructs Callback Info Structs
 * \brief Callback info structures that are used in asynchronous functions.
 *
 * @{
 */

typedef struct WGPUBufferMapCallbackInfo {
    WGPUChainedStruct * nextInChain;
    /**
     * Controls when the callback may be called.
     *
     * Has no default. The `INIT` macro sets this to (@ref WGPUCallbackMode)0.
     */
    WGPUCallbackMode mode;
    WGPUBufferMapCallback callback;
    WGPU_NULLABLE void* userdata1;
    WGPU_NULLABLE void* userdata2;
} WGPUBufferMapCallbackInfo WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUBufferMapCallbackInfo.
 */
#define WGPU_BUFFER_MAP_CALLBACK_INFO_INIT _wgpu_MAKE_INIT_STRUCT(WGPUBufferMapCallbackInfo, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.mode=*/_wgpu_ENUM_ZERO_INIT(WGPUCallbackMode) _wgpu_COMMA \
    /*.callback=*/NULL _wgpu_COMMA \
    /*.userdata1=*/NULL _wgpu_COMMA \
    /*.userdata2=*/NULL _wgpu_COMMA \
})

typedef struct WGPUCompilationInfoCallbackInfo {
    WGPUChainedStruct * nextInChain;
    /**
     * Controls when the callback may be called.
     *
     * Has no default. The `INIT` macro sets this to (@ref WGPUCallbackMode)0.
     */
    WGPUCallbackMode mode;
    WGPUCompilationInfoCallback callback;
    WGPU_NULLABLE void* userdata1;
    WGPU_NULLABLE void* userdata2;
} WGPUCompilationInfoCallbackInfo WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUCompilationInfoCallbackInfo.
 */
#define WGPU_COMPILATION_INFO_CALLBACK_INFO_INIT _wgpu_MAKE_INIT_STRUCT(WGPUCompilationInfoCallbackInfo, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.mode=*/_wgpu_ENUM_ZERO_INIT(WGPUCallbackMode) _wgpu_COMMA \
    /*.callback=*/NULL _wgpu_COMMA \
    /*.userdata1=*/NULL _wgpu_COMMA \
    /*.userdata2=*/NULL _wgpu_COMMA \
})

typedef struct WGPUCreateComputePipelineAsyncCallbackInfo {
    WGPUChainedStruct * nextInChain;
    /**
     * Controls when the callback may be called.
     *
     * Has no default. The `INIT` macro sets this to (@ref WGPUCallbackMode)0.
     */
    WGPUCallbackMode mode;
    WGPUCreateComputePipelineAsyncCallback callback;
    WGPU_NULLABLE void* userdata1;
    WGPU_NULLABLE void* userdata2;
} WGPUCreateComputePipelineAsyncCallbackInfo WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUCreateComputePipelineAsyncCallbackInfo.
 */
#define WGPU_CREATE_COMPUTE_PIPELINE_ASYNC_CALLBACK_INFO_INIT _wgpu_MAKE_INIT_STRUCT(WGPUCreateComputePipelineAsyncCallbackInfo, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.mode=*/_wgpu_ENUM_ZERO_INIT(WGPUCallbackMode) _wgpu_COMMA \
    /*.callback=*/NULL _wgpu_COMMA \
    /*.userdata1=*/NULL _wgpu_COMMA \
    /*.userdata2=*/NULL _wgpu_COMMA \
})

typedef struct WGPUCreateRenderPipelineAsyncCallbackInfo {
    WGPUChainedStruct * nextInChain;
    /**
     * Controls when the callback may be called.
     *
     * Has no default. The `INIT` macro sets this to (@ref WGPUCallbackMode)0.
     */
    WGPUCallbackMode mode;
    WGPUCreateRenderPipelineAsyncCallback callback;
    WGPU_NULLABLE void* userdata1;
    WGPU_NULLABLE void* userdata2;
} WGPUCreateRenderPipelineAsyncCallbackInfo WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUCreateRenderPipelineAsyncCallbackInfo.
 */
#define WGPU_CREATE_RENDER_PIPELINE_ASYNC_CALLBACK_INFO_INIT _wgpu_MAKE_INIT_STRUCT(WGPUCreateRenderPipelineAsyncCallbackInfo, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.mode=*/_wgpu_ENUM_ZERO_INIT(WGPUCallbackMode) _wgpu_COMMA \
    /*.callback=*/NULL _wgpu_COMMA \
    /*.userdata1=*/NULL _wgpu_COMMA \
    /*.userdata2=*/NULL _wgpu_COMMA \
})

typedef struct WGPUDeviceLostCallbackInfo {
    WGPUChainedStruct * nextInChain;
    /**
     * Controls when the callback may be called.
     *
     * Has no default. The `INIT` macro sets this to (@ref WGPUCallbackMode)0.
     */
    WGPUCallbackMode mode;
    WGPUDeviceLostCallback callback;
    WGPU_NULLABLE void* userdata1;
    WGPU_NULLABLE void* userdata2;
} WGPUDeviceLostCallbackInfo WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUDeviceLostCallbackInfo.
 */
#define WGPU_DEVICE_LOST_CALLBACK_INFO_INIT _wgpu_MAKE_INIT_STRUCT(WGPUDeviceLostCallbackInfo, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.mode=*/_wgpu_ENUM_ZERO_INIT(WGPUCallbackMode) _wgpu_COMMA \
    /*.callback=*/NULL _wgpu_COMMA \
    /*.userdata1=*/NULL _wgpu_COMMA \
    /*.userdata2=*/NULL _wgpu_COMMA \
})

typedef struct WGPUPopErrorScopeCallbackInfo {
    WGPUChainedStruct * nextInChain;
    /**
     * Controls when the callback may be called.
     *
     * Has no default. The `INIT` macro sets this to (@ref WGPUCallbackMode)0.
     */
    WGPUCallbackMode mode;
    WGPUPopErrorScopeCallback callback;
    WGPU_NULLABLE void* userdata1;
    WGPU_NULLABLE void* userdata2;
} WGPUPopErrorScopeCallbackInfo WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUPopErrorScopeCallbackInfo.
 */
#define WGPU_POP_ERROR_SCOPE_CALLBACK_INFO_INIT _wgpu_MAKE_INIT_STRUCT(WGPUPopErrorScopeCallbackInfo, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.mode=*/_wgpu_ENUM_ZERO_INIT(WGPUCallbackMode) _wgpu_COMMA \
    /*.callback=*/NULL _wgpu_COMMA \
    /*.userdata1=*/NULL _wgpu_COMMA \
    /*.userdata2=*/NULL _wgpu_COMMA \
})

typedef struct WGPUQueueWorkDoneCallbackInfo {
    WGPUChainedStruct * nextInChain;
    /**
     * Controls when the callback may be called.
     *
     * Has no default. The `INIT` macro sets this to (@ref WGPUCallbackMode)0.
     */
    WGPUCallbackMode mode;
    WGPUQueueWorkDoneCallback callback;
    WGPU_NULLABLE void* userdata1;
    WGPU_NULLABLE void* userdata2;
} WGPUQueueWorkDoneCallbackInfo WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUQueueWorkDoneCallbackInfo.
 */
#define WGPU_QUEUE_WORK_DONE_CALLBACK_INFO_INIT _wgpu_MAKE_INIT_STRUCT(WGPUQueueWorkDoneCallbackInfo, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.mode=*/_wgpu_ENUM_ZERO_INIT(WGPUCallbackMode) _wgpu_COMMA \
    /*.callback=*/NULL _wgpu_COMMA \
    /*.userdata1=*/NULL _wgpu_COMMA \
    /*.userdata2=*/NULL _wgpu_COMMA \
})

typedef struct WGPURequestAdapterCallbackInfo {
    WGPUChainedStruct * nextInChain;
    /**
     * Controls when the callback may be called.
     *
     * Has no default. The `INIT` macro sets this to (@ref WGPUCallbackMode)0.
     */
    WGPUCallbackMode mode;
    WGPURequestAdapterCallback callback;
    WGPU_NULLABLE void* userdata1;
    WGPU_NULLABLE void* userdata2;
} WGPURequestAdapterCallbackInfo WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPURequestAdapterCallbackInfo.
 */
#define WGPU_REQUEST_ADAPTER_CALLBACK_INFO_INIT _wgpu_MAKE_INIT_STRUCT(WGPURequestAdapterCallbackInfo, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.mode=*/_wgpu_ENUM_ZERO_INIT(WGPUCallbackMode) _wgpu_COMMA \
    /*.callback=*/NULL _wgpu_COMMA \
    /*.userdata1=*/NULL _wgpu_COMMA \
    /*.userdata2=*/NULL _wgpu_COMMA \
})

typedef struct WGPURequestDeviceCallbackInfo {
    WGPUChainedStruct * nextInChain;
    /**
     * Controls when the callback may be called.
     *
     * Has no default. The `INIT` macro sets this to (@ref WGPUCallbackMode)0.
     */
    WGPUCallbackMode mode;
    WGPURequestDeviceCallback callback;
    WGPU_NULLABLE void* userdata1;
    WGPU_NULLABLE void* userdata2;
} WGPURequestDeviceCallbackInfo WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPURequestDeviceCallbackInfo.
 */
#define WGPU_REQUEST_DEVICE_CALLBACK_INFO_INIT _wgpu_MAKE_INIT_STRUCT(WGPURequestDeviceCallbackInfo, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.mode=*/_wgpu_ENUM_ZERO_INIT(WGPUCallbackMode) _wgpu_COMMA \
    /*.callback=*/NULL _wgpu_COMMA \
    /*.userdata1=*/NULL _wgpu_COMMA \
    /*.userdata2=*/NULL _wgpu_COMMA \
})

typedef struct WGPUUncapturedErrorCallbackInfo {
    WGPUChainedStruct * nextInChain;
    WGPUUncapturedErrorCallback callback;
    WGPU_NULLABLE void* userdata1;
    WGPU_NULLABLE void* userdata2;
} WGPUUncapturedErrorCallbackInfo WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUUncapturedErrorCallbackInfo.
 */
#define WGPU_UNCAPTURED_ERROR_CALLBACK_INFO_INIT _wgpu_MAKE_INIT_STRUCT(WGPUUncapturedErrorCallbackInfo, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.callback=*/NULL _wgpu_COMMA \
    /*.userdata1=*/NULL _wgpu_COMMA \
    /*.userdata2=*/NULL _wgpu_COMMA \
})

/** @} */

/**
 * Default values can be set using @ref WGPU_ADAPTER_INFO_INIT as initializer.
 */
typedef struct WGPUAdapterInfo {
    WGPUChainedStruct * nextInChain;
    /**
     * This is an \ref OutputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView vendor;
    /**
     * This is an \ref OutputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView architecture;
    /**
     * This is an \ref OutputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView device;
    /**
     * This is an \ref OutputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView description;
    /**
     * The `INIT` macro sets this to @ref WGPUBackendType_Undefined.
     */
    WGPUBackendType backendType;
    /**
     * The `INIT` macro sets this to (@ref WGPUAdapterType)0.
     */
    WGPUAdapterType adapterType;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint32_t vendorID;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint32_t deviceID;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint32_t subgroupMinSize;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint32_t subgroupMaxSize;
} WGPUAdapterInfo WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUAdapterInfo.
 */
#define WGPU_ADAPTER_INFO_INIT _wgpu_MAKE_INIT_STRUCT(WGPUAdapterInfo, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.vendor=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.architecture=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.device=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.description=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.backendType=*/WGPUBackendType_Undefined _wgpu_COMMA \
    /*.adapterType=*/_wgpu_ENUM_ZERO_INIT(WGPUAdapterType) _wgpu_COMMA \
    /*.vendorID=*/0 _wgpu_COMMA \
    /*.deviceID=*/0 _wgpu_COMMA \
    /*.subgroupMinSize=*/0 _wgpu_COMMA \
    /*.subgroupMaxSize=*/0 _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_BLEND_COMPONENT_INIT as initializer.
 */
typedef struct WGPUBlendComponent {
    /**
     * If set to @ref WGPUBlendOperation_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUBlendOperation_Add.
     *
     * The `INIT` macro sets this to @ref WGPUBlendOperation_Undefined.
     */
    WGPUBlendOperation operation;
    /**
     * If set to @ref WGPUBlendFactor_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUBlendFactor_One.
     *
     * The `INIT` macro sets this to @ref WGPUBlendFactor_Undefined.
     */
    WGPUBlendFactor srcFactor;
    /**
     * If set to @ref WGPUBlendFactor_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUBlendFactor_Zero.
     *
     * The `INIT` macro sets this to @ref WGPUBlendFactor_Undefined.
     */
    WGPUBlendFactor dstFactor;
} WGPUBlendComponent WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUBlendComponent.
 */
#define WGPU_BLEND_COMPONENT_INIT _wgpu_MAKE_INIT_STRUCT(WGPUBlendComponent, { \
    /*.operation=*/WGPUBlendOperation_Undefined _wgpu_COMMA \
    /*.srcFactor=*/WGPUBlendFactor_Undefined _wgpu_COMMA \
    /*.dstFactor=*/WGPUBlendFactor_Undefined _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_BUFFER_BINDING_LAYOUT_INIT as initializer.
 */
typedef struct WGPUBufferBindingLayout {
    WGPUChainedStruct * nextInChain;
    /**
     * If set to @ref WGPUBufferBindingType_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUBufferBindingType_Uniform.
     *
     * The `INIT` macro sets this to @ref WGPUBufferBindingType_Undefined.
     */
    WGPUBufferBindingType type;
    /**
     * The `INIT` macro sets this to `WGPU_FALSE`.
     */
    WGPUBool hasDynamicOffset;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint64_t minBindingSize;
} WGPUBufferBindingLayout WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUBufferBindingLayout.
 */
#define WGPU_BUFFER_BINDING_LAYOUT_INIT _wgpu_MAKE_INIT_STRUCT(WGPUBufferBindingLayout, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.type=*/WGPUBufferBindingType_Undefined _wgpu_COMMA \
    /*.hasDynamicOffset=*/WGPU_FALSE _wgpu_COMMA \
    /*.minBindingSize=*/0 _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_BUFFER_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPUBufferDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
    /**
     * The `INIT` macro sets this to @ref WGPUBufferUsage_None.
     */
    WGPUBufferUsage usage;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint64_t size;
    /**
     * When true, the buffer is mapped in write mode at creation. It should thus be unmapped once its initial data has been written.
     *
     * @note Mapping at creation does **not** require the usage @ref WGPUBufferUsage_MapWrite.
     *
     * The `INIT` macro sets this to `WGPU_FALSE`.
     */
    WGPUBool mappedAtCreation;
} WGPUBufferDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUBufferDescriptor.
 */
#define WGPU_BUFFER_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUBufferDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.usage=*/WGPUBufferUsage_None _wgpu_COMMA \
    /*.size=*/0 _wgpu_COMMA \
    /*.mappedAtCreation=*/WGPU_FALSE _wgpu_COMMA \
})

/**
 * An RGBA color. Represents a `f32`, `i32`, or `u32` color using @ref DoubleAsSupertype.
 *
 * If any channel is non-finite, produces a @ref NonFiniteFloatValueError.
 *
 * Default values can be set using @ref WGPU_COLOR_INIT as initializer.
 */
typedef struct WGPUColor {
    /**
     * The `INIT` macro sets this to `0.`.
     */
    double r;
    /**
     * The `INIT` macro sets this to `0.`.
     */
    double g;
    /**
     * The `INIT` macro sets this to `0.`.
     */
    double b;
    /**
     * The `INIT` macro sets this to `0.`.
     */
    double a;
} WGPUColor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUColor.
 */
#define WGPU_COLOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUColor, { \
    /*.r=*/0. _wgpu_COMMA \
    /*.g=*/0. _wgpu_COMMA \
    /*.b=*/0. _wgpu_COMMA \
    /*.a=*/0. _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_COMMAND_BUFFER_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPUCommandBufferDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
} WGPUCommandBufferDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUCommandBufferDescriptor.
 */
#define WGPU_COMMAND_BUFFER_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUCommandBufferDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_COMMAND_ENCODER_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPUCommandEncoderDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
} WGPUCommandEncoderDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUCommandEncoderDescriptor.
 */
#define WGPU_COMMAND_ENCODER_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUCommandEncoderDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
})

/**
 * Note: While Compatibility Mode is optional to implement, this extension struct
 * is required to be supported (for both queries and requests) and behave as
 * defined in the WebGPU spec.
 *
 * Default values can be set using @ref WGPU_COMPATIBILITY_MODE_LIMITS_INIT as initializer.
 */
typedef struct WGPUCompatibilityModeLimits {
    WGPUChainedStruct chain;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxStorageBuffersInVertexStage;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxStorageTexturesInVertexStage;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxStorageBuffersInFragmentStage;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxStorageTexturesInFragmentStage;
} WGPUCompatibilityModeLimits WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUCompatibilityModeLimits.
 */
#define WGPU_COMPATIBILITY_MODE_LIMITS_INIT _wgpu_MAKE_INIT_STRUCT(WGPUCompatibilityModeLimits, { \
    /*.chain=*/_wgpu_MAKE_INIT_STRUCT(WGPUChainedStruct, { \
        /*.next=*/NULL _wgpu_COMMA \
        /*.sType=*/WGPUSType_CompatibilityModeLimits _wgpu_COMMA \
    }) _wgpu_COMMA \
    /*.maxStorageBuffersInVertexStage=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxStorageTexturesInVertexStage=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxStorageBuffersInFragmentStage=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxStorageTexturesInFragmentStage=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
})

/**
 * This is an @ref ImplementationAllocatedStructChain root.
 * Arbitrary chains must be handled gracefully by the application!
 *
 * Default values can be set using @ref WGPU_COMPILATION_MESSAGE_INIT as initializer.
 */
typedef struct WGPUCompilationMessage {
    WGPUChainedStruct * nextInChain;
    /**
     * A @ref LocalizableHumanReadableMessageString.
     *
     * This is an \ref OutputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView message;
    /**
     * Severity level of the message.
     *
     * The `INIT` macro sets this to (@ref WGPUCompilationMessageType)0.
     */
    WGPUCompilationMessageType type;
    /**
     * Line number where the message is attached, starting at 1.
     *
     * The `INIT` macro sets this to `0`.
     */
    uint64_t lineNum;
    /**
     * Offset in UTF-8 code units (bytes) from the beginning of the line, starting at 1.
     *
     * The `INIT` macro sets this to `0`.
     */
    uint64_t linePos;
    /**
     * Offset in UTF-8 code units (bytes) from the beginning of the shader code, starting at 0.
     *
     * The `INIT` macro sets this to `0`.
     */
    uint64_t offset;
    /**
     * Length in UTF-8 code units (bytes) of the span the message corresponds to.
     *
     * The `INIT` macro sets this to `0`.
     */
    uint64_t length;
} WGPUCompilationMessage WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUCompilationMessage.
 */
#define WGPU_COMPILATION_MESSAGE_INIT _wgpu_MAKE_INIT_STRUCT(WGPUCompilationMessage, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.message=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.type=*/_wgpu_ENUM_ZERO_INIT(WGPUCompilationMessageType) _wgpu_COMMA \
    /*.lineNum=*/0 _wgpu_COMMA \
    /*.linePos=*/0 _wgpu_COMMA \
    /*.offset=*/0 _wgpu_COMMA \
    /*.length=*/0 _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_CONSTANT_ENTRY_INIT as initializer.
 */
typedef struct WGPUConstantEntry {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView key;
    /**
     * Represents a WGSL numeric or boolean value using @ref DoubleAsSupertype.
     *
     * If non-finite, produces a @ref NonFiniteFloatValueError.
     *
     * The `INIT` macro sets this to `0.`.
     */
    double value;
} WGPUConstantEntry WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUConstantEntry.
 */
#define WGPU_CONSTANT_ENTRY_INIT _wgpu_MAKE_INIT_STRUCT(WGPUConstantEntry, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.key=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.value=*/0. _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_EXTENT_3D_INIT as initializer.
 */
typedef struct WGPUExtent3D {
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint32_t width;
    /**
     * The `INIT` macro sets this to `1`.
     */
    uint32_t height;
    /**
     * The `INIT` macro sets this to `1`.
     */
    uint32_t depthOrArrayLayers;
} WGPUExtent3D WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUExtent3D.
 */
#define WGPU_EXTENT_3D_INIT _wgpu_MAKE_INIT_STRUCT(WGPUExtent3D, { \
    /*.width=*/0 _wgpu_COMMA \
    /*.height=*/1 _wgpu_COMMA \
    /*.depthOrArrayLayers=*/1 _wgpu_COMMA \
})

/**
 * Chained in an @ref WGPUBindGroupEntry to set it to an @ref WGPUExternalTexture. This must have a corresponding @ref WGPUExternalTextureBindingLayout in the @ref WGPUBindGroupLayout.
 *
 * Default values can be set using @ref WGPU_EXTERNAL_TEXTURE_BINDING_ENTRY_INIT as initializer.
 */
typedef struct WGPUExternalTextureBindingEntry {
    WGPUChainedStruct chain;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUExternalTexture externalTexture;
} WGPUExternalTextureBindingEntry WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUExternalTextureBindingEntry.
 */
#define WGPU_EXTERNAL_TEXTURE_BINDING_ENTRY_INIT _wgpu_MAKE_INIT_STRUCT(WGPUExternalTextureBindingEntry, { \
    /*.chain=*/_wgpu_MAKE_INIT_STRUCT(WGPUChainedStruct, { \
        /*.next=*/NULL _wgpu_COMMA \
        /*.sType=*/WGPUSType_ExternalTextureBindingEntry _wgpu_COMMA \
    }) _wgpu_COMMA \
    /*.externalTexture=*/NULL _wgpu_COMMA \
})

/**
 * Chained in @ref WGPUBindGroupLayoutEntry to specify that the corresponding entries in an @ref WGPUBindGroup will contain an @ref WGPUExternalTexture.
 *
 * Default values can be set using @ref WGPU_EXTERNAL_TEXTURE_BINDING_LAYOUT_INIT as initializer.
 */
typedef struct WGPUExternalTextureBindingLayout {
    WGPUChainedStruct chain;
} WGPUExternalTextureBindingLayout WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUExternalTextureBindingLayout.
 */
#define WGPU_EXTERNAL_TEXTURE_BINDING_LAYOUT_INIT _wgpu_MAKE_INIT_STRUCT(WGPUExternalTextureBindingLayout, { \
    /*.chain=*/_wgpu_MAKE_INIT_STRUCT(WGPUChainedStruct, { \
        /*.next=*/NULL _wgpu_COMMA \
        /*.sType=*/WGPUSType_ExternalTextureBindingLayout _wgpu_COMMA \
    }) _wgpu_COMMA \
})

/**
 * Opaque handle to an asynchronous operation. See @ref Asynchronous-Operations for more information.
 *
 * Default values can be set using @ref WGPU_FUTURE_INIT as initializer.
 */
typedef struct WGPUFuture {
    /**
     * Opaque id of the @ref WGPUFuture
     *
     * The `INIT` macro sets this to `0`.
     */
    uint64_t id;
} WGPUFuture WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUFuture.
 */
#define WGPU_FUTURE_INIT _wgpu_MAKE_INIT_STRUCT(WGPUFuture, { \
    /*.id=*/0 _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_INSTANCE_LIMITS_INIT as initializer.
 */
typedef struct WGPUInstanceLimits {
    WGPUChainedStruct * nextInChain;
    /**
     * The maximum number @ref WGPUFutureWaitInfo supported in a call to ::wgpuInstanceWaitAny with `timeoutNS > 0`.
     *
     * The `INIT` macro sets this to `0`.
     */
    size_t timedWaitAnyMaxCount;
} WGPUInstanceLimits WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUInstanceLimits.
 */
#define WGPU_INSTANCE_LIMITS_INIT _wgpu_MAKE_INIT_STRUCT(WGPUInstanceLimits, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.timedWaitAnyMaxCount=*/0 _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_MULTISAMPLE_STATE_INIT as initializer.
 */
typedef struct WGPUMultisampleState {
    WGPUChainedStruct * nextInChain;
    /**
     * The `INIT` macro sets this to `1`.
     */
    uint32_t count;
    /**
     * The `INIT` macro sets this to `0xFFFFFFFF`.
     */
    uint32_t mask;
    /**
     * The `INIT` macro sets this to `WGPU_FALSE`.
     */
    WGPUBool alphaToCoverageEnabled;
} WGPUMultisampleState WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUMultisampleState.
 */
#define WGPU_MULTISAMPLE_STATE_INIT _wgpu_MAKE_INIT_STRUCT(WGPUMultisampleState, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.count=*/1 _wgpu_COMMA \
    /*.mask=*/0xFFFFFFFF _wgpu_COMMA \
    /*.alphaToCoverageEnabled=*/WGPU_FALSE _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_ORIGIN_3D_INIT as initializer.
 */
typedef struct WGPUOrigin3D {
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint32_t x;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint32_t y;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint32_t z;
} WGPUOrigin3D WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUOrigin3D.
 */
#define WGPU_ORIGIN_3D_INIT _wgpu_MAKE_INIT_STRUCT(WGPUOrigin3D, { \
    /*.x=*/0 _wgpu_COMMA \
    /*.y=*/0 _wgpu_COMMA \
    /*.z=*/0 _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_PASS_TIMESTAMP_WRITES_INIT as initializer.
 */
typedef struct WGPUPassTimestampWrites {
    WGPUChainedStruct * nextInChain;
    /**
     * Query set to write timestamps to.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUQuerySet querySet;
    /**
     * The `INIT` macro sets this to @ref WGPU_QUERY_SET_INDEX_UNDEFINED.
     */
    uint32_t beginningOfPassWriteIndex;
    /**
     * The `INIT` macro sets this to @ref WGPU_QUERY_SET_INDEX_UNDEFINED.
     */
    uint32_t endOfPassWriteIndex;
} WGPUPassTimestampWrites WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUPassTimestampWrites.
 */
#define WGPU_PASS_TIMESTAMP_WRITES_INIT _wgpu_MAKE_INIT_STRUCT(WGPUPassTimestampWrites, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.querySet=*/NULL _wgpu_COMMA \
    /*.beginningOfPassWriteIndex=*/WGPU_QUERY_SET_INDEX_UNDEFINED _wgpu_COMMA \
    /*.endOfPassWriteIndex=*/WGPU_QUERY_SET_INDEX_UNDEFINED _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPUPipelineLayoutDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
    /**
     * Array count for `bindGroupLayouts`. The `INIT` macro sets this to 0.
     */
    size_t bindGroupLayoutCount;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUBindGroupLayout const * bindGroupLayouts;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint32_t immediateSize;
} WGPUPipelineLayoutDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUPipelineLayoutDescriptor.
 */
#define WGPU_PIPELINE_LAYOUT_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUPipelineLayoutDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.bindGroupLayoutCount=*/0 _wgpu_COMMA \
    /*.bindGroupLayouts=*/NULL _wgpu_COMMA \
    /*.immediateSize=*/0 _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_PRIMITIVE_STATE_INIT as initializer.
 */
typedef struct WGPUPrimitiveState {
    WGPUChainedStruct * nextInChain;
    /**
     * If set to @ref WGPUPrimitiveTopology_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUPrimitiveTopology_TriangleList.
     *
     * The `INIT` macro sets this to @ref WGPUPrimitiveTopology_Undefined.
     */
    WGPUPrimitiveTopology topology;
    /**
     * The `INIT` macro sets this to @ref WGPUIndexFormat_Undefined.
     */
    WGPUIndexFormat stripIndexFormat;
    /**
     * If set to @ref WGPUFrontFace_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUFrontFace_CCW.
     *
     * The `INIT` macro sets this to @ref WGPUFrontFace_Undefined.
     */
    WGPUFrontFace frontFace;
    /**
     * If set to @ref WGPUCullMode_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUCullMode_None.
     *
     * The `INIT` macro sets this to @ref WGPUCullMode_Undefined.
     */
    WGPUCullMode cullMode;
    /**
     * The `INIT` macro sets this to `WGPU_FALSE`.
     */
    WGPUBool unclippedDepth;
} WGPUPrimitiveState WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUPrimitiveState.
 */
#define WGPU_PRIMITIVE_STATE_INIT _wgpu_MAKE_INIT_STRUCT(WGPUPrimitiveState, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.topology=*/WGPUPrimitiveTopology_Undefined _wgpu_COMMA \
    /*.stripIndexFormat=*/WGPUIndexFormat_Undefined _wgpu_COMMA \
    /*.frontFace=*/WGPUFrontFace_Undefined _wgpu_COMMA \
    /*.cullMode=*/WGPUCullMode_Undefined _wgpu_COMMA \
    /*.unclippedDepth=*/WGPU_FALSE _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_QUERY_SET_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPUQuerySetDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
    /**
     * The `INIT` macro sets this to (@ref WGPUQueryType)0.
     */
    WGPUQueryType type;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint32_t count;
} WGPUQuerySetDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUQuerySetDescriptor.
 */
#define WGPU_QUERY_SET_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUQuerySetDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.type=*/_wgpu_ENUM_ZERO_INIT(WGPUQueryType) _wgpu_COMMA \
    /*.count=*/0 _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_QUEUE_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPUQueueDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
} WGPUQueueDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUQueueDescriptor.
 */
#define WGPU_QUEUE_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUQueueDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_RENDER_BUNDLE_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPURenderBundleDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
} WGPURenderBundleDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPURenderBundleDescriptor.
 */
#define WGPU_RENDER_BUNDLE_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPURenderBundleDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_RENDER_BUNDLE_ENCODER_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPURenderBundleEncoderDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
    /**
     * Array count for `colorFormats`. The `INIT` macro sets this to 0.
     */
    size_t colorFormatCount;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUTextureFormat const * colorFormats;
    /**
     * The `INIT` macro sets this to @ref WGPUTextureFormat_Undefined.
     */
    WGPUTextureFormat depthStencilFormat;
    /**
     * The `INIT` macro sets this to `1`.
     */
    uint32_t sampleCount;
    /**
     * The `INIT` macro sets this to `WGPU_FALSE`.
     */
    WGPUBool depthReadOnly;
    /**
     * The `INIT` macro sets this to `WGPU_FALSE`.
     */
    WGPUBool stencilReadOnly;
} WGPURenderBundleEncoderDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPURenderBundleEncoderDescriptor.
 */
#define WGPU_RENDER_BUNDLE_ENCODER_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPURenderBundleEncoderDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.colorFormatCount=*/0 _wgpu_COMMA \
    /*.colorFormats=*/NULL _wgpu_COMMA \
    /*.depthStencilFormat=*/WGPUTextureFormat_Undefined _wgpu_COMMA \
    /*.sampleCount=*/1 _wgpu_COMMA \
    /*.depthReadOnly=*/WGPU_FALSE _wgpu_COMMA \
    /*.stencilReadOnly=*/WGPU_FALSE _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_INIT as initializer.
 */
typedef struct WGPURenderPassDepthStencilAttachment {
    WGPUChainedStruct * nextInChain;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUTextureView view;
    /**
     * The `INIT` macro sets this to @ref WGPULoadOp_Undefined.
     */
    WGPULoadOp depthLoadOp;
    /**
     * The `INIT` macro sets this to @ref WGPUStoreOp_Undefined.
     */
    WGPUStoreOp depthStoreOp;
    /**
     * This is a @ref NullableFloatingPointType.
     *
     * If `NaN`, indicates an `undefined` value (as defined by the JS spec).
     * Use @ref WGPU_DEPTH_CLEAR_VALUE_UNDEFINED to indicate this semantically.
     *
     * If infinite, produces a @ref NonFiniteFloatValueError.
     *
     * The `INIT` macro sets this to @ref WGPU_DEPTH_CLEAR_VALUE_UNDEFINED.
     */
    float depthClearValue;
    /**
     * The `INIT` macro sets this to `WGPU_FALSE`.
     */
    WGPUBool depthReadOnly;
    /**
     * The `INIT` macro sets this to @ref WGPULoadOp_Undefined.
     */
    WGPULoadOp stencilLoadOp;
    /**
     * The `INIT` macro sets this to @ref WGPUStoreOp_Undefined.
     */
    WGPUStoreOp stencilStoreOp;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint32_t stencilClearValue;
    /**
     * The `INIT` macro sets this to `WGPU_FALSE`.
     */
    WGPUBool stencilReadOnly;
} WGPURenderPassDepthStencilAttachment WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPURenderPassDepthStencilAttachment.
 */
#define WGPU_RENDER_PASS_DEPTH_STENCIL_ATTACHMENT_INIT _wgpu_MAKE_INIT_STRUCT(WGPURenderPassDepthStencilAttachment, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.view=*/NULL _wgpu_COMMA \
    /*.depthLoadOp=*/WGPULoadOp_Undefined _wgpu_COMMA \
    /*.depthStoreOp=*/WGPUStoreOp_Undefined _wgpu_COMMA \
    /*.depthClearValue=*/WGPU_DEPTH_CLEAR_VALUE_UNDEFINED _wgpu_COMMA \
    /*.depthReadOnly=*/WGPU_FALSE _wgpu_COMMA \
    /*.stencilLoadOp=*/WGPULoadOp_Undefined _wgpu_COMMA \
    /*.stencilStoreOp=*/WGPUStoreOp_Undefined _wgpu_COMMA \
    /*.stencilClearValue=*/0 _wgpu_COMMA \
    /*.stencilReadOnly=*/WGPU_FALSE _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_RENDER_PASS_MAX_DRAW_COUNT_INIT as initializer.
 */
typedef struct WGPURenderPassMaxDrawCount {
    WGPUChainedStruct chain;
    /**
     * The `INIT` macro sets this to `50000000`.
     */
    uint64_t maxDrawCount;
} WGPURenderPassMaxDrawCount WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPURenderPassMaxDrawCount.
 */
#define WGPU_RENDER_PASS_MAX_DRAW_COUNT_INIT _wgpu_MAKE_INIT_STRUCT(WGPURenderPassMaxDrawCount, { \
    /*.chain=*/_wgpu_MAKE_INIT_STRUCT(WGPUChainedStruct, { \
        /*.next=*/NULL _wgpu_COMMA \
        /*.sType=*/WGPUSType_RenderPassMaxDrawCount _wgpu_COMMA \
    }) _wgpu_COMMA \
    /*.maxDrawCount=*/50000000 _wgpu_COMMA \
})

/**
 * Extension providing requestAdapter options for implementations with WebXR interop (i.e. Wasm).
 *
 * Default values can be set using @ref WGPU_REQUEST_ADAPTER_WEBXR_OPTIONS_INIT as initializer.
 */
typedef struct WGPURequestAdapterWebXROptions {
    WGPUChainedStruct chain;
    /**
     * Sets the `xrCompatible` option in the JS API.
     *
     * The `INIT` macro sets this to `WGPU_FALSE`.
     */
    WGPUBool xrCompatible;
} WGPURequestAdapterWebXROptions WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPURequestAdapterWebXROptions.
 */
#define WGPU_REQUEST_ADAPTER_WEBXR_OPTIONS_INIT _wgpu_MAKE_INIT_STRUCT(WGPURequestAdapterWebXROptions, { \
    /*.chain=*/_wgpu_MAKE_INIT_STRUCT(WGPUChainedStruct, { \
        /*.next=*/NULL _wgpu_COMMA \
        /*.sType=*/WGPUSType_RequestAdapterWebXROptions _wgpu_COMMA \
    }) _wgpu_COMMA \
    /*.xrCompatible=*/WGPU_FALSE _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_SAMPLER_BINDING_LAYOUT_INIT as initializer.
 */
typedef struct WGPUSamplerBindingLayout {
    WGPUChainedStruct * nextInChain;
    /**
     * If set to @ref WGPUSamplerBindingType_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUSamplerBindingType_Filtering.
     *
     * The `INIT` macro sets this to @ref WGPUSamplerBindingType_Undefined.
     */
    WGPUSamplerBindingType type;
} WGPUSamplerBindingLayout WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUSamplerBindingLayout.
 */
#define WGPU_SAMPLER_BINDING_LAYOUT_INIT _wgpu_MAKE_INIT_STRUCT(WGPUSamplerBindingLayout, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.type=*/WGPUSamplerBindingType_Undefined _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_SAMPLER_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPUSamplerDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
    /**
     * If set to @ref WGPUAddressMode_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUAddressMode_ClampToEdge.
     *
     * The `INIT` macro sets this to @ref WGPUAddressMode_Undefined.
     */
    WGPUAddressMode addressModeU;
    /**
     * If set to @ref WGPUAddressMode_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUAddressMode_ClampToEdge.
     *
     * The `INIT` macro sets this to @ref WGPUAddressMode_Undefined.
     */
    WGPUAddressMode addressModeV;
    /**
     * If set to @ref WGPUAddressMode_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUAddressMode_ClampToEdge.
     *
     * The `INIT` macro sets this to @ref WGPUAddressMode_Undefined.
     */
    WGPUAddressMode addressModeW;
    /**
     * If set to @ref WGPUFilterMode_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUFilterMode_Nearest.
     *
     * The `INIT` macro sets this to @ref WGPUFilterMode_Undefined.
     */
    WGPUFilterMode magFilter;
    /**
     * If set to @ref WGPUFilterMode_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUFilterMode_Nearest.
     *
     * The `INIT` macro sets this to @ref WGPUFilterMode_Undefined.
     */
    WGPUFilterMode minFilter;
    /**
     * If set to @ref WGPUFilterMode_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUMipmapFilterMode_Nearest.
     *
     * The `INIT` macro sets this to @ref WGPUMipmapFilterMode_Undefined.
     */
    WGPUMipmapFilterMode mipmapFilter;
    /**
     * TODO
     *
     * If non-finite, produces a @ref NonFiniteFloatValueError.
     *
     * The `INIT` macro sets this to `0.f`.
     */
    float lodMinClamp;
    /**
     * TODO
     *
     * If non-finite, produces a @ref NonFiniteFloatValueError.
     *
     * The `INIT` macro sets this to `32.f`.
     */
    float lodMaxClamp;
    /**
     * The `INIT` macro sets this to @ref WGPUCompareFunction_Undefined.
     */
    WGPUCompareFunction compare;
    /**
     * The `INIT` macro sets this to `1`.
     */
    uint16_t maxAnisotropy;
} WGPUSamplerDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUSamplerDescriptor.
 */
#define WGPU_SAMPLER_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUSamplerDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.addressModeU=*/WGPUAddressMode_Undefined _wgpu_COMMA \
    /*.addressModeV=*/WGPUAddressMode_Undefined _wgpu_COMMA \
    /*.addressModeW=*/WGPUAddressMode_Undefined _wgpu_COMMA \
    /*.magFilter=*/WGPUFilterMode_Undefined _wgpu_COMMA \
    /*.minFilter=*/WGPUFilterMode_Undefined _wgpu_COMMA \
    /*.mipmapFilter=*/WGPUMipmapFilterMode_Undefined _wgpu_COMMA \
    /*.lodMinClamp=*/0.f _wgpu_COMMA \
    /*.lodMaxClamp=*/32.f _wgpu_COMMA \
    /*.compare=*/WGPUCompareFunction_Undefined _wgpu_COMMA \
    /*.maxAnisotropy=*/1 _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_SHADER_SOURCE_SPIRV_INIT as initializer.
 */
typedef struct WGPUShaderSourceSPIRV {
    WGPUChainedStruct chain;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint32_t codeSize;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    uint32_t const * code;
} WGPUShaderSourceSPIRV WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUShaderSourceSPIRV.
 */
#define WGPU_SHADER_SOURCE_SPIRV_INIT _wgpu_MAKE_INIT_STRUCT(WGPUShaderSourceSPIRV, { \
    /*.chain=*/_wgpu_MAKE_INIT_STRUCT(WGPUChainedStruct, { \
        /*.next=*/NULL _wgpu_COMMA \
        /*.sType=*/WGPUSType_ShaderSourceSPIRV _wgpu_COMMA \
    }) _wgpu_COMMA \
    /*.codeSize=*/0 _wgpu_COMMA \
    /*.code=*/NULL _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_SHADER_SOURCE_WGSL_INIT as initializer.
 */
typedef struct WGPUShaderSourceWGSL {
    WGPUChainedStruct chain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView code;
} WGPUShaderSourceWGSL WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUShaderSourceWGSL.
 */
#define WGPU_SHADER_SOURCE_WGSL_INIT _wgpu_MAKE_INIT_STRUCT(WGPUShaderSourceWGSL, { \
    /*.chain=*/_wgpu_MAKE_INIT_STRUCT(WGPUChainedStruct, { \
        /*.next=*/NULL _wgpu_COMMA \
        /*.sType=*/WGPUSType_ShaderSourceWGSL _wgpu_COMMA \
    }) _wgpu_COMMA \
    /*.code=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_STENCIL_FACE_STATE_INIT as initializer.
 */
typedef struct WGPUStencilFaceState {
    /**
     * If set to @ref WGPUCompareFunction_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUCompareFunction_Always.
     *
     * The `INIT` macro sets this to @ref WGPUCompareFunction_Undefined.
     */
    WGPUCompareFunction compare;
    /**
     * If set to @ref WGPUStencilOperation_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUStencilOperation_Keep.
     *
     * The `INIT` macro sets this to @ref WGPUStencilOperation_Undefined.
     */
    WGPUStencilOperation failOp;
    /**
     * If set to @ref WGPUStencilOperation_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUStencilOperation_Keep.
     *
     * The `INIT` macro sets this to @ref WGPUStencilOperation_Undefined.
     */
    WGPUStencilOperation depthFailOp;
    /**
     * If set to @ref WGPUStencilOperation_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUStencilOperation_Keep.
     *
     * The `INIT` macro sets this to @ref WGPUStencilOperation_Undefined.
     */
    WGPUStencilOperation passOp;
} WGPUStencilFaceState WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUStencilFaceState.
 */
#define WGPU_STENCIL_FACE_STATE_INIT _wgpu_MAKE_INIT_STRUCT(WGPUStencilFaceState, { \
    /*.compare=*/WGPUCompareFunction_Undefined _wgpu_COMMA \
    /*.failOp=*/WGPUStencilOperation_Undefined _wgpu_COMMA \
    /*.depthFailOp=*/WGPUStencilOperation_Undefined _wgpu_COMMA \
    /*.passOp=*/WGPUStencilOperation_Undefined _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_STORAGE_TEXTURE_BINDING_LAYOUT_INIT as initializer.
 */
typedef struct WGPUStorageTextureBindingLayout {
    WGPUChainedStruct * nextInChain;
    /**
     * If set to @ref WGPUStorageTextureAccess_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUStorageTextureAccess_WriteOnly.
     *
     * The `INIT` macro sets this to @ref WGPUStorageTextureAccess_Undefined.
     */
    WGPUStorageTextureAccess access;
    /**
     * The `INIT` macro sets this to @ref WGPUTextureFormat_Undefined.
     */
    WGPUTextureFormat format;
    /**
     * If set to @ref WGPUTextureViewDimension_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUTextureViewDimension_2D.
     *
     * The `INIT` macro sets this to @ref WGPUTextureViewDimension_Undefined.
     */
    WGPUTextureViewDimension viewDimension;
} WGPUStorageTextureBindingLayout WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUStorageTextureBindingLayout.
 */
#define WGPU_STORAGE_TEXTURE_BINDING_LAYOUT_INIT _wgpu_MAKE_INIT_STRUCT(WGPUStorageTextureBindingLayout, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.access=*/WGPUStorageTextureAccess_Undefined _wgpu_COMMA \
    /*.format=*/WGPUTextureFormat_Undefined _wgpu_COMMA \
    /*.viewDimension=*/WGPUTextureViewDimension_Undefined _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_SUPPORTED_FEATURES_INIT as initializer.
 */
typedef struct WGPUSupportedFeatures {
    /**
     * Array count for `features`. The `INIT` macro sets this to 0.
     */
    size_t featureCount;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUFeatureName const * features;
} WGPUSupportedFeatures WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUSupportedFeatures.
 */
#define WGPU_SUPPORTED_FEATURES_INIT _wgpu_MAKE_INIT_STRUCT(WGPUSupportedFeatures, { \
    /*.featureCount=*/0 _wgpu_COMMA \
    /*.features=*/NULL _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_SUPPORTED_INSTANCE_FEATURES_INIT as initializer.
 */
typedef struct WGPUSupportedInstanceFeatures {
    /**
     * Array count for `features`. The `INIT` macro sets this to 0.
     */
    size_t featureCount;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUInstanceFeatureName const * features;
} WGPUSupportedInstanceFeatures WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUSupportedInstanceFeatures.
 */
#define WGPU_SUPPORTED_INSTANCE_FEATURES_INIT _wgpu_MAKE_INIT_STRUCT(WGPUSupportedInstanceFeatures, { \
    /*.featureCount=*/0 _wgpu_COMMA \
    /*.features=*/NULL _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_SUPPORTED_WGSL_LANGUAGE_FEATURES_INIT as initializer.
 */
typedef struct WGPUSupportedWGSLLanguageFeatures {
    /**
     * Array count for `features`. The `INIT` macro sets this to 0.
     */
    size_t featureCount;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUWGSLLanguageFeatureName const * features;
} WGPUSupportedWGSLLanguageFeatures WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUSupportedWGSLLanguageFeatures.
 */
#define WGPU_SUPPORTED_WGSL_LANGUAGE_FEATURES_INIT _wgpu_MAKE_INIT_STRUCT(WGPUSupportedWGSLLanguageFeatures, { \
    /*.featureCount=*/0 _wgpu_COMMA \
    /*.features=*/NULL _wgpu_COMMA \
})

/**
 * Filled by @ref wgpuSurfaceGetCapabilities with what's supported for @ref wgpuSurfaceConfigure for a pair of @ref WGPUSurface and @ref WGPUAdapter.
 *
 * Default values can be set using @ref WGPU_SURFACE_CAPABILITIES_INIT as initializer.
 */
typedef struct WGPUSurfaceCapabilities {
    WGPUChainedStruct * nextInChain;
    /**
     * The bit set of supported @ref WGPUTextureUsage bits.
     * Guaranteed to contain @ref WGPUTextureUsage_RenderAttachment.
     *
     * The `INIT` macro sets this to @ref WGPUTextureUsage_None.
     */
    WGPUTextureUsage usages;
    /**
     * Array count for `formats`. The `INIT` macro sets this to 0.
     */
    size_t formatCount;
    /**
     * A list of supported @ref WGPUTextureFormat values, in order of preference.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUTextureFormat const * formats;
    /**
     * Array count for `presentModes`. The `INIT` macro sets this to 0.
     */
    size_t presentModeCount;
    /**
     * A list of supported @ref WGPUPresentMode values.
     * Guaranteed to contain @ref WGPUPresentMode_Fifo.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUPresentMode const * presentModes;
    /**
     * Array count for `alphaModes`. The `INIT` macro sets this to 0.
     */
    size_t alphaModeCount;
    /**
     * A list of supported @ref WGPUCompositeAlphaMode values.
     * @ref WGPUCompositeAlphaMode_Auto will be an alias for the first element and will never be present in this array.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUCompositeAlphaMode const * alphaModes;
} WGPUSurfaceCapabilities WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUSurfaceCapabilities.
 */
#define WGPU_SURFACE_CAPABILITIES_INIT _wgpu_MAKE_INIT_STRUCT(WGPUSurfaceCapabilities, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.usages=*/WGPUTextureUsage_None _wgpu_COMMA \
    /*.formatCount=*/0 _wgpu_COMMA \
    /*.formats=*/NULL _wgpu_COMMA \
    /*.presentModeCount=*/0 _wgpu_COMMA \
    /*.presentModes=*/NULL _wgpu_COMMA \
    /*.alphaModeCount=*/0 _wgpu_COMMA \
    /*.alphaModes=*/NULL _wgpu_COMMA \
})

/**
 * Extension of @ref WGPUSurfaceConfiguration for color spaces and HDR.
 *
 * Default values can be set using @ref WGPU_SURFACE_COLOR_MANAGEMENT_INIT as initializer.
 */
typedef struct WGPUSurfaceColorManagement {
    WGPUChainedStruct chain;
    /**
     * The `INIT` macro sets this to (@ref WGPUPredefinedColorSpace)0.
     */
    WGPUPredefinedColorSpace colorSpace;
    /**
     * The `INIT` macro sets this to (@ref WGPUToneMappingMode)0.
     */
    WGPUToneMappingMode toneMappingMode;
} WGPUSurfaceColorManagement WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUSurfaceColorManagement.
 */
#define WGPU_SURFACE_COLOR_MANAGEMENT_INIT _wgpu_MAKE_INIT_STRUCT(WGPUSurfaceColorManagement, { \
    /*.chain=*/_wgpu_MAKE_INIT_STRUCT(WGPUChainedStruct, { \
        /*.next=*/NULL _wgpu_COMMA \
        /*.sType=*/WGPUSType_SurfaceColorManagement _wgpu_COMMA \
    }) _wgpu_COMMA \
    /*.colorSpace=*/_wgpu_ENUM_ZERO_INIT(WGPUPredefinedColorSpace) _wgpu_COMMA \
    /*.toneMappingMode=*/_wgpu_ENUM_ZERO_INIT(WGPUToneMappingMode) _wgpu_COMMA \
})

/**
 * Options to @ref wgpuSurfaceConfigure for defining how a @ref WGPUSurface will be rendered to and presented to the user.
 * See @ref Surface-Configuration for more details.
 *
 * Default values can be set using @ref WGPU_SURFACE_CONFIGURATION_INIT as initializer.
 */
typedef struct WGPUSurfaceConfiguration {
    WGPUChainedStruct * nextInChain;
    /**
     * The @ref WGPUDevice to use to render to surface's textures.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUDevice device;
    /**
     * The @ref WGPUTextureFormat of the surface's textures.
     *
     * The `INIT` macro sets this to @ref WGPUTextureFormat_Undefined.
     */
    WGPUTextureFormat format;
    /**
     * The @ref WGPUTextureUsage of the surface's textures.
     *
     * The `INIT` macro sets this to @ref WGPUTextureUsage_RenderAttachment.
     */
    WGPUTextureUsage usage;
    /**
     * The width of the surface's textures.
     *
     * The `INIT` macro sets this to `0`.
     */
    uint32_t width;
    /**
     * The height of the surface's textures.
     *
     * The `INIT` macro sets this to `0`.
     */
    uint32_t height;
    /**
     * Array count for `viewFormats`. The `INIT` macro sets this to 0.
     */
    size_t viewFormatCount;
    /**
     * The additional @ref WGPUTextureFormat for @ref WGPUTextureView format reinterpretation of the surface's textures.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUTextureFormat const * viewFormats;
    /**
     * How the surface's frames will be composited on the screen.
     *
     * If set to @ref WGPUCompositeAlphaMode_Auto,
     * [defaults] to @ref WGPUCompositeAlphaMode_Inherit in native (allowing the mode
     * to be configured externally), and to @ref WGPUCompositeAlphaMode_Opaque in Wasm.
     *
     * The `INIT` macro sets this to @ref WGPUCompositeAlphaMode_Auto.
     */
    WGPUCompositeAlphaMode alphaMode;
    /**
     * When and in which order the surface's frames will be shown on the screen.
     *
     * If set to @ref WGPUPresentMode_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUPresentMode_Fifo.
     *
     * The `INIT` macro sets this to @ref WGPUPresentMode_Undefined.
     */
    WGPUPresentMode presentMode;
} WGPUSurfaceConfiguration WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUSurfaceConfiguration.
 */
#define WGPU_SURFACE_CONFIGURATION_INIT _wgpu_MAKE_INIT_STRUCT(WGPUSurfaceConfiguration, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.device=*/NULL _wgpu_COMMA \
    /*.format=*/WGPUTextureFormat_Undefined _wgpu_COMMA \
    /*.usage=*/WGPUTextureUsage_RenderAttachment _wgpu_COMMA \
    /*.width=*/0 _wgpu_COMMA \
    /*.height=*/0 _wgpu_COMMA \
    /*.viewFormatCount=*/0 _wgpu_COMMA \
    /*.viewFormats=*/NULL _wgpu_COMMA \
    /*.alphaMode=*/WGPUCompositeAlphaMode_Auto _wgpu_COMMA \
    /*.presentMode=*/WGPUPresentMode_Undefined _wgpu_COMMA \
})

/**
 * Chained in @ref WGPUSurfaceDescriptor to make an @ref WGPUSurface wrapping an Android [`ANativeWindow`](https://developer.android.com/ndk/reference/group/a-native-window).
 *
 * Default values can be set using @ref WGPU_SURFACE_SOURCE_ANDROID_NATIVE_WINDOW_INIT as initializer.
 */
typedef struct WGPUSurfaceSourceAndroidNativeWindow {
    WGPUChainedStruct chain;
    /**
     * The pointer to the [`ANativeWindow`](https://developer.android.com/ndk/reference/group/a-native-window) that will be wrapped by the @ref WGPUSurface.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    void * window;
} WGPUSurfaceSourceAndroidNativeWindow WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUSurfaceSourceAndroidNativeWindow.
 */
#define WGPU_SURFACE_SOURCE_ANDROID_NATIVE_WINDOW_INIT _wgpu_MAKE_INIT_STRUCT(WGPUSurfaceSourceAndroidNativeWindow, { \
    /*.chain=*/_wgpu_MAKE_INIT_STRUCT(WGPUChainedStruct, { \
        /*.next=*/NULL _wgpu_COMMA \
        /*.sType=*/WGPUSType_SurfaceSourceAndroidNativeWindow _wgpu_COMMA \
    }) _wgpu_COMMA \
    /*.window=*/NULL _wgpu_COMMA \
})

/**
 * Chained in @ref WGPUSurfaceDescriptor to make an @ref WGPUSurface wrapping a [`CAMetalLayer`](https://developer.apple.com/documentation/quartzcore/cametallayer?language=objc).
 *
 * Default values can be set using @ref WGPU_SURFACE_SOURCE_METAL_LAYER_INIT as initializer.
 */
typedef struct WGPUSurfaceSourceMetalLayer {
    WGPUChainedStruct chain;
    /**
     * The pointer to the [`CAMetalLayer`](https://developer.apple.com/documentation/quartzcore/cametallayer?language=objc) that will be wrapped by the @ref WGPUSurface.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    void * layer;
} WGPUSurfaceSourceMetalLayer WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUSurfaceSourceMetalLayer.
 */
#define WGPU_SURFACE_SOURCE_METAL_LAYER_INIT _wgpu_MAKE_INIT_STRUCT(WGPUSurfaceSourceMetalLayer, { \
    /*.chain=*/_wgpu_MAKE_INIT_STRUCT(WGPUChainedStruct, { \
        /*.next=*/NULL _wgpu_COMMA \
        /*.sType=*/WGPUSType_SurfaceSourceMetalLayer _wgpu_COMMA \
    }) _wgpu_COMMA \
    /*.layer=*/NULL _wgpu_COMMA \
})

/**
 * Chained in @ref WGPUSurfaceDescriptor to make an @ref WGPUSurface wrapping a [Wayland](https://wayland.freedesktop.org/) [`wl_surface`](https://wayland.freedesktop.org/docs/html/apa.html#protocol-spec-wl_surface).
 *
 * Default values can be set using @ref WGPU_SURFACE_SOURCE_WAYLAND_SURFACE_INIT as initializer.
 */
typedef struct WGPUSurfaceSourceWaylandSurface {
    WGPUChainedStruct chain;
    /**
     * A [`wl_display`](https://wayland.freedesktop.org/docs/html/apa.html#protocol-spec-wl_display) for this Wayland instance.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    void * display;
    /**
     * A [`wl_surface`](https://wayland.freedesktop.org/docs/html/apa.html#protocol-spec-wl_surface) that will be wrapped by the @ref WGPUSurface
     *
     * The `INIT` macro sets this to `NULL`.
     */
    void * surface;
} WGPUSurfaceSourceWaylandSurface WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUSurfaceSourceWaylandSurface.
 */
#define WGPU_SURFACE_SOURCE_WAYLAND_SURFACE_INIT _wgpu_MAKE_INIT_STRUCT(WGPUSurfaceSourceWaylandSurface, { \
    /*.chain=*/_wgpu_MAKE_INIT_STRUCT(WGPUChainedStruct, { \
        /*.next=*/NULL _wgpu_COMMA \
        /*.sType=*/WGPUSType_SurfaceSourceWaylandSurface _wgpu_COMMA \
    }) _wgpu_COMMA \
    /*.display=*/NULL _wgpu_COMMA \
    /*.surface=*/NULL _wgpu_COMMA \
})

/**
 * Chained in @ref WGPUSurfaceDescriptor to make an @ref WGPUSurface wrapping a Windows [`HWND`](https://learn.microsoft.com/en-us/windows/apps/develop/ui-input/retrieve-hwnd).
 *
 * Default values can be set using @ref WGPU_SURFACE_SOURCE_WINDOWS_HWND_INIT as initializer.
 */
typedef struct WGPUSurfaceSourceWindowsHWND {
    WGPUChainedStruct chain;
    /**
     * The [`HINSTANCE`](https://learn.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point) for this application.
     * Most commonly `GetModuleHandle(nullptr)`.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    void * hinstance;
    /**
     * The [`HWND`](https://learn.microsoft.com/en-us/windows/apps/develop/ui-input/retrieve-hwnd) that will be wrapped by the @ref WGPUSurface.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    void * hwnd;
} WGPUSurfaceSourceWindowsHWND WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUSurfaceSourceWindowsHWND.
 */
#define WGPU_SURFACE_SOURCE_WINDOWS_HWND_INIT _wgpu_MAKE_INIT_STRUCT(WGPUSurfaceSourceWindowsHWND, { \
    /*.chain=*/_wgpu_MAKE_INIT_STRUCT(WGPUChainedStruct, { \
        /*.next=*/NULL _wgpu_COMMA \
        /*.sType=*/WGPUSType_SurfaceSourceWindowsHWND _wgpu_COMMA \
    }) _wgpu_COMMA \
    /*.hinstance=*/NULL _wgpu_COMMA \
    /*.hwnd=*/NULL _wgpu_COMMA \
})

/**
 * Chained in @ref WGPUSurfaceDescriptor to make an @ref WGPUSurface wrapping an [XCB](https://xcb.freedesktop.org/) `xcb_window_t`.
 *
 * Default values can be set using @ref WGPU_SURFACE_SOURCE_XCB_WINDOW_INIT as initializer.
 */
typedef struct WGPUSurfaceSourceXCBWindow {
    WGPUChainedStruct chain;
    /**
     * The `xcb_connection_t` for the connection to the X server.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    void * connection;
    /**
     * The `xcb_window_t` for the window that will be wrapped by the @ref WGPUSurface.
     *
     * The `INIT` macro sets this to `0`.
     */
    uint32_t window;
} WGPUSurfaceSourceXCBWindow WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUSurfaceSourceXCBWindow.
 */
#define WGPU_SURFACE_SOURCE_XCB_WINDOW_INIT _wgpu_MAKE_INIT_STRUCT(WGPUSurfaceSourceXCBWindow, { \
    /*.chain=*/_wgpu_MAKE_INIT_STRUCT(WGPUChainedStruct, { \
        /*.next=*/NULL _wgpu_COMMA \
        /*.sType=*/WGPUSType_SurfaceSourceXCBWindow _wgpu_COMMA \
    }) _wgpu_COMMA \
    /*.connection=*/NULL _wgpu_COMMA \
    /*.window=*/0 _wgpu_COMMA \
})

/**
 * Chained in @ref WGPUSurfaceDescriptor to make an @ref WGPUSurface wrapping an [Xlib](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html) `Window`.
 *
 * Default values can be set using @ref WGPU_SURFACE_SOURCE_XLIB_WINDOW_INIT as initializer.
 */
typedef struct WGPUSurfaceSourceXlibWindow {
    WGPUChainedStruct chain;
    /**
     * A pointer to the [`Display`](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#Opening_the_Display) connected to the X server.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    void * display;
    /**
     * The [`Window`](https://www.x.org/releases/current/doc/libX11/libX11/libX11.html#Creating_Windows) that will be wrapped by the @ref WGPUSurface.
     *
     * The `INIT` macro sets this to `0`.
     */
    uint64_t window;
} WGPUSurfaceSourceXlibWindow WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUSurfaceSourceXlibWindow.
 */
#define WGPU_SURFACE_SOURCE_XLIB_WINDOW_INIT _wgpu_MAKE_INIT_STRUCT(WGPUSurfaceSourceXlibWindow, { \
    /*.chain=*/_wgpu_MAKE_INIT_STRUCT(WGPUChainedStruct, { \
        /*.next=*/NULL _wgpu_COMMA \
        /*.sType=*/WGPUSType_SurfaceSourceXlibWindow _wgpu_COMMA \
    }) _wgpu_COMMA \
    /*.display=*/NULL _wgpu_COMMA \
    /*.window=*/0 _wgpu_COMMA \
})

/**
 * Queried each frame from a @ref WGPUSurface to get a @ref WGPUTexture to render to along with some metadata.
 * See @ref Surface-Presenting for more details.
 *
 * Default values can be set using @ref WGPU_SURFACE_TEXTURE_INIT as initializer.
 */
typedef struct WGPUSurfaceTexture {
    WGPUChainedStruct * nextInChain;
    /**
     * The @ref WGPUTexture representing the frame that will be shown on the surface.
     * It is @ref ReturnedWithOwnership from @ref wgpuSurfaceGetCurrentTexture.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUTexture texture;
    /**
     * Whether the call to @ref wgpuSurfaceGetCurrentTexture succeeded and a hint as to why it might not have.
     *
     * The `INIT` macro sets this to (@ref WGPUSurfaceGetCurrentTextureStatus)0.
     */
    WGPUSurfaceGetCurrentTextureStatus status;
} WGPUSurfaceTexture WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUSurfaceTexture.
 */
#define WGPU_SURFACE_TEXTURE_INIT _wgpu_MAKE_INIT_STRUCT(WGPUSurfaceTexture, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.texture=*/NULL _wgpu_COMMA \
    /*.status=*/_wgpu_ENUM_ZERO_INIT(WGPUSurfaceGetCurrentTextureStatus) _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_TEXEL_COPY_BUFFER_LAYOUT_INIT as initializer.
 */
typedef struct WGPUTexelCopyBufferLayout {
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint64_t offset;
    /**
     * The `INIT` macro sets this to @ref WGPU_COPY_STRIDE_UNDEFINED.
     */
    uint32_t bytesPerRow;
    /**
     * The `INIT` macro sets this to @ref WGPU_COPY_STRIDE_UNDEFINED.
     */
    uint32_t rowsPerImage;
} WGPUTexelCopyBufferLayout WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUTexelCopyBufferLayout.
 */
#define WGPU_TEXEL_COPY_BUFFER_LAYOUT_INIT _wgpu_MAKE_INIT_STRUCT(WGPUTexelCopyBufferLayout, { \
    /*.offset=*/0 _wgpu_COMMA \
    /*.bytesPerRow=*/WGPU_COPY_STRIDE_UNDEFINED _wgpu_COMMA \
    /*.rowsPerImage=*/WGPU_COPY_STRIDE_UNDEFINED _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_TEXTURE_BINDING_LAYOUT_INIT as initializer.
 */
typedef struct WGPUTextureBindingLayout {
    WGPUChainedStruct * nextInChain;
    /**
     * If set to @ref WGPUTextureSampleType_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUTextureSampleType_Float.
     *
     * The `INIT` macro sets this to @ref WGPUTextureSampleType_Undefined.
     */
    WGPUTextureSampleType sampleType;
    /**
     * If set to @ref WGPUTextureViewDimension_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUTextureViewDimension_2D.
     *
     * The `INIT` macro sets this to @ref WGPUTextureViewDimension_Undefined.
     */
    WGPUTextureViewDimension viewDimension;
    /**
     * The `INIT` macro sets this to `WGPU_FALSE`.
     */
    WGPUBool multisampled;
} WGPUTextureBindingLayout WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUTextureBindingLayout.
 */
#define WGPU_TEXTURE_BINDING_LAYOUT_INIT _wgpu_MAKE_INIT_STRUCT(WGPUTextureBindingLayout, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.sampleType=*/WGPUTextureSampleType_Undefined _wgpu_COMMA \
    /*.viewDimension=*/WGPUTextureViewDimension_Undefined _wgpu_COMMA \
    /*.multisampled=*/WGPU_FALSE _wgpu_COMMA \
})

/**
 * Note: While Compatibility Mode is optional to implement, this extension struct
 * is required to be accepted (but per the WebGPU spec, its contents are ignored
 * on devices that have the @ref WGPUFeatureName_CoreFeaturesAndLimits feature).
 *
 * Default values can be set using @ref WGPU_TEXTURE_BINDING_VIEW_DIMENSION_INIT as initializer.
 */
typedef struct WGPUTextureBindingViewDimension {
    WGPUChainedStruct chain;
    /**
     * The `INIT` macro sets this to @ref WGPUTextureViewDimension_Undefined.
     */
    WGPUTextureViewDimension textureBindingViewDimension;
} WGPUTextureBindingViewDimension WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUTextureBindingViewDimension.
 */
#define WGPU_TEXTURE_BINDING_VIEW_DIMENSION_INIT _wgpu_MAKE_INIT_STRUCT(WGPUTextureBindingViewDimension, { \
    /*.chain=*/_wgpu_MAKE_INIT_STRUCT(WGPUChainedStruct, { \
        /*.next=*/NULL _wgpu_COMMA \
        /*.sType=*/WGPUSType_TextureBindingViewDimension _wgpu_COMMA \
    }) _wgpu_COMMA \
    /*.textureBindingViewDimension=*/WGPUTextureViewDimension_Undefined _wgpu_COMMA \
})

/**
 * When accessed by a shader, the red/green/blue/alpha channels are replaced
 * by the value corresponding to the component specified in r, g, b, and a,
 * respectively unlike the JS API which uses a string of length four, with
 * each character mapping to the texture view's red/green/blue/alpha channels.
 *
 * Default values can be set using @ref WGPU_TEXTURE_COMPONENT_SWIZZLE_INIT as initializer.
 */
typedef struct WGPUTextureComponentSwizzle {
    /**
     * The value that replaces the red channel in the shader.
     *
     * If set to @ref WGPUComponentSwizzle_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUComponentSwizzle_R.
     *
     * The `INIT` macro sets this to @ref WGPUComponentSwizzle_Undefined.
     */
    WGPUComponentSwizzle r;
    /**
     * The value that replaces the green channel in the shader.
     *
     * If set to @ref WGPUComponentSwizzle_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUComponentSwizzle_G.
     *
     * The `INIT` macro sets this to @ref WGPUComponentSwizzle_Undefined.
     */
    WGPUComponentSwizzle g;
    /**
     * The value that replaces the blue channel in the shader.
     *
     * If set to @ref WGPUComponentSwizzle_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUComponentSwizzle_B.
     *
     * The `INIT` macro sets this to @ref WGPUComponentSwizzle_Undefined.
     */
    WGPUComponentSwizzle b;
    /**
     * The value that replaces the alpha channel in the shader.
     *
     * If set to @ref WGPUComponentSwizzle_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUComponentSwizzle_A.
     *
     * The `INIT` macro sets this to @ref WGPUComponentSwizzle_Undefined.
     */
    WGPUComponentSwizzle a;
} WGPUTextureComponentSwizzle WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUTextureComponentSwizzle.
 */
#define WGPU_TEXTURE_COMPONENT_SWIZZLE_INIT _wgpu_MAKE_INIT_STRUCT(WGPUTextureComponentSwizzle, { \
    /*.r=*/WGPUComponentSwizzle_Undefined _wgpu_COMMA \
    /*.g=*/WGPUComponentSwizzle_Undefined _wgpu_COMMA \
    /*.b=*/WGPUComponentSwizzle_Undefined _wgpu_COMMA \
    /*.a=*/WGPUComponentSwizzle_Undefined _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_VERTEX_ATTRIBUTE_INIT as initializer.
 */
typedef struct WGPUVertexAttribute {
    WGPUChainedStruct * nextInChain;
    /**
     * The `INIT` macro sets this to (@ref WGPUVertexFormat)0.
     */
    WGPUVertexFormat format;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint64_t offset;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint32_t shaderLocation;
} WGPUVertexAttribute WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUVertexAttribute.
 */
#define WGPU_VERTEX_ATTRIBUTE_INIT _wgpu_MAKE_INIT_STRUCT(WGPUVertexAttribute, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.format=*/_wgpu_ENUM_ZERO_INIT(WGPUVertexFormat) _wgpu_COMMA \
    /*.offset=*/0 _wgpu_COMMA \
    /*.shaderLocation=*/0 _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_BIND_GROUP_ENTRY_INIT as initializer.
 */
typedef struct WGPUBindGroupEntry {
    WGPUChainedStruct * nextInChain;
    /**
     * Binding index in the bind group.
     *
     * The `INIT` macro sets this to `0`.
     */
    uint32_t binding;
    /**
     * Set this if the binding is a buffer object.
     * Otherwise must be null.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    WGPU_NULLABLE WGPUBuffer buffer;
    /**
     * If the binding is a buffer, this is the byte offset of the binding range.
     * Otherwise ignored.
     *
     * The `INIT` macro sets this to `0`.
     */
    uint64_t offset;
    /**
     * If the binding is a buffer, this is the byte size of the binding range
     * (@ref WGPU_WHOLE_SIZE means the binding ends at the end of the buffer).
     * Otherwise ignored.
     *
     * The `INIT` macro sets this to @ref WGPU_WHOLE_SIZE.
     */
    uint64_t size;
    /**
     * Set this if the binding is a sampler object.
     * Otherwise must be null.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    WGPU_NULLABLE WGPUSampler sampler;
    /**
     * Set this if the binding is a texture view object.
     * Otherwise must be null.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    WGPU_NULLABLE WGPUTextureView textureView;
} WGPUBindGroupEntry WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUBindGroupEntry.
 */
#define WGPU_BIND_GROUP_ENTRY_INIT _wgpu_MAKE_INIT_STRUCT(WGPUBindGroupEntry, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.binding=*/0 _wgpu_COMMA \
    /*.buffer=*/NULL _wgpu_COMMA \
    /*.offset=*/0 _wgpu_COMMA \
    /*.size=*/WGPU_WHOLE_SIZE _wgpu_COMMA \
    /*.sampler=*/NULL _wgpu_COMMA \
    /*.textureView=*/NULL _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT as initializer.
 */
typedef struct WGPUBindGroupLayoutEntry {
    WGPUChainedStruct * nextInChain;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint32_t binding;
    /**
     * The `INIT` macro sets this to @ref WGPUShaderStage_None.
     */
    WGPUShaderStage visibility;
    /**
     * If non-zero, this entry defines a binding array with this size.
     *
     * The `INIT` macro sets this to `0`.
     */
    uint32_t bindingArraySize;
    /**
     * The `INIT` macro sets this to zero (which sets the entry to `BindingNotUsed`).
     */
    WGPUBufferBindingLayout buffer;
    /**
     * The `INIT` macro sets this to zero (which sets the entry to `BindingNotUsed`).
     */
    WGPUSamplerBindingLayout sampler;
    /**
     * The `INIT` macro sets this to zero (which sets the entry to `BindingNotUsed`).
     */
    WGPUTextureBindingLayout texture;
    /**
     * The `INIT` macro sets this to zero (which sets the entry to `BindingNotUsed`).
     */
    WGPUStorageTextureBindingLayout storageTexture;
} WGPUBindGroupLayoutEntry WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUBindGroupLayoutEntry.
 */
#define WGPU_BIND_GROUP_LAYOUT_ENTRY_INIT _wgpu_MAKE_INIT_STRUCT(WGPUBindGroupLayoutEntry, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.binding=*/0 _wgpu_COMMA \
    /*.visibility=*/WGPUShaderStage_None _wgpu_COMMA \
    /*.bindingArraySize=*/0 _wgpu_COMMA \
    /*.buffer=*/_wgpu_STRUCT_ZERO_INIT _wgpu_COMMA \
    /*.sampler=*/_wgpu_STRUCT_ZERO_INIT _wgpu_COMMA \
    /*.texture=*/_wgpu_STRUCT_ZERO_INIT _wgpu_COMMA \
    /*.storageTexture=*/_wgpu_STRUCT_ZERO_INIT _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_BLEND_STATE_INIT as initializer.
 */
typedef struct WGPUBlendState {
    /**
     * The `INIT` macro sets this to @ref WGPU_BLEND_COMPONENT_INIT.
     */
    WGPUBlendComponent color;
    /**
     * The `INIT` macro sets this to @ref WGPU_BLEND_COMPONENT_INIT.
     */
    WGPUBlendComponent alpha;
} WGPUBlendState WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUBlendState.
 */
#define WGPU_BLEND_STATE_INIT _wgpu_MAKE_INIT_STRUCT(WGPUBlendState, { \
    /*.color=*/WGPU_BLEND_COMPONENT_INIT _wgpu_COMMA \
    /*.alpha=*/WGPU_BLEND_COMPONENT_INIT _wgpu_COMMA \
})

/**
 * This is an @ref ImplementationAllocatedStructChain root.
 * Arbitrary chains must be handled gracefully by the application!
 *
 * Default values can be set using @ref WGPU_COMPILATION_INFO_INIT as initializer.
 */
typedef struct WGPUCompilationInfo {
    WGPUChainedStruct * nextInChain;
    /**
     * Array count for `messages`. The `INIT` macro sets this to 0.
     */
    size_t messageCount;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUCompilationMessage const * messages;
} WGPUCompilationInfo WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUCompilationInfo.
 */
#define WGPU_COMPILATION_INFO_INIT _wgpu_MAKE_INIT_STRUCT(WGPUCompilationInfo, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.messageCount=*/0 _wgpu_COMMA \
    /*.messages=*/NULL _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_COMPUTE_PASS_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPUComputePassDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPU_NULLABLE WGPUPassTimestampWrites const * timestampWrites;
} WGPUComputePassDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUComputePassDescriptor.
 */
#define WGPU_COMPUTE_PASS_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUComputePassDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.timestampWrites=*/NULL _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_COMPUTE_STATE_INIT as initializer.
 */
typedef struct WGPUComputeState {
    WGPUChainedStruct * nextInChain;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUShaderModule module;
    /**
     * This is a \ref NullableInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView entryPoint;
    /**
     * Array count for `constants`. The `INIT` macro sets this to 0.
     */
    size_t constantCount;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUConstantEntry const * constants;
} WGPUComputeState WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUComputeState.
 */
#define WGPU_COMPUTE_STATE_INIT _wgpu_MAKE_INIT_STRUCT(WGPUComputeState, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.module=*/NULL _wgpu_COMMA \
    /*.entryPoint=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.constantCount=*/0 _wgpu_COMMA \
    /*.constants=*/NULL _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_DEPTH_STENCIL_STATE_INIT as initializer.
 */
typedef struct WGPUDepthStencilState {
    WGPUChainedStruct * nextInChain;
    /**
     * The `INIT` macro sets this to @ref WGPUTextureFormat_Undefined.
     */
    WGPUTextureFormat format;
    /**
     * The `INIT` macro sets this to @ref WGPUOptionalBool_Undefined.
     */
    WGPUOptionalBool depthWriteEnabled;
    /**
     * The `INIT` macro sets this to @ref WGPUCompareFunction_Undefined.
     */
    WGPUCompareFunction depthCompare;
    /**
     * The `INIT` macro sets this to @ref WGPU_STENCIL_FACE_STATE_INIT.
     */
    WGPUStencilFaceState stencilFront;
    /**
     * The `INIT` macro sets this to @ref WGPU_STENCIL_FACE_STATE_INIT.
     */
    WGPUStencilFaceState stencilBack;
    /**
     * The `INIT` macro sets this to `0xFFFFFFFF`.
     */
    uint32_t stencilReadMask;
    /**
     * The `INIT` macro sets this to `0xFFFFFFFF`.
     */
    uint32_t stencilWriteMask;
    /**
     * The `INIT` macro sets this to `0`.
     */
    int32_t depthBias;
    /**
     * TODO
     *
     * If non-finite, produces a @ref NonFiniteFloatValueError.
     *
     * The `INIT` macro sets this to `0.f`.
     */
    float depthBiasSlopeScale;
    /**
     * TODO
     *
     * If non-finite, produces a @ref NonFiniteFloatValueError.
     *
     * The `INIT` macro sets this to `0.f`.
     */
    float depthBiasClamp;
} WGPUDepthStencilState WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUDepthStencilState.
 */
#define WGPU_DEPTH_STENCIL_STATE_INIT _wgpu_MAKE_INIT_STRUCT(WGPUDepthStencilState, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.format=*/WGPUTextureFormat_Undefined _wgpu_COMMA \
    /*.depthWriteEnabled=*/WGPUOptionalBool_Undefined _wgpu_COMMA \
    /*.depthCompare=*/WGPUCompareFunction_Undefined _wgpu_COMMA \
    /*.stencilFront=*/WGPU_STENCIL_FACE_STATE_INIT _wgpu_COMMA \
    /*.stencilBack=*/WGPU_STENCIL_FACE_STATE_INIT _wgpu_COMMA \
    /*.stencilReadMask=*/0xFFFFFFFF _wgpu_COMMA \
    /*.stencilWriteMask=*/0xFFFFFFFF _wgpu_COMMA \
    /*.depthBias=*/0 _wgpu_COMMA \
    /*.depthBiasSlopeScale=*/0.f _wgpu_COMMA \
    /*.depthBiasClamp=*/0.f _wgpu_COMMA \
})

/**
 * Struct holding a future to wait on, and a `completed` boolean flag.
 *
 * Default values can be set using @ref WGPU_FUTURE_WAIT_INFO_INIT as initializer.
 */
typedef struct WGPUFutureWaitInfo {
    /**
     * The future to wait on.
     *
     * The `INIT` macro sets this to @ref WGPU_FUTURE_INIT.
     */
    WGPUFuture future;
    /**
     * Whether or not the future completed.
     *
     * The `INIT` macro sets this to `WGPU_FALSE`.
     */
    WGPUBool completed;
} WGPUFutureWaitInfo WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUFutureWaitInfo.
 */
#define WGPU_FUTURE_WAIT_INFO_INIT _wgpu_MAKE_INIT_STRUCT(WGPUFutureWaitInfo, { \
    /*.future=*/WGPU_FUTURE_INIT _wgpu_COMMA \
    /*.completed=*/WGPU_FALSE _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_INSTANCE_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPUInstanceDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * Array count for `requiredFeatures`. The `INIT` macro sets this to 0.
     */
    size_t requiredFeatureCount;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUInstanceFeatureName const * requiredFeatures;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPU_NULLABLE WGPUInstanceLimits const * requiredLimits;
} WGPUInstanceDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUInstanceDescriptor.
 */
#define WGPU_INSTANCE_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUInstanceDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.requiredFeatureCount=*/0 _wgpu_COMMA \
    /*.requiredFeatures=*/NULL _wgpu_COMMA \
    /*.requiredLimits=*/NULL _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_LIMITS_INIT as initializer.
 */
typedef struct WGPULimits {
    WGPUChainedStruct * nextInChain;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxTextureDimension1D;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxTextureDimension2D;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxTextureDimension3D;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxTextureArrayLayers;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxBindGroups;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxBindGroupsPlusVertexBuffers;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxBindingsPerBindGroup;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxDynamicUniformBuffersPerPipelineLayout;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxDynamicStorageBuffersPerPipelineLayout;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxSampledTexturesPerShaderStage;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxSamplersPerShaderStage;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxStorageBuffersPerShaderStage;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxStorageTexturesPerShaderStage;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxUniformBuffersPerShaderStage;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U64_UNDEFINED.
     */
    uint64_t maxUniformBufferBindingSize;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U64_UNDEFINED.
     */
    uint64_t maxStorageBufferBindingSize;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t minUniformBufferOffsetAlignment;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t minStorageBufferOffsetAlignment;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxVertexBuffers;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U64_UNDEFINED.
     */
    uint64_t maxBufferSize;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxVertexAttributes;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxVertexBufferArrayStride;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxInterStageShaderVariables;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxColorAttachments;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxColorAttachmentBytesPerSample;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxComputeWorkgroupStorageSize;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxComputeInvocationsPerWorkgroup;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxComputeWorkgroupSizeX;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxComputeWorkgroupSizeY;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxComputeWorkgroupSizeZ;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxComputeWorkgroupsPerDimension;
    /**
     * The `INIT` macro sets this to @ref WGPU_LIMIT_U32_UNDEFINED.
     */
    uint32_t maxImmediateSize;
} WGPULimits WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPULimits.
 */
#define WGPU_LIMITS_INIT _wgpu_MAKE_INIT_STRUCT(WGPULimits, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.maxTextureDimension1D=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxTextureDimension2D=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxTextureDimension3D=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxTextureArrayLayers=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxBindGroups=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxBindGroupsPlusVertexBuffers=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxBindingsPerBindGroup=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxDynamicUniformBuffersPerPipelineLayout=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxDynamicStorageBuffersPerPipelineLayout=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxSampledTexturesPerShaderStage=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxSamplersPerShaderStage=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxStorageBuffersPerShaderStage=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxStorageTexturesPerShaderStage=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxUniformBuffersPerShaderStage=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxUniformBufferBindingSize=*/WGPU_LIMIT_U64_UNDEFINED _wgpu_COMMA \
    /*.maxStorageBufferBindingSize=*/WGPU_LIMIT_U64_UNDEFINED _wgpu_COMMA \
    /*.minUniformBufferOffsetAlignment=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.minStorageBufferOffsetAlignment=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxVertexBuffers=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxBufferSize=*/WGPU_LIMIT_U64_UNDEFINED _wgpu_COMMA \
    /*.maxVertexAttributes=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxVertexBufferArrayStride=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxInterStageShaderVariables=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxColorAttachments=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxColorAttachmentBytesPerSample=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxComputeWorkgroupStorageSize=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxComputeInvocationsPerWorkgroup=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxComputeWorkgroupSizeX=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxComputeWorkgroupSizeY=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxComputeWorkgroupSizeZ=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxComputeWorkgroupsPerDimension=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
    /*.maxImmediateSize=*/WGPU_LIMIT_U32_UNDEFINED _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT as initializer.
 */
typedef struct WGPURenderPassColorAttachment {
    WGPUChainedStruct * nextInChain;
    /**
     * If `NULL`, indicates a hole in the parent
     * @ref WGPURenderPassDescriptor::colorAttachments array.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    WGPU_NULLABLE WGPUTextureView view;
    /**
     * The `INIT` macro sets this to @ref WGPU_DEPTH_SLICE_UNDEFINED.
     */
    uint32_t depthSlice;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPU_NULLABLE WGPUTextureView resolveTarget;
    /**
     * The `INIT` macro sets this to @ref WGPULoadOp_Undefined.
     */
    WGPULoadOp loadOp;
    /**
     * The `INIT` macro sets this to @ref WGPUStoreOp_Undefined.
     */
    WGPUStoreOp storeOp;
    /**
     * The `INIT` macro sets this to @ref WGPU_COLOR_INIT.
     */
    WGPUColor clearValue;
} WGPURenderPassColorAttachment WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPURenderPassColorAttachment.
 */
#define WGPU_RENDER_PASS_COLOR_ATTACHMENT_INIT _wgpu_MAKE_INIT_STRUCT(WGPURenderPassColorAttachment, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.view=*/NULL _wgpu_COMMA \
    /*.depthSlice=*/WGPU_DEPTH_SLICE_UNDEFINED _wgpu_COMMA \
    /*.resolveTarget=*/NULL _wgpu_COMMA \
    /*.loadOp=*/WGPULoadOp_Undefined _wgpu_COMMA \
    /*.storeOp=*/WGPUStoreOp_Undefined _wgpu_COMMA \
    /*.clearValue=*/WGPU_COLOR_INIT _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_REQUEST_ADAPTER_OPTIONS_INIT as initializer.
 */
typedef struct WGPURequestAdapterOptions {
    WGPUChainedStruct * nextInChain;
    /**
     * "Feature level" for the adapter request. If an adapter is returned, it must support the features and limits in the requested feature level.
     *
     * If set to @ref WGPUFeatureLevel_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUFeatureLevel_Core.
     * Additionally, implementations may ignore @ref WGPUFeatureLevel_Compatibility
     * and provide @ref WGPUFeatureLevel_Core instead.
     *
     * The `INIT` macro sets this to @ref WGPUFeatureLevel_Undefined.
     */
    WGPUFeatureLevel featureLevel;
    /**
     * The `INIT` macro sets this to @ref WGPUPowerPreference_Undefined.
     */
    WGPUPowerPreference powerPreference;
    /**
     * If true, requires the adapter to be a "fallback" adapter as defined by the JS spec.
     * If this is not possible, the request returns null.
     *
     * The `INIT` macro sets this to `WGPU_FALSE`.
     */
    WGPUBool forceFallbackAdapter;
    /**
     * If set, requires the adapter to have a particular backend type.
     * If this is not possible, the request returns null.
     *
     * The `INIT` macro sets this to @ref WGPUBackendType_Undefined.
     */
    WGPUBackendType backendType;
    /**
     * If set, requires the adapter to be able to output to a particular surface.
     * If this is not possible, the request returns null.
     *
     * The `INIT` macro sets this to `NULL`.
     */
    WGPU_NULLABLE WGPUSurface compatibleSurface;
} WGPURequestAdapterOptions WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPURequestAdapterOptions.
 */
#define WGPU_REQUEST_ADAPTER_OPTIONS_INIT _wgpu_MAKE_INIT_STRUCT(WGPURequestAdapterOptions, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.featureLevel=*/WGPUFeatureLevel_Undefined _wgpu_COMMA \
    /*.powerPreference=*/WGPUPowerPreference_Undefined _wgpu_COMMA \
    /*.forceFallbackAdapter=*/WGPU_FALSE _wgpu_COMMA \
    /*.backendType=*/WGPUBackendType_Undefined _wgpu_COMMA \
    /*.compatibleSurface=*/NULL _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_SHADER_MODULE_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPUShaderModuleDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
} WGPUShaderModuleDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUShaderModuleDescriptor.
 */
#define WGPU_SHADER_MODULE_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUShaderModuleDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
})

/**
 * The root descriptor for the creation of an @ref WGPUSurface with @ref wgpuInstanceCreateSurface.
 * It isn't sufficient by itself and must have one of the `WGPUSurfaceSource*` in its chain.
 * See @ref Surface-Creation for more details.
 *
 * Default values can be set using @ref WGPU_SURFACE_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPUSurfaceDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * Label used to refer to the object.
     *
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
} WGPUSurfaceDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUSurfaceDescriptor.
 */
#define WGPU_SURFACE_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUSurfaceDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_TEXEL_COPY_BUFFER_INFO_INIT as initializer.
 */
typedef struct WGPUTexelCopyBufferInfo {
    /**
     * The `INIT` macro sets this to @ref WGPU_TEXEL_COPY_BUFFER_LAYOUT_INIT.
     */
    WGPUTexelCopyBufferLayout layout;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUBuffer buffer;
} WGPUTexelCopyBufferInfo WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUTexelCopyBufferInfo.
 */
#define WGPU_TEXEL_COPY_BUFFER_INFO_INIT _wgpu_MAKE_INIT_STRUCT(WGPUTexelCopyBufferInfo, { \
    /*.layout=*/WGPU_TEXEL_COPY_BUFFER_LAYOUT_INIT _wgpu_COMMA \
    /*.buffer=*/NULL _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_TEXEL_COPY_TEXTURE_INFO_INIT as initializer.
 */
typedef struct WGPUTexelCopyTextureInfo {
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUTexture texture;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint32_t mipLevel;
    /**
     * The `INIT` macro sets this to @ref WGPU_ORIGIN_3D_INIT.
     */
    WGPUOrigin3D origin;
    /**
     * If set to @ref WGPUTextureAspect_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUTextureAspect_All.
     *
     * The `INIT` macro sets this to @ref WGPUTextureAspect_Undefined.
     */
    WGPUTextureAspect aspect;
} WGPUTexelCopyTextureInfo WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUTexelCopyTextureInfo.
 */
#define WGPU_TEXEL_COPY_TEXTURE_INFO_INIT _wgpu_MAKE_INIT_STRUCT(WGPUTexelCopyTextureInfo, { \
    /*.texture=*/NULL _wgpu_COMMA \
    /*.mipLevel=*/0 _wgpu_COMMA \
    /*.origin=*/WGPU_ORIGIN_3D_INIT _wgpu_COMMA \
    /*.aspect=*/WGPUTextureAspect_Undefined _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_TEXTURE_COMPONENT_SWIZZLE_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPUTextureComponentSwizzleDescriptor {
    WGPUChainedStruct chain;
    /**
     * The `INIT` macro sets this to @ref WGPU_TEXTURE_COMPONENT_SWIZZLE_INIT.
     */
    WGPUTextureComponentSwizzle swizzle;
} WGPUTextureComponentSwizzleDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUTextureComponentSwizzleDescriptor.
 */
#define WGPU_TEXTURE_COMPONENT_SWIZZLE_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUTextureComponentSwizzleDescriptor, { \
    /*.chain=*/_wgpu_MAKE_INIT_STRUCT(WGPUChainedStruct, { \
        /*.next=*/NULL _wgpu_COMMA \
        /*.sType=*/WGPUSType_TextureComponentSwizzleDescriptor _wgpu_COMMA \
    }) _wgpu_COMMA \
    /*.swizzle=*/WGPU_TEXTURE_COMPONENT_SWIZZLE_INIT _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_TEXTURE_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPUTextureDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
    /**
     * The `INIT` macro sets this to @ref WGPUTextureUsage_None.
     */
    WGPUTextureUsage usage;
    /**
     * If set to @ref WGPUTextureDimension_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUTextureDimension_2D.
     *
     * The `INIT` macro sets this to @ref WGPUTextureDimension_Undefined.
     */
    WGPUTextureDimension dimension;
    /**
     * The `INIT` macro sets this to @ref WGPU_EXTENT_3D_INIT.
     */
    WGPUExtent3D size;
    /**
     * The `INIT` macro sets this to @ref WGPUTextureFormat_Undefined.
     */
    WGPUTextureFormat format;
    /**
     * The `INIT` macro sets this to `1`.
     */
    uint32_t mipLevelCount;
    /**
     * The `INIT` macro sets this to `1`.
     */
    uint32_t sampleCount;
    /**
     * Array count for `viewFormats`. The `INIT` macro sets this to 0.
     */
    size_t viewFormatCount;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUTextureFormat const * viewFormats;
} WGPUTextureDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUTextureDescriptor.
 */
#define WGPU_TEXTURE_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUTextureDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.usage=*/WGPUTextureUsage_None _wgpu_COMMA \
    /*.dimension=*/WGPUTextureDimension_Undefined _wgpu_COMMA \
    /*.size=*/WGPU_EXTENT_3D_INIT _wgpu_COMMA \
    /*.format=*/WGPUTextureFormat_Undefined _wgpu_COMMA \
    /*.mipLevelCount=*/1 _wgpu_COMMA \
    /*.sampleCount=*/1 _wgpu_COMMA \
    /*.viewFormatCount=*/0 _wgpu_COMMA \
    /*.viewFormats=*/NULL _wgpu_COMMA \
})

/**
 * If `attributes` is empty *and* `stepMode` is @ref WGPUVertexStepMode_Undefined,
 * indicates a "hole" in the parent @ref WGPUVertexState `buffers` array,
 * with behavior equivalent to `null` in the JS API.
 *
 * If `attributes` is empty but `stepMode` is *not* @ref WGPUVertexStepMode_Undefined,
 * indicates a vertex buffer with no attributes, with behavior equivalent to
 * `{ attributes: [] }` in the JS API. (TODO: If the JS API changes not to
 * distinguish these cases, then this distinction doesn't matter and we can
 * remove this documentation.)
 *
 * If `stepMode` is @ref WGPUVertexStepMode_Undefined but `attributes` is *not* empty,
 * `stepMode` [defaults](@ref SentinelValues) to @ref WGPUVertexStepMode_Vertex.
 *
 * Default values can be set using @ref WGPU_VERTEX_BUFFER_LAYOUT_INIT as initializer.
 */
typedef struct WGPUVertexBufferLayout {
    WGPUChainedStruct * nextInChain;
    /**
     * The `INIT` macro sets this to @ref WGPUVertexStepMode_Undefined.
     */
    WGPUVertexStepMode stepMode;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint64_t arrayStride;
    /**
     * Array count for `attributes`. The `INIT` macro sets this to 0.
     */
    size_t attributeCount;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUVertexAttribute const * attributes;
} WGPUVertexBufferLayout WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUVertexBufferLayout.
 */
#define WGPU_VERTEX_BUFFER_LAYOUT_INIT _wgpu_MAKE_INIT_STRUCT(WGPUVertexBufferLayout, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.stepMode=*/WGPUVertexStepMode_Undefined _wgpu_COMMA \
    /*.arrayStride=*/0 _wgpu_COMMA \
    /*.attributeCount=*/0 _wgpu_COMMA \
    /*.attributes=*/NULL _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_BIND_GROUP_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPUBindGroupDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUBindGroupLayout layout;
    /**
     * Array count for `entries`. The `INIT` macro sets this to 0.
     */
    size_t entryCount;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUBindGroupEntry const * entries;
} WGPUBindGroupDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUBindGroupDescriptor.
 */
#define WGPU_BIND_GROUP_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUBindGroupDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.layout=*/NULL _wgpu_COMMA \
    /*.entryCount=*/0 _wgpu_COMMA \
    /*.entries=*/NULL _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPUBindGroupLayoutDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
    /**
     * Array count for `entries`. The `INIT` macro sets this to 0.
     */
    size_t entryCount;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUBindGroupLayoutEntry const * entries;
} WGPUBindGroupLayoutDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUBindGroupLayoutDescriptor.
 */
#define WGPU_BIND_GROUP_LAYOUT_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUBindGroupLayoutDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.entryCount=*/0 _wgpu_COMMA \
    /*.entries=*/NULL _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_COLOR_TARGET_STATE_INIT as initializer.
 */
typedef struct WGPUColorTargetState {
    WGPUChainedStruct * nextInChain;
    /**
     * The texture format of the target. If @ref WGPUTextureFormat_Undefined,
     * indicates a "hole" in the parent @ref WGPUFragmentState `targets` array:
     * the pipeline does not output a value at this `location`.
     *
     * The `INIT` macro sets this to @ref WGPUTextureFormat_Undefined.
     */
    WGPUTextureFormat format;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPU_NULLABLE WGPUBlendState const * blend;
    /**
     * The `INIT` macro sets this to @ref WGPUColorWriteMask_All.
     */
    WGPUColorWriteMask writeMask;
} WGPUColorTargetState WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUColorTargetState.
 */
#define WGPU_COLOR_TARGET_STATE_INIT _wgpu_MAKE_INIT_STRUCT(WGPUColorTargetState, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.format=*/WGPUTextureFormat_Undefined _wgpu_COMMA \
    /*.blend=*/NULL _wgpu_COMMA \
    /*.writeMask=*/WGPUColorWriteMask_All _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_COMPUTE_PIPELINE_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPUComputePipelineDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPU_NULLABLE WGPUPipelineLayout layout;
    /**
     * The `INIT` macro sets this to @ref WGPU_COMPUTE_STATE_INIT.
     */
    WGPUComputeState compute;
} WGPUComputePipelineDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUComputePipelineDescriptor.
 */
#define WGPU_COMPUTE_PIPELINE_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUComputePipelineDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.layout=*/NULL _wgpu_COMMA \
    /*.compute=*/WGPU_COMPUTE_STATE_INIT _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_DEVICE_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPUDeviceDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
    /**
     * Array count for `requiredFeatures`. The `INIT` macro sets this to 0.
     */
    size_t requiredFeatureCount;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUFeatureName const * requiredFeatures;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPU_NULLABLE WGPULimits const * requiredLimits;
    /**
     * The `INIT` macro sets this to @ref WGPU_QUEUE_DESCRIPTOR_INIT.
     */
    WGPUQueueDescriptor defaultQueue;
    /**
     * The `INIT` macro sets this to @ref WGPU_DEVICE_LOST_CALLBACK_INFO_INIT.
     */
    WGPUDeviceLostCallbackInfo deviceLostCallbackInfo;
    /**
     * Called when there is an uncaptured error on this device, from any thread.
     * See @ref ErrorScopes.
     *
     * **Important:** This callback does not have a configurable @ref WGPUCallbackMode; it may be called at any time (like @ref WGPUCallbackMode_AllowSpontaneous). As such, calls into the `webgpu.h` API from this callback are unsafe. See @ref CallbackReentrancy.
     *
     * The `INIT` macro sets this to @ref WGPU_UNCAPTURED_ERROR_CALLBACK_INFO_INIT.
     */
    WGPUUncapturedErrorCallbackInfo uncapturedErrorCallbackInfo;
} WGPUDeviceDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUDeviceDescriptor.
 */
#define WGPU_DEVICE_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUDeviceDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.requiredFeatureCount=*/0 _wgpu_COMMA \
    /*.requiredFeatures=*/NULL _wgpu_COMMA \
    /*.requiredLimits=*/NULL _wgpu_COMMA \
    /*.defaultQueue=*/WGPU_QUEUE_DESCRIPTOR_INIT _wgpu_COMMA \
    /*.deviceLostCallbackInfo=*/WGPU_DEVICE_LOST_CALLBACK_INFO_INIT _wgpu_COMMA \
    /*.uncapturedErrorCallbackInfo=*/WGPU_UNCAPTURED_ERROR_CALLBACK_INFO_INIT _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_RENDER_PASS_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPURenderPassDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
    /**
     * Array count for `colorAttachments`. The `INIT` macro sets this to 0.
     */
    size_t colorAttachmentCount;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPURenderPassColorAttachment const * colorAttachments;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPU_NULLABLE WGPURenderPassDepthStencilAttachment const * depthStencilAttachment;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPU_NULLABLE WGPUQuerySet occlusionQuerySet;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPU_NULLABLE WGPUPassTimestampWrites const * timestampWrites;
} WGPURenderPassDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPURenderPassDescriptor.
 */
#define WGPU_RENDER_PASS_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPURenderPassDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.colorAttachmentCount=*/0 _wgpu_COMMA \
    /*.colorAttachments=*/NULL _wgpu_COMMA \
    /*.depthStencilAttachment=*/NULL _wgpu_COMMA \
    /*.occlusionQuerySet=*/NULL _wgpu_COMMA \
    /*.timestampWrites=*/NULL _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPUTextureViewDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
    /**
     * The `INIT` macro sets this to @ref WGPUTextureFormat_Undefined.
     */
    WGPUTextureFormat format;
    /**
     * The `INIT` macro sets this to @ref WGPUTextureViewDimension_Undefined.
     */
    WGPUTextureViewDimension dimension;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint32_t baseMipLevel;
    /**
     * The `INIT` macro sets this to @ref WGPU_MIP_LEVEL_COUNT_UNDEFINED.
     */
    uint32_t mipLevelCount;
    /**
     * The `INIT` macro sets this to `0`.
     */
    uint32_t baseArrayLayer;
    /**
     * The `INIT` macro sets this to @ref WGPU_ARRAY_LAYER_COUNT_UNDEFINED.
     */
    uint32_t arrayLayerCount;
    /**
     * If set to @ref WGPUTextureAspect_Undefined,
     * [defaults](@ref SentinelValues) to @ref WGPUTextureAspect_All.
     *
     * The `INIT` macro sets this to @ref WGPUTextureAspect_Undefined.
     */
    WGPUTextureAspect aspect;
    /**
     * The `INIT` macro sets this to @ref WGPUTextureUsage_None.
     */
    WGPUTextureUsage usage;
} WGPUTextureViewDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUTextureViewDescriptor.
 */
#define WGPU_TEXTURE_VIEW_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPUTextureViewDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.format=*/WGPUTextureFormat_Undefined _wgpu_COMMA \
    /*.dimension=*/WGPUTextureViewDimension_Undefined _wgpu_COMMA \
    /*.baseMipLevel=*/0 _wgpu_COMMA \
    /*.mipLevelCount=*/WGPU_MIP_LEVEL_COUNT_UNDEFINED _wgpu_COMMA \
    /*.baseArrayLayer=*/0 _wgpu_COMMA \
    /*.arrayLayerCount=*/WGPU_ARRAY_LAYER_COUNT_UNDEFINED _wgpu_COMMA \
    /*.aspect=*/WGPUTextureAspect_Undefined _wgpu_COMMA \
    /*.usage=*/WGPUTextureUsage_None _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_VERTEX_STATE_INIT as initializer.
 */
typedef struct WGPUVertexState {
    WGPUChainedStruct * nextInChain;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUShaderModule module;
    /**
     * This is a \ref NullableInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView entryPoint;
    /**
     * Array count for `constants`. The `INIT` macro sets this to 0.
     */
    size_t constantCount;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUConstantEntry const * constants;
    /**
     * Array count for `buffers`. The `INIT` macro sets this to 0.
     */
    size_t bufferCount;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUVertexBufferLayout const * buffers;
} WGPUVertexState WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUVertexState.
 */
#define WGPU_VERTEX_STATE_INIT _wgpu_MAKE_INIT_STRUCT(WGPUVertexState, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.module=*/NULL _wgpu_COMMA \
    /*.entryPoint=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.constantCount=*/0 _wgpu_COMMA \
    /*.constants=*/NULL _wgpu_COMMA \
    /*.bufferCount=*/0 _wgpu_COMMA \
    /*.buffers=*/NULL _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_FRAGMENT_STATE_INIT as initializer.
 */
typedef struct WGPUFragmentState {
    WGPUChainedStruct * nextInChain;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUShaderModule module;
    /**
     * This is a \ref NullableInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView entryPoint;
    /**
     * Array count for `constants`. The `INIT` macro sets this to 0.
     */
    size_t constantCount;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUConstantEntry const * constants;
    /**
     * Array count for `targets`. The `INIT` macro sets this to 0.
     */
    size_t targetCount;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPUColorTargetState const * targets;
} WGPUFragmentState WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPUFragmentState.
 */
#define WGPU_FRAGMENT_STATE_INIT _wgpu_MAKE_INIT_STRUCT(WGPUFragmentState, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.module=*/NULL _wgpu_COMMA \
    /*.entryPoint=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.constantCount=*/0 _wgpu_COMMA \
    /*.constants=*/NULL _wgpu_COMMA \
    /*.targetCount=*/0 _wgpu_COMMA \
    /*.targets=*/NULL _wgpu_COMMA \
})

/**
 * Default values can be set using @ref WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT as initializer.
 */
typedef struct WGPURenderPipelineDescriptor {
    WGPUChainedStruct * nextInChain;
    /**
     * This is a \ref NonNullInputString.
     *
     * The `INIT` macro sets this to @ref WGPU_STRING_VIEW_INIT.
     */
    WGPUStringView label;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPU_NULLABLE WGPUPipelineLayout layout;
    /**
     * The `INIT` macro sets this to @ref WGPU_VERTEX_STATE_INIT.
     */
    WGPUVertexState vertex;
    /**
     * The `INIT` macro sets this to @ref WGPU_PRIMITIVE_STATE_INIT.
     */
    WGPUPrimitiveState primitive;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPU_NULLABLE WGPUDepthStencilState const * depthStencil;
    /**
     * The `INIT` macro sets this to @ref WGPU_MULTISAMPLE_STATE_INIT.
     */
    WGPUMultisampleState multisample;
    /**
     * The `INIT` macro sets this to `NULL`.
     */
    WGPU_NULLABLE WGPUFragmentState const * fragment;
} WGPURenderPipelineDescriptor WGPU_STRUCTURE_ATTRIBUTE;

/**
 * Initializer for @ref WGPURenderPipelineDescriptor.
 */
#define WGPU_RENDER_PIPELINE_DESCRIPTOR_INIT _wgpu_MAKE_INIT_STRUCT(WGPURenderPipelineDescriptor, { \
    /*.nextInChain=*/NULL _wgpu_COMMA \
    /*.label=*/WGPU_STRING_VIEW_INIT _wgpu_COMMA \
    /*.layout=*/NULL _wgpu_COMMA \
    /*.vertex=*/WGPU_VERTEX_STATE_INIT _wgpu_COMMA \
    /*.primitive=*/WGPU_PRIMITIVE_STATE_INIT _wgpu_COMMA \
    /*.depthStencil=*/NULL _wgpu_COMMA \
    /*.multisample=*/WGPU_MULTISAMPLE_STATE_INIT _wgpu_COMMA \
    /*.fragment=*/NULL _wgpu_COMMA \
})

/** @} */

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(WGPU_SKIP_PROCS)
// Global procs
/**
 * Proc pointer type for @ref wgpuCreateInstance:
 * > @copydoc wgpuCreateInstance
 */
typedef WGPUInstance (*WGPUProcCreateInstance)(WGPU_NULLABLE WGPUInstanceDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuGetInstanceFeatures:
 * > @copydoc wgpuGetInstanceFeatures
 */
typedef void (*WGPUProcGetInstanceFeatures)(WGPUSupportedInstanceFeatures * features) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuGetInstanceLimits:
 * > @copydoc wgpuGetInstanceLimits
 */
typedef WGPUStatus (*WGPUProcGetInstanceLimits)(WGPUInstanceLimits * limits) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuHasInstanceFeature:
 * > @copydoc wgpuHasInstanceFeature
 */
typedef WGPUBool (*WGPUProcHasInstanceFeature)(WGPUInstanceFeatureName feature) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuGetProcAddress:
 * > @copydoc wgpuGetProcAddress
 */
typedef WGPUProc (*WGPUProcGetProcAddress)(WGPUStringView procName) WGPU_FUNCTION_ATTRIBUTE;


// Procs of Adapter
/**
 * Proc pointer type for @ref wgpuAdapterGetFeatures:
 * > @copydoc wgpuAdapterGetFeatures
 */
typedef void (*WGPUProcAdapterGetFeatures)(WGPUAdapter adapter, WGPUSupportedFeatures * features) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuAdapterGetInfo:
 * > @copydoc wgpuAdapterGetInfo
 */
typedef WGPUStatus (*WGPUProcAdapterGetInfo)(WGPUAdapter adapter, WGPUAdapterInfo * info) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuAdapterGetLimits:
 * > @copydoc wgpuAdapterGetLimits
 */
typedef WGPUStatus (*WGPUProcAdapterGetLimits)(WGPUAdapter adapter, WGPULimits * limits) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuAdapterHasFeature:
 * > @copydoc wgpuAdapterHasFeature
 */
typedef WGPUBool (*WGPUProcAdapterHasFeature)(WGPUAdapter adapter, WGPUFeatureName feature) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuAdapterRequestDevice:
 * > @copydoc wgpuAdapterRequestDevice
 */
typedef WGPUFuture (*WGPUProcAdapterRequestDevice)(WGPUAdapter adapter, WGPU_NULLABLE WGPUDeviceDescriptor const * descriptor, WGPURequestDeviceCallbackInfo callbackInfo) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuAdapterAddRef:
 * > @copydoc wgpuAdapterAddRef
 */
typedef void (*WGPUProcAdapterAddRef)(WGPUAdapter adapter) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuAdapterRelease:
 * > @copydoc wgpuAdapterRelease
 */
typedef void (*WGPUProcAdapterRelease)(WGPUAdapter adapter) WGPU_FUNCTION_ATTRIBUTE;

// Procs of AdapterInfo
/**
 * Proc pointer type for @ref wgpuAdapterInfoFreeMembers:
 * > @copydoc wgpuAdapterInfoFreeMembers
 */
typedef void (*WGPUProcAdapterInfoFreeMembers)(WGPUAdapterInfo adapterInfo) WGPU_FUNCTION_ATTRIBUTE;

// Procs of BindGroup
/**
 * Proc pointer type for @ref wgpuBindGroupSetLabel:
 * > @copydoc wgpuBindGroupSetLabel
 */
typedef void (*WGPUProcBindGroupSetLabel)(WGPUBindGroup bindGroup, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuBindGroupAddRef:
 * > @copydoc wgpuBindGroupAddRef
 */
typedef void (*WGPUProcBindGroupAddRef)(WGPUBindGroup bindGroup) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuBindGroupRelease:
 * > @copydoc wgpuBindGroupRelease
 */
typedef void (*WGPUProcBindGroupRelease)(WGPUBindGroup bindGroup) WGPU_FUNCTION_ATTRIBUTE;

// Procs of BindGroupLayout
/**
 * Proc pointer type for @ref wgpuBindGroupLayoutSetLabel:
 * > @copydoc wgpuBindGroupLayoutSetLabel
 */
typedef void (*WGPUProcBindGroupLayoutSetLabel)(WGPUBindGroupLayout bindGroupLayout, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuBindGroupLayoutAddRef:
 * > @copydoc wgpuBindGroupLayoutAddRef
 */
typedef void (*WGPUProcBindGroupLayoutAddRef)(WGPUBindGroupLayout bindGroupLayout) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuBindGroupLayoutRelease:
 * > @copydoc wgpuBindGroupLayoutRelease
 */
typedef void (*WGPUProcBindGroupLayoutRelease)(WGPUBindGroupLayout bindGroupLayout) WGPU_FUNCTION_ATTRIBUTE;

// Procs of Buffer
/**
 * Proc pointer type for @ref wgpuBufferDestroy:
 * > @copydoc wgpuBufferDestroy
 */
typedef void (*WGPUProcBufferDestroy)(WGPUBuffer buffer) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuBufferGetConstMappedRange:
 * > @copydoc wgpuBufferGetConstMappedRange
 */
typedef void const * (*WGPUProcBufferGetConstMappedRange)(WGPUBuffer buffer, size_t offset, size_t size) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuBufferGetMappedRange:
 * > @copydoc wgpuBufferGetMappedRange
 */
typedef void * (*WGPUProcBufferGetMappedRange)(WGPUBuffer buffer, size_t offset, size_t size) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuBufferGetMapState:
 * > @copydoc wgpuBufferGetMapState
 */
typedef WGPUBufferMapState (*WGPUProcBufferGetMapState)(WGPUBuffer buffer) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuBufferGetSize:
 * > @copydoc wgpuBufferGetSize
 */
typedef uint64_t (*WGPUProcBufferGetSize)(WGPUBuffer buffer) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuBufferGetUsage:
 * > @copydoc wgpuBufferGetUsage
 */
typedef WGPUBufferUsage (*WGPUProcBufferGetUsage)(WGPUBuffer buffer) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuBufferMapAsync:
 * > @copydoc wgpuBufferMapAsync
 */
typedef WGPUFuture (*WGPUProcBufferMapAsync)(WGPUBuffer buffer, WGPUMapMode mode, size_t offset, size_t size, WGPUBufferMapCallbackInfo callbackInfo) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuBufferReadMappedRange:
 * > @copydoc wgpuBufferReadMappedRange
 */
typedef WGPUStatus (*WGPUProcBufferReadMappedRange)(WGPUBuffer buffer, size_t offset, void * data, size_t size) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuBufferSetLabel:
 * > @copydoc wgpuBufferSetLabel
 */
typedef void (*WGPUProcBufferSetLabel)(WGPUBuffer buffer, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuBufferUnmap:
 * > @copydoc wgpuBufferUnmap
 */
typedef void (*WGPUProcBufferUnmap)(WGPUBuffer buffer) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuBufferWriteMappedRange:
 * > @copydoc wgpuBufferWriteMappedRange
 */
typedef WGPUStatus (*WGPUProcBufferWriteMappedRange)(WGPUBuffer buffer, size_t offset, void const * data, size_t size) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuBufferAddRef:
 * > @copydoc wgpuBufferAddRef
 */
typedef void (*WGPUProcBufferAddRef)(WGPUBuffer buffer) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuBufferRelease:
 * > @copydoc wgpuBufferRelease
 */
typedef void (*WGPUProcBufferRelease)(WGPUBuffer buffer) WGPU_FUNCTION_ATTRIBUTE;

// Procs of CommandBuffer
/**
 * Proc pointer type for @ref wgpuCommandBufferSetLabel:
 * > @copydoc wgpuCommandBufferSetLabel
 */
typedef void (*WGPUProcCommandBufferSetLabel)(WGPUCommandBuffer commandBuffer, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuCommandBufferAddRef:
 * > @copydoc wgpuCommandBufferAddRef
 */
typedef void (*WGPUProcCommandBufferAddRef)(WGPUCommandBuffer commandBuffer) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuCommandBufferRelease:
 * > @copydoc wgpuCommandBufferRelease
 */
typedef void (*WGPUProcCommandBufferRelease)(WGPUCommandBuffer commandBuffer) WGPU_FUNCTION_ATTRIBUTE;

// Procs of CommandEncoder
/**
 * Proc pointer type for @ref wgpuCommandEncoderBeginComputePass:
 * > @copydoc wgpuCommandEncoderBeginComputePass
 */
typedef WGPUComputePassEncoder (*WGPUProcCommandEncoderBeginComputePass)(WGPUCommandEncoder commandEncoder, WGPU_NULLABLE WGPUComputePassDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuCommandEncoderBeginRenderPass:
 * > @copydoc wgpuCommandEncoderBeginRenderPass
 */
typedef WGPURenderPassEncoder (*WGPUProcCommandEncoderBeginRenderPass)(WGPUCommandEncoder commandEncoder, WGPURenderPassDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuCommandEncoderClearBuffer:
 * > @copydoc wgpuCommandEncoderClearBuffer
 */
typedef void (*WGPUProcCommandEncoderClearBuffer)(WGPUCommandEncoder commandEncoder, WGPUBuffer buffer, uint64_t offset, uint64_t size) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuCommandEncoderCopyBufferToBuffer:
 * > @copydoc wgpuCommandEncoderCopyBufferToBuffer
 */
typedef void (*WGPUProcCommandEncoderCopyBufferToBuffer)(WGPUCommandEncoder commandEncoder, WGPUBuffer source, uint64_t sourceOffset, WGPUBuffer destination, uint64_t destinationOffset, uint64_t size) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuCommandEncoderCopyBufferToTexture:
 * > @copydoc wgpuCommandEncoderCopyBufferToTexture
 */
typedef void (*WGPUProcCommandEncoderCopyBufferToTexture)(WGPUCommandEncoder commandEncoder, WGPUTexelCopyBufferInfo const * source, WGPUTexelCopyTextureInfo const * destination, WGPUExtent3D const * copySize) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuCommandEncoderCopyTextureToBuffer:
 * > @copydoc wgpuCommandEncoderCopyTextureToBuffer
 */
typedef void (*WGPUProcCommandEncoderCopyTextureToBuffer)(WGPUCommandEncoder commandEncoder, WGPUTexelCopyTextureInfo const * source, WGPUTexelCopyBufferInfo const * destination, WGPUExtent3D const * copySize) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuCommandEncoderCopyTextureToTexture:
 * > @copydoc wgpuCommandEncoderCopyTextureToTexture
 */
typedef void (*WGPUProcCommandEncoderCopyTextureToTexture)(WGPUCommandEncoder commandEncoder, WGPUTexelCopyTextureInfo const * source, WGPUTexelCopyTextureInfo const * destination, WGPUExtent3D const * copySize) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuCommandEncoderFinish:
 * > @copydoc wgpuCommandEncoderFinish
 */
typedef WGPUCommandBuffer (*WGPUProcCommandEncoderFinish)(WGPUCommandEncoder commandEncoder, WGPU_NULLABLE WGPUCommandBufferDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuCommandEncoderInsertDebugMarker:
 * > @copydoc wgpuCommandEncoderInsertDebugMarker
 */
typedef void (*WGPUProcCommandEncoderInsertDebugMarker)(WGPUCommandEncoder commandEncoder, WGPUStringView markerLabel) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuCommandEncoderPopDebugGroup:
 * > @copydoc wgpuCommandEncoderPopDebugGroup
 */
typedef void (*WGPUProcCommandEncoderPopDebugGroup)(WGPUCommandEncoder commandEncoder) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuCommandEncoderPushDebugGroup:
 * > @copydoc wgpuCommandEncoderPushDebugGroup
 */
typedef void (*WGPUProcCommandEncoderPushDebugGroup)(WGPUCommandEncoder commandEncoder, WGPUStringView groupLabel) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuCommandEncoderResolveQuerySet:
 * > @copydoc wgpuCommandEncoderResolveQuerySet
 */
typedef void (*WGPUProcCommandEncoderResolveQuerySet)(WGPUCommandEncoder commandEncoder, WGPUQuerySet querySet, uint32_t firstQuery, uint32_t queryCount, WGPUBuffer destination, uint64_t destinationOffset) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuCommandEncoderSetLabel:
 * > @copydoc wgpuCommandEncoderSetLabel
 */
typedef void (*WGPUProcCommandEncoderSetLabel)(WGPUCommandEncoder commandEncoder, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuCommandEncoderWriteTimestamp:
 * > @copydoc wgpuCommandEncoderWriteTimestamp
 */
typedef void (*WGPUProcCommandEncoderWriteTimestamp)(WGPUCommandEncoder commandEncoder, WGPUQuerySet querySet, uint32_t queryIndex) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuCommandEncoderAddRef:
 * > @copydoc wgpuCommandEncoderAddRef
 */
typedef void (*WGPUProcCommandEncoderAddRef)(WGPUCommandEncoder commandEncoder) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuCommandEncoderRelease:
 * > @copydoc wgpuCommandEncoderRelease
 */
typedef void (*WGPUProcCommandEncoderRelease)(WGPUCommandEncoder commandEncoder) WGPU_FUNCTION_ATTRIBUTE;

// Procs of ComputePassEncoder
/**
 * Proc pointer type for @ref wgpuComputePassEncoderDispatchWorkgroups:
 * > @copydoc wgpuComputePassEncoderDispatchWorkgroups
 */
typedef void (*WGPUProcComputePassEncoderDispatchWorkgroups)(WGPUComputePassEncoder computePassEncoder, uint32_t workgroupCountX, uint32_t workgroupCountY, uint32_t workgroupCountZ) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuComputePassEncoderDispatchWorkgroupsIndirect:
 * > @copydoc wgpuComputePassEncoderDispatchWorkgroupsIndirect
 */
typedef void (*WGPUProcComputePassEncoderDispatchWorkgroupsIndirect)(WGPUComputePassEncoder computePassEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuComputePassEncoderEnd:
 * > @copydoc wgpuComputePassEncoderEnd
 */
typedef void (*WGPUProcComputePassEncoderEnd)(WGPUComputePassEncoder computePassEncoder) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuComputePassEncoderInsertDebugMarker:
 * > @copydoc wgpuComputePassEncoderInsertDebugMarker
 */
typedef void (*WGPUProcComputePassEncoderInsertDebugMarker)(WGPUComputePassEncoder computePassEncoder, WGPUStringView markerLabel) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuComputePassEncoderPopDebugGroup:
 * > @copydoc wgpuComputePassEncoderPopDebugGroup
 */
typedef void (*WGPUProcComputePassEncoderPopDebugGroup)(WGPUComputePassEncoder computePassEncoder) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuComputePassEncoderPushDebugGroup:
 * > @copydoc wgpuComputePassEncoderPushDebugGroup
 */
typedef void (*WGPUProcComputePassEncoderPushDebugGroup)(WGPUComputePassEncoder computePassEncoder, WGPUStringView groupLabel) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuComputePassEncoderSetBindGroup:
 * > @copydoc wgpuComputePassEncoderSetBindGroup
 */
typedef void (*WGPUProcComputePassEncoderSetBindGroup)(WGPUComputePassEncoder computePassEncoder, uint32_t groupIndex, WGPU_NULLABLE WGPUBindGroup group, size_t dynamicOffsetCount, uint32_t const * dynamicOffsets) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuComputePassEncoderSetLabel:
 * > @copydoc wgpuComputePassEncoderSetLabel
 */
typedef void (*WGPUProcComputePassEncoderSetLabel)(WGPUComputePassEncoder computePassEncoder, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuComputePassEncoderSetPipeline:
 * > @copydoc wgpuComputePassEncoderSetPipeline
 */
typedef void (*WGPUProcComputePassEncoderSetPipeline)(WGPUComputePassEncoder computePassEncoder, WGPUComputePipeline pipeline) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuComputePassEncoderAddRef:
 * > @copydoc wgpuComputePassEncoderAddRef
 */
typedef void (*WGPUProcComputePassEncoderAddRef)(WGPUComputePassEncoder computePassEncoder) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuComputePassEncoderRelease:
 * > @copydoc wgpuComputePassEncoderRelease
 */
typedef void (*WGPUProcComputePassEncoderRelease)(WGPUComputePassEncoder computePassEncoder) WGPU_FUNCTION_ATTRIBUTE;

// Procs of ComputePipeline
/**
 * Proc pointer type for @ref wgpuComputePipelineGetBindGroupLayout:
 * > @copydoc wgpuComputePipelineGetBindGroupLayout
 */
typedef WGPUBindGroupLayout (*WGPUProcComputePipelineGetBindGroupLayout)(WGPUComputePipeline computePipeline, uint32_t groupIndex) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuComputePipelineSetLabel:
 * > @copydoc wgpuComputePipelineSetLabel
 */
typedef void (*WGPUProcComputePipelineSetLabel)(WGPUComputePipeline computePipeline, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuComputePipelineAddRef:
 * > @copydoc wgpuComputePipelineAddRef
 */
typedef void (*WGPUProcComputePipelineAddRef)(WGPUComputePipeline computePipeline) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuComputePipelineRelease:
 * > @copydoc wgpuComputePipelineRelease
 */
typedef void (*WGPUProcComputePipelineRelease)(WGPUComputePipeline computePipeline) WGPU_FUNCTION_ATTRIBUTE;

// Procs of Device
/**
 * Proc pointer type for @ref wgpuDeviceCreateBindGroup:
 * > @copydoc wgpuDeviceCreateBindGroup
 */
typedef WGPUBindGroup (*WGPUProcDeviceCreateBindGroup)(WGPUDevice device, WGPUBindGroupDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceCreateBindGroupLayout:
 * > @copydoc wgpuDeviceCreateBindGroupLayout
 */
typedef WGPUBindGroupLayout (*WGPUProcDeviceCreateBindGroupLayout)(WGPUDevice device, WGPUBindGroupLayoutDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceCreateBuffer:
 * > @copydoc wgpuDeviceCreateBuffer
 */
typedef WGPU_NULLABLE WGPUBuffer (*WGPUProcDeviceCreateBuffer)(WGPUDevice device, WGPUBufferDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceCreateCommandEncoder:
 * > @copydoc wgpuDeviceCreateCommandEncoder
 */
typedef WGPUCommandEncoder (*WGPUProcDeviceCreateCommandEncoder)(WGPUDevice device, WGPU_NULLABLE WGPUCommandEncoderDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceCreateComputePipeline:
 * > @copydoc wgpuDeviceCreateComputePipeline
 */
typedef WGPUComputePipeline (*WGPUProcDeviceCreateComputePipeline)(WGPUDevice device, WGPUComputePipelineDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceCreateComputePipelineAsync:
 * > @copydoc wgpuDeviceCreateComputePipelineAsync
 */
typedef WGPUFuture (*WGPUProcDeviceCreateComputePipelineAsync)(WGPUDevice device, WGPUComputePipelineDescriptor const * descriptor, WGPUCreateComputePipelineAsyncCallbackInfo callbackInfo) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceCreatePipelineLayout:
 * > @copydoc wgpuDeviceCreatePipelineLayout
 */
typedef WGPUPipelineLayout (*WGPUProcDeviceCreatePipelineLayout)(WGPUDevice device, WGPUPipelineLayoutDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceCreateQuerySet:
 * > @copydoc wgpuDeviceCreateQuerySet
 */
typedef WGPUQuerySet (*WGPUProcDeviceCreateQuerySet)(WGPUDevice device, WGPUQuerySetDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceCreateRenderBundleEncoder:
 * > @copydoc wgpuDeviceCreateRenderBundleEncoder
 */
typedef WGPURenderBundleEncoder (*WGPUProcDeviceCreateRenderBundleEncoder)(WGPUDevice device, WGPURenderBundleEncoderDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceCreateRenderPipeline:
 * > @copydoc wgpuDeviceCreateRenderPipeline
 */
typedef WGPURenderPipeline (*WGPUProcDeviceCreateRenderPipeline)(WGPUDevice device, WGPURenderPipelineDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceCreateRenderPipelineAsync:
 * > @copydoc wgpuDeviceCreateRenderPipelineAsync
 */
typedef WGPUFuture (*WGPUProcDeviceCreateRenderPipelineAsync)(WGPUDevice device, WGPURenderPipelineDescriptor const * descriptor, WGPUCreateRenderPipelineAsyncCallbackInfo callbackInfo) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceCreateSampler:
 * > @copydoc wgpuDeviceCreateSampler
 */
typedef WGPUSampler (*WGPUProcDeviceCreateSampler)(WGPUDevice device, WGPU_NULLABLE WGPUSamplerDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceCreateShaderModule:
 * > @copydoc wgpuDeviceCreateShaderModule
 */
typedef WGPUShaderModule (*WGPUProcDeviceCreateShaderModule)(WGPUDevice device, WGPUShaderModuleDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceCreateTexture:
 * > @copydoc wgpuDeviceCreateTexture
 */
typedef WGPUTexture (*WGPUProcDeviceCreateTexture)(WGPUDevice device, WGPUTextureDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceDestroy:
 * > @copydoc wgpuDeviceDestroy
 */
typedef void (*WGPUProcDeviceDestroy)(WGPUDevice device) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceGetAdapterInfo:
 * > @copydoc wgpuDeviceGetAdapterInfo
 */
typedef WGPUStatus (*WGPUProcDeviceGetAdapterInfo)(WGPUDevice device, WGPUAdapterInfo * adapterInfo) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceGetFeatures:
 * > @copydoc wgpuDeviceGetFeatures
 */
typedef void (*WGPUProcDeviceGetFeatures)(WGPUDevice device, WGPUSupportedFeatures * features) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceGetLimits:
 * > @copydoc wgpuDeviceGetLimits
 */
typedef WGPUStatus (*WGPUProcDeviceGetLimits)(WGPUDevice device, WGPULimits * limits) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceGetLostFuture:
 * > @copydoc wgpuDeviceGetLostFuture
 */
typedef WGPUFuture (*WGPUProcDeviceGetLostFuture)(WGPUDevice device) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceGetQueue:
 * > @copydoc wgpuDeviceGetQueue
 */
typedef WGPUQueue (*WGPUProcDeviceGetQueue)(WGPUDevice device) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceHasFeature:
 * > @copydoc wgpuDeviceHasFeature
 */
typedef WGPUBool (*WGPUProcDeviceHasFeature)(WGPUDevice device, WGPUFeatureName feature) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDevicePopErrorScope:
 * > @copydoc wgpuDevicePopErrorScope
 */
typedef WGPUFuture (*WGPUProcDevicePopErrorScope)(WGPUDevice device, WGPUPopErrorScopeCallbackInfo callbackInfo) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDevicePushErrorScope:
 * > @copydoc wgpuDevicePushErrorScope
 */
typedef void (*WGPUProcDevicePushErrorScope)(WGPUDevice device, WGPUErrorFilter filter) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceSetLabel:
 * > @copydoc wgpuDeviceSetLabel
 */
typedef void (*WGPUProcDeviceSetLabel)(WGPUDevice device, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceAddRef:
 * > @copydoc wgpuDeviceAddRef
 */
typedef void (*WGPUProcDeviceAddRef)(WGPUDevice device) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuDeviceRelease:
 * > @copydoc wgpuDeviceRelease
 */
typedef void (*WGPUProcDeviceRelease)(WGPUDevice device) WGPU_FUNCTION_ATTRIBUTE;

// Procs of ExternalTexture
/**
 * Proc pointer type for @ref wgpuExternalTextureSetLabel:
 * > @copydoc wgpuExternalTextureSetLabel
 */
typedef void (*WGPUProcExternalTextureSetLabel)(WGPUExternalTexture externalTexture, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuExternalTextureAddRef:
 * > @copydoc wgpuExternalTextureAddRef
 */
typedef void (*WGPUProcExternalTextureAddRef)(WGPUExternalTexture externalTexture) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuExternalTextureRelease:
 * > @copydoc wgpuExternalTextureRelease
 */
typedef void (*WGPUProcExternalTextureRelease)(WGPUExternalTexture externalTexture) WGPU_FUNCTION_ATTRIBUTE;

// Procs of Instance
/**
 * Proc pointer type for @ref wgpuInstanceCreateSurface:
 * > @copydoc wgpuInstanceCreateSurface
 */
typedef WGPUSurface (*WGPUProcInstanceCreateSurface)(WGPUInstance instance, WGPUSurfaceDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuInstanceGetWGSLLanguageFeatures:
 * > @copydoc wgpuInstanceGetWGSLLanguageFeatures
 */
typedef void (*WGPUProcInstanceGetWGSLLanguageFeatures)(WGPUInstance instance, WGPUSupportedWGSLLanguageFeatures * features) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuInstanceHasWGSLLanguageFeature:
 * > @copydoc wgpuInstanceHasWGSLLanguageFeature
 */
typedef WGPUBool (*WGPUProcInstanceHasWGSLLanguageFeature)(WGPUInstance instance, WGPUWGSLLanguageFeatureName feature) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuInstanceProcessEvents:
 * > @copydoc wgpuInstanceProcessEvents
 */
typedef void (*WGPUProcInstanceProcessEvents)(WGPUInstance instance) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuInstanceRequestAdapter:
 * > @copydoc wgpuInstanceRequestAdapter
 */
typedef WGPUFuture (*WGPUProcInstanceRequestAdapter)(WGPUInstance instance, WGPU_NULLABLE WGPURequestAdapterOptions const * options, WGPURequestAdapterCallbackInfo callbackInfo) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuInstanceWaitAny:
 * > @copydoc wgpuInstanceWaitAny
 */
typedef WGPUWaitStatus (*WGPUProcInstanceWaitAny)(WGPUInstance instance, size_t futureCount, WGPU_NULLABLE WGPUFutureWaitInfo * futures, uint64_t timeoutNS) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuInstanceAddRef:
 * > @copydoc wgpuInstanceAddRef
 */
typedef void (*WGPUProcInstanceAddRef)(WGPUInstance instance) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuInstanceRelease:
 * > @copydoc wgpuInstanceRelease
 */
typedef void (*WGPUProcInstanceRelease)(WGPUInstance instance) WGPU_FUNCTION_ATTRIBUTE;

// Procs of PipelineLayout
/**
 * Proc pointer type for @ref wgpuPipelineLayoutSetLabel:
 * > @copydoc wgpuPipelineLayoutSetLabel
 */
typedef void (*WGPUProcPipelineLayoutSetLabel)(WGPUPipelineLayout pipelineLayout, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuPipelineLayoutAddRef:
 * > @copydoc wgpuPipelineLayoutAddRef
 */
typedef void (*WGPUProcPipelineLayoutAddRef)(WGPUPipelineLayout pipelineLayout) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuPipelineLayoutRelease:
 * > @copydoc wgpuPipelineLayoutRelease
 */
typedef void (*WGPUProcPipelineLayoutRelease)(WGPUPipelineLayout pipelineLayout) WGPU_FUNCTION_ATTRIBUTE;

// Procs of QuerySet
/**
 * Proc pointer type for @ref wgpuQuerySetDestroy:
 * > @copydoc wgpuQuerySetDestroy
 */
typedef void (*WGPUProcQuerySetDestroy)(WGPUQuerySet querySet) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuQuerySetGetCount:
 * > @copydoc wgpuQuerySetGetCount
 */
typedef uint32_t (*WGPUProcQuerySetGetCount)(WGPUQuerySet querySet) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuQuerySetGetType:
 * > @copydoc wgpuQuerySetGetType
 */
typedef WGPUQueryType (*WGPUProcQuerySetGetType)(WGPUQuerySet querySet) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuQuerySetSetLabel:
 * > @copydoc wgpuQuerySetSetLabel
 */
typedef void (*WGPUProcQuerySetSetLabel)(WGPUQuerySet querySet, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuQuerySetAddRef:
 * > @copydoc wgpuQuerySetAddRef
 */
typedef void (*WGPUProcQuerySetAddRef)(WGPUQuerySet querySet) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuQuerySetRelease:
 * > @copydoc wgpuQuerySetRelease
 */
typedef void (*WGPUProcQuerySetRelease)(WGPUQuerySet querySet) WGPU_FUNCTION_ATTRIBUTE;

// Procs of Queue
/**
 * Proc pointer type for @ref wgpuQueueOnSubmittedWorkDone:
 * > @copydoc wgpuQueueOnSubmittedWorkDone
 */
typedef WGPUFuture (*WGPUProcQueueOnSubmittedWorkDone)(WGPUQueue queue, WGPUQueueWorkDoneCallbackInfo callbackInfo) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuQueueSetLabel:
 * > @copydoc wgpuQueueSetLabel
 */
typedef void (*WGPUProcQueueSetLabel)(WGPUQueue queue, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuQueueSubmit:
 * > @copydoc wgpuQueueSubmit
 */
typedef void (*WGPUProcQueueSubmit)(WGPUQueue queue, size_t commandCount, WGPUCommandBuffer const * commands) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuQueueWriteBuffer:
 * > @copydoc wgpuQueueWriteBuffer
 */
typedef void (*WGPUProcQueueWriteBuffer)(WGPUQueue queue, WGPUBuffer buffer, uint64_t bufferOffset, void const * data, size_t size) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuQueueWriteTexture:
 * > @copydoc wgpuQueueWriteTexture
 */
typedef void (*WGPUProcQueueWriteTexture)(WGPUQueue queue, WGPUTexelCopyTextureInfo const * destination, void const * data, size_t dataSize, WGPUTexelCopyBufferLayout const * dataLayout, WGPUExtent3D const * writeSize) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuQueueAddRef:
 * > @copydoc wgpuQueueAddRef
 */
typedef void (*WGPUProcQueueAddRef)(WGPUQueue queue) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuQueueRelease:
 * > @copydoc wgpuQueueRelease
 */
typedef void (*WGPUProcQueueRelease)(WGPUQueue queue) WGPU_FUNCTION_ATTRIBUTE;

// Procs of RenderBundle
/**
 * Proc pointer type for @ref wgpuRenderBundleSetLabel:
 * > @copydoc wgpuRenderBundleSetLabel
 */
typedef void (*WGPUProcRenderBundleSetLabel)(WGPURenderBundle renderBundle, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderBundleAddRef:
 * > @copydoc wgpuRenderBundleAddRef
 */
typedef void (*WGPUProcRenderBundleAddRef)(WGPURenderBundle renderBundle) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderBundleRelease:
 * > @copydoc wgpuRenderBundleRelease
 */
typedef void (*WGPUProcRenderBundleRelease)(WGPURenderBundle renderBundle) WGPU_FUNCTION_ATTRIBUTE;

// Procs of RenderBundleEncoder
/**
 * Proc pointer type for @ref wgpuRenderBundleEncoderDraw:
 * > @copydoc wgpuRenderBundleEncoderDraw
 */
typedef void (*WGPUProcRenderBundleEncoderDraw)(WGPURenderBundleEncoder renderBundleEncoder, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderBundleEncoderDrawIndexed:
 * > @copydoc wgpuRenderBundleEncoderDrawIndexed
 */
typedef void (*WGPUProcRenderBundleEncoderDrawIndexed)(WGPURenderBundleEncoder renderBundleEncoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderBundleEncoderDrawIndexedIndirect:
 * > @copydoc wgpuRenderBundleEncoderDrawIndexedIndirect
 */
typedef void (*WGPUProcRenderBundleEncoderDrawIndexedIndirect)(WGPURenderBundleEncoder renderBundleEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderBundleEncoderDrawIndirect:
 * > @copydoc wgpuRenderBundleEncoderDrawIndirect
 */
typedef void (*WGPUProcRenderBundleEncoderDrawIndirect)(WGPURenderBundleEncoder renderBundleEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderBundleEncoderFinish:
 * > @copydoc wgpuRenderBundleEncoderFinish
 */
typedef WGPURenderBundle (*WGPUProcRenderBundleEncoderFinish)(WGPURenderBundleEncoder renderBundleEncoder, WGPU_NULLABLE WGPURenderBundleDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderBundleEncoderInsertDebugMarker:
 * > @copydoc wgpuRenderBundleEncoderInsertDebugMarker
 */
typedef void (*WGPUProcRenderBundleEncoderInsertDebugMarker)(WGPURenderBundleEncoder renderBundleEncoder, WGPUStringView markerLabel) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderBundleEncoderPopDebugGroup:
 * > @copydoc wgpuRenderBundleEncoderPopDebugGroup
 */
typedef void (*WGPUProcRenderBundleEncoderPopDebugGroup)(WGPURenderBundleEncoder renderBundleEncoder) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderBundleEncoderPushDebugGroup:
 * > @copydoc wgpuRenderBundleEncoderPushDebugGroup
 */
typedef void (*WGPUProcRenderBundleEncoderPushDebugGroup)(WGPURenderBundleEncoder renderBundleEncoder, WGPUStringView groupLabel) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderBundleEncoderSetBindGroup:
 * > @copydoc wgpuRenderBundleEncoderSetBindGroup
 */
typedef void (*WGPUProcRenderBundleEncoderSetBindGroup)(WGPURenderBundleEncoder renderBundleEncoder, uint32_t groupIndex, WGPU_NULLABLE WGPUBindGroup group, size_t dynamicOffsetCount, uint32_t const * dynamicOffsets) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderBundleEncoderSetIndexBuffer:
 * > @copydoc wgpuRenderBundleEncoderSetIndexBuffer
 */
typedef void (*WGPUProcRenderBundleEncoderSetIndexBuffer)(WGPURenderBundleEncoder renderBundleEncoder, WGPUBuffer buffer, WGPUIndexFormat format, uint64_t offset, uint64_t size) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderBundleEncoderSetLabel:
 * > @copydoc wgpuRenderBundleEncoderSetLabel
 */
typedef void (*WGPUProcRenderBundleEncoderSetLabel)(WGPURenderBundleEncoder renderBundleEncoder, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderBundleEncoderSetPipeline:
 * > @copydoc wgpuRenderBundleEncoderSetPipeline
 */
typedef void (*WGPUProcRenderBundleEncoderSetPipeline)(WGPURenderBundleEncoder renderBundleEncoder, WGPURenderPipeline pipeline) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderBundleEncoderSetVertexBuffer:
 * > @copydoc wgpuRenderBundleEncoderSetVertexBuffer
 */
typedef void (*WGPUProcRenderBundleEncoderSetVertexBuffer)(WGPURenderBundleEncoder renderBundleEncoder, uint32_t slot, WGPU_NULLABLE WGPUBuffer buffer, uint64_t offset, uint64_t size) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderBundleEncoderAddRef:
 * > @copydoc wgpuRenderBundleEncoderAddRef
 */
typedef void (*WGPUProcRenderBundleEncoderAddRef)(WGPURenderBundleEncoder renderBundleEncoder) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderBundleEncoderRelease:
 * > @copydoc wgpuRenderBundleEncoderRelease
 */
typedef void (*WGPUProcRenderBundleEncoderRelease)(WGPURenderBundleEncoder renderBundleEncoder) WGPU_FUNCTION_ATTRIBUTE;

// Procs of RenderPassEncoder
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderBeginOcclusionQuery:
 * > @copydoc wgpuRenderPassEncoderBeginOcclusionQuery
 */
typedef void (*WGPUProcRenderPassEncoderBeginOcclusionQuery)(WGPURenderPassEncoder renderPassEncoder, uint32_t queryIndex) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderDraw:
 * > @copydoc wgpuRenderPassEncoderDraw
 */
typedef void (*WGPUProcRenderPassEncoderDraw)(WGPURenderPassEncoder renderPassEncoder, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderDrawIndexed:
 * > @copydoc wgpuRenderPassEncoderDrawIndexed
 */
typedef void (*WGPUProcRenderPassEncoderDrawIndexed)(WGPURenderPassEncoder renderPassEncoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderDrawIndexedIndirect:
 * > @copydoc wgpuRenderPassEncoderDrawIndexedIndirect
 */
typedef void (*WGPUProcRenderPassEncoderDrawIndexedIndirect)(WGPURenderPassEncoder renderPassEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderDrawIndirect:
 * > @copydoc wgpuRenderPassEncoderDrawIndirect
 */
typedef void (*WGPUProcRenderPassEncoderDrawIndirect)(WGPURenderPassEncoder renderPassEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderEnd:
 * > @copydoc wgpuRenderPassEncoderEnd
 */
typedef void (*WGPUProcRenderPassEncoderEnd)(WGPURenderPassEncoder renderPassEncoder) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderEndOcclusionQuery:
 * > @copydoc wgpuRenderPassEncoderEndOcclusionQuery
 */
typedef void (*WGPUProcRenderPassEncoderEndOcclusionQuery)(WGPURenderPassEncoder renderPassEncoder) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderExecuteBundles:
 * > @copydoc wgpuRenderPassEncoderExecuteBundles
 */
typedef void (*WGPUProcRenderPassEncoderExecuteBundles)(WGPURenderPassEncoder renderPassEncoder, size_t bundleCount, WGPURenderBundle const * bundles) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderInsertDebugMarker:
 * > @copydoc wgpuRenderPassEncoderInsertDebugMarker
 */
typedef void (*WGPUProcRenderPassEncoderInsertDebugMarker)(WGPURenderPassEncoder renderPassEncoder, WGPUStringView markerLabel) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderPopDebugGroup:
 * > @copydoc wgpuRenderPassEncoderPopDebugGroup
 */
typedef void (*WGPUProcRenderPassEncoderPopDebugGroup)(WGPURenderPassEncoder renderPassEncoder) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderPushDebugGroup:
 * > @copydoc wgpuRenderPassEncoderPushDebugGroup
 */
typedef void (*WGPUProcRenderPassEncoderPushDebugGroup)(WGPURenderPassEncoder renderPassEncoder, WGPUStringView groupLabel) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderSetBindGroup:
 * > @copydoc wgpuRenderPassEncoderSetBindGroup
 */
typedef void (*WGPUProcRenderPassEncoderSetBindGroup)(WGPURenderPassEncoder renderPassEncoder, uint32_t groupIndex, WGPU_NULLABLE WGPUBindGroup group, size_t dynamicOffsetCount, uint32_t const * dynamicOffsets) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderSetBlendConstant:
 * > @copydoc wgpuRenderPassEncoderSetBlendConstant
 */
typedef void (*WGPUProcRenderPassEncoderSetBlendConstant)(WGPURenderPassEncoder renderPassEncoder, WGPUColor const * color) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderSetIndexBuffer:
 * > @copydoc wgpuRenderPassEncoderSetIndexBuffer
 */
typedef void (*WGPUProcRenderPassEncoderSetIndexBuffer)(WGPURenderPassEncoder renderPassEncoder, WGPUBuffer buffer, WGPUIndexFormat format, uint64_t offset, uint64_t size) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderSetLabel:
 * > @copydoc wgpuRenderPassEncoderSetLabel
 */
typedef void (*WGPUProcRenderPassEncoderSetLabel)(WGPURenderPassEncoder renderPassEncoder, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderSetPipeline:
 * > @copydoc wgpuRenderPassEncoderSetPipeline
 */
typedef void (*WGPUProcRenderPassEncoderSetPipeline)(WGPURenderPassEncoder renderPassEncoder, WGPURenderPipeline pipeline) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderSetScissorRect:
 * > @copydoc wgpuRenderPassEncoderSetScissorRect
 */
typedef void (*WGPUProcRenderPassEncoderSetScissorRect)(WGPURenderPassEncoder renderPassEncoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderSetStencilReference:
 * > @copydoc wgpuRenderPassEncoderSetStencilReference
 */
typedef void (*WGPUProcRenderPassEncoderSetStencilReference)(WGPURenderPassEncoder renderPassEncoder, uint32_t reference) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderSetVertexBuffer:
 * > @copydoc wgpuRenderPassEncoderSetVertexBuffer
 */
typedef void (*WGPUProcRenderPassEncoderSetVertexBuffer)(WGPURenderPassEncoder renderPassEncoder, uint32_t slot, WGPU_NULLABLE WGPUBuffer buffer, uint64_t offset, uint64_t size) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderSetViewport:
 * > @copydoc wgpuRenderPassEncoderSetViewport
 */
typedef void (*WGPUProcRenderPassEncoderSetViewport)(WGPURenderPassEncoder renderPassEncoder, float x, float y, float width, float height, float minDepth, float maxDepth) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderAddRef:
 * > @copydoc wgpuRenderPassEncoderAddRef
 */
typedef void (*WGPUProcRenderPassEncoderAddRef)(WGPURenderPassEncoder renderPassEncoder) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPassEncoderRelease:
 * > @copydoc wgpuRenderPassEncoderRelease
 */
typedef void (*WGPUProcRenderPassEncoderRelease)(WGPURenderPassEncoder renderPassEncoder) WGPU_FUNCTION_ATTRIBUTE;

// Procs of RenderPipeline
/**
 * Proc pointer type for @ref wgpuRenderPipelineGetBindGroupLayout:
 * > @copydoc wgpuRenderPipelineGetBindGroupLayout
 */
typedef WGPUBindGroupLayout (*WGPUProcRenderPipelineGetBindGroupLayout)(WGPURenderPipeline renderPipeline, uint32_t groupIndex) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPipelineSetLabel:
 * > @copydoc wgpuRenderPipelineSetLabel
 */
typedef void (*WGPUProcRenderPipelineSetLabel)(WGPURenderPipeline renderPipeline, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPipelineAddRef:
 * > @copydoc wgpuRenderPipelineAddRef
 */
typedef void (*WGPUProcRenderPipelineAddRef)(WGPURenderPipeline renderPipeline) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuRenderPipelineRelease:
 * > @copydoc wgpuRenderPipelineRelease
 */
typedef void (*WGPUProcRenderPipelineRelease)(WGPURenderPipeline renderPipeline) WGPU_FUNCTION_ATTRIBUTE;

// Procs of Sampler
/**
 * Proc pointer type for @ref wgpuSamplerSetLabel:
 * > @copydoc wgpuSamplerSetLabel
 */
typedef void (*WGPUProcSamplerSetLabel)(WGPUSampler sampler, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuSamplerAddRef:
 * > @copydoc wgpuSamplerAddRef
 */
typedef void (*WGPUProcSamplerAddRef)(WGPUSampler sampler) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuSamplerRelease:
 * > @copydoc wgpuSamplerRelease
 */
typedef void (*WGPUProcSamplerRelease)(WGPUSampler sampler) WGPU_FUNCTION_ATTRIBUTE;

// Procs of ShaderModule
/**
 * Proc pointer type for @ref wgpuShaderModuleGetCompilationInfo:
 * > @copydoc wgpuShaderModuleGetCompilationInfo
 */
typedef WGPUFuture (*WGPUProcShaderModuleGetCompilationInfo)(WGPUShaderModule shaderModule, WGPUCompilationInfoCallbackInfo callbackInfo) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuShaderModuleSetLabel:
 * > @copydoc wgpuShaderModuleSetLabel
 */
typedef void (*WGPUProcShaderModuleSetLabel)(WGPUShaderModule shaderModule, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuShaderModuleAddRef:
 * > @copydoc wgpuShaderModuleAddRef
 */
typedef void (*WGPUProcShaderModuleAddRef)(WGPUShaderModule shaderModule) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuShaderModuleRelease:
 * > @copydoc wgpuShaderModuleRelease
 */
typedef void (*WGPUProcShaderModuleRelease)(WGPUShaderModule shaderModule) WGPU_FUNCTION_ATTRIBUTE;

// Procs of SupportedFeatures
/**
 * Proc pointer type for @ref wgpuSupportedFeaturesFreeMembers:
 * > @copydoc wgpuSupportedFeaturesFreeMembers
 */
typedef void (*WGPUProcSupportedFeaturesFreeMembers)(WGPUSupportedFeatures supportedFeatures) WGPU_FUNCTION_ATTRIBUTE;

// Procs of SupportedInstanceFeatures
/**
 * Proc pointer type for @ref wgpuSupportedInstanceFeaturesFreeMembers:
 * > @copydoc wgpuSupportedInstanceFeaturesFreeMembers
 */
typedef void (*WGPUProcSupportedInstanceFeaturesFreeMembers)(WGPUSupportedInstanceFeatures supportedInstanceFeatures) WGPU_FUNCTION_ATTRIBUTE;

// Procs of SupportedWGSLLanguageFeatures
/**
 * Proc pointer type for @ref wgpuSupportedWGSLLanguageFeaturesFreeMembers:
 * > @copydoc wgpuSupportedWGSLLanguageFeaturesFreeMembers
 */
typedef void (*WGPUProcSupportedWGSLLanguageFeaturesFreeMembers)(WGPUSupportedWGSLLanguageFeatures supportedWGSLLanguageFeatures) WGPU_FUNCTION_ATTRIBUTE;

// Procs of Surface
/**
 * Proc pointer type for @ref wgpuSurfaceConfigure:
 * > @copydoc wgpuSurfaceConfigure
 */
typedef void (*WGPUProcSurfaceConfigure)(WGPUSurface surface, WGPUSurfaceConfiguration const * config) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuSurfaceGetCapabilities:
 * > @copydoc wgpuSurfaceGetCapabilities
 */
typedef WGPUStatus (*WGPUProcSurfaceGetCapabilities)(WGPUSurface surface, WGPUAdapter adapter, WGPUSurfaceCapabilities * capabilities) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuSurfaceGetCurrentTexture:
 * > @copydoc wgpuSurfaceGetCurrentTexture
 */
typedef void (*WGPUProcSurfaceGetCurrentTexture)(WGPUSurface surface, WGPUSurfaceTexture * surfaceTexture) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuSurfacePresent:
 * > @copydoc wgpuSurfacePresent
 */
typedef WGPUStatus (*WGPUProcSurfacePresent)(WGPUSurface surface) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuSurfaceSetLabel:
 * > @copydoc wgpuSurfaceSetLabel
 */
typedef void (*WGPUProcSurfaceSetLabel)(WGPUSurface surface, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuSurfaceUnconfigure:
 * > @copydoc wgpuSurfaceUnconfigure
 */
typedef void (*WGPUProcSurfaceUnconfigure)(WGPUSurface surface) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuSurfaceAddRef:
 * > @copydoc wgpuSurfaceAddRef
 */
typedef void (*WGPUProcSurfaceAddRef)(WGPUSurface surface) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuSurfaceRelease:
 * > @copydoc wgpuSurfaceRelease
 */
typedef void (*WGPUProcSurfaceRelease)(WGPUSurface surface) WGPU_FUNCTION_ATTRIBUTE;

// Procs of SurfaceCapabilities
/**
 * Proc pointer type for @ref wgpuSurfaceCapabilitiesFreeMembers:
 * > @copydoc wgpuSurfaceCapabilitiesFreeMembers
 */
typedef void (*WGPUProcSurfaceCapabilitiesFreeMembers)(WGPUSurfaceCapabilities surfaceCapabilities) WGPU_FUNCTION_ATTRIBUTE;

// Procs of Texture
/**
 * Proc pointer type for @ref wgpuTextureCreateView:
 * > @copydoc wgpuTextureCreateView
 */
typedef WGPUTextureView (*WGPUProcTextureCreateView)(WGPUTexture texture, WGPU_NULLABLE WGPUTextureViewDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuTextureDestroy:
 * > @copydoc wgpuTextureDestroy
 */
typedef void (*WGPUProcTextureDestroy)(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuTextureGetDepthOrArrayLayers:
 * > @copydoc wgpuTextureGetDepthOrArrayLayers
 */
typedef uint32_t (*WGPUProcTextureGetDepthOrArrayLayers)(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuTextureGetDimension:
 * > @copydoc wgpuTextureGetDimension
 */
typedef WGPUTextureDimension (*WGPUProcTextureGetDimension)(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuTextureGetFormat:
 * > @copydoc wgpuTextureGetFormat
 */
typedef WGPUTextureFormat (*WGPUProcTextureGetFormat)(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuTextureGetHeight:
 * > @copydoc wgpuTextureGetHeight
 */
typedef uint32_t (*WGPUProcTextureGetHeight)(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuTextureGetMipLevelCount:
 * > @copydoc wgpuTextureGetMipLevelCount
 */
typedef uint32_t (*WGPUProcTextureGetMipLevelCount)(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuTextureGetSampleCount:
 * > @copydoc wgpuTextureGetSampleCount
 */
typedef uint32_t (*WGPUProcTextureGetSampleCount)(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuTextureGetTextureBindingViewDimension:
 * > @copydoc wgpuTextureGetTextureBindingViewDimension
 */
typedef WGPUTextureViewDimension (*WGPUProcTextureGetTextureBindingViewDimension)(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuTextureGetUsage:
 * > @copydoc wgpuTextureGetUsage
 */
typedef WGPUTextureUsage (*WGPUProcTextureGetUsage)(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuTextureGetWidth:
 * > @copydoc wgpuTextureGetWidth
 */
typedef uint32_t (*WGPUProcTextureGetWidth)(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuTextureSetLabel:
 * > @copydoc wgpuTextureSetLabel
 */
typedef void (*WGPUProcTextureSetLabel)(WGPUTexture texture, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuTextureAddRef:
 * > @copydoc wgpuTextureAddRef
 */
typedef void (*WGPUProcTextureAddRef)(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuTextureRelease:
 * > @copydoc wgpuTextureRelease
 */
typedef void (*WGPUProcTextureRelease)(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;

// Procs of TextureView
/**
 * Proc pointer type for @ref wgpuTextureViewSetLabel:
 * > @copydoc wgpuTextureViewSetLabel
 */
typedef void (*WGPUProcTextureViewSetLabel)(WGPUTextureView textureView, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuTextureViewAddRef:
 * > @copydoc wgpuTextureViewAddRef
 */
typedef void (*WGPUProcTextureViewAddRef)(WGPUTextureView textureView) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Proc pointer type for @ref wgpuTextureViewRelease:
 * > @copydoc wgpuTextureViewRelease
 */
typedef void (*WGPUProcTextureViewRelease)(WGPUTextureView textureView) WGPU_FUNCTION_ATTRIBUTE;

#endif  // !defined(WGPU_SKIP_PROCS)

#if !defined(WGPU_SKIP_DECLARATIONS)
/**
 * \defgroup GlobalFunctions Global Functions
 * \brief Functions that are not specific to an object.
 *
 * @{
 */
/**
 * Create a WGPUInstance
 *
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPUInstance wgpuCreateInstance(WGPU_NULLABLE WGPUInstanceDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Get the list of @ref WGPUInstanceFeatureName values supported by the instance.
 *
 * @param features
 * This parameter is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT void wgpuGetInstanceFeatures(WGPUSupportedInstanceFeatures * features) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Get the limits supported by the instance.
 *
 * @returns
 * Indicates if there was an @ref OutStructChainError.
 */
WGPU_EXPORT WGPUStatus wgpuGetInstanceLimits(WGPUInstanceLimits * limits) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Check whether a particular @ref WGPUInstanceFeatureName is supported by the instance.
 */
WGPU_EXPORT WGPUBool wgpuHasInstanceFeature(WGPUInstanceFeatureName feature) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Returns the "procedure address" (function pointer) of the named function.
 * The result must be cast to the appropriate proc pointer type.
 */
WGPU_EXPORT WGPUProc wgpuGetProcAddress(WGPUStringView procName) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup Methods Methods
 * \brief Functions that are relative to a specific object.
 *
 * @{
 */

/**
 * \defgroup WGPUAdapterMethods WGPUAdapter methods
 * \brief Functions whose first argument has type WGPUAdapter.
 *
 * @{
 */
/**
 * Get the list of @ref WGPUFeatureName values supported by the adapter.
 *
 * @param features
 * This parameter is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT void wgpuAdapterGetFeatures(WGPUAdapter adapter, WGPUSupportedFeatures * features) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @param info
 * This parameter is @ref ReturnedWithOwnership.
 *
 * @returns
 * Indicates if there was an @ref OutStructChainError.
 */
WGPU_EXPORT WGPUStatus wgpuAdapterGetInfo(WGPUAdapter adapter, WGPUAdapterInfo * info) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @returns
 * Indicates if there was an @ref OutStructChainError.
 */
WGPU_EXPORT WGPUStatus wgpuAdapterGetLimits(WGPUAdapter adapter, WGPULimits * limits) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT WGPUBool wgpuAdapterHasFeature(WGPUAdapter adapter, WGPUFeatureName feature) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT WGPUFuture wgpuAdapterRequestDevice(WGPUAdapter adapter, WGPU_NULLABLE WGPUDeviceDescriptor const * descriptor, WGPURequestDeviceCallbackInfo callbackInfo) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuAdapterAddRef(WGPUAdapter adapter) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuAdapterRelease(WGPUAdapter adapter) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUAdapterInfoMethods WGPUAdapterInfo methods
 * \brief Functions whose first argument has type WGPUAdapterInfo.
 *
 * @{
 */
/**
 * Frees members which were allocated by the API.
 */
WGPU_EXPORT void wgpuAdapterInfoFreeMembers(WGPUAdapterInfo adapterInfo) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUBindGroupMethods WGPUBindGroup methods
 * \brief Functions whose first argument has type WGPUBindGroup.
 *
 * @{
 */
WGPU_EXPORT void wgpuBindGroupSetLabel(WGPUBindGroup bindGroup, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuBindGroupAddRef(WGPUBindGroup bindGroup) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuBindGroupRelease(WGPUBindGroup bindGroup) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUBindGroupLayoutMethods WGPUBindGroupLayout methods
 * \brief Functions whose first argument has type WGPUBindGroupLayout.
 *
 * @{
 */
WGPU_EXPORT void wgpuBindGroupLayoutSetLabel(WGPUBindGroupLayout bindGroupLayout, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuBindGroupLayoutAddRef(WGPUBindGroupLayout bindGroupLayout) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuBindGroupLayoutRelease(WGPUBindGroupLayout bindGroupLayout) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUBufferMethods WGPUBuffer methods
 * \brief Functions whose first argument has type WGPUBuffer.
 *
 * @{
 */
WGPU_EXPORT void wgpuBufferDestroy(WGPUBuffer buffer) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Returns a const pointer to beginning of the mapped range.
 * It must not be written; writing to this range causes undefined behavior.
 * See @ref MappedRangeBehavior for error conditions and guarantees.
 * This function is safe to call inside spontaneous callbacks (see @ref CallbackReentrancy).
 *
 * In Wasm, if `memcpy`ing from this range, prefer using @ref wgpuBufferReadMappedRange
 * instead for better performance.
 *
 * @param offset
 * Byte offset relative to the beginning of the buffer.
 *
 * @param size
 * Byte size of the range to get.
 * If this is @ref WGPU_WHOLE_MAP_SIZE, it defaults to `buffer.size - offset`.
 * The returned pointer is valid for exactly this many bytes.
 */
WGPU_EXPORT void const * wgpuBufferGetConstMappedRange(WGPUBuffer buffer, size_t offset, size_t size) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Returns a mutable pointer to beginning of the mapped range.
 * See @ref MappedRangeBehavior for error conditions and guarantees.
 * This function is safe to call inside spontaneous callbacks (see @ref CallbackReentrancy).
 *
 * In Wasm, if `memcpy`ing into this range, prefer using @ref wgpuBufferWriteMappedRange
 * instead for better performance.
 *
 * @param offset
 * Byte offset relative to the beginning of the buffer.
 *
 * @param size
 * Byte size of the range to get.
 * If this is @ref WGPU_WHOLE_MAP_SIZE, it defaults to `buffer.size - offset`.
 * The returned pointer is valid for exactly this many bytes.
 */
WGPU_EXPORT void * wgpuBufferGetMappedRange(WGPUBuffer buffer, size_t offset, size_t size) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT WGPUBufferMapState wgpuBufferGetMapState(WGPUBuffer buffer) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT uint64_t wgpuBufferGetSize(WGPUBuffer buffer) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT WGPUBufferUsage wgpuBufferGetUsage(WGPUBuffer buffer) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @param mode
 * The mapping mode (read or write).
 *
 * @param offset
 * Byte offset relative to beginning of the buffer.
 *
 * @param size
 * Byte size of the region to map.
 * If this is @ref WGPU_WHOLE_MAP_SIZE, it defaults to `buffer.size - offset`.
 */
WGPU_EXPORT WGPUFuture wgpuBufferMapAsync(WGPUBuffer buffer, WGPUMapMode mode, size_t offset, size_t size, WGPUBufferMapCallbackInfo callbackInfo) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Copies a range of data from the buffer mapping into the provided destination pointer.
 * See @ref MappedRangeBehavior for error conditions and guarantees.
 * This function is safe to call inside spontaneous callbacks (see @ref CallbackReentrancy).
 *
 * In Wasm, this is more efficient than copying from a mapped range into a `malloc`'d range.
 *
 * @param offset
 * Byte offset relative to the beginning of the buffer.
 *
 * @param data
 * Destination, to read buffer data into.
 *
 * @param size
 * Number of bytes of data to read from the buffer.
 * (Note @ref WGPU_WHOLE_MAP_SIZE is *not* accepted here.)
 *
 * @returns
 * @ref WGPUStatus_Error if the copy did not occur.
 */
WGPU_EXPORT WGPUStatus wgpuBufferReadMappedRange(WGPUBuffer buffer, size_t offset, void * data, size_t size) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuBufferSetLabel(WGPUBuffer buffer, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuBufferUnmap(WGPUBuffer buffer) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Copies a range of data from the provided source pointer into the buffer mapping.
 * See @ref MappedRangeBehavior for error conditions and guarantees.
 * This function is safe to call inside spontaneous callbacks (see @ref CallbackReentrancy).
 *
 * In Wasm, this is more efficient than copying from a `malloc`'d range into a mapped range.
 *
 * @param offset
 * Byte offset relative to the beginning of the buffer.
 *
 * @param data
 * Source, to write buffer data from.
 *
 * @param size
 * Number of bytes of data to write to the buffer.
 * (Note @ref WGPU_WHOLE_MAP_SIZE is *not* accepted here.)
 *
 * @returns
 * @ref WGPUStatus_Error if the copy did not occur.
 */
WGPU_EXPORT WGPUStatus wgpuBufferWriteMappedRange(WGPUBuffer buffer, size_t offset, void const * data, size_t size) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuBufferAddRef(WGPUBuffer buffer) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuBufferRelease(WGPUBuffer buffer) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUCommandBufferMethods WGPUCommandBuffer methods
 * \brief Functions whose first argument has type WGPUCommandBuffer.
 *
 * @{
 */
WGPU_EXPORT void wgpuCommandBufferSetLabel(WGPUCommandBuffer commandBuffer, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuCommandBufferAddRef(WGPUCommandBuffer commandBuffer) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuCommandBufferRelease(WGPUCommandBuffer commandBuffer) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUCommandEncoderMethods WGPUCommandEncoder methods
 * \brief Functions whose first argument has type WGPUCommandEncoder.
 *
 * @{
 */
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPUComputePassEncoder wgpuCommandEncoderBeginComputePass(WGPUCommandEncoder commandEncoder, WGPU_NULLABLE WGPUComputePassDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPURenderPassEncoder wgpuCommandEncoderBeginRenderPass(WGPUCommandEncoder commandEncoder, WGPURenderPassDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuCommandEncoderClearBuffer(WGPUCommandEncoder commandEncoder, WGPUBuffer buffer, uint64_t offset, uint64_t size) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuCommandEncoderCopyBufferToBuffer(WGPUCommandEncoder commandEncoder, WGPUBuffer source, uint64_t sourceOffset, WGPUBuffer destination, uint64_t destinationOffset, uint64_t size) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuCommandEncoderCopyBufferToTexture(WGPUCommandEncoder commandEncoder, WGPUTexelCopyBufferInfo const * source, WGPUTexelCopyTextureInfo const * destination, WGPUExtent3D const * copySize) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuCommandEncoderCopyTextureToBuffer(WGPUCommandEncoder commandEncoder, WGPUTexelCopyTextureInfo const * source, WGPUTexelCopyBufferInfo const * destination, WGPUExtent3D const * copySize) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuCommandEncoderCopyTextureToTexture(WGPUCommandEncoder commandEncoder, WGPUTexelCopyTextureInfo const * source, WGPUTexelCopyTextureInfo const * destination, WGPUExtent3D const * copySize) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPUCommandBuffer wgpuCommandEncoderFinish(WGPUCommandEncoder commandEncoder, WGPU_NULLABLE WGPUCommandBufferDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuCommandEncoderInsertDebugMarker(WGPUCommandEncoder commandEncoder, WGPUStringView markerLabel) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuCommandEncoderPopDebugGroup(WGPUCommandEncoder commandEncoder) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuCommandEncoderPushDebugGroup(WGPUCommandEncoder commandEncoder, WGPUStringView groupLabel) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuCommandEncoderResolveQuerySet(WGPUCommandEncoder commandEncoder, WGPUQuerySet querySet, uint32_t firstQuery, uint32_t queryCount, WGPUBuffer destination, uint64_t destinationOffset) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuCommandEncoderSetLabel(WGPUCommandEncoder commandEncoder, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuCommandEncoderWriteTimestamp(WGPUCommandEncoder commandEncoder, WGPUQuerySet querySet, uint32_t queryIndex) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuCommandEncoderAddRef(WGPUCommandEncoder commandEncoder) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuCommandEncoderRelease(WGPUCommandEncoder commandEncoder) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUComputePassEncoderMethods WGPUComputePassEncoder methods
 * \brief Functions whose first argument has type WGPUComputePassEncoder.
 *
 * @{
 */
WGPU_EXPORT void wgpuComputePassEncoderDispatchWorkgroups(WGPUComputePassEncoder computePassEncoder, uint32_t workgroupCountX, uint32_t workgroupCountY, uint32_t workgroupCountZ) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuComputePassEncoderDispatchWorkgroupsIndirect(WGPUComputePassEncoder computePassEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuComputePassEncoderEnd(WGPUComputePassEncoder computePassEncoder) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuComputePassEncoderInsertDebugMarker(WGPUComputePassEncoder computePassEncoder, WGPUStringView markerLabel) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuComputePassEncoderPopDebugGroup(WGPUComputePassEncoder computePassEncoder) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuComputePassEncoderPushDebugGroup(WGPUComputePassEncoder computePassEncoder, WGPUStringView groupLabel) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuComputePassEncoderSetBindGroup(WGPUComputePassEncoder computePassEncoder, uint32_t groupIndex, WGPU_NULLABLE WGPUBindGroup group, size_t dynamicOffsetCount, uint32_t const * dynamicOffsets) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuComputePassEncoderSetLabel(WGPUComputePassEncoder computePassEncoder, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuComputePassEncoderSetPipeline(WGPUComputePassEncoder computePassEncoder, WGPUComputePipeline pipeline) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuComputePassEncoderAddRef(WGPUComputePassEncoder computePassEncoder) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuComputePassEncoderRelease(WGPUComputePassEncoder computePassEncoder) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUComputePipelineMethods WGPUComputePipeline methods
 * \brief Functions whose first argument has type WGPUComputePipeline.
 *
 * @{
 */
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPUBindGroupLayout wgpuComputePipelineGetBindGroupLayout(WGPUComputePipeline computePipeline, uint32_t groupIndex) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuComputePipelineSetLabel(WGPUComputePipeline computePipeline, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuComputePipelineAddRef(WGPUComputePipeline computePipeline) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuComputePipelineRelease(WGPUComputePipeline computePipeline) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUDeviceMethods WGPUDevice methods
 * \brief Functions whose first argument has type WGPUDevice.
 *
 * @{
 */
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPUBindGroup wgpuDeviceCreateBindGroup(WGPUDevice device, WGPUBindGroupDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPUBindGroupLayout wgpuDeviceCreateBindGroupLayout(WGPUDevice device, WGPUBindGroupLayoutDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * TODO
 *
 * If @ref WGPUBufferDescriptor::mappedAtCreation is `true` and the mapping allocation fails,
 * returns `NULL`.
 *
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPU_NULLABLE WGPUBuffer wgpuDeviceCreateBuffer(WGPUDevice device, WGPUBufferDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPUCommandEncoder wgpuDeviceCreateCommandEncoder(WGPUDevice device, WGPU_NULLABLE WGPUCommandEncoderDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPUComputePipeline wgpuDeviceCreateComputePipeline(WGPUDevice device, WGPUComputePipelineDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT WGPUFuture wgpuDeviceCreateComputePipelineAsync(WGPUDevice device, WGPUComputePipelineDescriptor const * descriptor, WGPUCreateComputePipelineAsyncCallbackInfo callbackInfo) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPUPipelineLayout wgpuDeviceCreatePipelineLayout(WGPUDevice device, WGPUPipelineLayoutDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPUQuerySet wgpuDeviceCreateQuerySet(WGPUDevice device, WGPUQuerySetDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPURenderBundleEncoder wgpuDeviceCreateRenderBundleEncoder(WGPUDevice device, WGPURenderBundleEncoderDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPURenderPipeline wgpuDeviceCreateRenderPipeline(WGPUDevice device, WGPURenderPipelineDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT WGPUFuture wgpuDeviceCreateRenderPipelineAsync(WGPUDevice device, WGPURenderPipelineDescriptor const * descriptor, WGPUCreateRenderPipelineAsyncCallbackInfo callbackInfo) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPUSampler wgpuDeviceCreateSampler(WGPUDevice device, WGPU_NULLABLE WGPUSamplerDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPUShaderModule wgpuDeviceCreateShaderModule(WGPUDevice device, WGPUShaderModuleDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPUTexture wgpuDeviceCreateTexture(WGPUDevice device, WGPUTextureDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuDeviceDestroy(WGPUDevice device) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @param adapterInfo
 * This parameter is @ref ReturnedWithOwnership.
 *
 * @returns
 * Indicates if there was an @ref OutStructChainError.
 */
WGPU_EXPORT WGPUStatus wgpuDeviceGetAdapterInfo(WGPUDevice device, WGPUAdapterInfo * adapterInfo) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Get the list of @ref WGPUFeatureName values supported by the device.
 *
 * @param features
 * This parameter is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT void wgpuDeviceGetFeatures(WGPUDevice device, WGPUSupportedFeatures * features) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @returns
 * Indicates if there was an @ref OutStructChainError.
 */
WGPU_EXPORT WGPUStatus wgpuDeviceGetLimits(WGPUDevice device, WGPULimits * limits) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @returns
 * The @ref WGPUFuture for the device-lost event of the device.
 */
WGPU_EXPORT WGPUFuture wgpuDeviceGetLostFuture(WGPUDevice device) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPUQueue wgpuDeviceGetQueue(WGPUDevice device) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT WGPUBool wgpuDeviceHasFeature(WGPUDevice device, WGPUFeatureName feature) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Pops an error scope to the current thread's error scope stack,
 * asynchronously returning the result. See @ref ErrorScopes.
 */
WGPU_EXPORT WGPUFuture wgpuDevicePopErrorScope(WGPUDevice device, WGPUPopErrorScopeCallbackInfo callbackInfo) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Pushes an error scope to the current thread's error scope stack.
 * See @ref ErrorScopes.
 */
WGPU_EXPORT void wgpuDevicePushErrorScope(WGPUDevice device, WGPUErrorFilter filter) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuDeviceSetLabel(WGPUDevice device, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuDeviceAddRef(WGPUDevice device) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuDeviceRelease(WGPUDevice device) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUExternalTextureMethods WGPUExternalTexture methods
 * \brief Functions whose first argument has type WGPUExternalTexture.
 *
 * @{
 */
WGPU_EXPORT void wgpuExternalTextureSetLabel(WGPUExternalTexture externalTexture, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuExternalTextureAddRef(WGPUExternalTexture externalTexture) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuExternalTextureRelease(WGPUExternalTexture externalTexture) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUInstanceMethods WGPUInstance methods
 * \brief Functions whose first argument has type WGPUInstance.
 *
 * @{
 */
/**
 * Creates a @ref WGPUSurface, see @ref Surface-Creation for more details.
 *
 * @param descriptor
 * The description of the @ref WGPUSurface to create.
 *
 * @returns
 * A new @ref WGPUSurface for this descriptor (or an error @ref WGPUSurface).
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPUSurface wgpuInstanceCreateSurface(WGPUInstance instance, WGPUSurfaceDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Get the list of @ref WGPUWGSLLanguageFeatureName values supported by the instance.
 */
WGPU_EXPORT void wgpuInstanceGetWGSLLanguageFeatures(WGPUInstance instance, WGPUSupportedWGSLLanguageFeatures * features) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT WGPUBool wgpuInstanceHasWGSLLanguageFeature(WGPUInstance instance, WGPUWGSLLanguageFeatureName feature) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Processes asynchronous events on this `WGPUInstance`, calling any callbacks for asynchronous operations created with @ref WGPUCallbackMode_AllowProcessEvents.
 *
 * See @ref Process-Events for more information.
 */
WGPU_EXPORT void wgpuInstanceProcessEvents(WGPUInstance instance) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT WGPUFuture wgpuInstanceRequestAdapter(WGPUInstance instance, WGPU_NULLABLE WGPURequestAdapterOptions const * options, WGPURequestAdapterCallbackInfo callbackInfo) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Wait for at least one WGPUFuture in `futures` to complete, and call callbacks of the respective completed asynchronous operations.
 *
 * See @ref Wait-Any for more information.
 */
WGPU_EXPORT WGPUWaitStatus wgpuInstanceWaitAny(WGPUInstance instance, size_t futureCount, WGPU_NULLABLE WGPUFutureWaitInfo * futures, uint64_t timeoutNS) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuInstanceAddRef(WGPUInstance instance) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuInstanceRelease(WGPUInstance instance) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUPipelineLayoutMethods WGPUPipelineLayout methods
 * \brief Functions whose first argument has type WGPUPipelineLayout.
 *
 * @{
 */
WGPU_EXPORT void wgpuPipelineLayoutSetLabel(WGPUPipelineLayout pipelineLayout, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuPipelineLayoutAddRef(WGPUPipelineLayout pipelineLayout) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuPipelineLayoutRelease(WGPUPipelineLayout pipelineLayout) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUQuerySetMethods WGPUQuerySet methods
 * \brief Functions whose first argument has type WGPUQuerySet.
 *
 * @{
 */
WGPU_EXPORT void wgpuQuerySetDestroy(WGPUQuerySet querySet) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT uint32_t wgpuQuerySetGetCount(WGPUQuerySet querySet) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT WGPUQueryType wgpuQuerySetGetType(WGPUQuerySet querySet) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuQuerySetSetLabel(WGPUQuerySet querySet, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuQuerySetAddRef(WGPUQuerySet querySet) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuQuerySetRelease(WGPUQuerySet querySet) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUQueueMethods WGPUQueue methods
 * \brief Functions whose first argument has type WGPUQueue.
 *
 * @{
 */
WGPU_EXPORT WGPUFuture wgpuQueueOnSubmittedWorkDone(WGPUQueue queue, WGPUQueueWorkDoneCallbackInfo callbackInfo) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuQueueSetLabel(WGPUQueue queue, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuQueueSubmit(WGPUQueue queue, size_t commandCount, WGPUCommandBuffer const * commands) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Produces a @ref DeviceError both content-timeline (`size` alignment) and device-timeline
 * errors defined by the WebGPU specification.
 */
WGPU_EXPORT void wgpuQueueWriteBuffer(WGPUQueue queue, WGPUBuffer buffer, uint64_t bufferOffset, void const * data, size_t size) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuQueueWriteTexture(WGPUQueue queue, WGPUTexelCopyTextureInfo const * destination, void const * data, size_t dataSize, WGPUTexelCopyBufferLayout const * dataLayout, WGPUExtent3D const * writeSize) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuQueueAddRef(WGPUQueue queue) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuQueueRelease(WGPUQueue queue) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPURenderBundleMethods WGPURenderBundle methods
 * \brief Functions whose first argument has type WGPURenderBundle.
 *
 * @{
 */
WGPU_EXPORT void wgpuRenderBundleSetLabel(WGPURenderBundle renderBundle, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderBundleAddRef(WGPURenderBundle renderBundle) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderBundleRelease(WGPURenderBundle renderBundle) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPURenderBundleEncoderMethods WGPURenderBundleEncoder methods
 * \brief Functions whose first argument has type WGPURenderBundleEncoder.
 *
 * @{
 */
WGPU_EXPORT void wgpuRenderBundleEncoderDraw(WGPURenderBundleEncoder renderBundleEncoder, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderBundleEncoderDrawIndexed(WGPURenderBundleEncoder renderBundleEncoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderBundleEncoderDrawIndexedIndirect(WGPURenderBundleEncoder renderBundleEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderBundleEncoderDrawIndirect(WGPURenderBundleEncoder renderBundleEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPURenderBundle wgpuRenderBundleEncoderFinish(WGPURenderBundleEncoder renderBundleEncoder, WGPU_NULLABLE WGPURenderBundleDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderBundleEncoderInsertDebugMarker(WGPURenderBundleEncoder renderBundleEncoder, WGPUStringView markerLabel) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderBundleEncoderPopDebugGroup(WGPURenderBundleEncoder renderBundleEncoder) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderBundleEncoderPushDebugGroup(WGPURenderBundleEncoder renderBundleEncoder, WGPUStringView groupLabel) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderBundleEncoderSetBindGroup(WGPURenderBundleEncoder renderBundleEncoder, uint32_t groupIndex, WGPU_NULLABLE WGPUBindGroup group, size_t dynamicOffsetCount, uint32_t const * dynamicOffsets) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderBundleEncoderSetIndexBuffer(WGPURenderBundleEncoder renderBundleEncoder, WGPUBuffer buffer, WGPUIndexFormat format, uint64_t offset, uint64_t size) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderBundleEncoderSetLabel(WGPURenderBundleEncoder renderBundleEncoder, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderBundleEncoderSetPipeline(WGPURenderBundleEncoder renderBundleEncoder, WGPURenderPipeline pipeline) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderBundleEncoderSetVertexBuffer(WGPURenderBundleEncoder renderBundleEncoder, uint32_t slot, WGPU_NULLABLE WGPUBuffer buffer, uint64_t offset, uint64_t size) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderBundleEncoderAddRef(WGPURenderBundleEncoder renderBundleEncoder) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderBundleEncoderRelease(WGPURenderBundleEncoder renderBundleEncoder) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPURenderPassEncoderMethods WGPURenderPassEncoder methods
 * \brief Functions whose first argument has type WGPURenderPassEncoder.
 *
 * @{
 */
WGPU_EXPORT void wgpuRenderPassEncoderBeginOcclusionQuery(WGPURenderPassEncoder renderPassEncoder, uint32_t queryIndex) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderDraw(WGPURenderPassEncoder renderPassEncoder, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderDrawIndexed(WGPURenderPassEncoder renderPassEncoder, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderDrawIndexedIndirect(WGPURenderPassEncoder renderPassEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderDrawIndirect(WGPURenderPassEncoder renderPassEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderEnd(WGPURenderPassEncoder renderPassEncoder) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderEndOcclusionQuery(WGPURenderPassEncoder renderPassEncoder) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderExecuteBundles(WGPURenderPassEncoder renderPassEncoder, size_t bundleCount, WGPURenderBundle const * bundles) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderInsertDebugMarker(WGPURenderPassEncoder renderPassEncoder, WGPUStringView markerLabel) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderPopDebugGroup(WGPURenderPassEncoder renderPassEncoder) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderPushDebugGroup(WGPURenderPassEncoder renderPassEncoder, WGPUStringView groupLabel) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderSetBindGroup(WGPURenderPassEncoder renderPassEncoder, uint32_t groupIndex, WGPU_NULLABLE WGPUBindGroup group, size_t dynamicOffsetCount, uint32_t const * dynamicOffsets) WGPU_FUNCTION_ATTRIBUTE;
/**
 * @param color
 * The RGBA blend constant. Represents an `f32` color using @ref DoubleAsSupertype.
 */
WGPU_EXPORT void wgpuRenderPassEncoderSetBlendConstant(WGPURenderPassEncoder renderPassEncoder, WGPUColor const * color) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderSetIndexBuffer(WGPURenderPassEncoder renderPassEncoder, WGPUBuffer buffer, WGPUIndexFormat format, uint64_t offset, uint64_t size) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderSetLabel(WGPURenderPassEncoder renderPassEncoder, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderSetPipeline(WGPURenderPassEncoder renderPassEncoder, WGPURenderPipeline pipeline) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderSetScissorRect(WGPURenderPassEncoder renderPassEncoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderSetStencilReference(WGPURenderPassEncoder renderPassEncoder, uint32_t reference) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderSetVertexBuffer(WGPURenderPassEncoder renderPassEncoder, uint32_t slot, WGPU_NULLABLE WGPUBuffer buffer, uint64_t offset, uint64_t size) WGPU_FUNCTION_ATTRIBUTE;
/**
 * TODO
 *
 * If any argument is non-finite, produces a @ref NonFiniteFloatValueError.
 */
WGPU_EXPORT void wgpuRenderPassEncoderSetViewport(WGPURenderPassEncoder renderPassEncoder, float x, float y, float width, float height, float minDepth, float maxDepth) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderAddRef(WGPURenderPassEncoder renderPassEncoder) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPassEncoderRelease(WGPURenderPassEncoder renderPassEncoder) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPURenderPipelineMethods WGPURenderPipeline methods
 * \brief Functions whose first argument has type WGPURenderPipeline.
 *
 * @{
 */
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPUBindGroupLayout wgpuRenderPipelineGetBindGroupLayout(WGPURenderPipeline renderPipeline, uint32_t groupIndex) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPipelineSetLabel(WGPURenderPipeline renderPipeline, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPipelineAddRef(WGPURenderPipeline renderPipeline) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuRenderPipelineRelease(WGPURenderPipeline renderPipeline) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUSamplerMethods WGPUSampler methods
 * \brief Functions whose first argument has type WGPUSampler.
 *
 * @{
 */
WGPU_EXPORT void wgpuSamplerSetLabel(WGPUSampler sampler, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuSamplerAddRef(WGPUSampler sampler) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuSamplerRelease(WGPUSampler sampler) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUShaderModuleMethods WGPUShaderModule methods
 * \brief Functions whose first argument has type WGPUShaderModule.
 *
 * @{
 */
WGPU_EXPORT WGPUFuture wgpuShaderModuleGetCompilationInfo(WGPUShaderModule shaderModule, WGPUCompilationInfoCallbackInfo callbackInfo) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuShaderModuleSetLabel(WGPUShaderModule shaderModule, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuShaderModuleAddRef(WGPUShaderModule shaderModule) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuShaderModuleRelease(WGPUShaderModule shaderModule) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUSupportedFeaturesMethods WGPUSupportedFeatures methods
 * \brief Functions whose first argument has type WGPUSupportedFeatures.
 *
 * @{
 */
/**
 * Frees members which were allocated by the API.
 */
WGPU_EXPORT void wgpuSupportedFeaturesFreeMembers(WGPUSupportedFeatures supportedFeatures) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUSupportedInstanceFeaturesMethods WGPUSupportedInstanceFeatures methods
 * \brief Functions whose first argument has type WGPUSupportedInstanceFeatures.
 *
 * @{
 */
/**
 * Frees members which were allocated by the API.
 */
WGPU_EXPORT void wgpuSupportedInstanceFeaturesFreeMembers(WGPUSupportedInstanceFeatures supportedInstanceFeatures) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUSupportedWGSLLanguageFeaturesMethods WGPUSupportedWGSLLanguageFeatures methods
 * \brief Functions whose first argument has type WGPUSupportedWGSLLanguageFeatures.
 *
 * @{
 */
/**
 * Frees members which were allocated by the API.
 */
WGPU_EXPORT void wgpuSupportedWGSLLanguageFeaturesFreeMembers(WGPUSupportedWGSLLanguageFeatures supportedWGSLLanguageFeatures) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUSurfaceMethods WGPUSurface methods
 * \brief Functions whose first argument has type WGPUSurface.
 *
 * @{
 */
/**
 * Configures parameters for rendering to `surface`.
 * Produces a @ref DeviceError for all content-timeline errors defined by the WebGPU specification.
 *
 * See @ref Surface-Configuration for more details.
 *
 * @param config
 * The new configuration to use.
 */
WGPU_EXPORT void wgpuSurfaceConfigure(WGPUSurface surface, WGPUSurfaceConfiguration const * config) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Provides information on how `adapter` is able to use `surface`.
 * See @ref Surface-Capabilities for more details.
 *
 * @param adapter
 * The @ref WGPUAdapter to get capabilities for presenting to this @ref WGPUSurface.
 *
 * @param capabilities
 * The structure to fill capabilities in.
 * It may contain memory allocations so @ref wgpuSurfaceCapabilitiesFreeMembers must be called to avoid memory leaks.
 * This parameter is @ref ReturnedWithOwnership.
 *
 * @returns
 * Indicates if there was an @ref OutStructChainError.
 */
WGPU_EXPORT WGPUStatus wgpuSurfaceGetCapabilities(WGPUSurface surface, WGPUAdapter adapter, WGPUSurfaceCapabilities * capabilities) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Returns the @ref WGPUTexture to render to `surface` this frame along with metadata on the frame.
 * Returns `NULL` and @ref WGPUSurfaceGetCurrentTextureStatus_Error if the surface is not configured.
 *
 * See @ref Surface-Presenting for more details.
 *
 * @param surfaceTexture
 * The structure to fill the @ref WGPUTexture and metadata in.
 */
WGPU_EXPORT void wgpuSurfaceGetCurrentTexture(WGPUSurface surface, WGPUSurfaceTexture * surfaceTexture) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Shows `surface`'s current texture to the user.
 * See @ref Surface-Presenting for more details.
 *
 * @returns
 * Returns @ref WGPUStatus_Error if the surface doesn't have a current texture.
 */
WGPU_EXPORT WGPUStatus wgpuSurfacePresent(WGPUSurface surface) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Modifies the label used to refer to `surface`.
 *
 * @param label
 * The new label.
 */
WGPU_EXPORT void wgpuSurfaceSetLabel(WGPUSurface surface, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
/**
 * Removes the configuration for `surface`.
 * See @ref Surface-Configuration for more details.
 */
WGPU_EXPORT void wgpuSurfaceUnconfigure(WGPUSurface surface) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuSurfaceAddRef(WGPUSurface surface) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuSurfaceRelease(WGPUSurface surface) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUSurfaceCapabilitiesMethods WGPUSurfaceCapabilities methods
 * \brief Functions whose first argument has type WGPUSurfaceCapabilities.
 *
 * @{
 */
/**
 * Frees members which were allocated by the API.
 */
WGPU_EXPORT void wgpuSurfaceCapabilitiesFreeMembers(WGPUSurfaceCapabilities surfaceCapabilities) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUTextureMethods WGPUTexture methods
 * \brief Functions whose first argument has type WGPUTexture.
 *
 * @{
 */
/**
 * @returns
 * This value is @ref ReturnedWithOwnership.
 */
WGPU_EXPORT WGPUTextureView wgpuTextureCreateView(WGPUTexture texture, WGPU_NULLABLE WGPUTextureViewDescriptor const * descriptor) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuTextureDestroy(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT uint32_t wgpuTextureGetDepthOrArrayLayers(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT WGPUTextureDimension wgpuTextureGetDimension(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT WGPUTextureFormat wgpuTextureGetFormat(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT uint32_t wgpuTextureGetHeight(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT uint32_t wgpuTextureGetMipLevelCount(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT uint32_t wgpuTextureGetSampleCount(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT WGPUTextureViewDimension wgpuTextureGetTextureBindingViewDimension(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT WGPUTextureUsage wgpuTextureGetUsage(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT uint32_t wgpuTextureGetWidth(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuTextureSetLabel(WGPUTexture texture, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuTextureAddRef(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuTextureRelease(WGPUTexture texture) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/**
 * \defgroup WGPUTextureViewMethods WGPUTextureView methods
 * \brief Functions whose first argument has type WGPUTextureView.
 *
 * @{
 */
WGPU_EXPORT void wgpuTextureViewSetLabel(WGPUTextureView textureView, WGPUStringView label) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuTextureViewAddRef(WGPUTextureView textureView) WGPU_FUNCTION_ATTRIBUTE;
WGPU_EXPORT void wgpuTextureViewRelease(WGPUTextureView textureView) WGPU_FUNCTION_ATTRIBUTE;

/** @} */

/** @} */

#endif  // !defined(WGPU_SKIP_DECLARATIONS)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // WEBGPU_H_
