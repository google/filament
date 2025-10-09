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

#include "JsonishLexer.h"
#include "JsonishParser.h"
#include "MaterialLexeme.h"
#include "MaterialLexer.h"
#include "ParametersProcessor.h"

#include <filamat/Enums.h>
#include <filamat/MaterialBuilder.h>
#include <filament-matp/Config.h>

#include <utils/sstream.h>
#include <utils/Status.h>
#include <utils/JobSystem.h>

#include <memory>
#include <utility>

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

utils::Status MaterialParser::processMaterial(const MaterialLexeme& jsonLexeme,
        MaterialBuilder& builder) const noexcept  {

    JsonishLexer jlexer;
    jlexer.lex(jsonLexeme.getStart(), jsonLexeme.getSize(), jsonLexeme.getLine());

    JsonishParser parser(jlexer.getLexemes());
    std::unique_ptr<JsonishObject> const json = parser.parse();

    // If it fails to parse, json value is always null.
    if (utils::Status jsonishParseStatus = parser.getParseStatus(); !jsonishParseStatus.isOk() ||
            json == nullptr) {
        return jsonishParseStatus;
    }

    ParametersProcessor parametersProcessor;
    return parametersProcessor.process(builder, *json);
}

utils::Status MaterialParser::processVertexShader(const MaterialLexeme& lexeme,
        MaterialBuilder& builder) const noexcept {

    MaterialLexeme const trimmedLexeme = lexeme.trimBlockMarkers();
    std::string const shaderStr = trimmedLexeme.getStringValue();

    // getLine() returns a line number, with 1 being the first line, but .material wants a 0-based
    // line number offset, where 0 is the first line.
    builder.materialVertex(shaderStr.c_str(), trimmedLexeme.getLine() - 1);
    return utils::Status::ok();
}

utils::Status MaterialParser::processFragmentShader(const MaterialLexeme& lexeme,
        MaterialBuilder& builder) const noexcept {

    MaterialLexeme const trimmedLexeme = lexeme.trimBlockMarkers();
    std::string const shaderStr = trimmedLexeme.getStringValue();

    // getLine() returns a line number, with 1 being the first line, but .material wants a 0-based
    // line number offset, where 0 is the first line.
    builder.material(shaderStr.c_str(), trimmedLexeme.getLine() - 1);
    return utils::Status::ok();
}

utils::Status MaterialParser::processComputeShader(const MaterialLexeme& lexeme,
        MaterialBuilder& builder) const noexcept {
    return MaterialParser::processFragmentShader(lexeme, builder);
}

utils::Status MaterialParser::ignoreLexeme(const MaterialLexeme&, MaterialBuilder&) const noexcept {
    return utils::Status::ok();
}

static utils::Status reflectParameters(const MaterialBuilder& builder) {
    size_t const count = builder.getParameterCount();
    const MaterialBuilder::ParameterList& parameters = builder.getParameters();
    utils::io::sstream ss;

    ss << "{" << utils::io::endl;
    ss << "  \"parameters\": [" << utils::io::endl;
    for (size_t i = 0; i < count; i++) {
        const MaterialBuilder::Parameter& parameter = parameters[i];
        ss << "    {" << utils::io::endl;
        ss << R"(      "name": ")" << parameter.name.c_str() << "\"," << utils::io::endl;
        if (parameter.isSampler()) {
            ss << R"(      "type": ")" <<
                      Enums::toString(parameter.samplerType) << "\"," << utils::io::endl;
            ss << R"(      "format": ")" <<
                      Enums::toString(parameter.format) << "\"," << utils::io::endl;
            ss << R"(      "precision": ")" <<
                      Enums::toString(parameter.precision) << "\"," << utils::io::endl;
            ss << R"(      "multisample": ")" <<
                      (parameter.multisample ? "true" : "false")<< "\"" << utils::io::endl;
        } else if (parameter.isUniform()) {
            ss << R"(      "type": ")" <<
                      Enums::toString(parameter.uniformType) << "\"," << utils::io::endl;
            ss << R"(      "size": ")" << parameter.size << "\"" << utils::io::endl;
        } else if (parameter.isSubpass()) {
            ss << R"(      "type": ")" <<
                      Enums::toString(parameter.subpassType) << "\"," << utils::io::endl;
            ss << R"(      "format": ")" <<
                      Enums::toString(parameter.format) << "\"," << utils::io::endl;
            ss << R"(      "precision": ")" <<
                      Enums::toString(parameter.precision) << "\"" << utils::io::endl;
        }
        ss << "    }";
        if (i < count - 1) ss << ",";
        ss << utils::io::endl;
    }
    ss << "  ]" << utils::io::endl;
    ss << "}" << utils::io::endl;

    return {utils::StatusCode::OK, ss.c_str()};
}

