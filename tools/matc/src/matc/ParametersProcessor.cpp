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

#include "ParametersProcessor.h"

#include <algorithm>
#include <iostream>

#include <ctype.h>
#include <private/filament/Variant.h>

#include <filamat/Enums.h>

using namespace filamat;
using namespace utils;

namespace matc {

static constexpr const char* PARAM_KEY_NAME              = "name";
static constexpr const char* PARAM_KEY_INTERPOLATION     = "interpolation";
static constexpr const char* PARAM_KEY_PARAMETERS        = "parameters";
static constexpr const char* PARAM_KEY_VARIABLES         = "variables";
static constexpr const char* PARAM_KEY_REQUIRES          = "requires";
static constexpr const char* PARAM_KEY_BLENDING          = "blending";
static constexpr const char* PARAM_KEY_VERTEX_DOMAIN     = "vertexDomain";
static constexpr const char* PARAM_KEY_CULLING           = "culling";
static constexpr const char* PARAM_KEY_COLOR_WRITE       = "colorWrite";
static constexpr const char* PARAM_KEY_DEPTH_WRITE       = "depthWrite";
static constexpr const char* PARAM_KEY_DEPTH_CULL        = "depthCulling";
static constexpr const char* PARAM_KEY_DOUBLE_SIDED      = "doubleSided";
static constexpr const char* PARAM_KEY_TRANSPARENCY_MODE = "transparency";
static constexpr const char* PARAM_KEY_MASK_THRESHOLD    = "maskThreshold";
static constexpr const char* PARAM_KEY_SHADOW_MULTIPLIER = "shadowMultiplier";
static constexpr const char* PARAM_KEY_SHADING           = "shadingModel";
static constexpr const char* PARAM_KEY_VARIANT_FILTER    = "variantFilter";

ParametersProcessor::ParametersProcessor() {
    mConfigProcessor[PARAM_KEY_NAME]              = &ParametersProcessor::processName;
    mConfigProcessor[PARAM_KEY_INTERPOLATION]     = &ParametersProcessor::processInterpolation;
    mConfigProcessor[PARAM_KEY_PARAMETERS]        = &ParametersProcessor::processParameters;
    mConfigProcessor[PARAM_KEY_VARIABLES]         = &ParametersProcessor::processVariables;
    mConfigProcessor[PARAM_KEY_REQUIRES]          = &ParametersProcessor::processRequires;
    mConfigProcessor[PARAM_KEY_BLENDING]          = &ParametersProcessor::processBlending;
    mConfigProcessor[PARAM_KEY_VERTEX_DOMAIN]     = &ParametersProcessor::processVertexDomain;
    mConfigProcessor[PARAM_KEY_CULLING]           = &ParametersProcessor::processCulling;
    mConfigProcessor[PARAM_KEY_COLOR_WRITE]       = &ParametersProcessor::processColorWrite;
    mConfigProcessor[PARAM_KEY_DEPTH_WRITE]       = &ParametersProcessor::processDepthWrite;
    mConfigProcessor[PARAM_KEY_DEPTH_CULL]        = &ParametersProcessor::processDepthCull;
    mConfigProcessor[PARAM_KEY_DOUBLE_SIDED]      = &ParametersProcessor::processDoubleSided;
    mConfigProcessor[PARAM_KEY_TRANSPARENCY_MODE] = &ParametersProcessor::processTransparencyMode;
    mConfigProcessor[PARAM_KEY_MASK_THRESHOLD]    = &ParametersProcessor::processMaskThreshold;
    mConfigProcessor[PARAM_KEY_SHADOW_MULTIPLIER] = &ParametersProcessor::processShadowMultiplier;
    mConfigProcessor[PARAM_KEY_SHADING]           = &ParametersProcessor::processShading;
    mConfigProcessor[PARAM_KEY_VARIANT_FILTER]    = &ParametersProcessor::processVariantFilter;

    mRootAsserts[PARAM_KEY_NAME]              = JsonishValue::Type::STRING;
    mRootAsserts[PARAM_KEY_INTERPOLATION]     = JsonishValue::Type::STRING;
    mRootAsserts[PARAM_KEY_PARAMETERS]        = JsonishValue::Type::ARRAY;
    mRootAsserts[PARAM_KEY_VARIABLES]         = JsonishValue::Type::ARRAY;
    mRootAsserts[PARAM_KEY_REQUIRES]          = JsonishValue::Type::ARRAY;
    mRootAsserts[PARAM_KEY_BLENDING]          = JsonishValue::Type::STRING;
    mRootAsserts[PARAM_KEY_VERTEX_DOMAIN]     = JsonishValue::Type::STRING;
    mRootAsserts[PARAM_KEY_CULLING]           = JsonishValue::Type::STRING;
    mRootAsserts[PARAM_KEY_COLOR_WRITE]       = JsonishValue::Type::BOOL;
    mRootAsserts[PARAM_KEY_DEPTH_WRITE]       = JsonishValue::Type::BOOL;
    mRootAsserts[PARAM_KEY_DEPTH_CULL]        = JsonishValue::Type::BOOL;
    mRootAsserts[PARAM_KEY_DOUBLE_SIDED]      = JsonishValue::Type::BOOL;
    mRootAsserts[PARAM_KEY_TRANSPARENCY_MODE] = JsonishValue::Type::STRING;
    mRootAsserts[PARAM_KEY_MASK_THRESHOLD]    = JsonishValue::Type::NUMBER;
    mRootAsserts[PARAM_KEY_SHADOW_MULTIPLIER] = JsonishValue::Type::BOOL;
    mRootAsserts[PARAM_KEY_SHADING]           = JsonishValue::Type::STRING;
    mRootAsserts[PARAM_KEY_VARIANT_FILTER]    = JsonishValue::Type::ARRAY;

    mStringToInterpolation["smooth"] = MaterialBuilder::Interpolation::SMOOTH;
    mStringToInterpolation["flat"] = MaterialBuilder::Interpolation::FLAT;

    mStringToCullingMode["back"] = MaterialBuilder::CullingMode::BACK;
    mStringToCullingMode["front"] = MaterialBuilder::CullingMode::FRONT;
    mStringToCullingMode["frontAndBack"] = MaterialBuilder::CullingMode::FRONT_AND_BACK;
    mStringToCullingMode["none"] = MaterialBuilder::CullingMode::NONE;

    mStringToTransparencyMode["default"] = MaterialBuilder::TransparencyMode::DEFAULT;
    mStringToTransparencyMode["twoPassesOneSide"] = MaterialBuilder::TransparencyMode::TWO_PASSES_ONE_SIDE;
    mStringToTransparencyMode["twoPassesTwoSides"] = MaterialBuilder::TransparencyMode::TWO_PASSES_TWO_SIDES;

    mStringToVertexDomain["device"] = MaterialBuilder::VertexDomain::DEVICE;
    mStringToVertexDomain["object"] = MaterialBuilder::VertexDomain::OBJECT;
    mStringToVertexDomain["world"] = MaterialBuilder::VertexDomain::WORLD;
    mStringToVertexDomain["view"] = MaterialBuilder::VertexDomain::VIEW;

    mStringToBlendingMode["add"] = MaterialBuilder::BlendingMode::ADD;
    mStringToBlendingMode["masked"] = MaterialBuilder::BlendingMode::MASKED;
    mStringToBlendingMode["opaque"] = MaterialBuilder::BlendingMode::OPAQUE;
    mStringToBlendingMode["transparent"] = MaterialBuilder::BlendingMode::TRANSPARENT;
    mStringToBlendingMode["fade"] = MaterialBuilder::BlendingMode::FADE;

    mStringToAttributeIndex["color"] = filament::VertexAttribute::COLOR;
    mStringToAttributeIndex["position"] = filament::VertexAttribute::POSITION;
    mStringToAttributeIndex["tangents"] = filament::VertexAttribute::TANGENTS;
    mStringToAttributeIndex["uv0"] = filament::VertexAttribute::UV0;
    mStringToAttributeIndex["uv1"] = filament::VertexAttribute::UV1;

    mStringToShading["cloth"] = MaterialBuilder::Shading::CLOTH;
    mStringToShading["lit"] = MaterialBuilder::Shading::LIT;
    mStringToShading["subsurface"] = MaterialBuilder::Shading::SUBSURFACE;
    mStringToShading["unlit"] = MaterialBuilder::Shading::UNLIT;

    mStringToVariant["directionalLighting"] = filament::Variant::DIRECTIONAL_LIGHTING;
    mStringToVariant["dynamicLighting"] = filament::Variant::DYNAMIC_LIGHTING;
    mStringToVariant["shadowReceiver"] = filament::Variant::SHADOW_RECEIVER;
    mStringToVariant["skinning"] = filament::Variant::SKINNING;
}

bool ParametersProcessor::process(filamat::MaterialBuilder& builder, const JsonishObject& jsonObject) {
    for(auto entry : jsonObject.getEntries()) {
        const std::string& key = entry.first;
        const JsonishValue* field = entry.second;
        if (mConfigProcessor.find(key) == mConfigProcessor.end()) {
            std::cerr << "Ignoring config entry (unknown key): \"" << key << "\"" << std::endl;
            continue;
        }

        // Verify type is what was expected.
        if (mRootAsserts.at(key) != field->getType()) {
            std::cerr << "Value for key:\"" << key << "\" is not what was expected" << std::endl;
            std::cerr << "Got :\"" << JsonishValue::typeToString(field->getType())<< "\" but expected '"
                   << JsonishValue::typeToString(mRootAsserts.at(key)) << "'" << std::endl;
            return false;
        }

        auto fPointer = mConfigProcessor[key];
        bool ok = (*this.*fPointer)(builder,*field);
        if (!ok) {
            std::cerr << "Error while processing material key:\"" << key << "\"" << std::endl;
            return false;
        }
    }
    return true;
}

bool ParametersProcessor::processName(filamat::MaterialBuilder& builder, const JsonishValue& value) {
    builder.name(value.toJsonString()->getString().c_str());
    return true;
}

bool ParametersProcessor::processInterpolation(filamat::MaterialBuilder& builder,
        const JsonishValue& value) {
    auto interpolationString = value.toJsonString();
    if (!isStringValidEnum(mStringToInterpolation, interpolationString->getString())) {
        return logEnumIssue(PARAM_KEY_INTERPOLATION, *interpolationString, mStringToInterpolation);
    }

    builder.interpolation(stringToEnum(mStringToInterpolation, interpolationString->getString()));
    return true;
}

bool ParametersProcessor::processParameters(filamat::MaterialBuilder& builder,
        const JsonishValue& v) {
    auto jsonArray = v.toJsonArray();

    bool ok = true;
    for (auto value : jsonArray->getElements()) {
        if (value->getType() == JsonishValue::Type::OBJECT) {
            ok |= processParameter(builder, *value->toJsonObject());
            continue;
        }
        std::cerr << PARAM_KEY_PARAMETERS << " must be an array of OBJECTs." << std::endl;
        return false;
    }
    return ok;
}

/**
 * Parses the supplied type string to return the array size it defines, or 0
 * if the type is not an array. If the type is an array, the type string is
 * modified to remove the array declaration part and keep only the base type.
 *
 * For instance:
 * "float" returns 0 and "float" is unmodified
 * "float[4]" returns 4 and "float[4]" is changed to "float"
 * "float[3" returns 0 and "float[3" is unmodified
 * "float[2]foo" returns 0 and "float[2]foo" is unmodified
 * "float[2foo]" returns 0 and "float[2foo]" is unmodified
 */
static size_t extractArraySize(std::string& type) {
    size_t start = type.find_first_of('[');
    // Not an array
    if (start == std::string::npos) {
        return 0;
    }

    size_t end = type.find_first_of(']', start);
    // If we cannot find ']' or if it's not the last character, return to fail later
    if (end == std::string::npos || end != type.length() - 1) {
        return 0;
    }

    // If not all of the characters in the array declaration are digits, return to fail later
    if (!std::all_of(type.cbegin() + start + 1, type.cbegin() + end,
            [](char c) { return isdigit(c); })) {
        return 0;
    }

    // Remove the [...] bit
    type.erase(start);

    // Return the size (we already validated this part of the string contains only digits)
    return std::stoul(type.c_str() + start + 1, nullptr);
}

bool ParametersProcessor::processParameter(filamat::MaterialBuilder& builder,
        const JsonishObject& jsonObject) const noexcept {

    const JsonishValue* typeValue = jsonObject.getValue("type");
    if (!typeValue) {
        std::cerr << PARAM_KEY_PARAMETERS << ": entry without key 'type'." << std::endl;
        return false;
    }
    if (typeValue->getType() != JsonishValue::STRING) {
        std::cerr << PARAM_KEY_PARAMETERS << ": type value must be STRING." << std::endl;
        return false;
    }

    const JsonishValue* nameValue = jsonObject.getValue("name");
    if (!nameValue) {
        std::cerr << PARAM_KEY_PARAMETERS << ": entry without 'name' key." << std::endl;
        return false;
    }
    if (nameValue->getType() != JsonishValue::STRING) {
        std::cerr << PARAM_KEY_PARAMETERS << ": name value must be STRING." << std::endl;
        return false;
    }

    const JsonishValue* precisionValue = jsonObject.getValue("precision");
    if (precisionValue) {
        if (precisionValue->getType() != JsonishValue::STRING) {
            std::cerr << PARAM_KEY_PARAMETERS << ": precision must be a STRING." << std::endl;
            return false;
        }

        auto precisionString = precisionValue->toJsonString();
        if (!Enums::isValid<SamplerPrecision>(precisionString->getString())){
            return logEnumIssue(PARAM_KEY_PARAMETERS,
                    *precisionString, Enums::map<SamplerPrecision>());
        }
    }

    const JsonishValue* formatValue = jsonObject.getValue("format");
    if (formatValue) {
        if (formatValue->getType() != JsonishValue::STRING) {
            std::cerr << PARAM_KEY_PARAMETERS<< ": format must be a STRING." << std::endl;
            return false;
        }

        auto formatString = formatValue->toJsonString();
        if (Enums::isValid<SamplerFormat>(formatString->getString())){
            return logEnumIssue(PARAM_KEY_PARAMETERS, *formatString, Enums::map<SamplerFormat>());
        }
    }

    auto typeString = typeValue->toJsonString()->getString();
    auto nameString = nameValue->toJsonString()->getString();

    size_t arraySize = extractArraySize(typeString);

    if (Enums::isValid<UniformType>(typeString)) {
        MaterialBuilder::UniformType type = Enums::toEnum<UniformType>(typeString);
        if (arraySize == 0) {
            builder.parameter(type, nameString.c_str());
        } else {
            builder.parameter(type, arraySize, nameString.c_str());
        }
    } else if (Enums::isValid<SamplerType>(typeString)) {
        if (arraySize > 0) {
            std::cerr << PARAM_KEY_PARAMETERS << ": the parameter with name '" << nameString << "'"
                    << " is an array of samplers of size " << arraySize << ". Arrays of samplers"
                    << " are currently not supported." << std::endl;
            return false;
        }

        MaterialBuilder::SamplerType type = Enums::toEnum<SamplerType>(typeString);
        if (precisionValue && formatValue) {
            auto format = Enums::toEnum<SamplerFormat>(formatValue->toJsonString()->getString());
            auto precision =
                    Enums::toEnum<SamplerPrecision>(precisionValue->toJsonString()->getString());
            builder.parameter(type, format, precision, nameString.c_str());
        } else if (formatValue) {
            auto format = Enums::toEnum<SamplerFormat>(formatValue->toJsonString()->getString());
            builder.parameter(type, format, nameString.c_str());
        } else if (precisionValue) {
            auto precision =
                    Enums::toEnum<SamplerPrecision>(precisionValue->toJsonString()->getString());
            builder.parameter(type, precision, nameString.c_str());
        } else {
            builder.parameter(type, nameString.c_str());
        }
    } else {
        std::cerr << PARAM_KEY_PARAMETERS << ": the type '" << typeString
               << "' for parameter with name '" << nameString << "' is neither a valid uniform "
               << "type nor a valid sampler type." << std::endl;
        return false;
    }

    return true;
}

bool ParametersProcessor::processVariables(filamat::MaterialBuilder& builder,
        const JsonishValue& value) {
    const JsonishArray* jsonArray = value.toJsonArray();
    const auto& elements = jsonArray->getElements();

    if (elements.size() > 4) {
        std::cerr << PARAM_KEY_VARIABLES << ": Max array size is 4." << std::endl;
        return false;
    }

    for (size_t i = 0; i < elements.size(); i++) {
        auto elementValue = elements[i];
        filamat::MaterialBuilder::Variable v = intToVariable(i);
        if (elementValue->getType() != JsonishValue::Type::STRING) {
            std::cerr << PARAM_KEY_VARIABLES << ": array index " << i << " is not a STRING. found:" <<
                    JsonishValue::typeToString(elementValue->getType()) << std::endl;
            return false;
        }
        builder.variable(v, elementValue->toJsonString()->getString().c_str());
    }

    return true;
}

bool ParametersProcessor::processRequires(filamat::MaterialBuilder& builder,
        const JsonishValue& value) {
    for (auto v : value.toJsonArray()->getElements()) {
        if (v->getType() != JsonishValue::Type::STRING) {
            std::cerr << PARAM_KEY_REQUIRES << ": entries must be STRINGs." << std::endl;
            return false;
        }

        auto jsonString = v->toJsonString();
        if (!isStringValidEnum(mStringToAttributeIndex, jsonString->getString())) {
            return logEnumIssue(PARAM_KEY_REQUIRES, *jsonString, mStringToAttributeIndex);
        }

        builder.require(stringToEnum(mStringToAttributeIndex, jsonString->getString()));
    }

    return true;
}

bool ParametersProcessor::processBlending(filamat::MaterialBuilder& builder,
        const JsonishValue& value) {
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(mStringToBlendingMode, jsonString->getString())) {
        std::cerr << PARAM_KEY_BLENDING << ": value is not a valid BlendMode." << std::endl;
        return false;
    }

