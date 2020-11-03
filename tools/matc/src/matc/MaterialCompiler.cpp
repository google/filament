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

#include <filamat/MaterialBuilder.h>

#include <filamat/Enums.h>

#include <utils/JobSystem.h>

#include "DirIncluder.h"
#include "MaterialLexeme.h"
#include "MaterialLexer.h"
#include "JsonishLexer.h"
#include "JsonishParser.h"
#include "ParametersProcessor.h"

#include <GlslangToSpv.h>

#include "sca/builtinResource.h"

using namespace utils;
using namespace filamat;
using namespace std::placeholders;

namespace matc {

static constexpr const char* CONFIG_KEY_MATERIAL= "material";
static constexpr const char* CONFIG_KEY_VERTEX_SHADER = "vertex";
static constexpr const char* CONFIG_KEY_FRAGMENT_SHADER = "fragment";
static constexpr const char* CONFIG_KEY_TOOL = "tool";

MaterialCompiler::MaterialCompiler() {
    mConfigProcessor[CONFIG_KEY_MATERIAL] = &MaterialCompiler::processMaterial;
    mConfigProcessor[CONFIG_KEY_VERTEX_SHADER] = &MaterialCompiler::processVertexShader;
    mConfigProcessor[CONFIG_KEY_FRAGMENT_SHADER] = &MaterialCompiler::processFragmentShader;
    mConfigProcessor[CONFIG_KEY_TOOL] = &MaterialCompiler::ignoreLexeme;

    mConfigProcessorJSON[CONFIG_KEY_MATERIAL] = &MaterialCompiler::processMaterialJSON;
    mConfigProcessorJSON[CONFIG_KEY_VERTEX_SHADER] = &MaterialCompiler::processVertexShaderJSON;
    mConfigProcessorJSON[CONFIG_KEY_FRAGMENT_SHADER] = &MaterialCompiler::processFragmentShaderJSON;
    mConfigProcessorJSON[CONFIG_KEY_TOOL] = &MaterialCompiler::ignoreLexemeJSON;
}

bool MaterialCompiler::processMaterial(const MaterialLexeme& jsonLexeme,
        MaterialBuilder& builder) const noexcept  {

    JsonishLexer jlexer;
    jlexer.lex(jsonLexeme.getStart(), jsonLexeme.getSize(), jsonLexeme.getLine());

    JsonishParser parser(jlexer.getLexemes());
    std::unique_ptr<JsonishObject> json = parser.parse();

    if (json == nullptr) {
        std::cerr << "JsonishParser error (see above)." << std::endl;
        return false;
    }

    ParametersProcessor parametersProcessor;
    bool ok = parametersProcessor.process(builder, *json);
    if (!ok) {
        std::cerr << "Error while processing material." << std::endl;
        return false;
    }

    return true;
}

bool MaterialCompiler::processVertexShader(const MaterialLexeme& lexeme,
        MaterialBuilder& builder) const noexcept {

    MaterialLexeme trimmedLexeme = lexeme.trimBlockMarkers();
    std::string shaderStr = trimmedLexeme.getStringValue();

    // getLine() returns a line number, with 1 being the first line, but .material wants a 0-based
    // line number offset, where 0 is the first line.
    builder.materialVertex(shaderStr.c_str(), trimmedLexeme.getLine() - 1);
    return true;
}

bool MaterialCompiler::processFragmentShader(const MaterialLexeme& lexeme,
        MaterialBuilder& builder) const noexcept {

    MaterialLexeme trimmedLexeme = lexeme.trimBlockMarkers();
    std::string shaderStr = trimmedLexeme.getStringValue();

    // getLine() returns a line number, with 1 being the first line, but .material wants a 0-based
    // line number offset, where 0 is the first line.
    builder.material(shaderStr.c_str(), trimmedLexeme.getLine() - 1);
    return true;
}

bool MaterialCompiler::ignoreLexeme(const MaterialLexeme& lexeme,
        MaterialBuilder& builder) const noexcept {
    return true;
}

bool MaterialCompiler::processMaterialJSON(const JsonishValue* value,
        filamat::MaterialBuilder& builder) const noexcept {

    if (!value) {
        std::cerr << "'material' block does not have a value, one is required." << std::endl;
        return false;
    }

    if (value->getType() != JsonishValue::OBJECT) {
        std::cerr << "'material' block has an invalid type: "
                << JsonishValue::typeToString(value->getType())
                << ", should be OBJECT."
                << std::endl;
        return false;
    }

    ParametersProcessor parametersProcessor;
    bool ok = parametersProcessor.process(builder, *value->toJsonObject());
    if (!ok) {
        std::cerr << "Error while processing material." << std::endl;
        return false;
    }

    return true;
}

bool MaterialCompiler::processVertexShaderJSON(const JsonishValue* value,
        filamat::MaterialBuilder& builder) const noexcept {

    if (!value) {
        std::cerr << "'vertex' block does not have a value, one is required." << std::endl;
        return false;
    }

    if (value->getType() != JsonishValue::STRING) {
        std::cerr << "'vertex' block has an invalid type: "
                << JsonishValue::typeToString(value->getType())
                << ", should be STRING."
                << std::endl;
        return false;
    }

    builder.materialVertex(value->toJsonString()->getString().c_str());
    return true;
}

bool MaterialCompiler::processFragmentShaderJSON(const JsonishValue* value,
        filamat::MaterialBuilder& builder) const noexcept {

    if (!value) {
        std::cerr << "'fragment' block does not have a value, one is required." << std::endl;
        return false;
    }

    if (value->getType() != JsonishValue::STRING) {
        std::cerr << "'fragment' block has an invalid type: "
                << JsonishValue::typeToString(value->getType())
                << ", should be STRING."
                << std::endl;
        return false;
    }

    builder.material(value->toJsonString()->getString().c_str());
    return true;
}

bool MaterialCompiler::ignoreLexemeJSON(const JsonishValue*,
        filamat::MaterialBuilder& builder) const noexcept {
    return true;
}

static bool reflectParameters(const MaterialBuilder& builder) {
    uint8_t count = builder.getParameterCount();
    const MaterialBuilder::ParameterList& parameters = builder.getParameters();

    std::cout << "{" << std::endl;
    std::cout << "  \"parameters\": [" << std::endl;
    for (uint8_t i = 0; i < count; i++) {
        const MaterialBuilder::Parameter& parameter = parameters[i];
        std::cout << "    {" << std::endl;
        std::cout << R"(      "name": ")" << parameter.name.c_str() << "\"," << std::endl;
        if (parameter.isSampler()) {
            std::cout << R"(      "type": ")" <<
                      Enums::toString(parameter.samplerType) << "\"," << std::endl;
            std::cout << R"(      "format": ")" <<
                      Enums::toString(parameter.format) << "\"," << std::endl;
            std::cout << R"(      "precision": ")" <<
                      Enums::toString(parameter.precision) << "\"" << std::endl;
        } else if (parameter.isUniform()) {
            std::cout << R"(      "type": ")" <<
                      Enums::toString(parameter.uniformType) << "\"," << std::endl;
            std::cout << R"(      "size": ")" << parameter.size << "\"" << std::endl;
        } else if (parameter.isSubpass()) {
            std::cout << R"(      "type": ")" <<
                      Enums::toString(parameter.subpassType) << "\"," << std::endl;
            std::cout << R"(      "format": ")" <<
                      Enums::toString(parameter.format) << "\"," << std::endl;
            std::cout << R"(      "precision": ")" <<
                      Enums::toString(parameter.precision) << "\"" << std::endl;
        }
        std::cout << "    }";
        if (i < count - 1) std::cout << ",";
        std::cout << std::endl;
    }
    std::cout << "  ]" << std::endl;
    std::cout << "}" << std::endl;

    return true;
}

