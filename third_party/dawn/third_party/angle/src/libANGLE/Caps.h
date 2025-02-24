//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_CAPS_H_
#define LIBANGLE_CAPS_H_

#include "angle_gl.h"
#include "libANGLE/Version.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/gles_extensions_autogen.h"
#include "libANGLE/renderer/Format.h"

#include <array>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace gl
{
struct TextureCaps
{
    TextureCaps();
    TextureCaps(const TextureCaps &other);
    TextureCaps &operator=(const TextureCaps &other);

    ~TextureCaps();

    // Supports for basic texturing: glTexImage, glTexSubImage, etc
    bool texturable = false;

    // Support for linear or anisotropic filtering
    bool filterable = false;

    // Support for being used as a framebuffer attachment, i.e. glFramebufferTexture2D
    bool textureAttachment = false;

    // Support for being used as a renderbuffer format, i.e. glFramebufferRenderbuffer
    bool renderbuffer = false;

    // Support for blend modes while being used as a framebuffer attachment
    bool blendable = false;

    // Set of supported sample counts, only guaranteed to be valid in ES3.
    SupportedSampleSet sampleCounts;

    // Get the maximum number of samples supported
    GLuint getMaxSamples() const;

    // Get the number of supported samples that is at least as many as requested.  Returns 0 if
    // there are no sample counts available
    GLuint getNearestSamples(GLuint requestedSamples) const;
};

TextureCaps GenerateMinimumTextureCaps(GLenum internalFormat,
                                       const Version &clientVersion,
                                       const Extensions &extensions);

class TextureCapsMap final : angle::NonCopyable
{
  public:
    TextureCapsMap();
    ~TextureCapsMap();

    // These methods are deprecated. Please use angle::Format for new features.
    void insert(GLenum internalFormat, const TextureCaps &caps);
    const TextureCaps &get(GLenum internalFormat) const;

    void clear();

    // Prefer using angle::Format methods.
    const TextureCaps &get(angle::FormatID formatID) const;
    void set(angle::FormatID formatID, const TextureCaps &caps);

  private:
    TextureCaps &get(angle::FormatID formatID);

    // Indexed by angle::FormatID
    angle::FormatMap<TextureCaps> mFormatData;
};

void InitMinimumTextureCapsMap(const Version &clientVersion,
                               const Extensions &extensions,
                               TextureCapsMap *capsMap);

// Returns true if all the formats required to support GL_ANGLE_compressed_texture_etc are
// present. Does not determine if they are natively supported without decompression.
bool DetermineCompressedTextureETCSupport(const TextureCapsMap &textureCaps);

// Determine support for signed normalized format renderability.
bool DetermineRenderSnormSupport(const TextureCapsMap &textureCaps, bool textureNorm16EXT);

// Pointer to a boolean member of the Extensions struct
using ExtensionBool = bool Extensions::*;

struct ExtensionInfo
{
    // If this extension can be enabled or disabled  with glRequestExtension
    // (GL_ANGLE_request_extension)
    bool Requestable = false;
    bool Disablable  = false;

    // Pointer to a boolean member of the Extensions struct
    ExtensionBool ExtensionsMember = nullptr;
};

using ExtensionInfoMap = std::map<std::string, ExtensionInfo>;
const ExtensionInfoMap &GetExtensionInfoMap();

struct Limitations
{
    Limitations();
    Limitations(const Limitations &other);

    Limitations &operator=(const Limitations &other);

    // Renderer doesn't support gl_FrontFacing in fragment shaders
    bool noFrontFacingSupport = false;

    // Renderer doesn't support GL_SAMPLE_ALPHA_TO_COVERAGE
    bool noSampleAlphaToCoverageSupport = false;

    // In glVertexAttribDivisorANGLE, attribute zero must have a zero divisor
    bool attributeZeroRequiresZeroDivisorInEXT = false;

    // Unable to support different values for front and back faces for stencil refs and masks
    bool noSeparateStencilRefsAndMasks = false;

    // Renderer doesn't support Simultaneous use of GL_CONSTANT_ALPHA/GL_ONE_MINUS_CONSTANT_ALPHA
    // and GL_CONSTANT_COLOR/GL_ONE_MINUS_CONSTANT_COLOR blend functions.
    bool noSimultaneousConstantColorAndAlphaBlendFunc = false;

    // Renderer always clamps constant blend color.
    bool noUnclampedBlendColor = false;

    // D3D9 does not support flexible varying register packing.
    bool noFlexibleVaryingPacking = false;

    // D3D does not support having multiple transform feedback outputs go to the same buffer.
    bool noDoubleBoundTransformFeedbackBuffers = false;

    // D3D does not support vertex attribute aliasing
    bool noVertexAttributeAliasing = false;

    // Renderer doesn't support GL_TEXTURE_COMPARE_MODE=GL_NONE on a shadow sampler.
    // TODO(http://anglebug.com/42263785): add validation code to front-end.
    bool noShadowSamplerCompareModeNone = false;

    // PVRTC1 textures must be squares.
    bool squarePvrtc1 = false;

    // ETC1 texture support is emulated.
    bool emulatedEtc1 = false;

    // ASTC texture support is emulated.
    bool emulatedAstc = false;

    // No compressed TEXTURE_3D support.
    bool noCompressedTexture3D = false;

    // D3D does not support compressed textures where the base mip level is not a multiple of 4
    bool compressedBaseMipLevelMultipleOfFour = false;

    // An extra limit for WebGL texture size. Ignored if 0.
    GLint webGLTextureSizeLimit = 0;

    // GL_ANGLE_multi_draw is emulated and should only be exposed to WebGL. Emulated by default in
    // shared renderer code.
    bool multidrawEmulated = true;

    // GL_ANGLE_base_vertex_base_instance is emulated and should only be exposed to WebGL. Emulated
    // by default in shared renderer code.
    bool baseInstanceBaseVertexEmulated = true;

    // EXT_base_instance is emulated and should only be exposed to WebGL. Emulated by default in
    // shared renderer code.
    bool baseInstanceEmulated = true;
};

struct TypePrecision
{
    TypePrecision();
    TypePrecision(const TypePrecision &other);

    TypePrecision &operator=(const TypePrecision &other);

    void setIEEEFloat();
    void setIEEEHalfFloat();
    void setTwosComplementInt(unsigned int bits);
    void setSimulatedFloat(unsigned int range, unsigned int precision);
    void setSimulatedInt(unsigned int range);

    void get(GLint *returnRange, GLint *returnPrecision) const;

    std::array<GLint, 2> range = {0, 0};
    GLint precision            = 0;
};

struct Caps
{
    Caps();
    Caps(const Caps &other);
    Caps &operator=(const Caps &other);

    ~Caps();

    // If the values could be got by using GetIntegeri_v, they should
    // be GLint instead of GLuint and call LimitToInt() to ensure
    // they will not overflow.

    GLfloat minInterpolationOffset        = 0;
    GLfloat maxInterpolationOffset        = 0;
    GLint subPixelInterpolationOffsetBits = 0;

    // ES 3.1 (April 29, 2015) 20.39: implementation dependent values
    GLint64 maxElementIndex       = 0;
    GLint max3DTextureSize        = 0;
    GLint max2DTextureSize        = 0;
    GLint maxRectangleTextureSize = 0;
    GLint maxArrayTextureLayers   = 0;
    GLfloat maxLODBias            = 0.0f;
    GLint maxCubeMapTextureSize   = 0;
    GLint maxRenderbufferSize     = 0;
    GLfloat minAliasedPointSize   = 1.0f;
    GLfloat maxAliasedPointSize   = 1.0f;
    GLfloat minAliasedLineWidth   = 0.0f;
    GLfloat maxAliasedLineWidth   = 0.0f;

    // ES 3.1 (April 29, 2015) 20.40: implementation dependent values (cont.)
    GLint maxDrawBuffers         = 0;
    GLint maxFramebufferWidth    = 0;
    GLint maxFramebufferHeight   = 0;
    GLint maxFramebufferSamples  = 0;
    GLint maxColorAttachments    = 0;
    GLint maxViewportWidth       = 0;
    GLint maxViewportHeight      = 0;
    GLint maxSampleMaskWords     = 0;
    GLint maxColorTextureSamples = 0;
    GLint maxDepthTextureSamples = 0;
    GLint maxIntegerSamples      = 0;
    GLint64 maxServerWaitTimeout = 0;

    // ES 3.1 (April 29, 2015) Table 20.41: Implementation dependent values (cont.)
    GLint maxVertexAttribRelativeOffset = 0;
    GLint maxVertexAttribBindings       = 0;
    GLint maxVertexAttribStride         = 0;
    GLint maxElementsIndices            = 0;
    GLint maxElementsVertices           = 0;
    std::vector<GLenum> compressedTextureFormats;
    std::vector<GLenum> programBinaryFormats;
    std::vector<GLenum> shaderBinaryFormats;
    TypePrecision vertexHighpFloat;
    TypePrecision vertexMediumpFloat;
    TypePrecision vertexLowpFloat;
    TypePrecision vertexHighpInt;
    TypePrecision vertexMediumpInt;
    TypePrecision vertexLowpInt;
    TypePrecision fragmentHighpFloat;
    TypePrecision fragmentMediumpFloat;
    TypePrecision fragmentLowpFloat;
    TypePrecision fragmentHighpInt;
    TypePrecision fragmentMediumpInt;
    TypePrecision fragmentLowpInt;

    // Implementation dependent limits required on all shader types.
    // TODO(jiawei.shao@intel.com): organize all such limits into ShaderMap.
    // ES 3.1 (April 29, 2015) Table 20.43: Implementation dependent Vertex shader limits
    // ES 3.1 (April 29, 2015) Table 20.44: Implementation dependent Fragment shader limits
    // ES 3.1 (April 29, 2015) Table 20.45: implementation dependent compute shader limits
    // GL_EXT_geometry_shader (May 31, 2016) Table 20.43gs: Implementation dependent geometry shader
    // limits
    // GL_EXT_geometry_shader (May 31, 2016) Table 20.46: Implementation dependent aggregate shader
    // limits
    ShaderMap<GLint> maxShaderUniformBlocks        = {};
    ShaderMap<GLint> maxShaderTextureImageUnits    = {};
    ShaderMap<GLint> maxShaderStorageBlocks        = {};
    ShaderMap<GLint> maxShaderUniformComponents    = {};
    ShaderMap<GLint> maxShaderAtomicCounterBuffers = {};
    ShaderMap<GLint> maxShaderAtomicCounters       = {};
    ShaderMap<GLint> maxShaderImageUniforms        = {};
    // Note that we can query MAX_COMPUTE_UNIFORM_COMPONENTS and MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT
    // by GetIntegerv, but we can only use GetInteger64v on MAX_VERTEX_UNIFORM_COMPONENTS and
    // MAX_FRAGMENT_UNIFORM_COMPONENTS. Currently we use GLuint64 to store all these values so that
    // we can put them together into one ShaderMap.
    ShaderMap<GLint64> maxCombinedShaderUniformComponents = {};

    // ES 3.1 (April 29, 2015) Table 20.43: Implementation dependent Vertex shader limits
    GLint maxVertexAttributes       = 0;
    GLint maxVertexUniformVectors   = 0;
    GLint maxVertexOutputComponents = 0;

    // ES 3.1 (April 29, 2015) Table 20.44: Implementation dependent Fragment shader limits
    GLint maxFragmentUniformVectors     = 0;
    GLint maxFragmentInputComponents    = 0;
    GLint minProgramTextureGatherOffset = 0;
    GLint maxProgramTextureGatherOffset = 0;
    GLint minProgramTexelOffset         = 0;
    GLint maxProgramTexelOffset         = 0;

    // ES 3.1 (April 29, 2015) Table 20.45: implementation dependent compute shader limits
    std::array<GLint, 3> maxComputeWorkGroupCount = {0, 0, 0};
    std::array<GLint, 3> maxComputeWorkGroupSize  = {0, 0, 0};
    GLint maxComputeWorkGroupInvocations          = 0;
    GLint maxComputeSharedMemorySize              = 0;

    // ES 3.1 (April 29, 2015) Table 20.46: implementation dependent aggregate shader limits
    GLint maxUniformBufferBindings         = 0;
    GLint64 maxUniformBlockSize            = 0;
    GLint uniformBufferOffsetAlignment     = 0;
    GLint maxCombinedUniformBlocks         = 0;
    GLint maxVaryingComponents             = 0;
    GLint maxVaryingVectors                = 0;
    GLint maxCombinedTextureImageUnits     = 0;
    GLint maxCombinedShaderOutputResources = 0;

    // ES 3.1 (April 29, 2015) Table 20.47: implementation dependent aggregate shader limits (cont.)
    GLint maxUniformLocations                = 0;
    GLint maxAtomicCounterBufferBindings     = 0;
    GLint maxAtomicCounterBufferSize         = 0;
    GLint maxCombinedAtomicCounterBuffers    = 0;
    GLint maxCombinedAtomicCounters          = 0;
    GLint maxImageUnits                      = 0;
    GLint maxCombinedImageUniforms           = 0;
    GLint maxShaderStorageBufferBindings     = 0;
    GLint64 maxShaderStorageBlockSize        = 0;
    GLint maxCombinedShaderStorageBlocks     = 0;
    GLint shaderStorageBufferOffsetAlignment = 0;

    // ES 3.1 (April 29, 2015) Table 20.48: implementation dependent transform feedback limits
    GLint maxTransformFeedbackInterleavedComponents = 0;
    GLint maxTransformFeedbackSeparateAttributes    = 0;
    GLint maxTransformFeedbackSeparateComponents    = 0;

    // ES 3.1 (April 29, 2015) Table 20.49: Framebuffer Dependent Values
    GLint maxSamples = 0;

    // GL_EXT_geometry_shader (May 31, 2016) Table 20.40: Implementation-Dependent Values (cont.)
    GLint maxFramebufferLayers = 0;
    GLint layerProvokingVertex = 0;

    // GL_EXT_geometry_shader (May 31, 2016) Table 20.43gs: Implementation dependent geometry shader
    // limits
    GLint maxGeometryInputComponents       = 0;
    GLint maxGeometryOutputComponents      = 0;
    GLint maxGeometryOutputVertices        = 0;
    GLint maxGeometryTotalOutputComponents = 0;
    GLint maxGeometryShaderInvocations     = 0;

    // GL_EXT_tessellation_shader
    GLint maxTessControlInputComponents       = 0;
    GLint maxTessControlOutputComponents      = 0;
    GLint maxTessControlTotalOutputComponents = 0;

    GLint maxTessPatchComponents = 0;
    GLint maxPatchVertices       = 0;
    GLint maxTessGenLevel        = 0;

    GLint maxTessEvaluationInputComponents  = 0;
    GLint maxTessEvaluationOutputComponents = 0;

    bool primitiveRestartForPatchesSupported = false;

    GLuint subPixelBits = 4;

    // GL_EXT_blend_func_extended
    GLuint maxDualSourceDrawBuffers = 0;

    // GL_EXT_texture_filter_anisotropic
    GLfloat maxTextureAnisotropy = 0.0f;

    // GL_EXT_disjoint_timer_query
    GLuint queryCounterBitsTimeElapsed = 0;
    GLuint queryCounterBitsTimestamp   = 0;

    // OVR_multiview
    GLuint maxViews = 1;

    // GL_KHR_debug
    GLuint maxDebugMessageLength   = 0;
    GLuint maxDebugLoggedMessages  = 0;
    GLuint maxDebugGroupStackDepth = 0;
    GLuint maxLabelLength          = 0;

    // GL_APPLE_clip_distance / GL_EXT_clip_cull_distance / GL_ANGLE_clip_cull_distance
    GLuint maxClipDistances                = 0;
    GLuint maxCullDistances                = 0;
    GLuint maxCombinedClipAndCullDistances = 0;

    // GL_ANGLE_shader_pixel_local_storage
    GLuint maxPixelLocalStoragePlanes                       = 0;
    GLuint maxColorAttachmentsWithActivePixelLocalStorage   = 0;
    GLuint maxCombinedDrawBuffersAndPixelLocalStoragePlanes = 0;

    // GL_EXT_shader_pixel_local_storage.
    GLuint maxShaderPixelLocalStorageFastSizeEXT = 0;

    // GLES1 emulation: Caps for ES 1.1. Taken from Table 6.20 / 6.22 in the OpenGL ES 1.1 spec.
    GLuint maxMultitextureUnits                 = 0;
    GLuint maxClipPlanes                        = 0;
    GLuint maxLights                            = 0;
    static constexpr int GlobalMatrixStackDepth = 16;
    GLuint maxModelviewMatrixStackDepth         = 0;
    GLuint maxProjectionMatrixStackDepth        = 0;
    GLuint maxTextureMatrixStackDepth           = 0;
    GLfloat minSmoothPointSize                  = 0.0f;
    GLfloat maxSmoothPointSize                  = 0.0f;
    GLfloat minSmoothLineWidth                  = 0.0f;
    GLfloat maxSmoothLineWidth                  = 0.0f;

    // ES 3.2 Table 21.40: Implementation Dependent Values
    GLfloat lineWidthGranularity    = 0.0f;
    GLfloat minMultisampleLineWidth = 0.0f;
    GLfloat maxMultisampleLineWidth = 0.0f;

    // ES 3.2 Table 21.42: Implementation Dependent Values (cont.)
    GLint maxTextureBufferSize         = 0;
    GLint textureBufferOffsetAlignment = 0;

    // GL_ARM_shader_framebuffer_fetch
    bool fragmentShaderFramebufferFetchMRT = false;
};

Caps GenerateMinimumCaps(const Version &clientVersion, const Extensions &extensions);
}  // namespace gl

