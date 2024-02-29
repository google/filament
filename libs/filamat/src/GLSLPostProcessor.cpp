/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "GLSLPostProcessor.h"

#include <GlslangToSpv.h>
#include <SPVRemapper.h>
#include <spirv-tools/libspirv.hpp>

#include <spirv_glsl.hpp>
#include <spirv_msl.hpp>

#include "backend/DriverEnums.h"
#include "sca/builtinResource.h"
#include "sca/GLSLTools.h"

#include "shaders/CodeGenerator.h"
#include "shaders/MaterialInfo.h"
#include "shaders/SibGenerator.h"

#include "MetalArgumentBuffer.h"
#include "SpirvFixup.h"
#include "utils/ostream.h"

#include <filament/MaterialEnums.h>

#include <utils/Log.h>

#include <sstream>
#include <unordered_map>
#include <vector>

using namespace glslang;
using namespace spirv_cross;
using namespace spvtools;
using namespace filament;
using namespace filament::backend;

namespace filamat {

using namespace utils;

namespace msl {  // this is only used for MSL

using BindingIndexMap = std::unordered_map<std::string, uint16_t>;

static void collectSibs(const GLSLPostProcessor::Config& config, SibVector& sibs) {
    switch (config.domain) {
        case MaterialDomain::SURFACE:
            UTILS_NOUNROLL
            for (uint8_t blockIndex = 0; blockIndex < CONFIG_SAMPLER_BINDING_COUNT; blockIndex++) {
                if (blockIndex == SamplerBindingPoints::PER_MATERIAL_INSTANCE) {
                    continue;
                }
                auto const* sib =
                        SibGenerator::getSib((SamplerBindingPoints)blockIndex, config.variant);
                if (sib && hasShaderType(sib->getStageFlags(), config.shaderType)) {
                    sibs.emplace_back(blockIndex, sib);
                }
            }
        case MaterialDomain::POST_PROCESS:
        case MaterialDomain::COMPUTE:
            break;
    }
    sibs.emplace_back((uint8_t) SamplerBindingPoints::PER_MATERIAL_INSTANCE,
            &config.materialInfo->sib);
}

} // namespace msl

GLSLPostProcessor::GLSLPostProcessor(MaterialBuilder::Optimization optimization, uint32_t flags)
        : mOptimization(optimization),
          mPrintShaders(flags & PRINT_SHADERS),
          mGenerateDebugInfo(flags & GENERATE_DEBUG_INFO) {
    // SPIRV error handler registration needs to occur only once. To avoid a race we do it up here
    // in the constructor, which gets invoked before MaterialBuilder kicks off jobs.
    spv::spirvbin_t::registerErrorHandler([](const std::string& str) {
        slog.e << str << io::endl;
    });
}

GLSLPostProcessor::~GLSLPostProcessor() = default;

static bool filterSpvOptimizerMessage(spv_message_level_t level) {
#ifdef NDEBUG
    // In release builds, only log errors.
    if (level == SPV_MSG_WARNING ||
        level == SPV_MSG_INFO ||
        level == SPV_MSG_DEBUG) {
        return false;
    }
#endif
    return true;
}

static std::string stringifySpvOptimizerMessage(spv_message_level_t level, const char* source,
        const spv_position_t& position, const char* message) {
    const char* levelString = nullptr;
    switch (level) {
        case SPV_MSG_FATAL:
            levelString = "FATAL";
            break;
        case SPV_MSG_INTERNAL_ERROR:
            levelString = "INTERNAL ERROR";
            break;
        case SPV_MSG_ERROR:
            levelString = "ERROR";
            break;
        case SPV_MSG_WARNING:
            levelString = "WARNING";
            break;
        case SPV_MSG_INFO:
            levelString = "INFO";
            break;
        case SPV_MSG_DEBUG:
            levelString = "DEBUG";
            break;
    }

    std::ostringstream oss;
    oss << levelString << ": ";
    if (source) oss << source << ":";
    oss << position.line << ":" << position.column << ":";
    oss << position.index << ": ";
    if (message) oss << message;

    return oss.str();
}

void GLSLPostProcessor::spirvToMsl(const SpirvBlob *spirv, std::string *outMsl,
        filament::backend::ShaderModel shaderModel, bool useFramebufferFetch, const SibVector& sibs,
        const ShaderMinifier* minifier) {

    using namespace msl;

    CompilerMSL mslCompiler(*spirv);
    CompilerGLSL::Options const options;
    mslCompiler.set_common_options(options);

    const CompilerMSL::Options::Platform platform =
        shaderModel == ShaderModel::MOBILE ?
            CompilerMSL::Options::Platform::iOS : CompilerMSL::Options::Platform::macOS;

    CompilerMSL::Options mslOptions = {};
    mslOptions.platform = platform,
    mslOptions.msl_version = shaderModel == ShaderModel::MOBILE ?
        CompilerMSL::Options::make_msl_version(2, 0) : CompilerMSL::Options::make_msl_version(2, 2);

    if (useFramebufferFetch) {
        mslOptions.use_framebuffer_fetch_subpasses = true;
        // On macOS, framebuffer fetch is only available starting with MSL 2.3. Filament will only
        // use framebuffer fetch materials on devices that support it.
        if (shaderModel == ShaderModel::DESKTOP) {
            mslOptions.msl_version = CompilerMSL::Options::make_msl_version(2, 3);
        }
    }

    mslOptions.argument_buffers = true;

    // We're using argument buffers for texture resources, however, we cannot rely on spirv-cross to
    // generate the argument buffer definitions.
    //
    // Consider a shader with 3 textures:
    // layout (set = 0, binding = 0) uniform sampler2D texture1;
    // layout (set = 0, binding = 1) uniform sampler2D texture2;
    // layout (set = 0, binding = 2) uniform sampler2D texture3;
    //
    // If only texture1 and texture2 are used in the material, then texture3 will be optimized away.
    // This results in an argument buffer like the following:
    // struct spvDescriptorSetBuffer0 {
    //     texture2d<float> texture1 [[id(0)]];
    //     sampler texture1Smplr [[id(1)]];
    //     texture2d<float> texture2 [[id(2)]];
    //     sampler texture2Smplr [[id(3)]];
    // };
    // Note that this happens even if "pad_argument_buffer_resources" and
    // "force_active_argument_buffer_resources" are true.
    //
    // This would be fine, except older Apple devices don't like it when the argument buffer in the
    // shader doesn't precisely match the one generated at runtime.
    //
    // So, we use the MetalArgumentBuffer class to replace spirv-cross' argument buffer definitions
    // with our own that contain all the textures/samples, even those optimized away.
    std::vector<MetalArgumentBuffer*> argumentBuffers;

    mslCompiler.set_msl_options(mslOptions);

    auto executionModel = mslCompiler.get_execution_model();

    // Metal Descriptor Sets
    // Descriptor set       Name                    Binding
    // ----------------------------------------------------------------------
    // 0                    Uniforms                Individual bindings
    // 1-4                  Sampler groups          [[buffer(27-30)]]
    // 5-7                  Unused
    //
    // Here we enumerate each sampler in each sampler group and map it to a Metal resource. Each
    // sampler group is its own descriptor set, and each descriptor set becomes an argument buffer.
    //
    // For example, in GLSL, we might have the following:
    // layout( set = 1, binding = 0 ) uniform sampler2D textureA;
    // layout( set = 1, binding = 1 ) uniform sampler2D textureB;
    //
    // This becomes the following MSL argument buffer:
    // struct spvDescriptorSetBuffer1 {
    //     texture2d<float> textureA [[id(0)]];
    //     sampler textureASmplr [[id(1)]];
    //     texture2d<float> textureB [[id(2)]];
    //     sampler textureBSmplr [[id(3)]];
    // };
    //
    // Which is then bound to the vertex/fragment functions:
    // constant spvDescriptorSetBuffer1& spvDescriptorSet1 [[buffer(27)]]
    for (auto [bindingPoint, sib] : sibs) {
        const auto& infoList = sib->getSamplerInfoList();

        // bindingPoint + 1, because the first descriptor set is for uniforms
        auto argBufferBuilder = MetalArgumentBuffer::Builder()
                .name("spvDescriptorSetBuffer" + std::to_string(int(bindingPoint + 1)));

        for (const auto& info: infoList) {
            const std::string name = info.uniformName.c_str();
            argBufferBuilder
                    .texture(info.offset * 2, name, info.type, info.format, info.multisample)
                    .sampler(info.offset * 2 + 1, name + "Smplr");
        }

        argumentBuffers.push_back(argBufferBuilder.build());

        // This MSLResourceBinding is how we control the [[buffer(n)]] binding of the argument
        // buffer itself;
        MSLResourceBinding argBufferBinding;
        // the baseType doesn't matter, but can't be UNKNOWN
        argBufferBinding.basetype = SPIRType::BaseType::Float;
        argBufferBinding.stage = executionModel;
        argBufferBinding.desc_set = bindingPoint + 1;
        argBufferBinding.binding = kArgumentBufferBinding;
        argBufferBinding.count = 1;
        argBufferBinding.msl_buffer =
                CodeGenerator::METAL_SAMPLER_GROUP_BINDING_START + bindingPoint;
        mslCompiler.add_msl_resource_binding(argBufferBinding);
    }

    auto updateResourceBindingDefault = [executionModel, &mslCompiler](const auto& resource) {
        auto set = mslCompiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        auto binding = mslCompiler.get_decoration(resource.id, spv::DecorationBinding);
        MSLResourceBinding newBinding;
        newBinding.basetype = SPIRType::BaseType::Void;
        newBinding.stage = executionModel;
        newBinding.desc_set = set;
        newBinding.binding = binding;
        newBinding.count = 1;
        newBinding.msl_texture =
        newBinding.msl_sampler =
        newBinding.msl_buffer = binding;
        mslCompiler.add_msl_resource_binding(newBinding);
    };

    auto uniformResources = mslCompiler.get_shader_resources();
    for (const auto& resource : uniformResources.uniform_buffers) {
        updateResourceBindingDefault(resource);
    }
    auto ssboResources = mslCompiler.get_shader_resources();
    for (const auto& resource : ssboResources.storage_buffers) {
        updateResourceBindingDefault(resource);
    }

    // Descriptor set 0 is uniforms. The add_discrete_descriptor_set call here prevents the uniforms
    // from becoming argument buffers.
    mslCompiler.add_discrete_descriptor_set(0);

    *outMsl = mslCompiler.compile();
    if (minifier) {
        *outMsl = minifier->removeWhitespace(*outMsl);
    }

    // Replace spirv-cross' generated argument buffers with our own.
    for (auto* argBuffer : argumentBuffers) {
        auto argBufferMsl = argBuffer->getMsl();
        MetalArgumentBuffer::replaceInShader(*outMsl, argBuffer->getName(), argBufferMsl);
        MetalArgumentBuffer::destroy(&argBuffer);
    }
}

bool GLSLPostProcessor::process(const std::string& inputShader, Config const& config,
        std::string* outputGlsl, SpirvBlob* outputSpirv, std::string* outputMsl) {
    using TargetLanguage = MaterialBuilder::TargetLanguage;

    if (config.targetLanguage == TargetLanguage::GLSL) {
        *outputGlsl = inputShader;
        if (mPrintShaders) {
            slog.i << *outputGlsl << io::endl;
        }
        return true;
    }

    InternalConfig internalConfig{
            .glslOutput = outputGlsl,
            .spirvOutput = outputSpirv,
            .mslOutput = outputMsl,
    };

    switch (config.shaderType) {
        case ShaderStage::VERTEX:
            internalConfig.shLang = EShLangVertex;
            break;
        case ShaderStage::FRAGMENT:
            internalConfig.shLang = EShLangFragment;
            break;
        case ShaderStage::COMPUTE:
            internalConfig.shLang = EShLangCompute;
            break;
    }

    TProgram program;
    TShader tShader(internalConfig.shLang);

    // The cleaner must be declared after the TShader to prevent ASAN failures.
    GLSLangCleaner const cleaner;

    const char* shaderCString = inputShader.c_str();
    tShader.setStrings(&shaderCString, 1);

    // This allows shaders to query if they will be run through glslang.
    // OpenGL shaders without optimization, for example, won't have this define.
    tShader.setPreamble("#define FILAMENT_GLSLANG\n");

    internalConfig.langVersion = GLSLTools::getGlslDefaultVersion(config.shaderModel);
    GLSLTools::prepareShaderParser(config.targetApi, config.targetLanguage, tShader,
            internalConfig.shLang, internalConfig.langVersion);

    EShMessages msg = GLSLTools::glslangFlagsFromTargetApi(config.targetApi, config.targetLanguage);
    if (config.hasFramebufferFetch) {
        // FIXME: subpasses require EShMsgVulkanRules, which I think is a mistake.
        //        SpvRules should be enough.
        //        I think this could cause the compilation to fail on gl_VertexID.
        using Type = std::underlying_type_t<EShMessages>;
        msg = EShMessages(Type(msg) | Type(EShMessages::EShMsgVulkanRules));
    }

    bool const ok = tShader.parse(&DefaultTBuiltInResource, internalConfig.langVersion, false, msg);
    if (!ok) {
        slog.e << tShader.getInfoLog() << io::endl;
        return false;
    }

    // add texture lod bias
    if (config.shaderType == backend::ShaderStage::FRAGMENT &&
        config.domain == MaterialDomain::SURFACE) {
        GLSLTools::textureLodBias(tShader);
    }

    program.addShader(&tShader);
    // Even though we only have a single shader stage, linking is still necessary to finalize
    // SPIR-V types
    bool const linkOk = program.link(msg);
    if (!linkOk) {
        slog.e << tShader.getInfoLog() << io::endl;
        return false;
    }

    switch (mOptimization) {
        case MaterialBuilder::Optimization::NONE:
            if (internalConfig.spirvOutput) {
                SpvOptions options;
                options.generateDebugInfo = mGenerateDebugInfo;
                GlslangToSpv(*program.getIntermediate(internalConfig.shLang),
                        *internalConfig.spirvOutput, &options);
                fixupClipDistance(*internalConfig.spirvOutput, config);
                if (internalConfig.mslOutput) {
                    auto sibs = SibVector::with_capacity(CONFIG_SAMPLER_BINDING_COUNT);
                    msl::collectSibs(config, sibs);
                    spirvToMsl(internalConfig.spirvOutput, internalConfig.mslOutput,
                            config.shaderModel, config.hasFramebufferFetch, sibs,
                            mGenerateDebugInfo ? &internalConfig.minifier : nullptr);
                }
            } else {
                slog.e << "GLSL post-processor invoked with optimization level NONE"
                        << io::endl;
            }
            break;
        case MaterialBuilder::Optimization::PREPROCESSOR:
            preprocessOptimization(tShader, config, internalConfig);
            break;
        case MaterialBuilder::Optimization::SIZE:
        case MaterialBuilder::Optimization::PERFORMANCE:
            if (!fullOptimization(tShader, config, internalConfig)) {
                return false;
            }
            break;
    }

    if (internalConfig.glslOutput) {
        if (!mGenerateDebugInfo) {
            *internalConfig.glslOutput =
                    internalConfig.minifier.removeWhitespace(
                            *internalConfig.glslOutput,
                            mOptimization == MaterialBuilder::Optimization::SIZE);

            // In theory this should only be enabled for SIZE, but in practice we often use PERFORMANCE.
            if (mOptimization != MaterialBuilder::Optimization::NONE) {
                *internalConfig.glslOutput =
                        internalConfig.minifier.renameStructFields(*internalConfig.glslOutput);
            }
        }
        if (mPrintShaders) {
            slog.i << *internalConfig.glslOutput << io::endl;
        }
    }
    return true;
}

void GLSLPostProcessor::preprocessOptimization(glslang::TShader& tShader,
        GLSLPostProcessor::Config const& config, InternalConfig& internalConfig) const {
    using TargetApi = MaterialBuilder::TargetApi;
    assert_invariant(bool(internalConfig.spirvOutput) == (config.targetApi != TargetApi::OPENGL));

    std::string glsl;
    TShader::ForbidIncluder forbidIncluder;

    const int version = GLSLTools::getGlslDefaultVersion(config.shaderModel);
    EShMessages const msg =
            GLSLTools::glslangFlagsFromTargetApi(config.targetApi, config.targetLanguage);
    bool ok = tShader.preprocess(&DefaultTBuiltInResource, version, ENoProfile, false, false,
            msg, &glsl, forbidIncluder);

    if (!ok) {
        slog.e << tShader.getInfoLog() << io::endl;
    }

    if (internalConfig.spirvOutput) {
        TProgram program;
        TShader spirvShader(internalConfig.shLang);

        // The cleaner must be declared after the TShader/TProgram which are setting the current
        // pool in the tls
        GLSLangCleaner const cleaner;

        const char* shaderCString = glsl.c_str();
        spirvShader.setStrings(&shaderCString, 1);
        GLSLTools::prepareShaderParser(config.targetApi, config.targetLanguage, spirvShader,
                internalConfig.shLang, internalConfig.langVersion);
        ok = spirvShader.parse(&DefaultTBuiltInResource, internalConfig.langVersion, false, msg);
        program.addShader(&spirvShader);
        // Even though we only have a single shader stage, linking is still necessary to finalize
        // SPIR-V types
        bool const linkOk = program.link(msg);
        if (!ok || !linkOk) {
            slog.e << spirvShader.getInfoLog() << io::endl;
        } else {
            SpvOptions options;
            options.generateDebugInfo = mGenerateDebugInfo;
            GlslangToSpv(*program.getIntermediate(internalConfig.shLang),
                    *internalConfig.spirvOutput, &options);
            fixupClipDistance(*internalConfig.spirvOutput, config);
        }
    }

    if (internalConfig.mslOutput) {
        auto sibs = SibVector::with_capacity(CONFIG_SAMPLER_BINDING_COUNT);
        msl::collectSibs(config, sibs);
        spirvToMsl(internalConfig.spirvOutput, internalConfig.mslOutput, config.shaderModel,
                config.hasFramebufferFetch, sibs,
                mGenerateDebugInfo ? &internalConfig.minifier : nullptr);
    }

    if (internalConfig.glslOutput) {
        *internalConfig.glslOutput = glsl;
    }
}

bool GLSLPostProcessor::fullOptimization(const TShader& tShader,
        GLSLPostProcessor::Config const& config, InternalConfig& internalConfig) const {
    SpirvBlob spirv;

    bool const optimizeForSize = mOptimization == MaterialBuilderBase::Optimization::SIZE;

    // Compile GLSL to to SPIR-V
    SpvOptions options;
    options.generateDebugInfo = mGenerateDebugInfo;
    GlslangToSpv(*tShader.getIntermediate(), spirv, &options);

    if (internalConfig.spirvOutput) {
        // Run the SPIR-V optimizer
        OptimizerPtr const optimizer = createOptimizer(mOptimization, config);
        optimizeSpirv(optimizer, spirv);
    } else {
        if (!optimizeForSize) {
            OptimizerPtr const optimizer = createOptimizer(mOptimization, config);
            optimizeSpirv(optimizer, spirv);
        }
    }

    fixupClipDistance(spirv, config);

    if (internalConfig.spirvOutput) {
        *internalConfig.spirvOutput = spirv;
    }

    if (internalConfig.mslOutput) {
        auto sibs = SibVector::with_capacity(CONFIG_SAMPLER_BINDING_COUNT);
        msl::collectSibs(config, sibs);
        spirvToMsl(&spirv, internalConfig.mslOutput, config.shaderModel, config.hasFramebufferFetch,
                sibs, mGenerateDebugInfo ? &internalConfig.minifier : nullptr);
    }

    // Transpile back to GLSL
    if (internalConfig.glslOutput) {
        CompilerGLSL::Options glslOptions;
        auto version = GLSLTools::getShadingLanguageVersion(
                config.shaderModel, config.featureLevel);
        glslOptions.es = version.second;
        glslOptions.version = version.first;
        glslOptions.enable_420pack_extension = glslOptions.version >= 420;
        glslOptions.fragment.default_float_precision = glslOptions.es ?
                CompilerGLSL::Options::Precision::Mediump : CompilerGLSL::Options::Precision::Highp;
        glslOptions.fragment.default_int_precision = glslOptions.es ?
                CompilerGLSL::Options::Precision::Mediump : CompilerGLSL::Options::Precision::Highp;

        // TODO: this should be done only on the "feature level 0" variant
        if (config.featureLevel == 0) {
            // convert UBOs to plain uniforms if we're at feature level 0
            glslOptions.emit_uniform_buffer_as_plain_uniforms = true;
        }

        // For stereo variants using multiview feature, this generates the shader code below.
        //   #extension GL_OVR_multiview2 : require
        //   layout(num_views = 2) in;
        if (config.variant.hasStereo() &&
            config.shaderType == ShaderStage::VERTEX &&
            config.materialInfo->stereoscopicType == StereoscopicType::MULTIVIEW) {
            // FIXME: This value should be changed along with the settings in CodeGenerator.cpp.
            glslOptions.ovr_multiview_view_count = 2;
        }

        CompilerGLSL glslCompiler(std::move(spirv));
        glslCompiler.set_common_options(glslOptions);

        if (!glslOptions.es) {
            // enable GL_ARB_shading_language_packing if available
            glslCompiler.add_header_line("#extension GL_ARB_shading_language_packing : enable");
        }

        if (tShader.getStage() == EShLangFragment && glslOptions.es) {
            for (auto i : config.glsl.subpassInputToColorLocation) {
                glslCompiler.remap_ext_framebuffer_fetch(i.first, i.second, true);
            }
        }

#ifdef SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS
        *internalConfig.glslOutput = glslCompiler.compile();
#else
        try {
            *internalConfig.glslOutput = glslCompiler.compile();
        } catch (spirv_cross::CompilerError e) {
            slog.e << "ERROR: " << e.what() << io::endl;
            return false;
        }
#endif

        // spirv-cross automatically redeclares gl_ClipDistance if it's used. Some drivers don't
        // like this, so we simply remove it.
        // According to EXT_clip_cull_distance, gl_ClipDistance can be
        // "implicitly sized by indexing it only with integral constant expressions".
        std::string& str = *internalConfig.glslOutput;
        const std::string clipDistanceDefinition = "out float gl_ClipDistance[2];";
        size_t const found = str.find(clipDistanceDefinition);
        if (found != std::string::npos) {
            str.replace(found, clipDistanceDefinition.length(), "");
        }
    }
    return true;
}

std::shared_ptr<spvtools::Optimizer> GLSLPostProcessor::createOptimizer(
        MaterialBuilder::Optimization optimization, Config const& config) {
    auto optimizer = std::make_shared<spvtools::Optimizer>(SPV_ENV_UNIVERSAL_1_3);

    optimizer->SetMessageConsumer([](spv_message_level_t level,
            const char* source, const spv_position_t& position, const char* message) {
        if (!filterSpvOptimizerMessage(level)) {
            return;
        }
        slog.e << stringifySpvOptimizerMessage(level, source, position, message)
                << io::endl;
    });

    if (optimization == MaterialBuilder::Optimization::SIZE) {
        registerSizePasses(*optimizer, config);
    } else if (optimization == MaterialBuilder::Optimization::PERFORMANCE) {
        registerPerformancePasses(*optimizer, config);
    }

    // Metal doesn't support relaxed precision, but does have support for float16 math operations.
    if (config.targetApi == MaterialBuilder::TargetApi::METAL) {
        optimizer->RegisterPass(CreateConvertRelaxedToHalfPass());
        optimizer->RegisterPass(CreateSimplificationPass());
        optimizer->RegisterPass(CreateRedundancyEliminationPass());
        optimizer->RegisterPass(CreateAggressiveDCEPass());
    }

    return optimizer;
}

void GLSLPostProcessor::optimizeSpirv(OptimizerPtr optimizer, SpirvBlob& spirv) const {
    if (!optimizer->Run(spirv.data(), spirv.size(), &spirv)) {
        slog.e << "SPIR-V optimizer pass failed" << io::endl;
        return;
    }

    // Remove dead module-level objects: functions, types, vars
    spv::spirvbin_t remapper(0);
    remapper.remap(spirv, spv::spirvbin_base_t::DCE_ALL);
}

void GLSLPostProcessor::fixupClipDistance(
        SpirvBlob& spirv, GLSLPostProcessor::Config const& config) const {
    if (!config.usesClipDistance) {
        return;
    }
    // This should match the version of SPIR-V used in GLSLTools::prepareShaderParser.
    SpirvTools const tools(SPV_ENV_UNIVERSAL_1_3);
    std::string disassembly;
    const bool result = tools.Disassemble(spirv, &disassembly);
    assert_invariant(result);
    if (filamat::fixupClipDistance(disassembly)) {
        spirv.clear();
        tools.Assemble(disassembly, &spirv);
        assert_invariant(tools.Validate(spirv));
    }
}

// CreateMergeReturnPass() causes these issues:
// - triggers a segfault with AMD OpenGL drivers on macOS
// - triggers a crash on some Adreno drivers (b/291140208, b/289401984, b/289393290)
// However Metal requires this pass in order to correctly generate half-precision MSL
//
// CreateSimplificationPass() creates a lot of problems:
// - Adreno GPU show artifacts after running simplification passes (Vulkan)
// - spirv-cross fails generating working glsl
//      (https://github.com/KhronosGroup/SPIRV-Cross/issues/2162)
// - generally it makes the code more complicated, e.g.: replacing for loops with
//   while-if-break, unclear if it helps for anything.
// However, the simplification passes below are necessary when targeting Metal, otherwise the
// result is mismatched half / float assignments in MSL.


void GLSLPostProcessor::registerPerformancePasses(Optimizer& optimizer, Config const& config) {
    auto RegisterPass = [&](spvtools::Optimizer::PassToken&& pass,
            MaterialBuilder::TargetApi apiFilter = MaterialBuilder::TargetApi::ALL) {
        if (!(config.targetApi & apiFilter)) {
            return;
        }
        optimizer.RegisterPass(std::move(pass));
    };

    RegisterPass(CreateWrapOpKillPass());
    RegisterPass(CreateDeadBranchElimPass());
    RegisterPass(CreateMergeReturnPass(), MaterialBuilder::TargetApi::METAL);
    RegisterPass(CreateInlineExhaustivePass());
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreatePrivateToLocalPass());
    RegisterPass(CreateLocalSingleBlockLoadStoreElimPass());
    RegisterPass(CreateLocalSingleStoreElimPass());
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreateScalarReplacementPass());
    RegisterPass(CreateLocalAccessChainConvertPass());
    RegisterPass(CreateLocalSingleBlockLoadStoreElimPass());
    RegisterPass(CreateLocalSingleStoreElimPass());
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreateLocalMultiStoreElimPass());
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreateCCPPass());
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreateRedundancyEliminationPass());
    RegisterPass(CreateCombineAccessChainsPass());
    RegisterPass(CreateSimplificationPass(), MaterialBuilder::TargetApi::METAL);
    RegisterPass(CreateVectorDCEPass());
    RegisterPass(CreateDeadInsertElimPass());
    RegisterPass(CreateDeadBranchElimPass());
    RegisterPass(CreateSimplificationPass(), MaterialBuilder::TargetApi::METAL);
    RegisterPass(CreateIfConversionPass());
    RegisterPass(CreateCopyPropagateArraysPass());
    RegisterPass(CreateReduceLoadSizePass());
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreateBlockMergePass());
    RegisterPass(CreateRedundancyEliminationPass());
    RegisterPass(CreateDeadBranchElimPass());
    RegisterPass(CreateBlockMergePass());
    RegisterPass(CreateSimplificationPass(), MaterialBuilder::TargetApi::METAL);
}