    builder.blending(stringToEnum(mStringToBlendingMode, jsonString->getString()));
    return true;
}

bool ParametersProcessor::processVertexDomain(filamat::MaterialBuilder& builder,
        const JsonishValue& value) {
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(mStringToVertexDomain, jsonString->getString())) {
        return logEnumIssue(PARAM_KEY_VERTEX_DOMAIN, *jsonString, mStringToVertexDomain);
    }

    builder.vertexDomain(stringToEnum(mStringToVertexDomain, jsonString->getString()));
    return true;
}

bool ParametersProcessor::processCulling(filamat::MaterialBuilder& builder,
        const JsonishValue& value) {
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(mStringToCullingMode, jsonString->getString())) {
        return logEnumIssue(PARAM_KEY_CULLING, *jsonString, mStringToCullingMode);
    }

    builder.culling(stringToEnum(mStringToCullingMode, jsonString->getString()));
    return true;
}

bool ParametersProcessor::processColorWrite(filamat::MaterialBuilder& builder,
        const JsonishValue& value) {
    builder.colorWrite(value.toJsonBool()->getBool());
    return true;
}

bool ParametersProcessor::processDepthWrite(filamat::MaterialBuilder& builder,
        const JsonishValue& value) {
    builder.depthWrite(value.toJsonBool()->getBool());
    return true;
}