namespace egl
{

struct Caps
{
    Caps();

    // Support for NPOT surfaces
    bool textureNPOT = false;

    // Support for Stencil8 configs
    bool stencil8 = false;
};

struct DisplayExtensions
{
    DisplayExtensions();

    // Generate a vector of supported extension strings
    std::vector<std::string> getStrings() const;

    // EGL_EXT_create_context_robustness
    bool createContextRobustness = false;

    // EGL_ANGLE_d3d_share_handle_client_buffer
    bool d3dShareHandleClientBuffer = false;

    // EGL_ANGLE_d3d_texture_client_buffer
    bool d3dTextureClientBuffer = false;

    // EGL_ANGLE_surface_d3d_texture_2d_share_handle
    bool surfaceD3DTexture2DShareHandle = false;

    // EGL_ANGLE_query_surface_pointer
    bool querySurfacePointer = false;

    // EGL_ANGLE_window_fixed_size
    bool windowFixedSize = false;

    // EGL_ANGLE_keyed_mutex
    bool keyedMutex = false;

    // EGL_ANGLE_surface_orientation
    bool surfaceOrientation = false;

    // EGL_NV_post_sub_buffer
    bool postSubBuffer = false;

    // EGL_KHR_create_context
    bool createContext = false;

    // EGL_KHR_image
    bool image = false;