bool MaterialCompiler::isValidJsonStart(const char* buffer, size_t size) const noexcept {
    // Skip all whitespace characters.
    const char* end = buffer + size;
    while (buffer != end && isspace(buffer[0])) {
        buffer++;
    }

    // A buffer made only of whitespace is not a valid JSON start.
    if (buffer == end) {
        return false;
    }

    const char c = buffer[0];

    // Take care of block, array, and string
    if (c == '{' || c == '[' || c == '"') {
        return true;
    }

    // boolean true
    if (c == 't' && (end - buffer) > 3 && strncmp(buffer, "true", 4) != 0) {
        return true;
    }

    // boolean false
    if (c == 'f' && (end - buffer) > 4 && strncmp(buffer, "false", 5) != 0) {
        return true;
    }

    // null literal
    return c == 'n' && (end - buffer) > 3 && strncmp(buffer, "null", 5) != 0;
}

bool MaterialCompiler::run(const Config& config) {
    Config::Input* input = config.getInput();
    ssize_t size = input->open();
    if (size <= 0) {
        return false;
    }
    auto buffer = input->read();

    utils::Path materialFilePath = utils::Path(input->getName()).getAbsolutePath();
    assert(materialFilePath.isFile());

    if (config.rawShaderMode()) {
        const std::string extension = materialFilePath.getExtension();
        glslang::InitializeProcess();
        bool success = compileRawShader(buffer.get(), size, config.getOutput(), extension.c_str());
        glslang::FinalizeProcess();
        return success;
    }

    MaterialBuilder::init();
    MaterialBuilder builder;
    // Before attempting an expensive lex, let's find out if we were sent pure JSON.
    bool parsed;
    if (isValidJsonStart(buffer.get(), size_t(size))) {
        parsed = parseMaterialAsJSON(buffer.get(), size_t(size), builder);
    } else {
        parsed = parseMaterial(buffer.get(), size_t(size), builder);
    }

    if (!parsed) {
        return false;
    }

    switch (config.getReflectionTarget()) {
        case Config::Metadata::NONE:
            break;
        case Config::Metadata::PARAMETERS:
            return reflectParameters(builder);
    }

    // Set the root include directory to the directory containing the material file.
    DirIncluder includer;
    includer.setIncludeDirectory(materialFilePath.getParent());

    builder
        .includeCallback(includer)
        .fileName(materialFilePath.getName().c_str())
        .platform(config.getPlatform())
        .targetApi(config.getTargetApi())
        .optimization(config.getOptimizationLevel())
        .printShaders(config.printShaders())
        .generateDebugInfo(config.isDebug())
        .variantFilter(config.getVariantFilter() | builder.getVariantFilter());

    for (const auto& define : config.getDefines()) {
        builder.shaderDefine(define.first.c_str(), define.second.c_str());
    }

    JobSystem js;
    js.adopt();

    // Write builder.build() to output.
    Package package = builder.build(js);

    js.emancipate();
    MaterialBuilder::shutdown();

    if (!package.isValid()) {
        std::cerr << "Could not compile material " << input->getName() << std::endl;
        return false;
    }
    return writePackage(package, config);
}

