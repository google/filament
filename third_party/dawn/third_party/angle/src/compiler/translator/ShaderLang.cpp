//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//
// Implement the top-level of interface to the compiler,
// as defined in ShaderLang.h
//

#include "GLSLANG/ShaderLang.h"

#include "common/PackedEnums.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/InitializeDll.h"
#include "compiler/translator/length_limits.h"
#ifdef ANGLE_ENABLE_HLSL
#    include "compiler/translator/hlsl/TranslatorHLSL.h"
#endif  // ANGLE_ENABLE_HLSL
#include "angle_gl.h"
#include "compiler/translator/VariablePacker.h"

namespace sh
{

namespace
{

bool isInitialized = false;

//
// This is the platform independent interface between an OGL driver
// and the shading language compiler.
//

template <typename VarT>
const std::vector<VarT> *GetVariableList(const TCompiler *compiler);

template <>
const std::vector<InterfaceBlock> *GetVariableList(const TCompiler *compiler)
{
    return &compiler->getInterfaceBlocks();
}

TCompiler *GetCompilerFromHandle(ShHandle handle)
{
    if (!handle)
    {
        return nullptr;
    }

    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    return base->getAsCompiler();
}

template <typename VarT>
const std::vector<VarT> *GetShaderVariables(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    if (!compiler)
    {
        return nullptr;
    }

    return GetVariableList<VarT>(compiler);
}

#ifdef ANGLE_ENABLE_HLSL
TranslatorHLSL *GetTranslatorHLSLFromHandle(ShHandle handle)
{
    if (!handle)
        return nullptr;
    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    return base->getAsTranslatorHLSL();
}
#endif  // ANGLE_ENABLE_HLSL

GLenum GetGeometryShaderPrimitiveTypeEnum(sh::TLayoutPrimitiveType primitiveType)
{
    switch (primitiveType)
    {
        case EptPoints:
            return GL_POINTS;
        case EptLines:
            return GL_LINES;
        case EptLinesAdjacency:
            return GL_LINES_ADJACENCY_EXT;
        case EptTriangles:
            return GL_TRIANGLES;
        case EptTrianglesAdjacency:
            return GL_TRIANGLES_ADJACENCY_EXT;

        case EptLineStrip:
            return GL_LINE_STRIP;
        case EptTriangleStrip:
            return GL_TRIANGLE_STRIP;

        case EptUndefined:
        default:
            UNREACHABLE();
            return GL_INVALID_VALUE;
    }
}

GLenum GetTessellationShaderTypeEnum(sh::TLayoutTessEvaluationType type)
{
    switch (type)
    {
        case EtetTriangles:
            return GL_TRIANGLES;
        case EtetQuads:
            return GL_QUADS;
        case EtetIsolines:
            return GL_ISOLINES;
        case EtetEqualSpacing:
            return GL_EQUAL;
        case EtetFractionalEvenSpacing:
            return GL_FRACTIONAL_EVEN;
        case EtetFractionalOddSpacing:
            return GL_FRACTIONAL_ODD;
        case EtetCw:
            return GL_CW;
        case EtetCcw:
            return GL_CCW;
        case EtetPointMode:
            return GL_TESS_GEN_POINT_MODE;

        case EtetUndefined:
        default:
            UNREACHABLE();
            return GL_INVALID_VALUE;
    }
}

}  // anonymous namespace

//
// Driver must call this first, once, before doing any other compiler operations.
// Subsequent calls to this function are no-op.
//
bool Initialize()
{
    if (!isInitialized)
    {
        isInitialized = InitProcess();
    }
    return isInitialized;
}

//
// Cleanup symbol tables
//
bool Finalize()
{
    if (isInitialized)
    {
        DetachProcess();
        isInitialized = false;
    }
    return true;
}

//
// Initialize built-in resources with minimum expected values.
//
void InitBuiltInResources(ShBuiltInResources *resources)
{
    // Make comparable.
    memset(resources, 0, sizeof(*resources));

    // Constants.
    resources->MaxVertexAttribs             = 8;
    resources->MaxVertexUniformVectors      = 128;
    resources->MaxVaryingVectors            = 8;
    resources->MaxVertexTextureImageUnits   = 0;
    resources->MaxCombinedTextureImageUnits = 8;
    resources->MaxTextureImageUnits         = 8;
    resources->MaxFragmentUniformVectors    = 16;
    resources->MaxDrawBuffers               = 1;

    // Extensions.
    resources->OES_standard_derivatives                       = 0;
    resources->OES_EGL_image_external                         = 0;
    resources->OES_EGL_image_external_essl3                   = 0;
    resources->NV_EGL_stream_consumer_external                = 0;
    resources->ARB_texture_rectangle                          = 0;
    resources->EXT_blend_func_extended                        = 0;
    resources->EXT_conservative_depth                         = 0;
    resources->EXT_draw_buffers                               = 0;
    resources->EXT_frag_depth                                 = 0;
    resources->EXT_shader_texture_lod                         = 0;
    resources->EXT_shader_framebuffer_fetch                   = 0;
    resources->EXT_shader_framebuffer_fetch_non_coherent      = 0;
    resources->NV_shader_framebuffer_fetch                    = 0;
    resources->ARM_shader_framebuffer_fetch                   = 0;
    resources->ARM_shader_framebuffer_fetch_depth_stencil     = 0;
    resources->OVR_multiview                                  = 0;
    resources->OVR_multiview2                                 = 0;
    resources->EXT_YUV_target                                 = 0;
    resources->EXT_geometry_shader                            = 0;
    resources->OES_geometry_shader                            = 0;
    resources->EXT_gpu_shader5                                = 0;
    resources->OES_gpu_shader5                                = 0;
    resources->OES_shader_io_blocks                           = 0;
    resources->EXT_shader_io_blocks                           = 0;
    resources->EXT_shader_non_constant_global_initializers    = 0;
    resources->NV_shader_noperspective_interpolation          = 0;
    resources->OES_texture_storage_multisample_2d_array       = 0;
    resources->OES_texture_3D                                 = 0;
    resources->ANGLE_shader_pixel_local_storage               = 0;
    resources->ANGLE_texture_multisample                      = 0;
    resources->ANGLE_multi_draw                               = 0;
    resources->ANGLE_base_vertex_base_instance                = 0;
    resources->ANGLE_base_vertex_base_instance_shader_builtin = 0;
    resources->WEBGL_video_texture                            = 0;
    resources->APPLE_clip_distance                            = 0;
    resources->OES_texture_cube_map_array                     = 0;
    resources->EXT_texture_cube_map_array                     = 0;
    resources->EXT_texture_query_lod                          = 0;
    resources->EXT_texture_shadow_lod                         = 0;
    resources->EXT_shadow_samplers                            = 0;
    resources->OES_shader_multisample_interpolation           = 0;
    resources->NV_draw_buffers                                = 0;
    resources->OES_shader_image_atomic                        = 0;
    resources->EXT_tessellation_shader                        = 0;
    resources->OES_tessellation_shader                        = 0;
    resources->OES_texture_buffer                             = 0;
    resources->EXT_texture_buffer                             = 0;
    resources->OES_sample_variables                           = 0;
    resources->EXT_clip_cull_distance                         = 0;
    resources->ANGLE_clip_cull_distance                       = 0;
    resources->KHR_blend_equation_advanced                    = 0;

    resources->MaxClipDistances                = 8;
    resources->MaxCullDistances                = 8;
    resources->MaxCombinedClipAndCullDistances = 8;

    // Disable highp precision in fragment shader by default.
    resources->FragmentPrecisionHigh = 0;

    // GLSL ES 3.0 constants.
    resources->MaxVertexOutputVectors  = 16;
    resources->MaxFragmentInputVectors = 15;
    resources->MinProgramTexelOffset   = -8;
    resources->MaxProgramTexelOffset   = 7;

    // Extensions constants.
    resources->MaxDualSourceDrawBuffers = 0;

    resources->MaxViewsOVR = 4;

    // Disable name hashing by default.
    resources->HashFunction = nullptr;

    resources->MaxExpressionComplexity = 256;
    resources->MaxStatementDepth       = 256;
    resources->MaxCallStackDepth       = 256;
    resources->MaxFunctionParameters   = 1024;

    // ES 3.1 Revision 4, 7.2 Built-in Constants

    // ES 3.1, Revision 4, 8.13 Texture minification
    // "The value of MIN_PROGRAM_TEXTURE_GATHER_OFFSET must be less than or equal to the value of
    // MIN_PROGRAM_TEXEL_OFFSET. The value of MAX_PROGRAM_TEXTURE_GATHER_OFFSET must be greater than
    // or equal to the value of MAX_PROGRAM_TEXEL_OFFSET"
    resources->MinProgramTextureGatherOffset = -8;
    resources->MaxProgramTextureGatherOffset = 7;

    resources->MaxImageUnits            = 4;
    resources->MaxVertexImageUniforms   = 0;
    resources->MaxFragmentImageUniforms = 0;
    resources->MaxComputeImageUniforms  = 4;
    resources->MaxCombinedImageUniforms = 4;

    resources->MaxUniformLocations = 1024;

    resources->MaxCombinedShaderOutputResources = 4;

    resources->MaxComputeWorkGroupCount[0] = 65535;
    resources->MaxComputeWorkGroupCount[1] = 65535;
    resources->MaxComputeWorkGroupCount[2] = 65535;
    resources->MaxComputeWorkGroupSize[0]  = 128;
    resources->MaxComputeWorkGroupSize[1]  = 128;
    resources->MaxComputeWorkGroupSize[2]  = 64;
    resources->MaxComputeUniformComponents = 512;
    resources->MaxComputeTextureImageUnits = 16;

    resources->MaxComputeAtomicCounters       = 8;
    resources->MaxComputeAtomicCounterBuffers = 1;

    resources->MaxVertexAtomicCounters   = 0;
    resources->MaxFragmentAtomicCounters = 0;
    resources->MaxCombinedAtomicCounters = 8;
    resources->MaxAtomicCounterBindings  = 1;

    resources->MaxVertexAtomicCounterBuffers   = 0;
    resources->MaxFragmentAtomicCounterBuffers = 0;
    resources->MaxCombinedAtomicCounterBuffers = 1;
    resources->MaxAtomicCounterBufferSize      = 32;

    resources->MaxUniformBufferBindings       = 32;
    resources->MaxShaderStorageBufferBindings = 4;

    resources->MaxGeometryUniformComponents     = 1024;
    resources->MaxGeometryUniformBlocks         = 12;
    resources->MaxGeometryInputComponents       = 64;
    resources->MaxGeometryOutputComponents      = 64;
    resources->MaxGeometryOutputVertices        = 256;
    resources->MaxGeometryTotalOutputComponents = 1024;
    resources->MaxGeometryTextureImageUnits     = 16;
    resources->MaxGeometryAtomicCounterBuffers  = 0;
    resources->MaxGeometryAtomicCounters        = 0;
    resources->MaxGeometryShaderStorageBlocks   = 0;
    resources->MaxGeometryShaderInvocations     = 32;
    resources->MaxGeometryImageUniforms         = 0;

    resources->MaxTessControlInputComponents       = 64;
    resources->MaxTessControlOutputComponents      = 64;
    resources->MaxTessControlTextureImageUnits     = 16;
    resources->MaxTessControlUniformComponents     = 1024;
    resources->MaxTessControlTotalOutputComponents = 2048;
    resources->MaxTessControlImageUniforms         = 0;
    resources->MaxTessControlAtomicCounters        = 0;
    resources->MaxTessControlAtomicCounterBuffers  = 0;

    resources->MaxTessPatchComponents = 120;
    resources->MaxPatchVertices       = 32;
    resources->MaxTessGenLevel        = 64;

    resources->MaxTessEvaluationInputComponents      = 64;
    resources->MaxTessEvaluationOutputComponents     = 64;
    resources->MaxTessEvaluationTextureImageUnits    = 16;
    resources->MaxTessEvaluationUniformComponents    = 1024;
    resources->MaxTessEvaluationImageUniforms        = 0;
    resources->MaxTessEvaluationAtomicCounters       = 0;
    resources->MaxTessEvaluationAtomicCounterBuffers = 0;

    resources->SubPixelBits = 8;

    resources->MaxSamples = 4;
}

//
// Driver calls these to create and destroy compiler objects.
//
ShHandle ConstructCompiler(sh::GLenum type,
                           ShShaderSpec spec,
                           ShShaderOutput output,
                           const ShBuiltInResources *resources)
{
    TShHandleBase *base = static_cast<TShHandleBase *>(ConstructCompiler(type, spec, output));
    if (base == nullptr)
    {
        return 0;
    }

    TCompiler *compiler = base->getAsCompiler();
    if (compiler == nullptr)
    {
        return 0;
    }

    // Generate built-in symbol table.
    if (!compiler->Init(*resources))
    {
        Destruct(base);
        return 0;
    }

    return base;
}

void Destruct(ShHandle handle)
{
    if (handle == 0)
        return;

    TShHandleBase *base = static_cast<TShHandleBase *>(handle);

    if (base->getAsCompiler())
        DeleteCompiler(base->getAsCompiler());
}

ShBuiltInResources GetBuiltInResources(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    ASSERT(compiler);
    return compiler->getBuiltInResources();
}

const std::string &GetBuiltInResourcesString(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    ASSERT(compiler);
    return compiler->getBuiltInResourcesString();
}

//
// Do an actual compile on the given strings.  The result is left
// in the given compile object.
//
// Return:  The return value of ShCompile is really boolean, indicating
// success or failure.
//
bool Compile(const ShHandle handle,
             const char *const shaderStrings[],
             size_t numStrings,
             const ShCompileOptions &compileOptions)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    ASSERT(compiler);

