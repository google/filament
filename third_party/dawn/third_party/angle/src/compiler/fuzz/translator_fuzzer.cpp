//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// translator_fuzzer.cpp: A libfuzzer fuzzer for the shader translator.

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <unordered_map>

#include "angle_gl.h"
#include "anglebase/no_destructor.h"
#include "common/hash_containers.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/util.h"

using namespace sh;

namespace
{
struct TranslatorCacheKey
{
    bool operator==(const TranslatorCacheKey &other) const
    {
        return type == other.type && spec == other.spec && output == other.output;
    }

    uint32_t type   = 0;
    uint32_t spec   = 0;
    uint32_t output = 0;
};
}  // anonymous namespace

namespace std
{

template <>
struct hash<TranslatorCacheKey>
{
    std::size_t operator()(const TranslatorCacheKey &k) const
    {
        return (hash<uint32_t>()(k.type) << 1) ^ (hash<uint32_t>()(k.spec) >> 1) ^
               hash<uint32_t>()(k.output);
    }
};
}  // namespace std

struct TCompilerDeleter
{
    void operator()(TCompiler *compiler) const { DeleteCompiler(compiler); }
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    ShaderDumpHeader header{};
    if (size <= sizeof(header))
    {
        return 0;
    }

    // Make sure the rest of data will be a valid C string so that we don't have to copy it.
    if (data[size - 1] != 0)
    {
        return 0;
    }

    memcpy(&header, data, sizeof(header));
    ShCompileOptions options{};
    memcpy(&options, &header.basicCompileOptions, offsetof(ShCompileOptions, metal));
    memcpy(&options.metal, &header.metalCompileOptions, sizeof(options.metal));
    memcpy(&options.pls, &header.plsCompileOptions, sizeof(options.pls));
    size -= sizeof(header);
    data += sizeof(header);
    uint32_t type = header.type;
    uint32_t spec = header.spec;

    if (type != GL_FRAGMENT_SHADER && type != GL_VERTEX_SHADER)
    {
        return 0;
    }

    if (spec != SH_GLES2_SPEC && type != SH_WEBGL_SPEC && spec != SH_GLES3_SPEC &&
        spec != SH_WEBGL2_SPEC)
    {
        return 0;
    }

    ShShaderOutput shaderOutput = static_cast<ShShaderOutput>(header.output);

    bool hasUnsupportedOptions = false;

    const bool hasMacGLSLOptions = options.rewriteFloatUnaryMinusOperator ||
                                   options.addAndTrueToLoopCondition ||
                                   options.rewriteDoWhileLoops || options.unfoldShortCircuit ||
                                   options.rewriteRowMajorMatrices;

    if (!IsOutputGLSL(shaderOutput) && !IsOutputESSL(shaderOutput))
    {
        hasUnsupportedOptions =
            hasUnsupportedOptions || options.emulateAtan2FloatFunction || options.clampFragDepth ||
            options.regenerateStructNames || options.rewriteRepeatedAssignToSwizzled ||
            options.useUnusedStandardSharedBlocks || options.selectViewInNvGLSLVertexShader;

        hasUnsupportedOptions = hasUnsupportedOptions || hasMacGLSLOptions;
    }
    else
    {
#if !defined(ANGLE_PLATFORM_APPLE)
        hasUnsupportedOptions = hasUnsupportedOptions || hasMacGLSLOptions;
#endif
    }
    if (!IsOutputSPIRV(shaderOutput))
    {
        hasUnsupportedOptions = hasUnsupportedOptions || options.useSpecializationConstant ||
                                options.addVulkanXfbEmulationSupportCode ||
                                options.roundOutputAfterDithering ||
                                options.addAdvancedBlendEquationsEmulation;
    }
    if (!IsOutputHLSL(shaderOutput))
    {
        hasUnsupportedOptions = hasUnsupportedOptions ||
                                options.expandSelectHLSLIntegerPowExpressions ||
                                options.allowTranslateUniformBlockToStructuredBuffer ||
                                options.rewriteIntegerUnaryMinusOperator;
    }

    // If there are any options not supported with this output, don't attempt to run the translator.
    if (hasUnsupportedOptions)
    {
        return 0;
    }

