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
#include <localintermediate.h>

#include <spirv_glsl.hpp>
#include <spirv_msl.hpp>

#include "builtinResource.h"

#include <iostream>
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

void SpvToMsl(const SpirvBlob* spirv, std::string* outMsl) {
    CompilerMSL mslCompiler(*spirv);
    mslCompiler.set_common_options(CompilerGLSL::Options {
        .vertex.fixup_clipspace = true
    });
    mslCompiler.set_msl_options(CompilerMSL::Options {
        .msl_version = CompilerMSL::Options::make_msl_version(1, 1)
    });

    auto executionModel = mslCompiler.get_execution_model();

    auto duplicateResourceBinding = [executionModel, &mslCompiler](const auto& resource) {
        auto set = mslCompiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        auto binding = mslCompiler.get_decoration(resource.id, spv::DecorationBinding);
        MSLResourceBinding newBinding;
        newBinding.stage = executionModel;
        newBinding.desc_set = set;
        newBinding.binding = binding;
        newBinding.msl_texture = binding;
        newBinding.msl_sampler = binding;
        newBinding.msl_buffer = binding;
        mslCompiler.add_msl_resource_binding(newBinding);
    };

    auto resources = mslCompiler.get_shader_resources();
    for (const auto& resource : resources.sampled_images) {
        duplicateResourceBinding(resource);
    }
    for (const auto& resource : resources.uniform_buffers) {
        duplicateResourceBinding(resource);
    }

    *outMsl = mslCompiler.compile();
}

} // anonymous namespace

void ShaderGenerator::init() {
    InitializeProcess();
}

void ShaderGenerator::shutdown() {
    FinalizeProcess();
}

ShaderGenerator::ShaderGenerator(std::string vertex, std::string fragment, Backend backend,
        bool isMobile) noexcept : mBackend(backend), mIsMobile(isMobile) {
    mVertexBlob = transpileShader(backend, isMobile, vertex, ShaderStage::VERTEX);
    mFragmentBlob = transpileShader(backend, isMobile, fragment, ShaderStage::FRAGMENT);
}

ShaderGenerator::Blob ShaderGenerator::transpileShader(Backend backend, bool isMobile, std::string shader,
            ShaderStage stage) noexcept {
    TProgram program;
    const EShLanguage language = stage == ShaderStage::VERTEX ? EShLangVertex : EShLangFragment;
    TShader tShader(language);

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
        assert(false);
    }

    program.addShader(&tShader);
    bool linkOk = program.link(msg);
    if (!linkOk) {
        std::cerr << tShader.getInfoLog() << std::endl;
        assert(false);
    }

    SpirvBlob spirv;

    GlslangToSpv(*program.getIntermediate(language), spirv);

    std::string result;

    assert(backend == Backend::OPENGL ||
           backend == Backend::METAL  ||
           backend == Backend::VULKAN);

    if (backend == Backend::OPENGL) {
        if (isMobile) {
            SpvToEs(&spirv, &result);
        } else {
            SpvToGlsl(&spirv, &result);
        }
        return Blob(result.c_str(), result.c_str() + result.length() + 1);
    } else if (backend == Backend::METAL) {
        SpvToMsl(&spirv, &result);
        return Blob(result.c_str(), result.c_str() + result.length() + 1);
    } else if (backend == Backend::VULKAN) {
        return Blob((uint8_t*) spirv.data(), (uint8_t*) (spirv.data() + spirv.size()));
    }

    return {};
}

Program ShaderGenerator::getProgram() noexcept {
    Program program;
    program.shader(Program::Shader::VERTEX, mVertexBlob.data(), mVertexBlob.size());
    program.shader(Program::Shader::FRAGMENT, mFragmentBlob.data(), mFragmentBlob.size());
    return program;
}

} // namespace test