    return compiler->compile(shaderStrings, numStrings, compileOptions);
}

void ClearResults(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    ASSERT(compiler);
    compiler->clearResults();
}

int GetShaderVersion(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    ASSERT(compiler);
    return compiler->getShaderVersion();
}

ShShaderOutput GetShaderOutputType(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    ASSERT(compiler);
    return compiler->getOutputType();
}

//
// Return any compiler log of messages for the application.
//
const std::string &GetInfoLog(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    ASSERT(compiler);

    TInfoSink &infoSink = compiler->getInfoSink();
    return infoSink.info.str();
}

//
// Return any object code.
//
const std::string &GetObjectCode(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    ASSERT(compiler);

    TInfoSink &infoSink = compiler->getInfoSink();
    return infoSink.obj.str();
}

//
// Return any object binary code.
//
const BinaryBlob &GetObjectBinaryBlob(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    ASSERT(compiler);

    TInfoSink &infoSink = compiler->getInfoSink();
    return infoSink.obj.getBinary();
}

bool GetShaderBinary(const ShHandle handle,
                     const char *const shaderStrings[],
                     size_t numStrings,
                     const ShCompileOptions &compileOptions,
                     ShaderBinaryBlob *const binaryOut)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    ASSERT(compiler);

    return compiler->getShaderBinary(handle, shaderStrings, numStrings, compileOptions, binaryOut);
}

