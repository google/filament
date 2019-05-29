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

template <class T>
static bool logEnumIssue(const std::string& key, const JsonishString& value,
        const std::unordered_map<std::string, T>& map) noexcept {
    std::cerr << "Error while processing key '" << key << "' value." << std::endl;
    std::cerr << "Value '" << value.getString() << "' is invalid. Valid values are:"
            << std::endl;
    for (auto entries : map) {
        std::cerr << "    " << entries.first  << std::endl;
    }
    return false;
}

template <class T>
static bool isStringValidEnum(const std::unordered_map<std::string, T>& map,
        const std::string& s) noexcept {
    return map.find(s) != map.end();
}

template <class T>
static T stringToEnum(const std::unordered_map<std::string, T>& map,
        const std::string& s) noexcept {
    return map.at(s);
}

static MaterialBuilder::Variable intToVariable(size_t i) noexcept {
    switch (i) {
        case 0:  return MaterialBuilder::Variable::CUSTOM0;
        case 1:  return MaterialBuilder::Variable::CUSTOM1;
        case 2:  return MaterialBuilder::Variable::CUSTOM2;
        case 3:  return MaterialBuilder::Variable::CUSTOM3;
        default: return MaterialBuilder::Variable::CUSTOM0;
    }
}

static bool processName(MaterialBuilder& builder, const JsonishValue& value) {
    builder.name(value.toJsonString()->getString().c_str());
    return true;
}

