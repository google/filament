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
#include "private/filament/DescriptorSets.h"
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

#ifdef FILAMENT_SUPPORTS_WEBGPU
#include <tint/tint.h>
#endif

using namespace glslang;
using namespace spirv_cross;
using namespace spvtools;
using namespace filament;
using namespace filament::backend;

namespace filamat {

using namespace utils;

namespace msl {  // this is only used for MSL

using BindingIndexMap = std::unordered_map<std::string, uint16_t>;

#ifndef DEBUG_LOG_DESCRIPTOR_SETS
#define DEBUG_LOG_DESCRIPTOR_SETS 0
#endif

const char* toString(DescriptorType type) {
    switch (type) {
        case DescriptorType::UNIFORM_BUFFER:
            return "UNIFORM_BUFFER";
        case DescriptorType::SHADER_STORAGE_BUFFER:
            return "SHADER_STORAGE_BUFFER";
        case DescriptorType::SAMPLER:
            return "SAMPLER";
        case DescriptorType::INPUT_ATTACHMENT:
            return "INPUT_ATTACHMENT";
        case DescriptorType::SAMPLER_EXTERNAL:
            return "SAMPLER_EXTERNAL";
    }
}

const char* toString(ShaderStageFlags flags) {
    std::vector<const char*> stages;
    if (any(flags & ShaderStageFlags::VERTEX)) {
        stages.push_back("VERTEX");
    }
    if (any(flags & ShaderStageFlags::FRAGMENT)) {
        stages.push_back("FRAGMENT");
    }
    if (any(flags & ShaderStageFlags::COMPUTE)) {
        stages.push_back("COMPUTE");
    }
    if (stages.empty()) {
        return "NONE";
    }
    static char buffer[64];
    buffer[0] = '\0';
    for (size_t i = 0; i < stages.size(); i++) {
        if (i > 0) {
            strcat(buffer, " | ");
        }
        strcat(buffer, stages[i]);
    }
    return buffer;
}

const char* prettyDescriptorFlags(DescriptorFlags flags) {
    if (flags == DescriptorFlags::DYNAMIC_OFFSET) {
        return "DYNAMIC_OFFSET";
    }
    return "NONE";
}

const char* prettyPrintSamplerType(SamplerType type) {
    switch (type) {
        case SamplerType::SAMPLER_2D:
            return "SAMPLER_2D";
        case SamplerType::SAMPLER_2D_ARRAY:
            return "SAMPLER_2D_ARRAY";
        case SamplerType::SAMPLER_CUBEMAP:
            return "SAMPLER_CUBEMAP";
        case SamplerType::SAMPLER_EXTERNAL:
            return "SAMPLER_EXTERNAL";
        case SamplerType::SAMPLER_3D:
            return "SAMPLER_3D";
        case SamplerType::SAMPLER_CUBEMAP_ARRAY:
            return "SAMPLER_CUBEMAP_ARRAY";
    }
}

DescriptorSetLayout getPerMaterialDescriptorSet(SamplerInterfaceBlock const& sib) noexcept {
    auto const& samplers = sib.getSamplerInfoList();

    DescriptorSetLayout layout;
    layout.bindings.reserve(1 + samplers.size());

    layout.bindings.push_back(DescriptorSetLayoutBinding { DescriptorType::UNIFORM_BUFFER,
            ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT,
            +PerMaterialBindingPoints::MATERIAL_PARAMS, DescriptorFlags::NONE, 0 });

    for (auto const& sampler : samplers) {
        layout.bindings.push_back(DescriptorSetLayoutBinding {
                (sampler.type == SamplerInterfaceBlock::Type::SAMPLER_EXTERNAL) ?
                        DescriptorType::SAMPLER_EXTERNAL : DescriptorType::SAMPLER,
                ShaderStageFlags::VERTEX | ShaderStageFlags::FRAGMENT, sampler.binding,
                DescriptorFlags::NONE, 0 });
    }

    return layout;
}

static void collectDescriptorsForSet(filament::DescriptorSetBindingPoints set,
        const GLSLPostProcessor::Config& config, DescriptorSetInfo& descriptors) {
    const MaterialInfo& material = *config.materialInfo;

    DescriptorSetLayout const info = [&]() {
        switch (set) {
            case DescriptorSetBindingPoints::PER_VIEW: {
                if (filament::Variant::isValidDepthVariant(config.variant)) {
                    return descriptor_sets::getDepthVariantLayout();
                }
                if (filament::Variant::isSSRVariant(config.variant)) {
                    return descriptor_sets::getSsrVariantLayout();
                }
                return descriptor_sets::getPerViewDescriptorSetLayout(config.domain,
                        config.variantFilter,
                        material.isLit || material.hasShadowMultiplier,
                        material.reflectionMode,
                        material.refractionMode);
            }
            case DescriptorSetBindingPoints::PER_RENDERABLE:
                return descriptor_sets::getPerRenderableLayout();
            case DescriptorSetBindingPoints::PER_MATERIAL:
                return getPerMaterialDescriptorSet(config.materialInfo->sib);
            default:
                return DescriptorSetLayout {};
        }
    }();

    auto samplerList = [&]() {
        switch (set) {
            case DescriptorSetBindingPoints::PER_VIEW:
                return SibGenerator::getPerViewSib(config.variant).getSamplerInfoList();
            case DescriptorSetBindingPoints::PER_RENDERABLE:
                return SibGenerator::getPerRenderableSib(config.variant).getSamplerInfoList();
            case DescriptorSetBindingPoints::PER_MATERIAL:
                return config.materialInfo->sib.getSamplerInfoList();
            default:
                return SamplerInterfaceBlock::SamplerInfoList {};
        }
    }();

    // remove all the samplers that are not included in the descriptor-set layout
    samplerList.erase(std::remove_if(samplerList.begin(), samplerList.end(),
                              [&info](auto const& entry) {
                                  auto pos = std::find_if(info.bindings.begin(),
                                          info.bindings.end(), [&entry](const auto& item) {
                                              return item.binding == entry.binding;
                                          });
                                  return pos == info.bindings.end();
                              }),
            samplerList.end());

    auto getDescriptorName = [&](DescriptorSetBindingPoints set, descriptor_binding_t binding) {
        if (set == DescriptorSetBindingPoints::PER_MATERIAL) {
            auto pos = std::find_if(samplerList.begin(), samplerList.end(),
                    [&](const auto& entry) { return entry.binding == binding; });
            if (pos == samplerList.end()) {
                return descriptor_sets::getDescriptorName(set, binding);
            }
            SamplerInterfaceBlock::SamplerInfo& sampler = *pos;
            return sampler.uniformName;
        }
        return descriptor_sets::getDescriptorName(set, binding);
    };

    for (size_t i = 0; i < info.bindings.size(); i++) {
        backend::descriptor_binding_t binding = info.bindings[i].binding;
        auto name = getDescriptorName(set, binding);
        if (info.bindings[i].type == DescriptorType::SAMPLER ||
            info.bindings[i].type == DescriptorType::SAMPLER_EXTERNAL) {
            auto pos = std::find_if(samplerList.begin(), samplerList.end(),
                    [&](const auto& entry) { return entry.binding == binding; });
            assert_invariant(pos != samplerList.end());
            SamplerInterfaceBlock::SamplerInfo& sampler = *pos;
            descriptors.emplace_back(name, info.bindings[i], sampler);
        } else {
            descriptors.emplace_back(name, info.bindings[i], std::nullopt);
        }
    }

    std::sort(descriptors.begin(), descriptors.end(), [](const auto& a, const auto& b) {
        return std::get<1>(a).binding < std::get<1>(b).binding;
    });
}

void prettyPrintDescriptorSetInfoVector(DescriptorSets const& sets) {
    auto getName = [](uint8_t set) {
        switch (set) {
            case +DescriptorSetBindingPoints::PER_VIEW:
                return "perViewDescriptorSetLayout";
            case +DescriptorSetBindingPoints::PER_RENDERABLE:
                return "perRenderableDescriptorSetLayout";
            case +DescriptorSetBindingPoints::PER_MATERIAL:
                return "perMaterialDescriptorSetLayout";
            default:
                return "unknown";
        }
    };
    for (size_t setIndex = 0; setIndex < MAX_DESCRIPTOR_SET_COUNT; setIndex++) {
        auto const& descriptors = sets[setIndex];
        printf("[DS] info (%s) = [\n", getName(setIndex));
        for (auto const& descriptor : descriptors) {
            auto const& [name, info, sampler] = descriptor;
            if (info.type == DescriptorType::SAMPLER ||
                info.type == DescriptorType::SAMPLER_EXTERNAL) {
                assert_invariant(sampler.has_value());
                printf("    {name = %s, binding = %d, type = %s, count = %d, stage = %s, flags = "
                       "%s, samplerType = %s}",
                        name.c_str_safe(), info.binding, toString(info.type), info.count,
                        toString(info.stageFlags), prettyDescriptorFlags(info.flags),
                        prettyPrintSamplerType(sampler->type));
            } else {
                printf("    {name = %s, binding = %d, type = %s, count = %d, stage = %s, flags = "
                       "%s}",
                        name.c_str_safe(), info.binding, toString(info.type), info.count,
                        toString(info.stageFlags), prettyDescriptorFlags(info.flags));
            }
            printf(",\n");
        }
        printf("]\n");
    }
}

static void collectDescriptorSets(const GLSLPostProcessor::Config& config, DescriptorSets& sets) {
    auto perViewDescriptors = DescriptorSetInfo::with_capacity(MAX_DESCRIPTOR_COUNT);
    collectDescriptorsForSet(DescriptorSetBindingPoints::PER_VIEW, config, perViewDescriptors);
    sets[+DescriptorSetBindingPoints::PER_VIEW] = std::move(perViewDescriptors);

    auto perRenderableDescriptors = DescriptorSetInfo::with_capacity(MAX_DESCRIPTOR_COUNT);
    collectDescriptorsForSet(
            DescriptorSetBindingPoints::PER_RENDERABLE, config, perRenderableDescriptors);
    sets[+DescriptorSetBindingPoints::PER_RENDERABLE] = std::move(perRenderableDescriptors);

    auto perMaterialDescriptors = DescriptorSetInfo::with_capacity(MAX_DESCRIPTOR_COUNT);
    collectDescriptorsForSet(
            DescriptorSetBindingPoints::PER_MATERIAL, config, perMaterialDescriptors);
    sets[+DescriptorSetBindingPoints::PER_MATERIAL] = std::move(perMaterialDescriptors);
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

    // Similar to above, we need to do a no-op remap to init a static table in the remapper before
    // the jobs start using remap().
    spv::spirvbin_t remapper(0);
    // We need to provide at least a valid header to not crash.
    SpirvBlob spirv {
        0x07230203,// MAGIC
        0,         // VERSION
        0,         // GENERATOR
        0,         // BOUND
        0          // SCHEMA, must be 0
    };
    remapper.remap(spirv, 0);
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

void GLSLPostProcessor::spirvToMsl(const SpirvBlob* spirv, std::string* outMsl,
        filament::backend::ShaderStage stage, filament::backend::ShaderModel shaderModel,
        bool useFramebufferFetch, const DescriptorSets& descriptorSets,
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
    mslOptions.ios_support_base_vertex_instance = true;
    mslOptions.dynamic_offsets_buffer_index = 25;

    mslCompiler.set_msl_options(mslOptions);



    auto executionModel = mslCompiler.get_execution_model();

    // Map each descriptor set (argument buffer) to a [[buffer(n)]] binding.
    // For example, mapDescriptorSet(0, 21) says "map descriptor set 0 to [[buffer(21)]]"
    auto mapDescriptorSet = [&mslCompiler](uint32_t set, uint32_t buffer) {
        MSLResourceBinding argBufferBinding;
        argBufferBinding.basetype = SPIRType::BaseType::Float;
        argBufferBinding.stage = mslCompiler.get_execution_model();
        argBufferBinding.desc_set = set;
        argBufferBinding.binding = kArgumentBufferBinding;
        argBufferBinding.count = 1;
        argBufferBinding.msl_buffer = buffer;
        mslCompiler.add_msl_resource_binding(argBufferBinding);
    };
    for (int i = 0; i < MAX_DESCRIPTOR_SET_COUNT; i++) {
        mapDescriptorSet(i, CodeGenerator::METAL_DESCRIPTOR_SET_BINDING_START + i);
    }

    auto resources = mslCompiler.get_shader_resources();

    // We're using argument buffers for descriptor sets, however, we cannot rely on spirv-cross to
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
    // with our own that contain all the descriptors, even those optimized away.
    std::vector<MetalArgumentBuffer*> argumentBuffers;
    size_t dynamicOffsetsBufferIndex = 0;
    for (size_t setIndex = 0; setIndex < MAX_DESCRIPTOR_SET_COUNT; setIndex++) {
        auto const& descriptors = descriptorSets[setIndex];
        auto argBufferBuilder = MetalArgumentBuffer::Builder().name(
                "spvDescriptorSetBuffer" + std::to_string(int(setIndex)));
        for (auto const& descriptor : descriptors) {
            auto const& [name, info, sampler] = descriptor;
            if (!hasShaderType(info.stageFlags, stage)) {
                if (any(info.flags & DescriptorFlags::DYNAMIC_OFFSET)) {
                    // We still need to increment the dynamic offset index
                    dynamicOffsetsBufferIndex++;
                }
                continue;
            }
            switch (info.type) {
                case DescriptorType::INPUT_ATTACHMENT:
                    // TODO: Handle INPUT_ATTACHMENT case
                    break;
                case DescriptorType::UNIFORM_BUFFER:
                case DescriptorType::SHADER_STORAGE_BUFFER: {
                    std::string lowercasedName = name.c_str();
                    assert_invariant(!lowercasedName.empty());
                    lowercasedName[0] = std::tolower(lowercasedName[0]);
                    argBufferBuilder
                            .buffer(info.binding * 2 + 0, name.c_str(), lowercasedName);
                    if (any(info.flags & DescriptorFlags::DYNAMIC_OFFSET)) {
                        // Note: this requires that the sets and descriptors are sorted (at least
                        // the uniforms).
                        mslCompiler.add_dynamic_buffer(
                                setIndex, info.binding * 2 + 0, dynamicOffsetsBufferIndex++);
                    }
                    break;
                }

                case DescriptorType::SAMPLER:
                case DescriptorType::SAMPLER_EXTERNAL: {
                    assert_invariant(sampler.has_value());
                    const std::string samplerName = std::string(name.c_str()) + "Smplr";
                    argBufferBuilder
                            .texture(info.binding * 2 + 0, name.c_str(), sampler->type,
                                    sampler->format, sampler->multisample)
                            .sampler(info.binding * 2 + 1, samplerName);
                    break;
                }
            }
        }
        argumentBuffers.push_back(argBufferBuilder.build());
    }

    // Bind push constants to [buffer(26)]
    MSLResourceBinding pushConstantBinding;
    // the baseType doesn't matter, but can't be UNKNOWN
    pushConstantBinding.basetype = SPIRType::BaseType::Struct;
    pushConstantBinding.stage = executionModel;
    pushConstantBinding.desc_set = kPushConstDescSet;
    pushConstantBinding.binding = kPushConstBinding;
    pushConstantBinding.count = 1;
    pushConstantBinding.msl_buffer = CodeGenerator::METAL_PUSH_CONSTANT_BUFFER_INDEX;
    mslCompiler.add_msl_resource_binding(pushConstantBinding);

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

void GLSLPostProcessor::rebindImageSamplerForWGSL(std::vector<uint32_t> &spirv) {
    constexpr size_t HEADER_SIZE = 5;
    size_t const dataSize = spirv.size();
    uint32_t *data = spirv.data();

    std::set<uint32_t> samplerTargetIDs;

    auto pass = [&](uint32_t targetOp, std::function<void(uint32_t)> f) {
        for (uint32_t cursor = HEADER_SIZE, cursorEnd = dataSize; cursor < cursorEnd;) {
            uint32_t const firstWord = data[cursor];
            uint32_t const wordCount = firstWord >> 16;
            uint32_t const op = firstWord & 0x0000FFFF;
            if (targetOp == op) {
                f(cursor + 1);
            }
            cursor += wordCount;
        }
    };

    //Parse through debug name info to determine which bindings are samplers and which are not.
    // This is possible because the sampler splitting pass outputs sampler and texture pairs of the form:
    // `uniform sampler2D var_x` => `uniform sampler var_sampler` and `uniform texture2D var_texture`;
    // TODO: This works, but may limit what optimizations can be done and has the potential to collide with user
    // variable names. Ideally, trace usage to determine binding type.
    pass(spv::Op::OpName, [&](uint32_t pos) {
        auto target = data[pos];
        char *name = (char *) &data[pos + 1];
        std::string_view view(name);
        if (view.find("_sampler") != std::string_view::npos) {
            samplerTargetIDs.insert(target);
        }
    });

    // Write out the offset bindings
    pass(spv::Op::OpDecorate, [&](uint32_t pos) {
        uint32_t const type = data[pos + 1];
        if (type == spv::Decoration::DecorationBinding) {
            uint32_t const targetVar = data[pos];
            if (samplerTargetIDs.find(targetVar) != samplerTargetIDs.end()) {
                data[pos + 2] = data[pos + 2] * 2 + 1;
            } else {
                data[pos + 2] = data[pos + 2] * 2;
            }
        }
    });
}

bool GLSLPostProcessor::spirvToWgsl(SpirvBlob *spirv, std::string *outWsl) {
#if FILAMENT_SUPPORTS_WEBGPU
    //We need to run some opt-passes at all times to transpile to WGSL
    auto optimizer = createEmptyOptimizer();
    optimizer->RegisterPass(CreateSplitCombinedImageSamplerPass());
    optimizeSpirv(optimizer, *spirv);

    //After splitting the image samplers, we need to remap the bindings to separate them.
    rebindImageSamplerForWGSL(*spirv);

    //Allow non-uniform derivitives due to our nested shaders. See https://github.com/gpuweb/gpuweb/issues/3479
    const tint::spirv::reader::Options readerOpts{true};
    tint::wgsl::writer::Options writerOpts{};

    tint::Program tintRead = tint::spirv::reader::Read(*spirv, readerOpts);

    if (tintRead.Diagnostics().ContainsErrors()) {
        //We know errors can potentially crop up, and want the ability to ignore them if needed for sample bringup
#ifndef FILAMENT_WEBGPU_IGNORE_TNT_READ_ERRORS
        slog.e << "Tint Reader Error: " << tintRead.Diagnostics().Str() << io::endl;
        spv_context context = spvContextCreate(SPV_ENV_VULKAN_1_1_SPIRV_1_4);
        spv_text text = nullptr;
        spv_diagnostic diagnostic = nullptr;
        spv_result_t result = spvBinaryToText(
            context,
            spirv->data(),
            spirv->size(),
            SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES | SPV_BINARY_TO_TEXT_OPTION_COLOR,
            &text,
            &diagnostic);
        slog.e << "Beginning SpirV-output dump with ret " << result << "\n\n" << text->str << "\n\nEndSPIRV\n" <<
                io::endl;
        spvTextDestroy(text);
        slog.e << "Tint Reader Error: " << tintRead.Diagnostics().Str() << io::endl;
        return false;
#endif
    }

    tint::Result<tint::wgsl::writer::Output> wgslOut = tint::wgsl::writer::Generate(tintRead,writerOpts);
    /// An instance of SuccessType that can be used to check a tint Result.
    tint::SuccessType tintSuccess;

    if (wgslOut != tintSuccess) {
        slog.e << "Tint writer error: " << wgslOut.Failure().reason << io::endl;
        return false;
    }
    *outWsl = wgslOut->wgsl;
    return true;
#else
    slog.i << "Trying to emit WGSL without including WebGPU dependencies, please set CMake arg FILAMENT_SUPPORTS_WEBGPU and FILAMENT_SUPPORTS_WEBGPU" << io::endl;
    return false;
#endif

}

bool GLSLPostProcessor::process(const std::string& inputShader, Config const& config,
                                std::string* outputGlsl, SpirvBlob* outputSpirv, std::string* outputMsl, std::string* outputWgsl) {
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
            .wgslOutput = outputWgsl,
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
                    DescriptorSets descriptors {};
                    msl::collectDescriptorSets(config, descriptors);
#if DEBUG_LOG_DESCRIPTOR_SETS == 1
                    msl::prettyPrintDescriptorSetInfoVector(descriptors);
#endif
                    spirvToMsl(internalConfig.spirvOutput, internalConfig.mslOutput,
                            config.shaderType, config.shaderModel, config.hasFramebufferFetch, descriptors,
                            mGenerateDebugInfo ? &internalConfig.minifier : nullptr);
                }
                if (internalConfig.wgslOutput) {
                    if (!spirvToWgsl(internalConfig.spirvOutput, internalConfig.wgslOutput)) {
                        return false;
                    }
                }
            } else {
                slog.e << "GLSL post-processor invoked with optimization level NONE"
                        << io::endl;
            }
            break;
        case MaterialBuilder::Optimization::PREPROCESSOR:
            if (!preprocessOptimization(tShader, config, internalConfig)) {
                return false;
            }
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

bool GLSLPostProcessor::preprocessOptimization(glslang::TShader& tShader,
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
        return false;
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
            return false;
        } else {
            SpvOptions options;
            options.generateDebugInfo = mGenerateDebugInfo;
            GlslangToSpv(*program.getIntermediate(internalConfig.shLang),
                    *internalConfig.spirvOutput, &options);
            fixupClipDistance(*internalConfig.spirvOutput, config);
        }
    }

