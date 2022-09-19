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
#include <localintermediate.h>

#include <spirv_glsl.hpp>
#include <spirv_msl.hpp>

#include "sca/builtinResource.h"
#include "sca/GLSLTools.h"
#include "shaders/CodeGenerator.h"
#include "shaders/MaterialInfo.h"

#include <filament/MaterialEnums.h>
#include <private/filament/Variant.h>
#include "SibGenerator.h"

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

template<typename F>
static void forEachSib(const GLSLPostProcessor::Config& config, F fn) {
    const auto& samplerBindings = config.materialInfo->samplerBindings;
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
                    fn(blockIndex, sib);
                }
            }
        case MaterialDomain::POST_PROCESS:
        case MaterialDomain::COMPUTE:
            break;
    }
    fn((uint8_t) SamplerBindingPoints::PER_MATERIAL_INSTANCE, &config.materialInfo->sib);
}

}; // namespace msl

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

void GLSLPostProcessor::spirvToToMsl(const SpirvBlob *spirv, std::string *outMsl,
        const Config &config, ShaderMinifier& minifier) {

    using namespace msl;

    CompilerMSL mslCompiler(*spirv);
    CompilerGLSL::Options options;
    mslCompiler.set_common_options(options);

    const CompilerMSL::Options::Platform platform =
        config.shaderModel == ShaderModel::MOBILE ?
            CompilerMSL::Options::Platform::iOS : CompilerMSL::Options::Platform::macOS;

    CompilerMSL::Options mslOptions = {};
    mslOptions.platform = platform,
    mslOptions.msl_version = config.shaderModel == ShaderModel::MOBILE ?
        CompilerMSL::Options::make_msl_version(2, 0) : CompilerMSL::Options::make_msl_version(2, 2);

    if (config.hasFramebufferFetch) {
        mslOptions.use_framebuffer_fetch_subpasses = true;
        // On macOS, framebuffer fetch is only available starting with MSL 2.3. Filament will only
        // use framebuffer fetch materials on devices that support it.
        if (config.shaderModel == ShaderModel::DESKTOP) {
            mslOptions.msl_version = CompilerMSL::Options::make_msl_version(2, 3);
        }
    }

    mslOptions.argument_buffers = true;

    // Necessary to keep all argument buffers the same size across material variants.
    mslOptions.pad_argument_buffer_resources = true;

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
    forEachSib(config, [&executionModel, &mslCompiler](uint8_t bindingPoint, const SamplerInterfaceBlock* sib) {
        const auto& infoList = sib->getSamplerInfoList();
        for (const auto& info: infoList) {
            // A MSLResourceBinding tells SPIRV-Cross how to map a sampler2D with a given set and
            // binding to Metal resources.
            // For example, textureB in the above example has:
            //   bindingPoint = 0      (it's the first sampler group)
            //   set = 1               (bindingPoint + 1, the first descriptor set is for uniforms)
            //   offset = 1            (it's the second sampler in the sampler group)
            // The equivalent MSL texture argument is assigned the [[id(2)]] binding and its sampler
            // resource assigned the [[id(3)]] binding (controlled via msl_texture and msl_sampler).
            MSLResourceBinding newBinding;
            newBinding.basetype = SPIRType::BaseType::SampledImage;
            newBinding.stage = executionModel;
            newBinding.desc_set = bindingPoint + 1;
            newBinding.binding = info.offset;
            newBinding.count = 1;
            newBinding.msl_texture = info.offset * 2;
            newBinding.msl_sampler = info.offset * 2 + 1;
            mslCompiler.add_msl_resource_binding(newBinding);
        }
        // This final MSLResourceBinding is how we control the [[buffer(n)]] binding of the argument
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
    });

    auto updateResourceBindingDefault = [executionModel, &mslCompiler]
            (const auto& resource, const BindingIndexMap* map = nullptr) {
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

    auto resources = mslCompiler.get_shader_resources();
    for (const auto& resource : resources.uniform_buffers) {
        updateResourceBindingDefault(resource);
    }

    // Descriptor set 0 is uniforms. The add_discrete_descriptor_set call here prevents the uniforms
    // from becoming argument buffers.
    mslCompiler.add_discrete_descriptor_set(0);

    *outMsl = mslCompiler.compile();
    *outMsl = minifier.removeWhitespace(*outMsl);
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
    GLSLangCleaner cleaner;

    const char* shaderCString = inputShader.c_str();
    tShader.setStrings(&shaderCString, 1);

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

    bool ok = tShader.parse(&DefaultTBuiltInResource, internalConfig.langVersion, false, msg);
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
    bool linkOk = program.link(msg);
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
                if (internalConfig.mslOutput) {
                    spirvToToMsl(internalConfig.spirvOutput, internalConfig.mslOutput, config,
                            internalConfig.minifier);
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
            fullOptimization(tShader, config, internalConfig);
            break;
    }

    if (internalConfig.glslOutput) {
        *internalConfig.glslOutput =
                internalConfig.minifier.removeWhitespace(*internalConfig.glslOutput);

        // In theory this should only be enabled for SIZE, but in practice we often use PERFORMANCE.
        if (mOptimization != MaterialBuilder::Optimization::NONE) {
           *internalConfig.glslOutput =
                   internalConfig.minifier.renameStructFields(*internalConfig.glslOutput);
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
    EShMessages msg = GLSLTools::glslangFlagsFromTargetApi(config.targetApi, config.targetLanguage);
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
        GLSLangCleaner cleaner;

        const char* shaderCString = glsl.c_str();
        spirvShader.setStrings(&shaderCString, 1);
        GLSLTools::prepareShaderParser(config.targetApi, config.targetLanguage, spirvShader,
                internalConfig.shLang, internalConfig.langVersion);
        ok = spirvShader.parse(&DefaultTBuiltInResource, internalConfig.langVersion, false, msg);
        program.addShader(&spirvShader);
        // Even though we only have a single shader stage, linking is still necessary to finalize
        // SPIR-V types
        bool linkOk = program.link(msg);
        if (!ok || !linkOk) {
            slog.e << spirvShader.getInfoLog() << io::endl;
        } else {
            SpvOptions options;
            options.generateDebugInfo = mGenerateDebugInfo;
            GlslangToSpv(*program.getIntermediate(internalConfig.shLang),
                    *internalConfig.spirvOutput, &options);
        }
    }

    if (internalConfig.mslOutput) {
        spirvToToMsl(internalConfig.spirvOutput, internalConfig.mslOutput, config,
                internalConfig.minifier);
    }

    if (internalConfig.glslOutput) {
        *internalConfig.glslOutput = glsl;
    }
}

void GLSLPostProcessor::fullOptimization(const TShader& tShader,
        GLSLPostProcessor::Config const& config, InternalConfig& internalConfig) const {
    SpirvBlob spirv;

    // Compile GLSL to to SPIR-V
    SpvOptions options;
    options.generateDebugInfo = mGenerateDebugInfo;
    GlslangToSpv(*tShader.getIntermediate(), spirv, &options);

    // Run the SPIR-V optimizer
    OptimizerPtr optimizer = createOptimizer(mOptimization, config);
    optimizeSpirv(optimizer, spirv);

    if (internalConfig.spirvOutput) {
        *internalConfig.spirvOutput = spirv;
    }

    if (internalConfig.mslOutput) {
        spirvToToMsl(&spirv, internalConfig.mslOutput, config, internalConfig.minifier);
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

        *internalConfig.glslOutput = glslCompiler.compile();
    }
}

std::shared_ptr<spvtools::Optimizer> GLSLPostProcessor::createOptimizer(
        MaterialBuilder::Optimization optimization, Config const& config) {
    auto optimizer = std::make_shared<spvtools::Optimizer>(SPV_ENV_UNIVERSAL_1_0);

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
        // Metal doesn't support relaxed precision, but does have support for float16 math operations.
        if (config.targetApi == MaterialBuilder::TargetApi::METAL) {
            optimizer->RegisterPass(CreateConvertRelaxedToHalfPass());
            optimizer->RegisterPass(CreateSimplificationPass());
            optimizer->RegisterPass(CreateRedundancyEliminationPass());
            optimizer->RegisterPass(CreateAggressiveDCEPass());
        }
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

void GLSLPostProcessor::registerPerformancePasses(Optimizer& optimizer, Config const& config) {
    optimizer
            .RegisterPass(CreateWrapOpKillPass())
            .RegisterPass(CreateDeadBranchElimPass());

    if (config.shaderModel != ShaderModel::DESKTOP ||
            config.targetApi != MaterialBuilder::TargetApi::OPENGL) {
        // this triggers a segfault with AMD OpenGL drivers on MacOS
        // note that Metal also requires this pass in order to correctly generate half-precision MSL
        optimizer.RegisterPass(CreateMergeReturnPass());
    }

    optimizer
            .RegisterPass(CreateInlineExhaustivePass())
            .RegisterPass(CreateAggressiveDCEPass())
            .RegisterPass(CreatePrivateToLocalPass())
            .RegisterPass(CreateLocalSingleBlockLoadStoreElimPass())
            .RegisterPass(CreateLocalSingleStoreElimPass())
            .RegisterPass(CreateAggressiveDCEPass())
            .RegisterPass(CreateScalarReplacementPass())
            .RegisterPass(CreateLocalAccessChainConvertPass())
            .RegisterPass(CreateLocalSingleBlockLoadStoreElimPass())
            .RegisterPass(CreateLocalSingleStoreElimPass())
            .RegisterPass(CreateAggressiveDCEPass())
            .RegisterPass(CreateLocalMultiStoreElimPass())
            .RegisterPass(CreateAggressiveDCEPass())
            .RegisterPass(CreateCCPPass())
            .RegisterPass(CreateAggressiveDCEPass())
            .RegisterPass(CreateRedundancyEliminationPass())
            .RegisterPass(CreateCombineAccessChainsPass())
            .RegisterPass(CreateSimplificationPass())
            .RegisterPass(CreateVectorDCEPass())
            .RegisterPass(CreateDeadInsertElimPass())
            .RegisterPass(CreateDeadBranchElimPass())
            .RegisterPass(CreateSimplificationPass())
            .RegisterPass(CreateIfConversionPass())
            .RegisterPass(CreateCopyPropagateArraysPass())
            .RegisterPass(CreateReduceLoadSizePass())
            .RegisterPass(CreateAggressiveDCEPass())
            .RegisterPass(CreateBlockMergePass())
            .RegisterPass(CreateRedundancyEliminationPass())
            .RegisterPass(CreateDeadBranchElimPass())
            .RegisterPass(CreateBlockMergePass())
            .RegisterPass(CreateSimplificationPass());
}

void GLSLPostProcessor::registerSizePasses(Optimizer& optimizer, Config const& config) {
    optimizer
            .RegisterPass(CreateWrapOpKillPass())
            .RegisterPass(CreateDeadBranchElimPass());

    if (config.shaderModel != ShaderModel::DESKTOP) {
        // this triggers a segfault with AMD drivers on MacOS
        optimizer.RegisterPass(CreateMergeReturnPass());
    }

    optimizer
            .RegisterPass(CreateInlineExhaustivePass())
            .RegisterPass(CreateEliminateDeadFunctionsPass())
            .RegisterPass(CreatePrivateToLocalPass())
            .RegisterPass(CreateScalarReplacementPass(0))
            .RegisterPass(CreateLocalMultiStoreElimPass())
            .RegisterPass(CreateCCPPass())
            .RegisterPass(CreateLoopUnrollPass(true))
            .RegisterPass(CreateDeadBranchElimPass())
            .RegisterPass(CreateSimplificationPass())
            .RegisterPass(CreateScalarReplacementPass(0))
            .RegisterPass(CreateLocalSingleStoreElimPass())
            .RegisterPass(CreateIfConversionPass())
            .RegisterPass(CreateSimplificationPass())
            .RegisterPass(CreateAggressiveDCEPass())
            .RegisterPass(CreateDeadBranchElimPass())
            .RegisterPass(CreateBlockMergePass())
            .RegisterPass(CreateLocalAccessChainConvertPass())
            .RegisterPass(CreateLocalSingleBlockLoadStoreElimPass())
            .RegisterPass(CreateAggressiveDCEPass())
            .RegisterPass(CreateCopyPropagateArraysPass())
            .RegisterPass(CreateVectorDCEPass())
            .RegisterPass(CreateDeadInsertElimPass())
            // this breaks UBO layout
            //.RegisterPass(CreateEliminateDeadMembersPass())
            .RegisterPass(CreateLocalSingleStoreElimPass())
            .RegisterPass(CreateBlockMergePass())
            .RegisterPass(CreateLocalMultiStoreElimPass())
            .RegisterPass(CreateRedundancyEliminationPass())
            .RegisterPass(CreateSimplificationPass())
            .RegisterPass(CreateAggressiveDCEPass())
            .RegisterPass(CreateCFGCleanupPass());
}

} // namespace filamat