static bool processInterpolation(MaterialBuilder& builder, const JsonishValue& value) {
    static const std::unordered_map<std::string, MaterialBuilder::Interpolation> strToEnum {
        { "smooth", MaterialBuilder::Interpolation::SMOOTH },
        { "flat", MaterialBuilder::Interpolation::FLAT },
    };
    auto interpolationString = value.toJsonString();
    if (!isStringValidEnum(strToEnum, interpolationString->getString())) {
        return logEnumIssue("interpolation", *interpolationString, strToEnum);
    }
    builder.interpolation(stringToEnum(strToEnum, interpolationString->getString()));
    return true;
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

static bool processParameter(MaterialBuilder& builder, const JsonishObject& jsonObject) noexcept {

    const JsonishValue* typeValue = jsonObject.getValue("type");
    if (!typeValue) {
        std::cerr << "parameters: entry without key 'type'." << std::endl;
        return false;
    }
    if (typeValue->getType() != JsonishValue::STRING) {
        std::cerr << "parameters: type value must be STRING." << std::endl;
        return false;
    }

    const JsonishValue* nameValue = jsonObject.getValue("name");
    if (!nameValue) {
        std::cerr << "parameters: entry without 'name' key." << std::endl;
        return false;
    }
    if (nameValue->getType() != JsonishValue::STRING) {
        std::cerr << "parameters: name value must be STRING." << std::endl;
        return false;
    }

    const JsonishValue* precisionValue = jsonObject.getValue("precision");
    if (precisionValue) {
        if (precisionValue->getType() != JsonishValue::STRING) {
            std::cerr << "parameters: precision must be a STRING." << std::endl;
            return false;
        }

        auto precisionString = precisionValue->toJsonString();
        if (!Enums::isValid<SamplerPrecision>(precisionString->getString())){
            return logEnumIssue("parameters", *precisionString, Enums::map<SamplerPrecision>());
        }
    }

    const JsonishValue* formatValue = jsonObject.getValue("format");
    if (formatValue) {
        if (formatValue->getType() != JsonishValue::STRING) {
            std::cerr << "parameters: format must be a STRING." << std::endl;
            return false;
        }

        auto formatString = formatValue->toJsonString();
        if (!Enums::isValid<SamplerFormat>(formatString->getString())){
            return logEnumIssue("parameters", *formatString, Enums::map<SamplerFormat>());
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
            std::cerr << "parameters: the parameter with name '" << nameString << "'"
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
        std::cerr << "parameters: the type '" << typeString
               << "' for parameter with name '" << nameString << "' is neither a valid uniform "
               << "type nor a valid sampler type." << std::endl;
        return false;
    }

    return true;
}

static bool processParameters(MaterialBuilder& builder, const JsonishValue& v) {
    auto jsonArray = v.toJsonArray();

    bool ok = true;
    for (auto value : jsonArray->getElements()) {
        if (value->getType() == JsonishValue::Type::OBJECT) {
            ok |= processParameter(builder, *value->toJsonObject());
            continue;
        }
        std::cerr << "parameters must be an array of OBJECTs." << std::endl;
        return false;
    }
    return ok;
}

static bool processVariables(MaterialBuilder& builder, const JsonishValue& value) {
    const JsonishArray* jsonArray = value.toJsonArray();
    const auto& elements = jsonArray->getElements();

    if (elements.size() > 4) {
        std::cerr << "variables: Max array size is 4." << std::endl;
        return false;
    }

    for (size_t i = 0; i < elements.size(); i++) {
        auto elementValue = elements[i];
        MaterialBuilder::Variable v = intToVariable(i);
        if (elementValue->getType() != JsonishValue::Type::STRING) {
            std::cerr << "variables: array index " << i << " is not a STRING. found:" <<
                    JsonishValue::typeToString(elementValue->getType()) << std::endl;
            return false;
        }
        builder.variable(v, elementValue->toJsonString()->getString().c_str());
    }

    return true;
}

static bool processRequires(MaterialBuilder& builder, const JsonishValue& value) {
    static const std::unordered_map<std::string, filament::VertexAttribute> strToEnum {
        { "color", filament::VertexAttribute::COLOR },
        { "position", filament::VertexAttribute::POSITION },
        { "tangents", filament::VertexAttribute::TANGENTS },
        { "uv0", filament::VertexAttribute::UV0 },
        { "uv1", filament::VertexAttribute::UV1 },
    };
    for (auto v : value.toJsonArray()->getElements()) {
        if (v->getType() != JsonishValue::Type::STRING) {
            std::cerr << "requires: entries must be STRINGs." << std::endl;
            return false;
        }

        auto jsonString = v->toJsonString();
        if (!isStringValidEnum(strToEnum, jsonString->getString())) {
            return logEnumIssue("requires", *jsonString, strToEnum);
        }

        builder.require(stringToEnum(strToEnum, jsonString->getString()));
    }

    return true;
}

static bool processBlending(MaterialBuilder& builder, const JsonishValue& value) {
    static const std::unordered_map<std::string, MaterialBuilder::BlendingMode> strToEnum {
        { "add", MaterialBuilder::BlendingMode::ADD },
        { "masked", MaterialBuilder::BlendingMode::MASKED },
        { "opaque", MaterialBuilder::BlendingMode::OPAQUE },
        { "transparent", MaterialBuilder::BlendingMode::TRANSPARENT },
        { "fade", MaterialBuilder::BlendingMode::FADE },
        { "multiply", MaterialBuilder::BlendingMode::MULTIPLY },
        { "screen", MaterialBuilder::BlendingMode::SCREEN },
    };
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(strToEnum, jsonString->getString())) {
        return logEnumIssue("blending", *jsonString, strToEnum);
    }

    builder.blending(stringToEnum(strToEnum, jsonString->getString()));
    return true;
}

static bool processPostLightingBlending(MaterialBuilder& builder, const JsonishValue& value) {
    static const std::unordered_map<std::string, MaterialBuilder::BlendingMode> strToEnum {
        { "add", MaterialBuilder::BlendingMode::ADD },
        { "opaque", MaterialBuilder::BlendingMode::OPAQUE },
        { "transparent", MaterialBuilder::BlendingMode::TRANSPARENT },
        { "multiply", MaterialBuilder::BlendingMode::MULTIPLY },
        { "screen", MaterialBuilder::BlendingMode::SCREEN },
    };
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(strToEnum, jsonString->getString())) {
        return logEnumIssue("postLightingBlending", *jsonString, strToEnum);
    }

    builder.postLightingBlending(stringToEnum(strToEnum, jsonString->getString()));
    return true;
}

static bool processVertexDomain(MaterialBuilder& builder, const JsonishValue& value) {
    static const std::unordered_map<std::string, MaterialBuilder::VertexDomain> strToEnum {
        { "device", MaterialBuilder::VertexDomain::DEVICE},
        { "object", MaterialBuilder::VertexDomain::OBJECT},
        { "world", MaterialBuilder::VertexDomain::WORLD},
        { "view", MaterialBuilder::VertexDomain::VIEW},
    };
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(strToEnum, jsonString->getString())) {
        return logEnumIssue("vertex_domain", *jsonString, strToEnum);
    }

    builder.vertexDomain(stringToEnum(strToEnum, jsonString->getString()));
    return true;
}

static bool processCulling(MaterialBuilder& builder, const JsonishValue& value) {
    static const std::unordered_map<std::string, MaterialBuilder::CullingMode> strToEnum {
        { "back", MaterialBuilder::CullingMode::BACK },
        { "front", MaterialBuilder::CullingMode::FRONT },
        { "frontAndBack", MaterialBuilder::CullingMode::FRONT_AND_BACK },
        { "none", MaterialBuilder::CullingMode::NONE },
    };
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(strToEnum, jsonString->getString())) {
        return logEnumIssue("culling", *jsonString, strToEnum);
    }

    builder.culling(stringToEnum(strToEnum, jsonString->getString()));
    return true;
}