    // Make sure the rest of the options are in a valid range.
    options.pls.fragmentSyncType = static_cast<ShFragmentSynchronizationType>(
        static_cast<uint32_t>(options.pls.fragmentSyncType) %
        static_cast<uint32_t>(ShFragmentSynchronizationType::InvalidEnum));

    std::vector<uint32_t> validOutputs;
    validOutputs.push_back(SH_ESSL_OUTPUT);
    validOutputs.push_back(SH_GLSL_COMPATIBILITY_OUTPUT);
    validOutputs.push_back(SH_GLSL_130_OUTPUT);
    validOutputs.push_back(SH_GLSL_140_OUTPUT);
    validOutputs.push_back(SH_GLSL_150_CORE_OUTPUT);
    validOutputs.push_back(SH_GLSL_330_CORE_OUTPUT);
    validOutputs.push_back(SH_GLSL_400_CORE_OUTPUT);
    validOutputs.push_back(SH_GLSL_410_CORE_OUTPUT);
    validOutputs.push_back(SH_GLSL_420_CORE_OUTPUT);
    validOutputs.push_back(SH_GLSL_430_CORE_OUTPUT);
    validOutputs.push_back(SH_GLSL_440_CORE_OUTPUT);
    validOutputs.push_back(SH_GLSL_450_CORE_OUTPUT);
    validOutputs.push_back(SH_SPIRV_VULKAN_OUTPUT);
    validOutputs.push_back(SH_HLSL_3_0_OUTPUT);
    validOutputs.push_back(SH_HLSL_4_1_OUTPUT);
    bool found = false;
    for (auto valid : validOutputs)
    {
        found = found || (valid == shaderOutput);
    }
    if (!found)
    {
        return 0;
    }

    if (!sh::Initialize())
    {
        return 0;
    }

    TranslatorCacheKey key;
    key.type   = type;
    key.spec   = spec;
    key.output = shaderOutput;

    using UniqueTCompiler = std::unique_ptr<TCompiler, TCompilerDeleter>;
    static angle::base::NoDestructor<angle::HashMap<TranslatorCacheKey, UniqueTCompiler>>
        translators;

    if (translators->find(key) == translators->end())
    {
        UniqueTCompiler translator(
            ConstructCompiler(type, static_cast<ShShaderSpec>(spec), shaderOutput));

        if (translator == nullptr)
        {
            return 0;
        }

        ShBuiltInResources resources;
        sh::InitBuiltInResources(&resources);

        // Enable all the extensions to have more coverage
        resources.OES_standard_derivatives        = 1;
        resources.OES_EGL_image_external          = 1;
        resources.OES_EGL_image_external_essl3    = 1;
        resources.NV_EGL_stream_consumer_external = 1;
        resources.ARB_texture_rectangle           = 1;
        resources.EXT_blend_func_extended         = 1;
        resources.EXT_conservative_depth          = 1;
        resources.EXT_draw_buffers                = 1;
        resources.EXT_frag_depth                  = 1;
        resources.EXT_shader_texture_lod          = 1;
        resources.EXT_shader_framebuffer_fetch    = 1;
        resources.NV_shader_framebuffer_fetch     = 1;
        resources.ARM_shader_framebuffer_fetch    = 1;
        resources.ARM_shader_framebuffer_fetch_depth_stencil = 1;
        resources.EXT_YUV_target                  = 1;
        resources.APPLE_clip_distance             = 1;
        resources.MaxDualSourceDrawBuffers        = 1;
        resources.EXT_gpu_shader5                 = 1;
        resources.MaxClipDistances                = 1;
        resources.EXT_shadow_samplers             = 1;
        resources.EXT_clip_cull_distance          = 1;
        resources.ANGLE_clip_cull_distance        = 1;
        resources.EXT_primitive_bounding_box      = 1;
        resources.OES_primitive_bounding_box      = 1;

        if (!translator->Init(resources))
        {
            return 0;
        }

        (*translators)[key] = std::move(translator);
    }

    auto &translator = (*translators)[key];

    options.limitExpressionComplexity = true;
    const char *shaderStrings[]       = {reinterpret_cast<const char *>(data)};
    translator->compile(shaderStrings, 1, options);

    return 0;
}