const std::map<std::string, std::string> *GetNameHashingMap(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    ASSERT(compiler);
    return &(compiler->getNameMap());
}

const std::vector<ShaderVariable> *GetUniforms(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    if (!compiler)
    {
        return nullptr;
    }
    return &compiler->getUniforms();
}

const std::vector<ShaderVariable> *GetInputVaryings(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    if (compiler == nullptr)
    {
        return nullptr;
    }
    return &compiler->getInputVaryings();
}

const std::vector<ShaderVariable> *GetOutputVaryings(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    if (compiler == nullptr)
    {
        return nullptr;
    }
    return &compiler->getOutputVaryings();
}

const std::vector<ShaderVariable> *GetVaryings(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    if (compiler == nullptr)
    {
        return nullptr;
    }

    switch (compiler->getShaderType())
    {
        case GL_VERTEX_SHADER:
            return &compiler->getOutputVaryings();
        case GL_FRAGMENT_SHADER:
            return &compiler->getInputVaryings();
        case GL_COMPUTE_SHADER:
            ASSERT(compiler->getOutputVaryings().empty() && compiler->getInputVaryings().empty());
            return &compiler->getOutputVaryings();
        // Since geometry shaders have both input and output varyings, we shouldn't call GetVaryings
        // on a geometry shader.
        default:
            return nullptr;
    }
}