    // EGL_KHR_image_base
    bool imageBase = false;

    // EGL_KHR_image_pixmap
    bool imagePixmap = false;

    // EGL_KHR_gl_texture_2D_image
    bool glTexture2DImage = false;

    // EGL_KHR_gl_texture_cubemap_image
    bool glTextureCubemapImage = false;

    // EGL_KHR_gl_texture_3D_image
    bool glTexture3DImage = false;

    // EGL_KHR_gl_renderbuffer_image
    bool glRenderbufferImage = false;

    // EGL_KHR_get_all_proc_addresses
    bool getAllProcAddresses = false;

    // EGL_ANGLE_direct_composition
    bool directComposition = false;

    // EGL_ANGLE_windows_ui_composition
    bool windowsUIComposition = false;

    // KHR_create_context_no_error
    bool createContextNoError = false;

    // EGL_KHR_stream
    bool stream = false;

    // EGL_KHR_stream_consumer_gltexture
    bool streamConsumerGLTexture = false;

    // EGL_NV_stream_consumer_gltexture_yuv
    bool streamConsumerGLTextureYUV = false;

    // EGL_ANGLE_stream_producer_d3d_texture
    bool streamProducerD3DTexture = false;

    // EGL_KHR_fence_sync
    bool fenceSync = false;

