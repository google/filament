//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Compiler.cpp: implements the gl::Compiler class.

#include "libANGLE/Compiler.h"

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/State.h"
#include "libANGLE/renderer/CompilerImpl.h"
#include "libANGLE/renderer/GLImplFactory.h"

namespace gl
{

namespace
{

// To know when to call sh::Initialize and sh::Finalize.
size_t gActiveCompilers = 0;

}  // anonymous namespace

Compiler::Compiler(rx::GLImplFactory *implFactory, const State &state, egl::Display *display)
    : mImplementation(implFactory->createCompiler()),
      mSpec(SelectShaderSpec(state)),
      mOutputType(mImplementation->getTranslatorOutputType()),
      mResources()
{
    // TODO(http://anglebug.com/42262462): Update for GL version specific validation
    ASSERT(state.getClientMajorVersion() == 1 || state.getClientMajorVersion() == 2 ||
           state.getClientMajorVersion() == 3 || state.getClientMajorVersion() == 4);

    {
        std::lock_guard<angle::SimpleMutex> lock(display->getDisplayGlobalMutex());
        if (gActiveCompilers == 0)
        {
            sh::Initialize();
        }
        ++gActiveCompilers;
    }

    const Caps &caps             = state.getCaps();
    const Extensions &extensions = state.getExtensions();

    sh::InitBuiltInResources(&mResources);
    mResources.MaxVertexAttribs             = caps.maxVertexAttributes;
    mResources.MaxVertexUniformVectors      = caps.maxVertexUniformVectors;
    mResources.MaxVaryingVectors            = caps.maxVaryingVectors;
    mResources.MaxVertexTextureImageUnits   = caps.maxShaderTextureImageUnits[ShaderType::Vertex];
    mResources.MaxCombinedTextureImageUnits = caps.maxCombinedTextureImageUnits;
    mResources.MaxTextureImageUnits         = caps.maxShaderTextureImageUnits[ShaderType::Fragment];
    mResources.MaxFragmentUniformVectors    = caps.maxFragmentUniformVectors;
    mResources.MaxDrawBuffers               = caps.maxDrawBuffers;
    mResources.OES_standard_derivatives     = extensions.standardDerivativesOES;
    mResources.EXT_draw_buffers             = extensions.drawBuffersEXT;
    mResources.EXT_shader_texture_lod       = extensions.shaderTextureLodEXT;
    mResources.EXT_shader_non_constant_global_initializers =
        extensions.shaderNonConstantGlobalInitializersEXT;
    mResources.OES_EGL_image_external          = extensions.EGLImageExternalOES;
    mResources.OES_EGL_image_external_essl3    = extensions.EGLImageExternalEssl3OES;
    mResources.NV_EGL_stream_consumer_external = extensions.EGLStreamConsumerExternalNV;
    mResources.NV_shader_noperspective_interpolation =
        extensions.shaderNoperspectiveInterpolationNV;
    mResources.ARB_texture_rectangle = extensions.textureRectangleANGLE;
    mResources.EXT_gpu_shader5       = extensions.gpuShader5EXT;
    mResources.OES_gpu_shader5       = extensions.gpuShader5OES;
    mResources.OES_shader_io_blocks  = extensions.shaderIoBlocksOES;
    mResources.EXT_shader_io_blocks  = extensions.shaderIoBlocksEXT;
    mResources.OES_texture_storage_multisample_2d_array =
        extensions.textureStorageMultisample2dArrayOES;
    mResources.OES_texture_3D = extensions.texture3DOES;
    mResources.ANGLE_base_vertex_base_instance_shader_builtin =
        extensions.baseVertexBaseInstanceShaderBuiltinANGLE;
    mResources.ANGLE_multi_draw                 = extensions.multiDrawANGLE;
    mResources.ANGLE_shader_pixel_local_storage = extensions.shaderPixelLocalStorageANGLE;
    mResources.ANGLE_texture_multisample        = extensions.textureMultisampleANGLE;
    mResources.APPLE_clip_distance              = extensions.clipDistanceAPPLE;
    // OES_shader_multisample_interpolation
    mResources.OES_shader_multisample_interpolation = extensions.shaderMultisampleInterpolationOES;
    mResources.OES_shader_image_atomic              = extensions.shaderImageAtomicOES;
    // TODO: use shader precision caps to determine if high precision is supported?
    mResources.FragmentPrecisionHigh = 1;
    mResources.EXT_frag_depth        = extensions.fragDepthEXT;

    // OVR_multiview state
    mResources.OVR_multiview = extensions.multiviewOVR;

    // OVR_multiview2 state
    mResources.OVR_multiview2 = extensions.multiview2OVR;
    mResources.MaxViewsOVR    = caps.maxViews;

    // EXT_multisampled_render_to_texture and EXT_multisampled_render_to_texture2
    mResources.EXT_multisampled_render_to_texture  = extensions.multisampledRenderToTextureEXT;
    mResources.EXT_multisampled_render_to_texture2 = extensions.multisampledRenderToTexture2EXT;

    // WEBGL_video_texture
    mResources.WEBGL_video_texture = extensions.videoTextureWEBGL;

    // OES_texture_cube_map_array
    mResources.OES_texture_cube_map_array = extensions.textureCubeMapArrayOES;
    mResources.EXT_texture_cube_map_array = extensions.textureCubeMapArrayEXT;

    // EXT_texture_query_lod
    mResources.EXT_texture_query_lod = extensions.textureQueryLodEXT;

    // EXT_texture_shadow_lod
    mResources.EXT_texture_shadow_lod = extensions.textureShadowLodEXT;

    // EXT_shadow_samplers
    mResources.EXT_shadow_samplers = extensions.shadowSamplersEXT;

    // OES_texture_buffer
    mResources.OES_texture_buffer = extensions.textureBufferOES;
    mResources.EXT_texture_buffer = extensions.textureBufferEXT;

    // GL_EXT_YUV_target
    mResources.EXT_YUV_target = extensions.YUVTargetEXT;

    mResources.EXT_shader_framebuffer_fetch_non_coherent =
        extensions.shaderFramebufferFetchNonCoherentEXT;

    mResources.EXT_shader_framebuffer_fetch = extensions.shaderFramebufferFetchEXT;

    // GL_EXT_clip_cull_distance
    mResources.EXT_clip_cull_distance = extensions.clipCullDistanceEXT;

    // GL_ANGLE_clip_cull_distance
    mResources.ANGLE_clip_cull_distance = extensions.clipCullDistanceANGLE;

    // GL_EXT_primitive_bounding_box
    mResources.EXT_primitive_bounding_box = extensions.primitiveBoundingBoxEXT;

    // GL_OES_primitive_bounding_box
    mResources.OES_primitive_bounding_box = extensions.primitiveBoundingBoxOES;

    // GL_EXT_separate_shader_objects
    mResources.EXT_separate_shader_objects = extensions.separateShaderObjectsEXT;

    // GL_ARM_shader_framebuffer_fetch
    mResources.ARM_shader_framebuffer_fetch = extensions.shaderFramebufferFetchARM;

    // GL_ARM_shader_framebuffer_fetch_depth_stencil
    mResources.ARM_shader_framebuffer_fetch_depth_stencil =
        extensions.shaderFramebufferFetchDepthStencilARM;

    // GLSL ES 3.0 constants
    mResources.MaxVertexOutputVectors  = caps.maxVertexOutputComponents / 4;
    mResources.MaxFragmentInputVectors = caps.maxFragmentInputComponents / 4;
    mResources.MinProgramTexelOffset   = caps.minProgramTexelOffset;
    mResources.MaxProgramTexelOffset   = caps.maxProgramTexelOffset;

    // EXT_blend_func_extended
    mResources.EXT_blend_func_extended  = extensions.blendFuncExtendedEXT;
    mResources.MaxDualSourceDrawBuffers = caps.maxDualSourceDrawBuffers;

    // EXT_conservative_depth
    mResources.EXT_conservative_depth = extensions.conservativeDepthEXT;

    // APPLE_clip_distance / EXT_clip_cull_distance / ANGLE_clip_cull_distance
    mResources.MaxClipDistances                = caps.maxClipDistances;
    mResources.MaxCullDistances                = caps.maxCullDistances;
    mResources.MaxCombinedClipAndCullDistances = caps.maxCombinedClipAndCullDistances;

    // ANGLE_shader_pixel_local_storage.
    mResources.MaxPixelLocalStoragePlanes = caps.maxPixelLocalStoragePlanes;
    mResources.MaxColorAttachmentsWithActivePixelLocalStorage =
        caps.maxColorAttachmentsWithActivePixelLocalStorage;
    mResources.MaxCombinedDrawBuffersAndPixelLocalStoragePlanes =
        caps.maxCombinedDrawBuffersAndPixelLocalStoragePlanes;

    // OES_sample_variables
    mResources.OES_sample_variables = extensions.sampleVariablesOES;
    mResources.MaxSamples           = caps.maxSamples;

    // ANDROID_extension_pack_es31a
    mResources.ANDROID_extension_pack_es31a = extensions.extensionPackEs31aANDROID;

    // KHR_blend_equation_advanced
    mResources.KHR_blend_equation_advanced = extensions.blendEquationAdvancedKHR;

    // GLSL ES 3.1 constants
    mResources.MaxProgramTextureGatherOffset    = caps.maxProgramTextureGatherOffset;
    mResources.MinProgramTextureGatherOffset    = caps.minProgramTextureGatherOffset;
    mResources.MaxImageUnits                    = caps.maxImageUnits;
    mResources.MaxVertexImageUniforms           = caps.maxShaderImageUniforms[ShaderType::Vertex];
    mResources.MaxFragmentImageUniforms         = caps.maxShaderImageUniforms[ShaderType::Fragment];
    mResources.MaxComputeImageUniforms          = caps.maxShaderImageUniforms[ShaderType::Compute];
    mResources.MaxCombinedImageUniforms         = caps.maxCombinedImageUniforms;
    mResources.MaxCombinedShaderOutputResources = caps.maxCombinedShaderOutputResources;
    mResources.MaxUniformLocations              = caps.maxUniformLocations;

    for (size_t index = 0u; index < 3u; ++index)
    {
        mResources.MaxComputeWorkGroupCount[index] = caps.maxComputeWorkGroupCount[index];
        mResources.MaxComputeWorkGroupSize[index]  = caps.maxComputeWorkGroupSize[index];
    }

    mResources.MaxComputeUniformComponents = caps.maxShaderUniformComponents[ShaderType::Compute];
    mResources.MaxComputeTextureImageUnits = caps.maxShaderTextureImageUnits[ShaderType::Compute];

    mResources.MaxComputeAtomicCounters = caps.maxShaderAtomicCounters[ShaderType::Compute];
    mResources.MaxComputeAtomicCounterBuffers =
        caps.maxShaderAtomicCounterBuffers[ShaderType::Compute];

    mResources.MaxVertexAtomicCounters   = caps.maxShaderAtomicCounters[ShaderType::Vertex];
    mResources.MaxFragmentAtomicCounters = caps.maxShaderAtomicCounters[ShaderType::Fragment];
    mResources.MaxCombinedAtomicCounters = caps.maxCombinedAtomicCounters;
    mResources.MaxAtomicCounterBindings  = caps.maxAtomicCounterBufferBindings;
    mResources.MaxVertexAtomicCounterBuffers =
        caps.maxShaderAtomicCounterBuffers[ShaderType::Vertex];
    mResources.MaxFragmentAtomicCounterBuffers =
        caps.maxShaderAtomicCounterBuffers[ShaderType::Fragment];
    mResources.MaxCombinedAtomicCounterBuffers = caps.maxCombinedAtomicCounterBuffers;
    mResources.MaxAtomicCounterBufferSize      = caps.maxAtomicCounterBufferSize;

    mResources.MaxUniformBufferBindings       = caps.maxUniformBufferBindings;
    mResources.MaxShaderStorageBufferBindings = caps.maxShaderStorageBufferBindings;

    // Needed by point size clamping workaround
    mResources.MinPointSize = caps.minAliasedPointSize;
    mResources.MaxPointSize = caps.maxAliasedPointSize;

    if (state.getClientMajorVersion() == 2 && !extensions.drawBuffersEXT)
    {
        mResources.MaxDrawBuffers = 1;
    }

    // Geometry Shader constants
    mResources.EXT_geometry_shader          = extensions.geometryShaderEXT;
    mResources.OES_geometry_shader          = extensions.geometryShaderOES;
    mResources.MaxGeometryUniformComponents = caps.maxShaderUniformComponents[ShaderType::Geometry];
    mResources.MaxGeometryUniformBlocks     = caps.maxShaderUniformBlocks[ShaderType::Geometry];
    mResources.MaxGeometryInputComponents   = caps.maxGeometryInputComponents;
    mResources.MaxGeometryOutputComponents  = caps.maxGeometryOutputComponents;
    mResources.MaxGeometryOutputVertices    = caps.maxGeometryOutputVertices;
    mResources.MaxGeometryTotalOutputComponents = caps.maxGeometryTotalOutputComponents;
    mResources.MaxGeometryTextureImageUnits = caps.maxShaderTextureImageUnits[ShaderType::Geometry];

    mResources.MaxGeometryAtomicCounterBuffers =
        caps.maxShaderAtomicCounterBuffers[ShaderType::Geometry];
    mResources.MaxGeometryAtomicCounters      = caps.maxShaderAtomicCounters[ShaderType::Geometry];
    mResources.MaxGeometryShaderStorageBlocks = caps.maxShaderStorageBlocks[ShaderType::Geometry];
    mResources.MaxGeometryShaderInvocations   = caps.maxGeometryShaderInvocations;
    mResources.MaxGeometryImageUniforms       = caps.maxShaderImageUniforms[ShaderType::Geometry];

    // Tessellation Shader constants
    mResources.EXT_tessellation_shader        = extensions.tessellationShaderEXT;
    mResources.OES_tessellation_shader        = extensions.tessellationShaderOES;
    mResources.MaxTessControlInputComponents  = caps.maxTessControlInputComponents;
    mResources.MaxTessControlOutputComponents = caps.maxTessControlOutputComponents;
    mResources.MaxTessControlTextureImageUnits =
        caps.maxShaderTextureImageUnits[ShaderType::TessControl];
    mResources.MaxTessControlUniformComponents =
        caps.maxShaderUniformComponents[ShaderType::TessControl];
    mResources.MaxTessControlTotalOutputComponents = caps.maxTessControlTotalOutputComponents;
    mResources.MaxTessControlImageUniforms  = caps.maxShaderImageUniforms[ShaderType::TessControl];
    mResources.MaxTessControlAtomicCounters = caps.maxShaderAtomicCounters[ShaderType::TessControl];
    mResources.MaxTessControlAtomicCounterBuffers =
        caps.maxShaderAtomicCounterBuffers[ShaderType::TessControl];

    mResources.MaxTessPatchComponents = caps.maxTessPatchComponents;
    mResources.MaxPatchVertices       = caps.maxPatchVertices;
    mResources.MaxTessGenLevel        = caps.maxTessGenLevel;

    mResources.MaxTessEvaluationInputComponents  = caps.maxTessEvaluationInputComponents;
    mResources.MaxTessEvaluationOutputComponents = caps.maxTessEvaluationOutputComponents;
    mResources.MaxTessEvaluationTextureImageUnits =
        caps.maxShaderTextureImageUnits[ShaderType::TessEvaluation];
    mResources.MaxTessEvaluationUniformComponents =
        caps.maxShaderUniformComponents[ShaderType::TessEvaluation];
    mResources.MaxTessEvaluationImageUniforms =
        caps.maxShaderImageUniforms[ShaderType::TessEvaluation];
    mResources.MaxTessEvaluationAtomicCounters =
        caps.maxShaderAtomicCounters[ShaderType::TessEvaluation];
    mResources.MaxTessEvaluationAtomicCounterBuffers =
        caps.maxShaderAtomicCounterBuffers[ShaderType::TessEvaluation];

    // Subpixel bits.
    mResources.SubPixelBits = static_cast<int>(caps.subPixelBits);
}

Compiler::~Compiler() = default;

void Compiler::onDestroy(const Context *context)
{
    std::lock_guard<angle::SimpleMutex> lock(context->getDisplay()->getDisplayGlobalMutex());
    for (auto &pool : mPools)
    {
        for (ShCompilerInstance &instance : pool)
        {
            instance.destroy();
        }
    }
    --gActiveCompilers;
    if (gActiveCompilers == 0)
    {
        sh::Finalize();
    }
}

ShCompilerInstance Compiler::getInstance(ShaderType type)
{
    ASSERT(type != ShaderType::InvalidEnum);
    auto &pool = mPools[type];
    if (pool.empty())
    {
        ShHandle handle = sh::ConstructCompiler(ToGLenum(type), mSpec, mOutputType, &mResources);
        ASSERT(handle);
        return ShCompilerInstance(handle, mOutputType, type);
    }
    else
    {
        ShCompilerInstance instance = std::move(pool.back());
        pool.pop_back();
        return instance;
    }
}

void Compiler::putInstance(ShCompilerInstance &&instance)
{
    static constexpr size_t kMaxPoolSize = 32;
    auto &pool                           = mPools[instance.getShaderType()];
    if (pool.size() < kMaxPoolSize)
    {
        pool.push_back(std::move(instance));
    }
    else
    {
        instance.destroy();
    }
}

ShShaderSpec Compiler::SelectShaderSpec(const State &state)
{
    const GLint majorVersion = state.getClientMajorVersion();
    const GLint minorVersion = state.getClientMinorVersion();
    bool isWebGL             = state.isWebGL();

    if (majorVersion >= 3)
    {
        switch (minorVersion)
        {
            case 2:
                ASSERT(!isWebGL);
                return SH_GLES3_2_SPEC;
            case 1:
                return isWebGL ? SH_WEBGL3_SPEC : SH_GLES3_1_SPEC;
            case 0:
                return isWebGL ? SH_WEBGL2_SPEC : SH_GLES3_SPEC;
            default:
                UNREACHABLE();
        }
    }

    // GLES1 emulation: Use GLES3 shader spec.
    if (!isWebGL && majorVersion == 1)
    {
        return SH_GLES3_SPEC;
    }

    return isWebGL ? SH_WEBGL_SPEC : SH_GLES2_SPEC;
}

ShCompilerInstance::ShCompilerInstance() : mHandle(nullptr) {}

ShCompilerInstance::ShCompilerInstance(ShHandle handle,
                                       ShShaderOutput outputType,
                                       ShaderType shaderType)
    : mHandle(handle), mOutputType(outputType), mShaderType(shaderType)
{}

ShCompilerInstance::~ShCompilerInstance()
{
    ASSERT(mHandle == nullptr);
}

void ShCompilerInstance::destroy()
{
    if (mHandle != nullptr)
    {
        sh::Destruct(mHandle);
        mHandle = nullptr;
    }
}

ShCompilerInstance::ShCompilerInstance(ShCompilerInstance &&other)
    : mHandle(other.mHandle), mOutputType(other.mOutputType), mShaderType(other.mShaderType)
{
    other.mHandle = nullptr;
}

ShCompilerInstance &ShCompilerInstance::operator=(ShCompilerInstance &&other)
{
    mHandle       = other.mHandle;
    mOutputType   = other.mOutputType;
    mShaderType   = other.mShaderType;
    other.mHandle = nullptr;
    return *this;
}

ShHandle ShCompilerInstance::getHandle()
{
    return mHandle;
}

ShaderType ShCompilerInstance::getShaderType() const
{
    return mShaderType;
}

ShBuiltInResources ShCompilerInstance::getBuiltInResources() const
{
    return sh::GetBuiltInResources(mHandle);
}

ShShaderOutput ShCompilerInstance::getShaderOutputType() const
{
    return mOutputType;
}

}  // namespace gl