const std::vector<ShaderVariable> *GetAttributes(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    if (!compiler)
    {
        return nullptr;
    }
    return &compiler->getAttributes();
}

const std::vector<ShaderVariable> *GetOutputVariables(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    if (!compiler)
    {
        return nullptr;
    }
    return &compiler->getOutputVariables();
}

const std::vector<InterfaceBlock> *GetInterfaceBlocks(const ShHandle handle)
{
    return GetShaderVariables<InterfaceBlock>(handle);
}

const std::vector<InterfaceBlock> *GetUniformBlocks(const ShHandle handle)
{
    ASSERT(handle);
    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    TCompiler *compiler = base->getAsCompiler();
    ASSERT(compiler);

    return &compiler->getUniformBlocks();
}

const std::vector<InterfaceBlock> *GetShaderStorageBlocks(const ShHandle handle)
{
    ASSERT(handle);
    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    TCompiler *compiler = base->getAsCompiler();
    ASSERT(compiler);

    return &compiler->getShaderStorageBlocks();
}

WorkGroupSize GetComputeShaderLocalGroupSize(const ShHandle handle)
{
    ASSERT(handle);

    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    TCompiler *compiler = base->getAsCompiler();
    ASSERT(compiler);

    return compiler->getComputeShaderLocalSize();
}