    // EGL_KHR_wait_sync
    bool waitSync = false;

    // EGL_ANGLE_create_context_webgl_compatibility
    bool createContextWebGLCompatibility = false;

    // EGL_CHROMIUM_create_context_bind_generates_resource
    bool createContextBindGeneratesResource = false;

    // EGL_CHROMIUM_sync_control
    bool syncControlCHROMIUM = false;

    // EGL_ANGLE_sync_control_rate
    bool syncControlRateANGLE = false;

    // EGL_KHR_swap_buffers_with_damage
    bool swapBuffersWithDamage = false;

    // EGL_EXT_pixel_format_float
    bool pixelFormatFloat = false;

    // EGL_KHR_surfaceless_context
    bool surfacelessContext = false;

    // EGL_ANGLE_display_texture_share_group
    bool displayTextureShareGroup = false;

    // EGL_ANGLE_display_semaphore_share_group
    bool displaySemaphoreShareGroup = false;

    // EGL_ANGLE_create_context_client_arrays
    bool createContextClientArrays = false;

    // EGL_ANGLE_program_cache_control
    bool programCacheControlANGLE = false;

    // EGL_ANGLE_robust_resource_initialization
    bool robustResourceInitializationANGLE = false;

    // EGL_ANGLE_iosurface_client_buffer
    bool iosurfaceClientBuffer = false;