utils::Status MaterialParser::processMaterialJSON(const JsonishValue* value,
        filamat::MaterialBuilder& builder) const noexcept {

    if (!value) {
        return utils::Status::invalidArgument(
                "'material' block does not have a value, one is required.");
    }

    if (value->getType() != JsonishValue::OBJECT) {
        utils::io::sstream errorMessage;
        errorMessage << "'material' block has an invalid type: "
                  << JsonishValue::typeToString(value->getType())
                  << ", should be OBJECT.";

        return utils::Status::invalidArgument(errorMessage.c_str());
    }

    ParametersProcessor parametersProcessor;
    return parametersProcessor.process(builder, *value->toJsonObject());
}

utils::Status MaterialParser::processVertexShaderJSON(const JsonishValue* value,
        filamat::MaterialBuilder& builder) const noexcept {

    if (!value) {
        return utils::Status::invalidArgument(
                "'vertex' block does not have a value, one is required.");
    }

    if (value->getType() != JsonishValue::STRING) {
        utils::io::sstream errorMessage;
        errorMessage << "'vertex' block has an invalid type: "
                  << JsonishValue::typeToString(value->getType())
                  << ", should be STRING.";
        return utils::Status::invalidArgument(errorMessage.c_str());
    }

    builder.materialVertex(value->toJsonString()->getString().c_str());
    return utils::Status::ok();
}

utils::Status MaterialParser::processFragmentShaderJSON(const JsonishValue* value,
        filamat::MaterialBuilder& builder) const noexcept {

    if (!value) {
        return utils::Status::invalidArgument(
                "'fragment' block does not have a value, one is required.");
    }

    if (value->getType() != JsonishValue::STRING) {
        utils::io::sstream errorMessage;
        errorMessage << "'fragment' block has an invalid type: "
                  << JsonishValue::typeToString(value->getType())
                  << ", should be STRING.";
        return utils::Status::invalidArgument(errorMessage.c_str());
    }

    builder.material(value->toJsonString()->getString().c_str());
    return utils::Status::ok();
}

utils::Status MaterialParser::processComputeShaderJSON(const JsonishValue* value,
        filamat::MaterialBuilder& builder) const noexcept {

    if (!value) {
        return utils::Status::invalidArgument(
                "'compute' block does not have a value, one is required.");
    }

    if (value->getType() != JsonishValue::STRING) {
        utils::io::sstream errorMessage;
        errorMessage << "'compute' block has an invalid type: "
                  << JsonishValue::typeToString(value->getType())
                  << ", should be STRING.";
        return utils::Status::invalidArgument(errorMessage.c_str());
    }

    builder.material(value->toJsonString()->getString().c_str());
    return utils::Status::ok();
}

utils::Status MaterialParser::ignoreLexemeJSON(const JsonishValue*,
        filamat::MaterialBuilder&) const noexcept {
    return utils::Status::ok();
}

utils::Status MaterialParser::isValidJsonStart(const char* buffer, size_t size) const noexcept {
    // Skip all whitespace characters.
    const char* end = buffer + size;
    while (buffer != end && isspace(buffer[0])) {
        buffer++;
    }

    // A buffer made only of whitespace is not a valid JSON start.
    if (buffer == end) {
        return utils::Status::invalidArgument(
                "A buffer made only of whitespace is not a valid JSON start.");
    }

    const char c = buffer[0];

    // Take care of block, array, and string
    if (c == '{' || c == '[' || c == '"') {
        return utils::Status::ok();
    }

    // boolean true
    if (c == 't' && (end - buffer) > 3 && strncmp(buffer, "true", 4) != 0) {
        return utils::Status::ok();
    }

    // boolean false
    if (c == 'f' && (end - buffer) > 4 && strncmp(buffer, "false", 5) != 0) {
        return utils::Status::ok();
    }

    // null literal
    if (c == 'n' && (end - buffer) > 3 && strncmp(buffer, "null", 5) != 0) {
        return utils::Status::ok();
    }

    return utils::Status::invalidArgument("Unknown character while parsing JSON.");
}

utils::Status MaterialParser::parseMaterialAsJSON(const char* buffer, size_t size,
        filamat::MaterialBuilder& builder) const noexcept {

    JsonishLexer jlexer;
    jlexer.lex(buffer, size, 1);

    JsonishParser parser(jlexer.getLexemes());
    std::unique_ptr<JsonishObject> json = parser.parse();
    if (json == nullptr) {
        return utils::Status::internal("Could not parse JSON material file");
    }

    for (auto& entry : json->getEntries()) {
        const std::string& key = entry.first;
        if (mConfigProcessorJSON.find(key) == mConfigProcessorJSON.end()) {
            utils::io::sstream errorMessage;
            errorMessage << "Unknown identifier '" << key << "'";
            return utils::Status::invalidArgument(errorMessage.c_str());
        }

        // Retrieve function member pointer
        MaterialConfigProcessorJSON const p =  mConfigProcessorJSON.at(key);
        // Call it.
        if (utils::Status status = (*this.*p)(entry.second, builder); !status.isOk()) {
            return status;
        }
    }

    return utils::Status::ok();
}

