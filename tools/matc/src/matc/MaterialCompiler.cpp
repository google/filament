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

#include "MaterialCompiler.h"

#include <memory>
#include <iostream>
#include <utility>

#include <filamat/MaterialBuilder.h>

#include <filament-matp/Config.h>

#include <utils/JobSystem.h>
#include <utils/Log.h>
#include <utils/Path.h>

#include <GlslangToSpv.h>
#include "glslang/Include/intermediate.h"

#include "sca/builtinResource.h"

#include <smolv.h>

using namespace utils;
using namespace filamat;
using namespace std::placeholders;

namespace matc {

bool MaterialCompiler::run(const matp::Config& config) {
    matp::Config::Input* input = config.getInput();
    ssize_t size = input->open();
    if (size <= 0) {
        std::cerr << "Input file is empty" << std::endl;
        return false;
    }
    std::unique_ptr<const char[]> buffer = input->read();

    mParser.processTemplateSubstitutions(config, size, buffer);

    if (config.rawShaderMode()) {
        utils::Path const materialFilePath = utils::Path(config.getInput()->getName()).getAbsolutePath();
        assert(materialFilePath.isFile());
        const std::string extension = materialFilePath.getExtension();
        glslang::InitializeProcess();
        bool const success = compileRawShader(buffer.get(), size, config.isDebug(), config.getOutput(),
                extension.c_str());
        glslang::FinalizeProcess();
        return success;
    }

    MaterialBuilder::init();
    MaterialBuilder builder;

    if (!mParser.parse(builder, config, input, size, buffer)) {
        return false;
    }

    // If we're reflecting parameters, the MaterialParser will have handled it inside of parse().
    // We should return here to avoid actually building a material.
    if (config.getReflectionTarget() != matp::Config::Metadata::NONE) {
        return true;
    }

    JobSystem js;
    js.adopt();

    // Write builder.build() to output.
    Package const package = builder.build(js);

    js.emancipate();
    MaterialBuilder::shutdown();

    if (!package.isValid()) {
        std::cerr << "Could not compile material " << config.getInput()->getName() << std::endl;
        return false;
    }

    return writePackage(package, config);
}

bool MaterialCompiler::checkParameters(const matp::Config& config) {
    // Check for input file.
    if (config.getInput() == nullptr) {
        std::cerr << "Missing input filename." << std::endl;
        return false;
    }

    // If we have reflection we don't need an output file
    if (config.getReflectionTarget() != matp::Config::Metadata::NONE) {
        return true;
    }

    // Check for output format.
    if (config.getOutput() == nullptr) {
        std::cerr << "Missing output filename." << std::endl;
        return false;
    }

    return true;
}

// this can either be here or material parser
bool MaterialCompiler::compileRawShader(const char* glsl, size_t size, bool isDebug,
        matp::Config::Output* output, const char* ext) const noexcept {
    using namespace glslang;
    using namespace filament::backend;
    using SpirvBlob = std::vector<uint32_t>;

    const EShLanguage shLang = !strcmp(ext, "vs") ? EShLangVertex : EShLangFragment;

    // Add a terminating null by making a copy of the GLSL string.
    std::string const nullTerminated(glsl, size);
    glsl = nullTerminated.c_str();

    TShader tShader(shLang);
    tShader.setStrings(&glsl, 1);

    const int version = 110;
    EShMessages msg = EShMessages::EShMsgDefault;
    msg = (EShMessages) (EShMessages::EShMsgVulkanRules | EShMessages::EShMsgSpvRules);

    tShader.setAutoMapBindings(true);

    bool const parseOk = tShader.parse(&DefaultTBuiltInResource, version, false, msg);
    if (!parseOk) {
        std::cerr << "ERROR: Unable to parse " << ext << ":" << std::endl;
        std::cerr << tShader.getInfoLog() << std::endl;
        return false;
    }

    // Even though we only have a single shader stage, linking is still necessary to finalize
    // SPIR-V types
    TProgram program;
    program.addShader(&tShader);
    bool const linkOk = program.link(msg);
    if (!linkOk) {
        std::cerr << "ERROR: link failed " << std::endl << tShader.getInfoLog() << std::endl;
        return false;
    }

    SpirvBlob spirv;
    SpvOptions options;
    GlslangToSpv(*tShader.getIntermediate(), spirv, &options);

    if (spirv.empty()) {
        std::cerr << "SPIRV blob is empty." << std::endl;
        return false;
    }

    if (!output->open()) {
        std::cerr << "Unable to create SPIRV file." << std::endl;
        return false;
    }

    const uint32_t flags = isDebug ? 0 : smolv::kEncodeFlagStripDebugInfo;

    smolv::ByteArray compressed;
    if (!smolv::Encode(spirv.data(), spirv.size() * 4, compressed, flags)) {
        utils::slog.e << "Error with SPIRV compression" << utils::io::endl;
    }

    output->write(compressed.data(), compressed.size());
    output->close();

    return true;
}

} // namespace matc