    // EGL_ANGLE_metal_texture_client_buffer
    bool mtlTextureClientBuffer = false;

    // EGL_ANGLE_create_context_extensions_enabled
    bool createContextExtensionsEnabled = false;

    // EGL_ANDROID_presentation_time
    bool presentationTime = false;

    // EGL_ANDROID_blob_cache
    bool blobCache = false;

    // EGL_ANDROID_image_native_buffer
    bool imageNativeBuffer = false;

    // EGL_ANDROID_get_frame_timestamps
    bool getFrameTimestamps = false;

    // EGL_ANDROID_front_buffer_auto_refresh
    bool frontBufferAutoRefreshANDROID = false;

    // EGL_ANGLE_timestamp_surface_attribute
    bool timestampSurfaceAttributeANGLE = false;

    // EGL_ANDROID_recordable
    bool recordable = false;

    // EGL_ANGLE_power_preference
    bool powerPreference = false;

    // EGL_ANGLE_wait_until_work_scheduled
    bool waitUntilWorkScheduled = false;

    // EGL_ANGLE_image_d3d11_texture
    bool imageD3D11Texture = false;

    // EGL_ANDROID_get_native_client_buffer
    bool getNativeClientBufferANDROID = false;

    // EGL_ANDROID_create_native_client_buffer
    bool createNativeClientBufferANDROID = false;