int GetVertexShaderNumViews(const ShHandle handle)
{
    ASSERT(handle);
    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    TCompiler *compiler = base->getAsCompiler();
    ASSERT(compiler);

    return compiler->getNumViews();
}

const std::vector<ShPixelLocalStorageFormat> *GetPixelLocalStorageFormats(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    ASSERT(compiler);

    return &compiler->GetPixelLocalStorageFormats();
}

uint32_t GetShaderSpecConstUsageBits(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    if (compiler == nullptr)
    {
        return 0;
    }
    return compiler->getSpecConstUsageBits().bits();
}

bool CheckVariablesWithinPackingLimits(int maxVectors, const std::vector<ShaderVariable> &variables)
{
    return CheckVariablesInPackingLimits(maxVectors, variables);
}

bool GetShaderStorageBlockRegister(const ShHandle handle,
                                   const std::string &shaderStorageBlockName,
                                   unsigned int *indexOut)
{
#ifdef ANGLE_ENABLE_HLSL
    ASSERT(indexOut);

    TranslatorHLSL *translator = GetTranslatorHLSLFromHandle(handle);
    ASSERT(translator);

    if (!translator->hasShaderStorageBlock(shaderStorageBlockName))
    {
        return false;
    }

    *indexOut = translator->getShaderStorageBlockRegister(shaderStorageBlockName);
    return true;
#else
    return false;
#endif  // ANGLE_ENABLE_HLSL
}

bool GetUniformBlockRegister(const ShHandle handle,
                             const std::string &uniformBlockName,
                             unsigned int *indexOut)
{
#ifdef ANGLE_ENABLE_HLSL
    ASSERT(indexOut);

    TranslatorHLSL *translator = GetTranslatorHLSLFromHandle(handle);
    ASSERT(translator);

    if (!translator->hasUniformBlock(uniformBlockName))
    {
        return false;
    }

    *indexOut = translator->getUniformBlockRegister(uniformBlockName);
    return true;
#else
    return false;
#endif  // ANGLE_ENABLE_HLSL
}

bool ShouldUniformBlockUseStructuredBuffer(const ShHandle handle,
                                           const std::string &uniformBlockName)
{
#ifdef ANGLE_ENABLE_HLSL
    TranslatorHLSL *translator = GetTranslatorHLSLFromHandle(handle);
    ASSERT(translator);

    return translator->shouldUniformBlockUseStructuredBuffer(uniformBlockName);
#else
    return false;
#endif  // ANGLE_ENABLE_HLSL
}

const std::map<std::string, unsigned int> *GetUniformRegisterMap(const ShHandle handle)
{
#ifdef ANGLE_ENABLE_HLSL
    TranslatorHLSL *translator = GetTranslatorHLSLFromHandle(handle);
    ASSERT(translator);

    return translator->getUniformRegisterMap();
#else
    return nullptr;
#endif  // ANGLE_ENABLE_HLSL
}