void GLSLPostProcessor::registerSizePasses(Optimizer& optimizer, Config const& config) {
    auto RegisterPass = [&](spvtools::Optimizer::PassToken&& pass,
            MaterialBuilder::TargetApi apiFilter = MaterialBuilder::TargetApi::ALL) {
        if (!(config.targetApi & apiFilter)) {
            return;
        }
        optimizer.RegisterPass(std::move(pass));
    };

    RegisterPass(CreateWrapOpKillPass());
    RegisterPass(CreateDeadBranchElimPass());
    RegisterPass(CreateMergeReturnPass(), MaterialBuilder::TargetApi::METAL);
    RegisterPass(CreateInlineExhaustivePass());
    RegisterPass(CreateEliminateDeadFunctionsPass());
    RegisterPass(CreatePrivateToLocalPass());
    RegisterPass(CreateScalarReplacementPass(0));
    RegisterPass(CreateLocalMultiStoreElimPass());
    RegisterPass(CreateCCPPass());
    RegisterPass(CreateLoopUnrollPass(true));
    RegisterPass(CreateDeadBranchElimPass());
    RegisterPass(CreateSimplificationPass(), MaterialBuilder::TargetApi::METAL);
    RegisterPass(CreateScalarReplacementPass(0));
    RegisterPass(CreateLocalSingleStoreElimPass());
    RegisterPass(CreateIfConversionPass());
    RegisterPass(CreateSimplificationPass(), MaterialBuilder::TargetApi::METAL);
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreateDeadBranchElimPass());
    RegisterPass(CreateBlockMergePass());
    RegisterPass(CreateLocalAccessChainConvertPass());
    RegisterPass(CreateLocalSingleBlockLoadStoreElimPass());
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreateCopyPropagateArraysPass());
    RegisterPass(CreateVectorDCEPass());
    RegisterPass(CreateDeadInsertElimPass());
    // this breaks UBO layout
    //RegisterPass(CreateEliminateDeadMembersPass());
    RegisterPass(CreateLocalSingleStoreElimPass());
    RegisterPass(CreateBlockMergePass());
    RegisterPass(CreateLocalMultiStoreElimPass());
    RegisterPass(CreateRedundancyEliminationPass());
    RegisterPass(CreateSimplificationPass(), MaterialBuilder::TargetApi::METAL);
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreateCFGCleanupPass());
}

} // namespace filamat