bool MaterialCompiler::checkParameters(const Config& config) {
    // Check for input file.
    if (config.getInput() == nullptr) {
        std::cerr << "Missing input filename." << std::endl;
        return false;
    }

    // If we have reflection we don't need an output file
    if (config.getReflectionTarget() != Config::Metadata::NONE) {
        return true;
    }

    // Check for output format.
    if (config.getOutput() == nullptr) {
        std::cerr << "Missing output filename." << std::endl;
        return false;
    }

    return true;
}

bool MaterialCompiler::parseMaterialAsJSON(const char* buffer, size_t size,
        filamat::MaterialBuilder& builder) const noexcept {

    JsonishLexer jlexer;
    jlexer.lex(buffer, size, 1);

    JsonishParser parser(jlexer.getLexemes());
    std::unique_ptr<JsonishObject> json = parser.parse();
    if (json == nullptr) {
        std::cerr << "Could not parse JSON material file" << std::endl;
        return false;
    }

    for (auto& entry : json->getEntries()) {
        const std::string& key = entry.first;
        if (mConfigProcessorJSON.find(key) == mConfigProcessorJSON.end()) {
            std::cerr << "Unknown identifier '" << key << "'" << std::endl;
            return false;
        }

        // Retrieve function member pointer
        MaterialConfigProcessorJSON p =  mConfigProcessorJSON.at(key);
        // Call it.
        bool ok = (*this.*p)(entry.second, builder);
        if (!ok) {
            std::cerr << "Error while processing block with key:'" << key << "'" << std::endl;
            return false;
        }
    }

    return true;
}