    // EGL_ANDROID_native_fence_sync
    bool nativeFenceSyncANDROID = false;

    // EGL_ANGLE_create_context_backwards_compatible
    bool createContextBackwardsCompatible = false;

    // EGL_KHR_no_config_context
    bool noConfigContext = false;

    // EGL_IMG_context_priority
    bool contextPriority = false;

    // EGL_ANGLE_ggp_stream_descriptor
    bool ggpStreamDescriptor = false;

    // EGL_ANGLE_swap_with_frame_token
    bool swapWithFrameToken = false;

    // EGL_KHR_gl_colorspace
    bool glColorspace = false;

    // EGL_EXT_gl_colorspace_display_p3_linear
    bool glColorspaceDisplayP3Linear = false;

    // EGL_EXT_gl_colorspace_display_p3
    bool glColorspaceDisplayP3 = false;

    // EGL_EXT_gl_colorspace_scrgb
    bool glColorspaceScrgb = false;

    // EGL_EXT_gl_colorspace_scrgb_linear
    bool glColorspaceScrgbLinear = false;

    // EGL_EXT_gl_colorspace_display_p3_passthrough
    bool glColorspaceDisplayP3Passthrough = false;

    // EGL_ANGLE_colorspace_attribute_passthrough
    bool eglColorspaceAttributePassthroughANGLE = false;

