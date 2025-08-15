/*
* Copyright (C) 2025 The Android Open Source Project
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

#include <filament-matp/MaterialParser.h>

#include <memory>
#include <iostream>
#include <utility>

#include <filamat/MaterialBuilder.h>

#include <filamat/Enums.h>

#include <utils/Log.h>
#include <utils/JobSystem.h>

#include "DirIncluder.h"
#include "JsonishLexer.h"
#include "JsonishParser.h"
#include "MaterialLexeme.h"
#include "MaterialLexer.h"
#include "ParametersProcessor.h"
#include <filament-matp/Config.h>

using namespace utils;
using namespace filamat;
using namespace std::placeholders;

namespace matp {

static constexpr const char* CONFIG_KEY_MATERIAL= "material";
static constexpr const char* CONFIG_KEY_VERTEX_SHADER = "vertex";
static constexpr const char* CONFIG_KEY_FRAGMENT_SHADER = "fragment";
static constexpr const char* CONFIG_KEY_COMPUTE_SHADER = "compute";
static constexpr const char* CONFIG_KEY_TOOL = "tool";

MaterialParser::MaterialParser() {
    mConfigProcessor[CONFIG_KEY_MATERIAL] = &MaterialParser::processMaterial;
    mConfigProcessor[CONFIG_KEY_VERTEX_SHADER] = &MaterialParser::processVertexShader;
    mConfigProcessor[CONFIG_KEY_FRAGMENT_SHADER] = &MaterialParser::processFragmentShader;
    mConfigProcessor[CONFIG_KEY_COMPUTE_SHADER] = &MaterialParser::processComputeShader;
    mConfigProcessor[CONFIG_KEY_TOOL] = &MaterialParser::ignoreLexeme;

    mConfigProcessorJSON[CONFIG_KEY_MATERIAL] = &MaterialParser::processMaterialJSON;
    mConfigProcessorJSON[CONFIG_KEY_VERTEX_SHADER] = &MaterialParser::processVertexShaderJSON;
    mConfigProcessorJSON[CONFIG_KEY_FRAGMENT_SHADER] = &MaterialParser::processFragmentShaderJSON;
    mConfigProcessorJSON[CONFIG_KEY_COMPUTE_SHADER] = &MaterialParser::processComputeShaderJSON;
    mConfigProcessorJSON[CONFIG_KEY_TOOL] = &MaterialParser::ignoreLexemeJSON;
}

bool MaterialParser::processMaterial(const MaterialLexeme& jsonLexeme,
        MaterialBuilder& builder) const noexcept  {

    JsonishLexer jlexer;
    jlexer.lex(jsonLexeme.getStart(), jsonLexeme.getSize(), jsonLexeme.getLine());

    JsonishParser parser(jlexer.getLexemes());
    std::unique_ptr<JsonishObject> const json = parser.parse();

    if (json == nullptr) {
        std::cerr << "JsonishParser error (see above)." << std::endl;
        return false;
    }

    ParametersProcessor parametersProcessor;
    bool const ok = parametersProcessor.process(builder, *json);
    if (!ok) {
        std::cerr << "Error while processing material." << std::endl;
        return false;
    }

    return true;
}

bool MaterialParser::processVertexShader(const MaterialLexeme& lexeme,
        MaterialBuilder& builder) const noexcept {

    MaterialLexeme const trimmedLexeme = lexeme.trimBlockMarkers();
    std::string const shaderStr = trimmedLexeme.getStringValue();

    // getLine() returns a line number, with 1 being the first line, but .material wants a 0-based
    // line number offset, where 0 is the first line.
    builder.materialVertex(shaderStr.c_str(), trimmedLexeme.getLine() - 1);
    return true;
}

bool MaterialParser::processFragmentShader(const MaterialLexeme& lexeme,
        MaterialBuilder& builder) const noexcept {

    MaterialLexeme const trimmedLexeme = lexeme.trimBlockMarkers();
    std::string const shaderStr = trimmedLexeme.getStringValue();

    // getLine() returns a line number, with 1 being the first line, but .material wants a 0-based
    // line number offset, where 0 is the first line.
    builder.material(shaderStr.c_str(), trimmedLexeme.getLine() - 1);
    return true;
}

bool MaterialParser::processComputeShader(const MaterialLexeme& lexeme,
        MaterialBuilder& builder) const noexcept {
    return MaterialParser::processFragmentShader(lexeme, builder);
}

bool MaterialParser::ignoreLexeme(const MaterialLexeme&, MaterialBuilder&) const noexcept {
    return true;
}

static bool reflectParameters(const MaterialBuilder& builder) {
    uint8_t const count = builder.getParameterCount();
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
                      Enums::toString(parameter.precision) << "\"," << std::endl;
            std::cout << R"(      "multisample": ")" <<
                      (parameter.multisample ? "true" : "false")<< "\"" << std::endl;
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

bool MaterialParser::processMaterialJSON(const JsonishValue* value,
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
    bool const ok = parametersProcessor.process(builder, *value->toJsonObject());
    if (!ok) {
        std::cerr << "Error while processing material." << std::endl;
        return false;
    }

    return true;
}

bool MaterialParser::processVertexShaderJSON(const JsonishValue* value,
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

bool MaterialParser::processFragmentShaderJSON(const JsonishValue* value,
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
bool MaterialParser::processComputeShaderJSON(const JsonishValue* value,
        filamat::MaterialBuilder& builder) const noexcept {

    if (!value) {
        std::cerr << "'compute' block does not have a value, one is required." << std::endl;
        return false;
    }

    if (value->getType() != JsonishValue::STRING) {
        std::cerr << "'compute' block has an invalid type: "
                  << JsonishValue::typeToString(value->getType())
                  << ", should be STRING."
                  << std::endl;
        return false;
    }

    builder.material(value->toJsonString()->getString().c_str());
    return true;
}

bool MaterialParser::ignoreLexemeJSON(const JsonishValue*,
        filamat::MaterialBuilder&) const noexcept {
    return true;
}

bool MaterialParser::isValidJsonStart(const char* buffer, size_t size) const noexcept {
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

bool MaterialParser::parseMaterialAsJSON(const char* buffer, size_t size,
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
        MaterialConfigProcessorJSON const p =  mConfigProcessorJSON.at(key);
        // Call it.
        bool const ok = (*this.*p)(entry.second, builder);
        if (!ok) {
            std::cerr << "Error while processing block with key:'" << key << "'" << std::endl;
            return false;
        }
    }

    return true;
}

bool MaterialParser::parseMaterial(const char* buffer, size_t size,
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
                          << identifier
                          << "' at line:" << lexeme.getLine()
                          << " position:" << lexeme.getLinePosition() << std::endl;
                return false;
            }
        } else if (lexeme.getType() == MaterialType::BLOCK) {
            MaterialConfigProcessor const processor = mConfigProcessor.at(identifier);
            if (!(*this.*processor)(lexeme, builder)) {
                std::cerr << "Error while processing block with key:'" << identifier << "'"
                          << std::endl;
                return false;
            }
        }
    }
    return true;
}

bool MaterialParser::processMaterialParameters(filamat::MaterialBuilder& builder,
        const Config& config) const {
    ParametersProcessor parametersProcessor;
    bool ok = true;
    for (const auto& param : config.getMaterialParameters()) {
        ok &= parametersProcessor.process(builder, param.first, param.second);
    }
    return ok;
}

bool MaterialParser::processTemplateSubstitutions(
        const Config& config, ssize_t& size, std::unique_ptr<const char[]>& buffer) {
    const auto& templateMap = config.getTemplateMap();
    ssize_t modifiedSize = size;
    bool modified = false;
    if (!templateMap.empty()) {
        for (ssize_t cursor = 0, n = size - 1; cursor < n; ++cursor) {
            if (UTILS_LIKELY(buffer[cursor] != '$' || buffer[cursor + 1] != '{')) {
                continue;
            }
            cursor += 2;
            ssize_t endCursor = cursor;
            while (true) {
                if (endCursor == size) {
                    std::cerr << "Unexpected end of file" << std::endl;
                    return false;
                }
                if (buffer[endCursor] == '}') {
                    break;
                }
                ++endCursor;
            }
            // At this point, cursor points to the F in ${FOO} and endCursor points to the }.
            std::string_view const macro(&buffer[cursor], endCursor - cursor);
            if (auto iter = templateMap.find(macro); iter != templateMap.end()) {
                modifiedSize -= macro.size() + 3;
                modifiedSize += iter->second.size();
            } else {
                std::cerr << "Undefined template macro:" << macro << std::endl;
                return false;
            }
            modified = true;
        }
    }

    // Second pass of template substitution allocates a new buffer and performs the actual
    // substitutions. Since the first pass did not return early, we can safely assume no errors.
    if (modified) {
        auto modifiedBuffer = std::make_unique<char[]>(modifiedSize);
        for (ssize_t cursor = 0, dstCursor = 0; cursor < size; ++cursor) {
            const ssize_t next = cursor + 1;
            if (UTILS_LIKELY(buffer[cursor] != '$' || next >= size || buffer[next] != '{')) {
                modifiedBuffer[dstCursor++] = buffer[cursor];
                continue;
            }
            cursor += 2;
            ssize_t endCursor = cursor;
            while (buffer[endCursor] != '}') ++endCursor;
            // At this point, cursor points to the F in ${FOO} and endCursor points to the }.
            std::string_view const macro(&buffer[cursor], endCursor - cursor);
            const std::string& val = templateMap.find(macro)->second;
            for (size_t i = 0, n = val.size(); i < n; ++i, ++dstCursor) {
                modifiedBuffer[dstCursor] = val[i];
            }
            cursor = endCursor;
        }
        buffer = std::move(modifiedBuffer);
        size = modifiedSize;
    }
    return true;
}

bool MaterialParser::parse(filamat::MaterialBuilder& builder,
        const Config& config,
        Config::Input* input, ssize_t& size, std::unique_ptr<const char[]>& buffer) {

    utils::Path const materialFilePath = utils::Path(input->getName()).getAbsolutePath();
    assert(materialFilePath.isFile());

    if (builder.getFeatureLevel() > config.getFeatureLevel()) {
        std::cerr << "Material feature level (" << +builder.getFeatureLevel()
            << ") is higher than maximum allowed (" << +config.getFeatureLevel() << ")" << std::endl;
        return false;
    }

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
            .noSamplerValidation(config.noSamplerValidation())
            .includeEssl1(config.includeEssl1())
            .includeCallback(includer)
            .fileName(materialFilePath.getName().c_str())
            .platform(config.getPlatform())
            .targetApi(config.getTargetApi())
            .optimization(config.getOptimizationLevel())
            .workarounds(config.getWorkarounds())
            .printShaders(config.printShaders())
            .saveRawVariants(config.saveRawVariants())
            .generateDebugInfo(config.isDebug())
            .variantFilter(config.getVariantFilter() | builder.getVariantFilter());

    for (const auto& define : config.getDefines()) {
        builder.shaderDefine(define.first.c_str(), define.second.c_str());
    }

    if (!processMaterialParameters(builder, config)) {
        std::cerr << "Error while processing material parameters." << std::endl;
        return false;
    }

    return true;
}

} // namespace matp