static bool processColorWrite(MaterialBuilder& builder, const JsonishValue& value) {
    builder.colorWrite(value.toJsonBool()->getBool());
    return true;
}

static bool processDepthWrite(MaterialBuilder& builder, const JsonishValue& value) {
    builder.depthWrite(value.toJsonBool()->getBool());
    return true;
}

static bool processDepthCull(MaterialBuilder& builder, const JsonishValue& value) {
    builder.depthCulling(value.toJsonBool()->getBool());
    return true;
}

static bool processDoubleSided(MaterialBuilder& builder, const JsonishValue& value) {
    builder.doubleSided(value.toJsonBool()->getBool());
    return true;
}

static bool processTransparencyMode(MaterialBuilder& builder, const JsonishValue& value) {
    static const std::unordered_map<std::string, MaterialBuilder::TransparencyMode> strToEnum {
        { "default", MaterialBuilder::TransparencyMode::DEFAULT },
        { "twoPassesOneSide", MaterialBuilder::TransparencyMode::TWO_PASSES_ONE_SIDE },
        { "twoPassesTwoSides", MaterialBuilder::TransparencyMode::TWO_PASSES_TWO_SIDES },
    };
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(strToEnum, jsonString->getString())) {
        return logEnumIssue("transparency_mode", *jsonString, strToEnum);
    }

    builder.transparencyMode(stringToEnum(strToEnum, jsonString->getString()));
    return true;
}

static bool processMaskThreshold(MaterialBuilder& builder, const JsonishValue& value) {
    builder.maskThreshold(value.toJsonNumber()->getFloat());
    return true;
}

static bool processShadowMultiplier(MaterialBuilder& builder, const JsonishValue& value) {
    builder.shadowMultiplier(value.toJsonBool()->getBool());
    return true;
}

static bool processSpecularAntiAliasing(MaterialBuilder& builder, const JsonishValue& value) {
    builder.specularAntiAliasing(value.toJsonBool()->getBool());
    return true;
}

static bool processSpecularAntiAliasingVariance(MaterialBuilder& builder, const JsonishValue& value) {
    builder.specularAntiAliasingVariance(value.toJsonNumber()->getFloat());
    return true;
}

static bool processSpecularAntiAliasingThreshold(MaterialBuilder& builder, const JsonishValue& value) {
    builder.specularAntiAliasingThreshold(value.toJsonNumber()->getFloat());
    return true;
}

static bool processClearCoatIorChange(MaterialBuilder& builder, const JsonishValue& value) {
    builder.clearCoatIorChange(value.toJsonBool()->getBool());
    return true;
}

static bool processFlipUV(MaterialBuilder& builder, const JsonishValue& value) {
    builder.flipUV(value.toJsonBool()->getBool());
    return true;
}

static bool processMultiBounceAO(MaterialBuilder& builder, const JsonishValue& value) {
    builder.multiBounceAmbientOcclusion(value.toJsonBool()->getBool());
    return true;
}

static bool processSpecularAmbientOcclusion(MaterialBuilder& builder, const JsonishValue& value) {
    builder.specularAmbientOcclusion(value.toJsonBool()->getBool());
    return true;
}

static bool processShading(MaterialBuilder& builder, const JsonishValue& value) {
    static const std::unordered_map<std::string, MaterialBuilder::Shading> strToEnum {
        { "cloth",              MaterialBuilder::Shading::CLOTH },
        { "lit",                MaterialBuilder::Shading::LIT },
        { "subsurface",         MaterialBuilder::Shading::SUBSURFACE },
        { "unlit",              MaterialBuilder::Shading::UNLIT },
        { "specularGlossiness", MaterialBuilder::Shading::SPECULAR_GLOSSINESS },
    };
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(strToEnum, jsonString->getString())) {
        return logEnumIssue("shading", *jsonString, strToEnum);
    }

    builder.shading(stringToEnum(strToEnum, jsonString->getString()));
    return true;
}

