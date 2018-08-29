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

#include <iostream>
#include <sstream>
#include <vector>

#include <GlslangToSpv.h>
#include <SPVRemapper.h>
#include <localintermediate.h>

#include <spirv_glsl.hpp>

#include "builtinResource.h"
#include "GLSLTools.h"

using namespace glslang;
using namespace spirv_cross;
using namespace spvtools;

namespace matc {

GLSLPostProcessor::GLSLPostProcessor(const Config& config): mConfig(config) {
}

GLSLPostProcessor::~GLSLPostProcessor() {
}

static uint32_t shaderVersionFromModel(filament::driver::ShaderModel model) {
    switch (model) {
        case filament::driver::ShaderModel::UNKNOWN:
        case filament::driver::ShaderModel::GL_ES_30:
            return 300;
        case filament::driver::ShaderModel::GL_CORE_41:
            return 410;
    }
}

static void errorHandler(const std::string& str) {
    std::cerr << str << std::endl;
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

/**
 * Shrinks the specified string and returns a new string as the result.
 * To shrink the string, this method performs the following transforms:
 * - Remove leading white spaces at the beginning of each line
 * - Remove empty lines
 */
static std::string shrinkString(const std::string& s) {
    size_t cur = 0;

    std::string r;
    r.reserve(s.length());

    while (cur < s.length()) {
        size_t pos = cur;
        size_t len = 0;

        while (s[cur] != '\n') {
            cur++;
            len++;
        }

        size_t newPos = s.find_first_not_of(" \t", pos);
        if (newPos == std::string::npos) newPos = pos;

        r.append(s, newPos, len - (newPos - pos));
        r += '\n';

        while (s[cur] == '\n') {
            cur++;
        }
    }

    return r;
}

bool GLSLPostProcessor::process(const std::string& inputShader,
        filament::driver::ShaderType shaderType, filament::driver::ShaderModel shaderModel,
        std::string* outputGlsl, SpirvBlob* outputSpirv) {

    // If TargetApi is Vulkan, then we need post-processing even if there's no optimization.
    using TargetApi = Config::TargetApi;
    const TargetApi targetApi = outputSpirv ? TargetApi::VULKAN : TargetApi::OPENGL;
    if (targetApi == TargetApi::OPENGL &&
            mConfig.getOptimizationLevel() == Config::Optimization::NONE) {
        *outputGlsl = inputShader;
        if (mConfig.printShaders()) {
            std::cout << *outputGlsl << std::endl;
        }
        return true;
    }

    mGlslOutput = outputGlsl;
    mSpirvOutput = outputSpirv;

    if (shaderType == filament::driver::VERTEX) {
        mShLang = EShLangVertex;
    } else {
        mShLang = EShLangFragment;
    }

    TShader tShader(mShLang);

    // The cleaner must be declared after the TShader to prevent ASAN failures.
    GLSLangCleaner cleaner;

    const char* shaderCString = inputShader.c_str();
    tShader.setStrings(&shaderCString, 1);

    mLangVersion = GLSLTools::glslangVersionFromShaderModel(shaderModel);
    GLSLTools::prepareShaderParser(tShader, mShLang, mLangVersion, mConfig.getOptimizationLevel());
    EShMessages msg = GLSLTools::glslangFlagsFromTargetApi(targetApi);
    bool ok = tShader.parse(&DefaultTBuiltInResource, mLangVersion, false, msg);
    if (!ok) {
        std::cerr << tShader.getInfoLog() << std::endl;
        return false;
    }

    switch (mConfig.getOptimizationLevel()) {
        case Config::Optimization::NONE:
            if (mSpirvOutput) {
                GlslangToSpv(*tShader.getIntermediate(), *mSpirvOutput);
            } else {
                std::cerr << "GLSL post-processor invoked with optimization level NONE"
                        << std::endl;
            }
            break;
        case Config::Optimization::PREPROCESSOR:
            preprocessOptimization(tShader, shaderModel);
            break;
        case Config::Optimization::SIZE:
        case Config::Optimization::PERFORMANCE:
            fullOptimization(tShader, shaderModel);
            break;
    }

    if (mGlslOutput) {
        *mGlslOutput = shrinkString(*mGlslOutput);
        if (mConfig.printShaders()) {
            std::cout << *mGlslOutput << std::endl;
        }
    }
    return true;
}

void GLSLPostProcessor::preprocessOptimization(glslang::TShader& tShader,
        const filament::driver::ShaderModel shaderModel) const {
    using TargetApi = Config::TargetApi;

    std::string glsl;
    TShader::ForbidIncluder forbidIncluder;

    int version = GLSLTools::glslangVersionFromShaderModel(shaderModel);
    const TargetApi targetApi = mSpirvOutput ? TargetApi::VULKAN : TargetApi::OPENGL;
    EShMessages msg = GLSLTools::glslangFlagsFromTargetApi(targetApi);
    bool ok = tShader.preprocess(&DefaultTBuiltInResource, version, ENoProfile, false, false,
            msg, &glsl, forbidIncluder);

    if (!ok) {
        std::cerr << tShader.getInfoLog() << std::endl;
    }

    if (mSpirvOutput) {
        TShader spirvShader(mShLang);
        const char* shaderCString = glsl.c_str();
        spirvShader.setStrings(&shaderCString, 1);
        GLSLTools::prepareShaderParser(spirvShader, mShLang, mLangVersion,
                mConfig.getOptimizationLevel());
        ok = spirvShader.parse(&DefaultTBuiltInResource, mLangVersion, false, msg);
        if (!ok) {
            std::cerr << spirvShader.getInfoLog() << std::endl;
        } else {
            GlslangToSpv(*spirvShader.getIntermediate(), *mSpirvOutput);
        }
    }

    if (mGlslOutput) {
        *mGlslOutput = glsl;
    }
}

void GLSLPostProcessor::fullOptimization(const TShader& tShader,
        const filament::driver::ShaderModel shaderModel) const {
    SpirvBlob spirv;

    // Compile GLSL to to SPIR-V
    GlslangToSpv(*tShader.getIntermediate(), spirv);

    // Run the SPIR-V optimizer
    Optimizer optimizer(SPV_ENV_UNIVERSAL_1_3);
    optimizer.SetMessageConsumer([](spv_message_level_t level,
            const char* source, const spv_position_t& position, const char* message) {
        std::cerr << stringifySpvOptimizerMessage(level, source, position, message) << std::endl;
    });

    Config::Optimization optimizationLevel = mConfig.getOptimizationLevel();
    if (optimizationLevel == Config::Optimization::SIZE) {
        registerSizePasses(optimizer);
    } else if (optimizationLevel == Config::Optimization::PERFORMANCE) {
        registerPerformancePasses(optimizer);
    }

    if (!optimizer.Run(spirv.data(), spirv.size(), &spirv)) {
        std::cerr << "SPIR-V optimizer pass failed" << std::endl;
        return;
    }

    // Remove dead module-level objects: functions, types, vars
    spv::spirvbin_t remapper(0);
    remapper.registerErrorHandler(errorHandler);
    remapper.remap(spirv, spv::spirvbin_base_t::DCE_ALL);

    if (mSpirvOutput) {
        *mSpirvOutput = spirv;
    }

    // Transpile back to GLSL
    if (mGlslOutput) {
        CompilerGLSL::Options glslOptions;
        glslOptions.es = shaderModel == filament::driver::ShaderModel::GL_ES_30;
        glslOptions.version = shaderVersionFromModel(shaderModel);
        glslOptions.enable_420pack_extension = glslOptions.version >= 420;
        glslOptions.fragment.default_float_precision = glslOptions.es ?
                CompilerGLSL::Options::Precision::Mediump : CompilerGLSL::Options::Precision::Highp;
        glslOptions.fragment.default_int_precision = glslOptions.es ?
                CompilerGLSL::Options::Precision::Mediump : CompilerGLSL::Options::Precision::Highp;

        CompilerGLSL glslCompiler(move(spirv));
        glslCompiler.set_common_options(glslOptions);

        *mGlslOutput = glslCompiler.compile();
    }
}

void GLSLPostProcessor::registerPerformancePasses(Optimizer& optimizer) const {
    optimizer
            .RegisterPass(CreateMergeReturnPass())
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

void GLSLPostProcessor::registerSizePasses(Optimizer& optimizer) const {
    optimizer
            .RegisterPass(CreateMergeReturnPass())
            .RegisterPass(CreateInlineExhaustivePass())
            .RegisterPass(CreateAggressiveDCEPass())
            .RegisterPass(CreatePrivateToLocalPass())
            .RegisterPass(CreateScalarReplacementPass())
            .RegisterPass(CreateLocalAccessChainConvertPass())
            .RegisterPass(CreateLocalSingleBlockLoadStoreElimPass())
            .RegisterPass(CreateLocalSingleStoreElimPass())
            .RegisterPass(CreateAggressiveDCEPass())
            .RegisterPass(CreateSimplificationPass())
            .RegisterPass(CreateDeadInsertElimPass())
            .RegisterPass(CreateLocalMultiStoreElimPass())
            .RegisterPass(CreateAggressiveDCEPass())
            .RegisterPass(CreateCCPPass())
            .RegisterPass(CreateAggressiveDCEPass())
            .RegisterPass(CreateDeadBranchElimPass())
            .RegisterPass(CreateIfConversionPass())
            .RegisterPass(CreateAggressiveDCEPass())
            .RegisterPass(CreateBlockMergePass())
            .RegisterPass(CreateSimplificationPass())
            .RegisterPass(CreateDeadInsertElimPass())
            .RegisterPass(CreateRedundancyEliminationPass())
            .RegisterPass(CreateCFGCleanupPass())
            .RegisterPass(CreateAggressiveDCEPass());
}

} // namespace matc