const std::set<std::string> *GetSlowCompilingUniformBlockSet(const ShHandle handle)
{
#ifdef ANGLE_ENABLE_HLSL
    TranslatorHLSL *translator = GetTranslatorHLSLFromHandle(handle);
    ASSERT(translator);

    return translator->getSlowCompilingUniformBlockSet();
#else
    return nullptr;
#endif  // ANGLE_ENABLE_HLSL
}

unsigned int GetReadonlyImage2DRegisterIndex(const ShHandle handle)
{
#ifdef ANGLE_ENABLE_HLSL
    TranslatorHLSL *translator = GetTranslatorHLSLFromHandle(handle);
    ASSERT(translator);

    return translator->getReadonlyImage2DRegisterIndex();
#else
    return 0;
#endif  // ANGLE_ENABLE_HLSL
}

unsigned int GetImage2DRegisterIndex(const ShHandle handle)
{
#ifdef ANGLE_ENABLE_HLSL
    TranslatorHLSL *translator = GetTranslatorHLSLFromHandle(handle);
    ASSERT(translator);

    return translator->getImage2DRegisterIndex();
#else
    return 0;
#endif  // ANGLE_ENABLE_HLSL
}

const std::set<std::string> *GetUsedImage2DFunctionNames(const ShHandle handle)
{
#ifdef ANGLE_ENABLE_HLSL
    TranslatorHLSL *translator = GetTranslatorHLSLFromHandle(handle);
    ASSERT(translator);

    return translator->getUsedImage2DFunctionNames();
#else
    return nullptr;
#endif  // ANGLE_ENABLE_HLSL
}

uint8_t GetClipDistanceArraySize(const ShHandle handle)
{
    ASSERT(handle);
    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    TCompiler *compiler = base->getAsCompiler();
    ASSERT(compiler);

    return compiler->getClipDistanceArraySize();
}

uint8_t GetCullDistanceArraySize(const ShHandle handle)
{
    ASSERT(handle);
    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    TCompiler *compiler = base->getAsCompiler();
    ASSERT(compiler);

    return compiler->getCullDistanceArraySize();
}

uint32_t GetMetadataFlags(const ShHandle handle)
{
    ASSERT(handle);

    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    TCompiler *compiler = base->getAsCompiler();
    ASSERT(compiler);

    return compiler->getMetadataFlags().bits();
}

GLenum GetGeometryShaderInputPrimitiveType(const ShHandle handle)
{
    ASSERT(handle);

    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    TCompiler *compiler = base->getAsCompiler();
    ASSERT(compiler);

    return GetGeometryShaderPrimitiveTypeEnum(compiler->getGeometryShaderInputPrimitiveType());
}

GLenum GetGeometryShaderOutputPrimitiveType(const ShHandle handle)
{
    ASSERT(handle);

    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    TCompiler *compiler = base->getAsCompiler();
    ASSERT(compiler);

    return GetGeometryShaderPrimitiveTypeEnum(compiler->getGeometryShaderOutputPrimitiveType());
}

int GetGeometryShaderInvocations(const ShHandle handle)
{
    ASSERT(handle);

    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    TCompiler *compiler = base->getAsCompiler();
    ASSERT(compiler);

    return compiler->getGeometryShaderInvocations();
}

int GetGeometryShaderMaxVertices(const ShHandle handle)
{
    ASSERT(handle);

    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    TCompiler *compiler = base->getAsCompiler();
    ASSERT(compiler);

    int maxVertices = compiler->getGeometryShaderMaxVertices();
    ASSERT(maxVertices >= 0);
    return maxVertices;
}

int GetTessControlShaderVertices(const ShHandle handle)
{
    ASSERT(handle);

    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    TCompiler *compiler = base->getAsCompiler();
    ASSERT(compiler);

    int vertices = compiler->getTessControlShaderOutputVertices();
    return vertices;
}

GLenum GetTessGenMode(const ShHandle handle)
{
    ASSERT(handle);

    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    TCompiler *compiler = base->getAsCompiler();
    ASSERT(compiler);

    return GetTessellationShaderTypeEnum(compiler->getTessEvaluationShaderInputPrimitiveType());
}

