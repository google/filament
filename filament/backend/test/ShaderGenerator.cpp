/*
 * Copyright (C) 2019 The Android Open Source Project
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

#include "ShaderGenerator.h"

#include <GlslangToSpv.h>
#include <SPVRemapper.h>

#include <spirv_glsl.hpp>
#include <spirv_msl.hpp>

#include "../src/GLSLPostProcessor.h"

#include "builtinResource.h"

#include <utils/FixedCapacityVector.h>

#include <iostream>
#include <utility>
#include <vector>

namespace test {

using namespace glslang;
using namespace spirv_cross;

using namespace filament::backend;

using SpirvBlob = std::vector<unsigned int>;

namespace {

void SpvToEs(const SpirvBlob* spirv, std::string* outEs) {
    CompilerGLSL glslCompiler(*spirv);
    glslCompiler.set_common_options(CompilerGLSL::Options {
        .version = 300,
        .es = true,
        .enable_420pack_extension = false
    });
    *outEs = glslCompiler.compile();
}

void SpvToGlsl(const SpirvBlob* spirv, std::string* outGlsl) {
    CompilerGLSL glslCompiler(*spirv);
    glslCompiler.set_common_options(CompilerGLSL::Options {
        .version = 410,
        .es = false,
        .enable_420pack_extension = false
    });
    *outGlsl = glslCompiler.compile();
}

} // anonymous namespace

void ShaderGenerator::init() {
    InitializeProcess();
}

void ShaderGenerator::shutdown() {
    FinalizeProcess();
}

ShaderGenerator::ShaderGenerator(std::string vertex, std::string fragment, Backend backend,
        bool isMobile, filamat::DescriptorSets&& descriptorSets) noexcept
    : mBackend(backend),
      mVertexBlob(transpileShader(
              ShaderStage::VERTEX, std::move(vertex), backend, isMobile, descriptorSets)),
      mFragmentBlob(transpileShader(
              ShaderStage::FRAGMENT, std::move(fragment), backend, isMobile, descriptorSets)) {
    switch (backend) {
        case Backend::OPENGL:
            mShaderLanguage = filament::backend::ShaderLanguage::ESSL3;
            break;
        case Backend::VULKAN:
            mShaderLanguage = filament::backend::ShaderLanguage::SPIRV;
            break;
        case Backend::METAL:
            mShaderLanguage = filament::backend::ShaderLanguage::MSL;
            break;
        case Backend::NOOP:
            mShaderLanguage = filament::backend::ShaderLanguage::ESSL3;
            break;
    }
}

ShaderGenerator::Blob ShaderGenerator::transpileShader(ShaderStage stage, std::string shader,
        Backend backend, bool isMobile, const filamat::DescriptorSets& descriptorSets) noexcept {
    TProgram program;
    const EShLanguage language = stage == ShaderStage::VERTEX ? EShLangVertex : EShLangFragment;
    TShader tShader(language);

    // Add a target environment define after the #version declaration.
    size_t pos = shader.find("#version");
    pos += 8;
    while (shader[pos] != '\n') {
        pos++;
    }
    pos++;
    if (backend == Backend::OPENGL) {
        shader.insert(pos, "#define TARGET_OPENGL_ENVIRONMENT\n");
    } else if (backend == Backend::METAL) {
        shader.insert(pos, "#define TARGET_METAL_ENVIRONMENT\n");
    } else if (backend == Backend::VULKAN) {
        shader.insert(pos, "#define TARGET_VULKAN_ENVIRONMENT\n");
    }

    const char* shaderCString = shader.c_str();
    tShader.setStrings(&shaderCString, 1);

    const int version = 110;
    EShMessages msg = EShMessages::EShMsgDefault;
    msg = (EShMessages) (EShMessages::EShMsgVulkanRules | EShMessages::EShMsgSpvRules);

    tShader.setAutoMapBindings(true);

    bool ok = tShader.parse(&DefaultTBuiltInResource, version, false, msg);

    if (!ok) {
        std::cerr << "ERROR: Unable to parse " <<
            (stage == ShaderStage::VERTEX ? "vertex" : "fragment") << " shader:" << std::endl;
        std::cerr << tShader.getInfoLog() << std::endl;
        assert_invariant(false);
    }

    program.addShader(&tShader);
    bool linkOk = program.link(msg);
    if (!linkOk) {
        std::cerr << tShader.getInfoLog() << std::endl;
        assert_invariant(false);
    }

    SpirvBlob spirv;

    GlslangToSpv(*program.getIntermediate(language), spirv);

    std::string result;

    assert_invariant(backend == Backend::OPENGL ||
           backend == Backend::METAL  ||
           backend == Backend::VULKAN);

    if (backend == Backend::OPENGL) {
        if (isMobile) {
            SpvToEs(&spirv, &result);
        } else {
            SpvToGlsl(&spirv, &result);
        }
        return { result.c_str(), result.c_str() + result.length() + 1 };
    } else if (backend == Backend::METAL) {
        const auto sm = isMobile ? ShaderModel::MOBILE : ShaderModel::DESKTOP;
        filamat::GLSLPostProcessor::spirvToMsl(
                &spirv, &result, stage, sm, false, descriptorSets, nullptr);
        return { result.c_str(), result.c_str() + result.length() + 1 };
    } else if (backend == Backend::VULKAN) {
        return { (uint8_t*)spirv.data(), (uint8_t*)(spirv.data() + spirv.size()) };
    }

    return {};
}

Program ShaderGenerator::getProgram(filament::backend::DriverApi&) noexcept {
    Program program;
    program.shaderLanguage(mShaderLanguage);
    program.shader(ShaderStage::VERTEX, mVertexBlob.data(), mVertexBlob.size());
    program.shader(ShaderStage::FRAGMENT, mFragmentBlob.data(), mFragmentBlob.size());
    return program;
}

} // namespace test