static bool processVariantFilter(MaterialBuilder& builder, const JsonishValue& value) {
    // We avoid using an initializer list for this particular map to avoid build errors that are
    // due to static initialization ordering.
    static const std::unordered_map<std::string, uint8_t> strToEnum  = [] {
        std::unordered_map<std::string, uint8_t> strToEnum;
        strToEnum["directionalLighting"] = filament::Variant::DIRECTIONAL_LIGHTING;
        strToEnum["dynamicLighting"] = filament::Variant::DYNAMIC_LIGHTING;
        strToEnum["shadowReceiver"] = filament::Variant::SHADOW_RECEIVER;
        strToEnum["skinning"] = filament::Variant::SKINNING;
        return strToEnum;
    }();
    uint8_t variantFilter = 0;
    const JsonishArray* jsonArray = value.toJsonArray();
    const auto& elements = jsonArray->getElements();

    for (size_t i = 0; i < elements.size(); i++) {
        auto elementValue = elements[i];
        if (elementValue->getType() != JsonishValue::Type::STRING) {
            std::cerr << "variant_filter: array index " << i <<
                      " is not a STRING. found:" <<
                      JsonishValue::typeToString(elementValue->getType()) << std::endl;
            return false;
        }

        const std::string& s = elementValue->toJsonString()->getString();
        if (!isStringValidEnum(strToEnum, s)) {
            std::cerr << "variant_filter: variant " << s <<
                      " is not a valid variant" << std::endl;
        }

        variantFilter |= strToEnum.at(s);
    }

    builder.variantFilter(variantFilter);
    return true;
}

ParametersProcessor::ParametersProcessor() {
    using Type = JsonishValue::Type;
    mParameters["name"]                          = { &processName, Type::STRING };
    mParameters["interpolation"]                 = { &processInterpolation, Type::STRING };
    mParameters["parameters"]                    = { &processParameters, Type::ARRAY };
    mParameters["variables"]                     = { &processVariables, Type::ARRAY };
    mParameters["requires"]                      = { &processRequires, Type::ARRAY };
    mParameters["blending"]                      = { &processBlending, Type::STRING };
    mParameters["postLightingBlending"]          = { &processPostLightingBlending, Type::STRING };
    mParameters["vertexDomain"]                  = { &processVertexDomain, Type::STRING };
    mParameters["culling"]                       = { &processCulling, Type::STRING };
    mParameters["colorWrite"]                    = { &processColorWrite, Type::BOOL };
    mParameters["depthWrite"]                    = { &processDepthWrite, Type::BOOL };
    mParameters["depthCulling"]                  = { &processDepthCull, Type::BOOL };
    mParameters["doubleSided"]                   = { &processDoubleSided, Type::BOOL };
    mParameters["transparency"]                  = { &processTransparencyMode, Type::STRING };
    mParameters["maskThreshold"]                 = { &processMaskThreshold, Type::NUMBER };
    mParameters["shadowMultiplier"]              = { &processShadowMultiplier, Type::BOOL };
    mParameters["shadingModel"]                  = { &processShading, Type::STRING };
    mParameters["variantFilter"]                 = { &processVariantFilter, Type::ARRAY };
    mParameters["specularAntiAliasing"]          = { &processSpecularAntiAliasing, Type::BOOL };
    mParameters["specularAntiAliasingVariance"]  = { &processSpecularAntiAliasingVariance, Type::NUMBER };
    mParameters["specularAntiAliasingThreshold"] = { &processSpecularAntiAliasingThreshold, Type::NUMBER };
    mParameters["clearCoatIorChange"]            = { &processClearCoatIorChange, Type::BOOL };
    mParameters["flipUV"]                        = { &processFlipUV, Type::BOOL };
    mParameters["multiBounceAmbientOcclusion"]   = { &processMultiBounceAO, Type::BOOL };
    mParameters["specularAmbientOcclusion"]      = { &processSpecularAmbientOcclusion, Type::BOOL };
}

bool ParametersProcessor::process(MaterialBuilder& builder, const JsonishObject& jsonObject) {
    for(const auto& entry : jsonObject.getEntries()) {
        const std::string& key = entry.first;
        const JsonishValue* field = entry.second;
        if (mParameters.find(key) == mParameters.end()) {
            std::cerr << "Ignoring config entry (unknown key): \"" << key << "\"" << std::endl;
            continue;
        }

        // Verify type is what was expected.
        if (mParameters.at(key).rootAssert != field->getType()) {
            std::cerr << "Value for key:\"" << key << "\" is not what was expected" << std::endl;
            std::cerr << "Got :\"" << JsonishValue::typeToString(field->getType())<< "\" but expected '"
                   << JsonishValue::typeToString(mParameters.at(key).rootAssert) << "'" << std::endl;
            return false;
        }

        auto fPointer = mParameters[key].callback;
        bool ok = fPointer(builder, *field);
        if (!ok) {
            std::cerr << "Error while processing material key:\"" << key << "\"" << std::endl;
            return false;
        }
    }
    return true;
}

} // namespace matc