    // EGL_EXT_gl_colorspace_bt2020_linear
    bool glColorspaceBt2020Linear = false;

    // EGL_EXT_gl_colorspace_bt2020_pq
    bool glColorspaceBt2020Pq = false;

    // EGL_EXT_gl_colorspace_bt2020_hlg
    bool glColorspaceBt2020Hlg = false;

    // EGL_ANDROID_framebuffer_target
    bool framebufferTargetANDROID = false;

    // EGL_EXT_image_gl_colorspace
    bool imageGlColorspace = false;

    // EGL_EXT_image_dma_buf_import
    bool imageDmaBufImportEXT = false;

    // EGL_EXT_image_dma_buf_import_modifiers
    bool imageDmaBufImportModifiersEXT = false;

    // EGL_NOK_texture_from_pixmap
    bool textureFromPixmapNOK = false;

    // EGL_NV_robustness_video_memory_purge
    bool robustnessVideoMemoryPurgeNV = false;

    // EGL_KHR_reusable_sync
    bool reusableSyncKHR = false;

    // EGL_ANGLE_external_context_and_surface
    bool externalContextAndSurface = false;

    // EGL_EXT_buffer_age
    bool bufferAgeEXT = false;

    // EGL_KHR_mutable_render_buffer
    bool mutableRenderBufferKHR = false;

    // EGL_EXT_protected_content
    bool protectedContentEXT = false;

    // EGL_ANGLE_create_surface_swap_interval
    bool createSurfaceSwapIntervalANGLE = false;

    // EGL_ANGLE_context_virtualization
    bool contextVirtualizationANGLE = false;

    // EGL_KHR_lock_surface3
    bool lockSurface3KHR = false;

    // EGL_ANGLE_vulkan_image
    bool vulkanImageANGLE = false;

    // EGL_ANGLE_metal_create_context_ownership_identity
    bool metalCreateContextOwnershipIdentityANGLE = false;

    // EGL_KHR_partial_update
    bool partialUpdateKHR = false;

    // EGL_ANGLE_metal_shared_event_sync
    bool mtlSyncSharedEventANGLE = false;

    // EGL_ANGLE_global_fence_sync
    bool globalFenceSyncANGLE = false;

    // EGL_ANGLE_memory_usage_report
    bool memoryUsageReportANGLE = false;

    // EGL_EXT_surface_compression
    bool surfaceCompressionEXT = false;
};

struct DeviceExtensions
{
    DeviceExtensions();