bool MaterialCompiler::parseMaterial(const char* buffer, size_t size,
         filamat::MaterialBuilder& builder) const noexcept {

    MaterialLexer materialLexer;
    materialLexer.lex(buffer, size);
    auto lexemes = materialLexer.getLexemes();

    // Make sure the lexer did not stumble upon unknown character. This could mean we received
    // a binary file.
    for (auto lexeme : lexemes) {
        if (lexeme.getType() == MaterialType::UNKNOWN) {
            std::cerr << "Unexpected character at line:" << lexeme.getLine()
                      << " position:" << lexeme.getLinePosition() << std::endl;
            return false;
        }
    }

    // Make a first quick pass just to make sure the format was respected (the material format is
    // a series of IDENTIFIER, BLOCK pairs).
    if (lexemes.size() < 2) {
        std::cerr << "Input MUST be an alternation of [identifier, block] pairs." << std::endl;
        return false;
    }
    for (size_t i = 0; i < lexemes.size(); i += 2) {
        auto lexeme = lexemes.at(i);
        if (lexeme.getType() != MaterialType::IDENTIFIER) {
            std::cerr << "An identifier was expected at line:" << lexeme.getLine()
                      << " position:" << lexeme.getLinePosition() << std::endl;
            return false;
        }

        if (i == lexemes.size() - 1) {
            std::cerr << "Identifier at line:" << lexeme.getLine()
                      << " position:" << lexeme.getLinePosition()
                      << " must be followed by a block." << std::endl;
            return false;
        }

        auto nextLexeme = lexemes.at(i + 1);
        if (nextLexeme.getType() != MaterialType::BLOCK) {
            std::cerr << "A block was expected at line:" << lexeme.getLine()
                      << " position:" << lexeme.getLinePosition() << std::endl;
            return false;
        }
    }


    std::string identifier;
    for (auto lexeme : lexemes) {
        if (lexeme.getType() == MaterialType::IDENTIFIER) {
            identifier = lexeme.getStringValue();
            if (mConfigProcessor.find(identifier) == mConfigProcessor.end()) {
                std::cerr << "Unknown identifier '"
                          << lexeme.getStringValue()
                          << "' at line:" << lexeme.getLine()
                          << " position:" << lexeme.getLinePosition() << std::endl;
                return false;
            }
        } else if (lexeme.getType() == MaterialType::BLOCK) {
            MaterialConfigProcessor processor = mConfigProcessor.at(identifier);
            if (!(*this.*processor)(lexeme, builder)) {
                std::cerr << "Error while processing block with key:'" << identifier << "'"
                          << std::endl;
                return false;
            }
        }
    }
    return true;
}

bool MaterialCompiler::compileRawShader(const char* glsl, size_t size,
        Config::Output* output, const char* ext) const noexcept {
    using namespace glslang;
    using namespace filament::backend;
    using SpirvBlob = std::vector<uint32_t>;

    const EShLanguage shLang = !strcmp(ext, "vs") ? EShLangVertex : EShLangFragment;

    TShader tShader(shLang);
    tShader.setStrings(&glsl, 1);

    const int version = 110;
    EShMessages msg = EShMessages::EShMsgDefault;
    msg = (EShMessages) (EShMessages::EShMsgVulkanRules | EShMessages::EShMsgSpvRules);

    tShader.setAutoMapBindings(true);

    bool parseOk = tShader.parse(&DefaultTBuiltInResource, version, false, msg);
    if (!parseOk) {
        std::cerr << "ERROR: Unable to parse " << ext << ":" << std::endl;
        std::cerr << tShader.getInfoLog() << std::endl;
        return false;
    }

    // Even though we only have a single shader stage, linking is still necessary to finalize
    // SPIR-V types
    TProgram program;
    program.addShader(&tShader);
    bool linkOk = program.link(msg);
    if (!linkOk) {
        std::cerr << "ERROR: link failed " << std::endl << tShader.getInfoLog() << std::endl;
        return false;
    }

    SpirvBlob spirv;
    SpvOptions options;
    GlslangToSpv(*tShader.getIntermediate(), spirv, &options);

    if (spirv.size() == 0) {
        std::cerr << "SPIRV blob is empty." << std::endl;
        return false;
    }

    if (!output->open()) {
        std::cerr << "Unable to create SPIRV file." << std::endl;
        return false;
    }

    output->write((const uint8_t*) spirv.data(), sizeof(uint32_t) * spirv.size());
    output->close();

    return true;
}

} // namespace matc