bool ParametersProcessor::processDepthCull(filamat::MaterialBuilder& builder,
        const JsonishValue& value) {
    builder.depthCulling(value.toJsonBool()->getBool());
    return true;
}

bool ParametersProcessor::processDoubleSided(filamat::MaterialBuilder& builder,
        const JsonishValue& value) {
    builder.doubleSided(value.toJsonBool()->getBool());
    return true;
}

bool ParametersProcessor::processTransparencyMode(filamat::MaterialBuilder& builder,
        const JsonishValue& value) {
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(mStringToTransparencyMode, jsonString->getString())) {
        return logEnumIssue(PARAM_KEY_TRANSPARENCY_MODE, *jsonString, mStringToTransparencyMode);
    }

    builder.transparencyMode(stringToEnum(mStringToTransparencyMode, jsonString->getString()));
    return true;
}

bool ParametersProcessor::processMaskThreshold(filamat::MaterialBuilder& builder,
        const JsonishValue& value) {
    builder.maskThreshold(value.toJsonNumber()->getFloat());
    return true;
}

bool ParametersProcessor::processShadowMultiplier(filamat::MaterialBuilder& builder,
        const JsonishValue& value) {
    builder.shadowMultiplier(value.toJsonBool()->getBool());
    return true;
}

bool ParametersProcessor::processShading(filamat::MaterialBuilder& builder,
        const JsonishValue& value) {
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(mStringToShading, jsonString->getString())) {
        return logEnumIssue(PARAM_KEY_SHADING, *jsonString, mStringToShading);
    }

    builder.shading(stringToEnum(mStringToShading, jsonString->getString()));
    return true;
}