    // Generate a vector of supported extension strings
    std::vector<std::string> getStrings() const;

    // EGL_ANGLE_device_d3d
    bool deviceD3D = false;

    // EGL_ANGLE_device_d3d9
    bool deviceD3D9 = false;

    // EGL_ANGLE_device_d3d11
    bool deviceD3D11 = false;

    // EGL_ANGLE_device_cgl
    bool deviceCGL = false;

    // EGL_ANGLE_device_metal
    bool deviceMetal = false;

    // EGL_ANGLE_device_vulkan
    bool deviceVulkan = false;

    // EGL_EXT_device_drm
    bool deviceDrmEXT = false;

    // EGL_EXT_device_drm_render_node
    bool deviceDrmRenderNodeEXT = false;
};

struct ClientExtensions
{
    ClientExtensions();
    ClientExtensions(const ClientExtensions &other);

    // Generate a vector of supported extension strings
    std::vector<std::string> getStrings() const;

    // EGL_EXT_client_extensions
    bool clientExtensions = false;

    // EGL_EXT_platform_base
    bool platformBase = false;

    // EGL_EXT_platform_device
    bool platformDevice = false;

    // EGL_KHR_platform_gbm
    bool platformGbmKHR = false;

    // EGL_EXT_platform_wayland
    bool platformWaylandEXT = false;

    // EGL_MESA_platform_surfaceless
    bool platformSurfacelessMESA = false;

    // EGL_ANGLE_platform_angle
    bool platformANGLE = false;

    // EGL_ANGLE_platform_angle_d3d
    bool platformANGLED3D = false;

    // EGL_ANGLE_platform_angle_d3d11on12
    bool platformANGLED3D11ON12 = false;

    // EGL_ANGLE_platform_angle_d3d_luid
    bool platformANGLED3DLUID = false;

    // EGL_ANGLE_platform_angle_opengl
    bool platformANGLEOpenGL = false;

    // EGL_ANGLE_platform_angle_null
    bool platformANGLENULL = false;

    // EGL_ANGLE_platform_angle_webgpu
    bool platformANGLEWebgpu = false;

    // EGL_ANGLE_platform_angle_vulkan
    bool platformANGLEVulkan = false;

    // EGL_ANGLE_platform_angle_vulkan_device_uuid
    bool platformANGLEVulkanDeviceUUID = false;

    // EGL_ANGLE_platform_angle_metal
    bool platformANGLEMetal = false;

    // EGL_ANGLE_platform_angle_device_context_volatile_cgl
    bool platformANGLEDeviceContextVolatileCgl = false;

    // EGL_ANGLE_platform_angle_device_id
    bool platformANGLEDeviceId = false;

    // EGL_ANGLE_device_creation
    bool deviceCreation = false;

    // EGL_ANGLE_device_creation_d3d11
    bool deviceCreationD3D11 = false;

    // EGL_ANGLE_x11_visual
    bool x11Visual = false;

    // EGL_ANGLE_experimental_present_path
    bool experimentalPresentPath = false;

    // EGL_KHR_client_get_all_proc_addresses
    bool clientGetAllProcAddresses = false;

    // EGL_KHR_debug
    bool debug = false;

    // EGL_ANGLE_feature_control
    bool featureControlANGLE = false;

    // EGL_ANGLE_platform_angle_device_type_swiftshader
    bool platformANGLEDeviceTypeSwiftShader = false;

    // EGL_ANGLE_platform_angle_device_type_egl_angle
    bool platformANGLEDeviceTypeEGLANGLE = false;

    // EGL_EXT_device_query
    bool deviceQueryEXT = false;

    // EGL_ANGLE_display_power_preference
    bool displayPowerPreferenceANGLE = false;

    // EGL_ANGLE_no_error
    bool noErrorANGLE = false;
};

}  // namespace egl

#endif  // LIBANGLE_CAPS_H_