    if (internalConfig.mslOutput) {
        DescriptorSets descriptors {};
        msl::collectDescriptorSets(config, descriptors);
#if DEBUG_LOG_DESCRIPTOR_SETS == 1
        msl::prettyPrintDescriptorSetInfoVector(descriptors);
#endif
        spirvToMsl(internalConfig.spirvOutput, internalConfig.mslOutput, config.shaderType,
                config.shaderModel, config.hasFramebufferFetch, descriptors,
                mGenerateDebugInfo ? &internalConfig.minifier : nullptr);
    }
    if (internalConfig.wgslOutput) {
        if (!spirvToWgsl(internalConfig.spirvOutput, internalConfig.wgslOutput)) {
            return false;
        }
    }


    if (internalConfig.glslOutput) {
        *internalConfig.glslOutput = glsl;
    }
    return true;
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
        DescriptorSets descriptors {};
        msl::collectDescriptorSets(config, descriptors);
#if DEBUG_LOG_DESCRIPTOR_SETS == 1
        msl::prettyPrintDescriptorSetInfoVector(descriptors);
#endif
        spirvToMsl(&spirv, internalConfig.mslOutput, config.shaderType, config.shaderModel,
                config.hasFramebufferFetch, descriptors,
                mGenerateDebugInfo ? &internalConfig.minifier : nullptr);
    }
    if (internalConfig.wgslOutput) {
        if (!spirvToWgsl(&spirv, internalConfig.wgslOutput)) {
            return false;
        }
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

        if (config.variant.hasStereo() && config.shaderType == ShaderStage::VERTEX) {
            switch (config.materialInfo->stereoscopicType) {
            case StereoscopicType::MULTIVIEW:
                // For stereo variants using multiview feature, this generates the shader code below.
                //   #extension GL_OVR_multiview2 : require
                //   layout(num_views = 2) in;
                glslOptions.ovr_multiview_view_count = config.materialInfo->stereoscopicEyeCount;
                break;
            case StereoscopicType::INSTANCED:
            case StereoscopicType::NONE:
                // Nothing to generate
                break;
            }
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

std::shared_ptr<spvtools::Optimizer> GLSLPostProcessor::createEmptyOptimizer() {
    auto optimizer = std::make_shared<spvtools::Optimizer>(SPV_ENV_UNIVERSAL_1_3);
    optimizer->SetMessageConsumer([](spv_message_level_t level,
            const char* source, const spv_position_t& position, const char* message) {
        if (!filterSpvOptimizerMessage(level)) {
            return;
        }
        slog.e << stringifySpvOptimizerMessage(level, source, position, message)
                << io::endl;
    });
    return optimizer;
}

std::shared_ptr<spvtools::Optimizer> GLSLPostProcessor::createOptimizer(
        MaterialBuilder::Optimization optimization, Config const& config) {
    auto optimizer = createEmptyOptimizer();

    if (optimization == MaterialBuilder::Optimization::SIZE) {
        // When optimizing for size, we don't run the SPIR-V through any size optimization passes
        // when targeting MSL. This results in better line dictionary compression. We do, however,
        // still register the passes necessary (below) to support half precision floating point
        // math.
        if (config.targetApi != MaterialBuilder::TargetApi::METAL) {
            registerSizePasses(*optimizer, config);
        }
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

void GLSLPostProcessor::optimizeSpirv(OptimizerPtr optimizer, SpirvBlob& spirv) {
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
// CreateMergeReturnPass() also creates issues with Tint conversion related to the
// bitwise "<<" Operator used in shaders/src/surface_light_directional.fs against
// a signed integer.
//
// CreateSimplificationPass() creates a lot of problems:
// - Adreno GPU show artifacts after running simplification passes (Vulkan)
// - spirv-cross fails generating working glsl
//      (https://github.com/KhronosGroup/SPIRV-Cross/issues/2162)
// - generally it makes the code more complicated, e.g.: replacing for loops with
//   while-if-break, unclear if it helps for anything.
// However, the simplification passes below are necessary when targeting Metal, otherwise the
// result is mismatched half / float assignments in MSL.

// CreateInlineExhaustivePass() expects CreateMergeReturnPass() to be run beforehand
// (Throwing many warnings if this is not the case), but we don't consistently do so for the above
// reasons. While running it alone may have some value, we will disable it for the new WebGPU backend
// while minimizing other changes.


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
    RegisterPass(CreateInlineExhaustivePass(), MaterialBuilder::TargetApi::ALL & ~MaterialBuilder::TargetApi::WEBGPU);
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
    //  Disable for WebGPU, see comment above registerPerformancePasses()
    RegisterPass(CreateInlineExhaustivePass(), MaterialBuilder::TargetApi::ALL & ~MaterialBuilder::TargetApi::WEBGPU);
    RegisterPass(CreateEliminateDeadFunctionsPass());
    RegisterPass(CreatePrivateToLocalPass());
    RegisterPass(CreateScalarReplacementPass(0));
    RegisterPass(CreateLocalMultiStoreElimPass());
    RegisterPass(CreateCCPPass());
    RegisterPass(CreateLoopUnrollPass(true));
    RegisterPass(CreateDeadBranchElimPass());
    RegisterPass(CreateScalarReplacementPass(0));
    RegisterPass(CreateLocalSingleStoreElimPass());
    RegisterPass(CreateIfConversionPass());
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
    RegisterPass(CreateAggressiveDCEPass());
    RegisterPass(CreateCFGCleanupPass());
}

} // namespace filamat