bool ParametersProcessor::processVariantFilter(filamat::MaterialBuilder& builder,
        const JsonishValue& value) {
    uint8_t variantFilter = 0;
    const JsonishArray* jsonArray = value.toJsonArray();
    const auto& elements = jsonArray->getElements();

    for (size_t i = 0; i < elements.size(); i++) {
        auto elementValue = elements[i];
        if (elementValue->getType() != JsonishValue::Type::STRING) {
            std::cerr << PARAM_KEY_VARIANT_FILTER << ": array index " << i <<
                      " is not a STRING. found:" <<
                      JsonishValue::typeToString(elementValue->getType()) << std::endl;
            return false;
        }

        const std::string& s = elementValue->toJsonString()->getString();
        if (!isStringValidEnum(mStringToVariant, s)) {
            std::cerr << PARAM_KEY_VARIANT_FILTER << ": variant " << s <<
                      " is not a valid variant" << std::endl;
        }

        variantFilter |= mStringToVariant[s];
    }

    builder.variantFilter(variantFilter);
    return true;
}

filamat::MaterialBuilder::Variable ParametersProcessor::intToVariable(size_t i) const noexcept {
    switch (i) {
        case 0:  return MaterialBuilder::Variable::CUSTOM0;
        case 1:  return MaterialBuilder::Variable::CUSTOM1;
        case 2:  return MaterialBuilder::Variable::CUSTOM2;
        case 3:  return MaterialBuilder::Variable::CUSTOM3;
        default: return MaterialBuilder::Variable::CUSTOM0;
    }
}

} // namespace matc