GLenum GetTessGenSpacing(const ShHandle handle)
{
    ASSERT(handle);

    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    TCompiler *compiler = base->getAsCompiler();
    ASSERT(compiler);

    return GetTessellationShaderTypeEnum(compiler->getTessEvaluationShaderInputVertexSpacingType());
}

GLenum GetTessGenVertexOrder(const ShHandle handle)
{
    ASSERT(handle);

    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    TCompiler *compiler = base->getAsCompiler();
    ASSERT(compiler);

    return GetTessellationShaderTypeEnum(compiler->getTessEvaluationShaderInputOrderingType());
}

GLenum GetTessGenPointMode(const ShHandle handle)
{
    ASSERT(handle);

    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    TCompiler *compiler = base->getAsCompiler();
    ASSERT(compiler);

    return GetTessellationShaderTypeEnum(compiler->getTessEvaluationShaderInputPointType());
}

unsigned int GetShaderSharedMemorySize(const ShHandle handle)
{
    ASSERT(handle);

    TShHandleBase *base = static_cast<TShHandleBase *>(handle);
    TCompiler *compiler = base->getAsCompiler();
    ASSERT(compiler);

    unsigned int sharedMemorySize = compiler->getSharedMemorySize();
    return sharedMemorySize;
}

uint32_t GetAdvancedBlendEquations(const ShHandle handle)
{
    TCompiler *compiler = GetCompilerFromHandle(handle);
    ASSERT(compiler);

    return compiler->getAdvancedBlendEquations().bits();
}

// Can't prefix with just _ because then we might introduce a double underscore, which is not safe
// in GLSL (ESSL 3.00.6 section 3.8: All identifiers containing a double underscore are reserved for
// use by the underlying implementation). u is short for user-defined.
const char kUserDefinedNamePrefix[] = "_u";

const char *BlockLayoutTypeToString(BlockLayoutType type)
{
    switch (type)
    {
        case BlockLayoutType::BLOCKLAYOUT_STD140:
            return "std140";
        case BlockLayoutType::BLOCKLAYOUT_STD430:
            return "std430";
        case BlockLayoutType::BLOCKLAYOUT_PACKED:
            return "packed";
        case BlockLayoutType::BLOCKLAYOUT_SHARED:
            return "shared";
        default:
            return "invalid";
    }
}

const char *BlockTypeToString(BlockType type)
{
    switch (type)
    {
        case BlockType::kBlockBuffer:
            return "buffer";
        case BlockType::kBlockUniform:
            return "uniform";
        default:
            return "invalid";
    }
}

const char *InterpolationTypeToString(InterpolationType type)
{
    switch (type)
    {
        case InterpolationType::INTERPOLATION_SMOOTH:
            return "smooth";
        case InterpolationType::INTERPOLATION_CENTROID:
            return "centroid";
        case InterpolationType::INTERPOLATION_SAMPLE:
            return "sample";
        case InterpolationType::INTERPOLATION_FLAT:
            return "flat";
        case InterpolationType::INTERPOLATION_NOPERSPECTIVE:
            return "noperspective";
        case InterpolationType::INTERPOLATION_NOPERSPECTIVE_CENTROID:
            return "noperspective centroid";
        case InterpolationType::INTERPOLATION_NOPERSPECTIVE_SAMPLE:
            return "noperspective sample";
        default:
            return "invalid";
    }
}
}  // namespace sh

ShCompileOptions::ShCompileOptions()
{
    memset(this, 0, sizeof(*this));
}

ShCompileOptions::ShCompileOptions(const ShCompileOptions &other)
{
    memcpy(this, &other, sizeof(*this));
}
ShCompileOptions &ShCompileOptions::operator=(const ShCompileOptions &other)
{
    memcpy(this, &other, sizeof(*this));
    return *this;
}

ShBuiltInResources::ShBuiltInResources()
{
    memset(this, 0, sizeof(*this));
}

ShBuiltInResources::ShBuiltInResources(const ShBuiltInResources &other)
{
    memcpy(this, &other, sizeof(*this));
}
ShBuiltInResources &ShBuiltInResources::operator=(const ShBuiltInResources &other)
{
    memcpy(this, &other, sizeof(*this));
    return *this;
}