utils::Status MaterialParser::parseMaterial(const char* buffer, size_t size,
        filamat::MaterialBuilder& builder) const noexcept {

    MaterialLexer materialLexer;
    materialLexer.lex(buffer, size);
    auto lexemes = materialLexer.getLexemes();

    // Make sure the lexer did not stumble upon unknown character. This could mean we received
    // a binary file.
    for (auto lexeme : lexemes) {
        if (lexeme.getType() == MaterialType::UNKNOWN) {
            utils::io::sstream errorMessage;
            errorMessage << "Unexpected character at line:" << lexeme.getLine()
                      << " position:" << lexeme.getLinePosition();
            return utils::Status::invalidArgument(errorMessage.c_str());
        }
    }

    // Make a first quick pass just to make sure the format was respected (the material format is
    // a series of IDENTIFIER, BLOCK pairs).
    if (lexemes.size() < 2) {
        return utils::Status::invalidArgument(
                "Input MUST be an alternation of [identifier, block] pairs.");
    }
    for (size_t i = 0; i < lexemes.size(); i += 2) {
        auto lexeme = lexemes.at(i);
        if (lexeme.getType() != MaterialType::IDENTIFIER) {
            utils::io::sstream errorMessage;
            errorMessage << "An identifier was expected at line:" << lexeme.getLine()
                      << " position:" << lexeme.getLinePosition();
            return utils::Status::invalidArgument(errorMessage.c_str());
        }

        if (i == lexemes.size() - 1) {
            utils::io::sstream errorMessage;
            errorMessage << "Identifier at line:" << lexeme.getLine()
                      << " position:" << lexeme.getLinePosition()
                      << " must be followed by a block.";
            return utils::Status::invalidArgument(errorMessage.c_str());
        }

        auto nextLexeme = lexemes.at(i + 1);
        if (nextLexeme.getType() != MaterialType::BLOCK) {
            utils::io::sstream errorMessage;
            errorMessage << "A block was expected at line:" << lexeme.getLine()
                      << " position:" << lexeme.getLinePosition();
            return utils::Status::invalidArgument(errorMessage.c_str());
        }
    }


    std::string identifier;
    for (auto lexeme : lexemes) {
        if (lexeme.getType() == MaterialType::IDENTIFIER) {
            identifier = lexeme.getStringValue();
            if (mConfigProcessor.find(identifier) == mConfigProcessor.end()) {
                utils::io::sstream errorMessage;
                errorMessage << "Unknown identifier '"
                          << identifier
                          << "' at line:" << lexeme.getLine()
                          << " position:" << lexeme.getLinePosition();
                return utils::Status::invalidArgument(errorMessage.c_str());
            }
        } else if (lexeme.getType() == MaterialType::BLOCK) {
            MaterialConfigProcessor const processor = mConfigProcessor.at(identifier);
            if (utils::Status status = (*this.*processor)(lexeme, builder); !status.isOk()) {
                return status;
            }
        }
    }
    return utils::Status::ok();
}

utils::Status MaterialParser::processMaterialParameters(filamat::MaterialBuilder& builder,
        const Config& config) const {
    ParametersProcessor parametersProcessor;
    utils::Status status;
    for (const auto& param : config.getMaterialParameters()) {
        utils::Status s = parametersProcessor.process(builder, param.first, param.second);
        if (!s.isOk()) {
            status = s;
        }
    }
    return status;
}

utils::Status MaterialParser::processTemplateSubstitutions(
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
                    return utils::Status::internal("Unexpected end of file");
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
                utils::io::sstream errorMessage;
                errorMessage << "Undefined template macro:" << macro;
                return utils::Status::invalidArgument(errorMessage.c_str());
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
    return utils::Status::ok();
}

utils::Status MaterialParser::parse(filamat::MaterialBuilder& builder,
        const Config& config, ssize_t& size, std::unique_ptr<const char[]>& buffer) {

    if (builder.getFeatureLevel() > config.getFeatureLevel()) {
        utils::io::sstream errorMessage;
        errorMessage << "Material feature level (" << +builder.getFeatureLevel()
            << ") is higher than maximum allowed (" << +config.getFeatureLevel() << ")";
        return utils::Status::invalidArgument(errorMessage.c_str());
    }

    // Before attempting an expensive lex, let's find out if we were sent pure JSON.
    utils::Status parsedStatus;
    if (utils::Status validJson = isValidJsonStart(buffer.get(), size_t(size));
            validJson.isOk()) {
        parsedStatus = parseMaterialAsJSON(buffer.get(), size_t(size), builder);
    } else {
        parsedStatus = parseMaterial(buffer.get(), size_t(size), builder);
    }

    if (!parsedStatus.isOk()) {
        return parsedStatus;
    }

    switch (config.getReflectionTarget()) {
        case Config::Metadata::NONE:
            break;
        case Config::Metadata::PARAMETERS:
            return reflectParameters(builder);
    }

    builder
            .noSamplerValidation(config.noSamplerValidation())
            .includeEssl1(config.includeEssl1())
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

    if (utils::Status processedStatus = processMaterialParameters(builder, config);
            !processedStatus.isOk()) {
        return processedStatus;
    }

    return utils::Status::ok();
}

} // namespace matp