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

#include <filamat/Enums.h>

#include <private/filament/BufferInterfaceBlock.h>
#include <private/filament/Variant.h>

#include <backend/DriverEnums.h>

#include <math/vec3.h>

#include <algorithm>
#include <iostream>
#include <string_view>

#include <ctype.h>

using namespace filamat;
using namespace utils;

namespace matp {

template <class T>
static bool logEnumIssue(const std::string& key, const JsonishString& value,
        const std::unordered_map<std::string, T>& map) noexcept {
    std::cerr << "Error while processing key '" << key << "' value." << std::endl;
    std::cerr << "Value '" << value.getString() << "' is invalid. Valid values are:"
            << std::endl;
    for (const auto& entries : map) {
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
        case 4:  return MaterialBuilder::Variable::CUSTOM4;
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
 * "float[]" returns -1 and "float[]" is changed to "float"
 * "float[3" returns 0 and "float[3" is unmodified
 * "float[2]foo" returns 0 and "float[2]foo" is unmodified
 * "float[2foo]" returns 0 and "float[2foo]" is unmodified
 */
static ssize_t extractArraySize(std::string& type) {
    auto start = type.find_first_of('[');
    // Not an array
    if (start == std::string::npos) {
        return 0;
    }

    auto end = type.find_first_of(']', start);
    // If we cannot find ']' or if it's not the last character, return to fail later
    if (end == std::string::npos || end != type.length() - 1) {
        return 0;
    }

    // If not all the characters in the array declaration are digits, return to fail later
    if (!std::all_of(type.cbegin() + start + 1, type.cbegin() + end,
            [](char c) { return isdigit(c); })) {
        return 0;
    }

    // Remove the [...] bit
    type.erase(start);

    // handle an empty size array: []
    if (end - start == 1) {
        return -1;
    }

    // Return the size (we already validated this part of the string contains only digits)
    return (ssize_t)std::stoul(type.c_str() + start + 1, nullptr);
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

    const JsonishValue* transformNameValue = jsonObject.getValue("transformName");
    if (transformNameValue && transformNameValue->getType() != JsonishValue::STRING) {
        std::cerr << "parameters: transformName value must be STRING." << std::endl;
        return false;
    }

    const JsonishValue* precisionValue = jsonObject.getValue("precision");
    if (precisionValue) {
        if (precisionValue->getType() != JsonishValue::STRING) {
            std::cerr << "parameters: precision must be a STRING." << std::endl;
            return false;
        }
        auto precisionString = precisionValue->toJsonString();
        if (!Enums::isValid<ParameterPrecision>(precisionString->getString())){
            return logEnumIssue("parameters", *precisionString, Enums::map<ParameterPrecision>());
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

    const JsonishValue* filterableValue = jsonObject.getValue("filterable");
    if (filterableValue) {
        if (filterableValue->getType() != JsonishValue::BOOL) {
            std::cerr << "parameters: filterable must be a BOOL." << std::endl;
            return false;
        }
    }
    const JsonishValue* multiSampleValue = jsonObject.getValue("multisample");
    if (multiSampleValue) {
        if (multiSampleValue->getType() != JsonishValue::BOOL) {
            std::cerr << "parameters: multisample must be a BOOL." << std::endl;
            return false;
        }
    }

    auto typeString = typeValue->toJsonString()->getString();
    auto nameString = nameValue->toJsonString()->getString();

    const JsonishValue* stagesValue = jsonObject.getValue("stages");
    using filament::backend::ShaderStageFlags;
    std::optional<ShaderStageFlags> stages;
    if (stagesValue) {
        ShaderStageFlags parsedStages = ShaderStageFlags::NONE;
        if (stagesValue->getType() != JsonishValue::ARRAY) {
            std::cerr << "parameters: stages must be an ARRAY." << std::endl;
            return false;
        }
        for (auto value: stagesValue->toJsonArray()->getElements()) {
            if (value->getType() == JsonishValue::Type::STRING) {
                using namespace std::literals;
                using Qualifier = filament::BufferInterfaceBlock::Qualifier;
                auto stageString = value->toJsonString()->getString();
                if (Enums::isValid<ShaderStageType>(stageString)) {
                    parsedStages |= Enums::toEnum<ShaderStageType>(stageString);
                } else {
                    std::cerr << "stages: the stage '" << stageString
                              << "' for parameter with name '" << nameString
                              << "' is not a valid shader stage." << std::endl;
                    return false;
                }
                continue;
            }
            std::cerr << "parameters: stages must be an array of STRINGs." << std::endl;
            return false;
        }
        stages = parsedStages;
    }

    size_t const arraySize = extractArraySize(typeString);

    if (Enums::isValid<UniformType>(typeString)) {
        if (stages.has_value()) {
            std::cerr << "parameters: the uniform parameter with name '" << nameString << "'"
                      << " has shader stages specified. Shader stages are only supported for"
                      << " samplers." << std::endl;
            return false;
        }
        MaterialBuilder::UniformType const type = Enums::toEnum<UniformType>(typeString);
        ParameterPrecision precision = ParameterPrecision::DEFAULT;
        if (precisionValue) {
            precision =
                    Enums::toEnum<ParameterPrecision>(precisionValue->toJsonString()->getString());
        }
        if (arraySize == 0) {
            builder.parameter(nameString.c_str(), type, precision);
        } else {
            builder.parameter(nameString.c_str(), arraySize, type, precision);
        }
    } else if (Enums::isValid<SamplerType>(typeString)) {
        if (arraySize > 0) {
            std::cerr << "parameters: the parameter with name '" << nameString << "'"
                    << " is an array of samplers of size " << arraySize << ". Arrays of samplers"
                    << " are currently not supported." << std::endl;
            return false;
        }

        MaterialBuilder::SamplerType const type = Enums::toEnum<SamplerType>(typeString);

        auto format = formatValue ? Enums::toEnum<SamplerFormat>(
                formatValue->toJsonString()->getString()) : SamplerFormat::FLOAT;

        auto precision = precisionValue ? Enums::toEnum<ParameterPrecision>(
                precisionValue->toJsonString()->getString()) : ParameterPrecision::DEFAULT;

        if (format == SamplerFormat::SHADOW) {
            std::cerr << "Materials should not be able to define a shadow sampler";
            return false;
        }

        if (format == SamplerFormat::INT && filterableValue) {
            std::cerr << "parameters: the parameter with name '" << nameString << "'"
                      << " is an integer sampler. The `filterable` attribute must not be defined."
                      << std::endl;
            return false;
        }

        // For samplers without `filterable` defined, we use the following logic
        //   - float sampler can be filterable or not, default to filterable
        //   - int sampler is not filterable (checked above)
        //   - shadow sampler must be filterable (checked above)
        auto filterable =
                filterableValue ? filterableValue->toJsonBool()->getBool() : true;

        auto multisample = multiSampleValue ? multiSampleValue->toJsonBool()->getBool() : false;

        if (transformNameValue) {
            if (type != MaterialBuilder::SamplerType::SAMPLER_EXTERNAL) {
                std::cerr << "parameters: the parameter with name '" << nameString << "'"
                    << " is a sampler of type " << typeString << " and has a transformName."
                    << " Transform names are only supported for external samplers."
                    << std::endl;
                return false;
            }
            auto transformName = transformNameValue->toJsonString()->getString();
            builder.parameter(nameString.c_str(), type, format, precision, filterable,
                    multisample, transformName.c_str(), stages);
        } else {
            builder.parameter(nameString.c_str(), type, format, precision, filterable,
                    multisample, "", stages);
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
            ok &= processParameter(builder, *value->toJsonObject());
            continue;
        }
        std::cerr << "parameters must be an array of OBJECTs." << std::endl;
        return false;
    }
    return ok;
}

static bool processConstant(MaterialBuilder& builder, const JsonishObject& jsonObject) noexcept {
    const JsonishValue* typeValue = jsonObject.getValue("type");
    if (!typeValue) {
        std::cerr << "constants: entry without key 'type'." << std::endl;
        return false;
    }
    if (typeValue->getType() != JsonishValue::STRING) {
        std::cerr << "constants: type value must be STRING." << std::endl;
        return false;
    }

    const JsonishValue* nameValue = jsonObject.getValue("name");
    if (!nameValue) {
        std::cerr << "constants: entry without 'name' key." << std::endl;
        return false;
    }
    if (nameValue->getType() != JsonishValue::STRING) {
        std::cerr << "constants: name value must be STRING." << std::endl;
        return false;
    }

    auto typeString = typeValue->toJsonString()->getString();
    auto nameString = nameValue->toJsonString()->getString();
    const JsonishValue* defaultValue = jsonObject.getValue("default");

    if (Enums::isValid<ConstantType>(typeString)) {
        auto type = Enums::toEnum<ConstantType>(typeString);
        switch (type) {
            case ConstantType::INT: {
                int32_t intDefault = 0;
                if (defaultValue) {
                    if (defaultValue->getType() != JsonishValue::NUMBER) {
                        std::cerr << "constants: INT constants must have NUMBER default value"
                                  << std::endl;
                        return false;
                    }
                    // FIXME: Jsonish doesn't distinguish between integers and floats.
                    intDefault = (int32_t)defaultValue->toJsonNumber()->getFloat();
                }
                builder.constant(nameString.c_str(), type, intDefault);
                break;
            }
            case ConstantType::FLOAT: {
                float floatDefault = 0.0f;
                if (defaultValue) {
                    if (defaultValue->getType() != JsonishValue::NUMBER) {
                        std::cerr << "constants: FLOAT constants must have NUMBER default value"
                                  << std::endl;
                        return false;
                    }
                    floatDefault = defaultValue->toJsonNumber()->getFloat();
                }
                builder.constant(nameString.c_str(), type, floatDefault);
                break;
            }
            case ConstantType::BOOL:
                bool boolDefault = false;
                if (defaultValue) {
                    if (defaultValue->getType() != JsonishValue::BOOL) {
                        std::cerr << "constants: BOOL constants must have BOOL default value"
                                  << std::endl;
                        return false;
                    }
                    boolDefault = defaultValue->toJsonBool()->getBool();
                }
                builder.constant(nameString.c_str(), type, boolDefault);
                break;
        }
    } else {
        std::cerr << "constants: the type '" << typeString
                  << "' for constant with name '" << nameString << "' is not a valid constant "
                  << "parameter type." << std::endl;
        return false;
    }

    return true;
}

static bool processConstants(MaterialBuilder& builder, const JsonishValue& v) {
    auto jsonArray = v.toJsonArray();

    bool ok = true;
    for (auto value : jsonArray->getElements()) {
        if (value->getType() == JsonishValue::Type::OBJECT) {
            ok &= processConstant(builder, *value->toJsonObject());
            continue;
        }
        std::cerr << "constants must be an array of OBJECTs." << std::endl;
        return false;
    }
    return ok;
}

static bool processBufferField(filament::BufferInterfaceBlock::Builder& builder,
        const JsonishObject& jsonObject) noexcept {

    const JsonishValue* nameValue = jsonObject.getValue("name");
    if (!nameValue) {
        std::cerr << "buffers: entry without 'name' key." << std::endl;
        return false;
    }
    if (nameValue->getType() != JsonishValue::STRING) {
        std::cerr << "buffers: name value must be STRING." << std::endl;
        return false;
    }

    const JsonishValue* typeValue = jsonObject.getValue("type");
    if (!typeValue) {
        std::cerr << "buffers: entry without key 'type'." << std::endl;
        return false;
    }
    if (typeValue->getType() != JsonishValue::STRING) {
        std::cerr << "buffers: type value must be STRING." << std::endl;
        return false;
    }

    const JsonishValue* precisionValue = jsonObject.getValue("precision");
    if (precisionValue) {
        if (precisionValue->getType() != JsonishValue::STRING) {
            std::cerr << "buffers: precision must be a STRING." << std::endl;
            return false;
        }

        auto precisionString = precisionValue->toJsonString();
        if (!Enums::isValid<ParameterPrecision>(precisionString->getString())){
            return logEnumIssue("buffers", *precisionString, Enums::map<ParameterPrecision>());
        }
    }

    auto typeString = typeValue->toJsonString()->getString();
    auto nameString = nameValue->toJsonString()->getString();

    size_t arraySize = extractArraySize(typeString);

    if (Enums::isValid<UniformType>(typeString)) {
        MaterialBuilder::UniformType type = Enums::toEnum<UniformType>(typeString);
        ParameterPrecision precision = ParameterPrecision::DEFAULT;
        if (precisionValue) {
            precision = Enums::toEnum<ParameterPrecision>(
                    precisionValue->toJsonString()->getString());
        }
        if (arraySize == -1) {
            builder.addVariableSizedArray({
                { nameString.data(), nameString.size() }, 0, type, precision });
        } else {
            builder.add({{
                { nameString.data(), nameString.size() }, uint32_t(arraySize), type, precision } });
        }
    } else {
        std::cerr << "buffers: the type '" << typeString << "' for parameter with name '"
                  << nameString << "' is not a valid buffer field type." << std::endl;
        return false;
    }

    return true;
}

static bool processBuffer(MaterialBuilder& builder,
        const JsonishObject& jsonObject) noexcept {

    filament::BufferInterfaceBlock::Builder bibb;

    bibb.target(filament::BufferInterfaceBlock::Target::SSBO);
    bibb.alignment(filament::BufferInterfaceBlock::Alignment::std430);

    const JsonishValue* nameValue = jsonObject.getValue("name");
    if (!nameValue) {
        std::cerr << "buffers: entry without 'name' key." << std::endl;
        return false;
    }
    if (nameValue->getType() != JsonishValue::STRING) {
        std::cerr << "buffers: name value must be STRING." << std::endl;
        return false;
    }

    const JsonishValue* qualifiersValue = jsonObject.getValue("qualifiers");
    if (!qualifiersValue) {
        std::cerr << "buffers: entry without key 'qualifiers'." << std::endl;
        return false;
    }
    if (qualifiersValue->getType() != JsonishValue::ARRAY) {
        std::cerr << "buffers: qualifiers value must be an ARRAY." << std::endl;
        return false;
    }

    const JsonishValue* fieldsValue = jsonObject.getValue("fields");
    if (!fieldsValue) {
        std::cerr << "buffers: entry without key 'fields'." << std::endl;
        return false;
    }
    if (fieldsValue->getType() != JsonishValue::ARRAY) {
        std::cerr << "buffers: fields value must be an ARRAY." << std::endl;
        return false;
    }

    auto nameString = nameValue->toJsonString()->getString();
    bibb.name({ nameString.data(), nameString.size() });

    for (auto value : qualifiersValue->toJsonArray()->getElements()) {
        if (value->getType() == JsonishValue::Type::STRING) {
            using namespace std::literals;
            using Qualifier = filament::BufferInterfaceBlock::Qualifier;
            auto qualifierString = value->toJsonString()->getString();
            if (qualifierString == "coherent"sv) {
                bibb.qualifier(Qualifier::COHERENT);
            } else if (qualifierString == "writeonly"sv) {
                bibb.qualifier(Qualifier::WRITEONLY);
            } else if (qualifierString == "readonly"sv) {
                bibb.qualifier(Qualifier::READONLY);
            } else if (qualifierString == "volatile"sv) {
                bibb.qualifier(Qualifier::VOLATILE);
            } else if (qualifierString == "restrict"sv) {
                bibb.qualifier(Qualifier::RESTRICT);
            }
            continue;
        }
        std::cerr << "buffers: qualifiers must be an array of STRINGs." << std::endl;
        return false;
    }

    bool ok = true;
    for (auto value : fieldsValue->toJsonArray()->getElements()) {

        if (bibb.hasVariableSizeArray()) {
            std::cerr << "buffers: a variable size array must be the only and last field." << std::endl;
            return false;
        }

        if (value->getType() == JsonishValue::Type::OBJECT) {
            ok &= processBufferField(bibb, *value->toJsonObject());
            continue;
        }
        std::cerr << "buffers: fields must be an array of OBJECTs." << std::endl;
        return false;
    }

    builder.buffer(bibb.build());
    return ok;
}

static bool processBuffers(MaterialBuilder& builder, const JsonishValue& v) {
    auto jsonArray = v.toJsonArray();
    bool ok = true;
    for (auto value : jsonArray->getElements()) {
        if (value->getType() == JsonishValue::Type::OBJECT) {
            ok &= processBuffer(builder, *value->toJsonObject());
            continue;
        }
        std::cerr << "buffers must be an array of OBJECTs." << std::endl;
        return false;
    }
    return ok;
}


static bool processSubpass(MaterialBuilder& builder, const JsonishObject& jsonObject) noexcept {

    const JsonishValue* typeValue = jsonObject.getValue("type");
    if (!typeValue) {
        std::cerr << "subpasses: entry without key 'type'." << std::endl;
        return false;
    }
    if (typeValue->getType() != JsonishValue::STRING) {
        std::cerr << "subpasses: type value must be STRING." << std::endl;
        return false;
    }

    const JsonishValue* nameValue = jsonObject.getValue("name");
    if (!nameValue) {
        std::cerr << "subpasses: entry without 'name' key." << std::endl;
        return false;
    }
    if (nameValue->getType() != JsonishValue::STRING) {
        std::cerr << "subpasses: name value must be STRING." << std::endl;
        return false;
    }

    const JsonishValue* precisionValue = jsonObject.getValue("precision");
    if (precisionValue) {
        if (precisionValue->getType() != JsonishValue::STRING) {
            std::cerr << "subpasses: precision must be a STRING." << std::endl;
            return false;
        }

        auto precisionString = precisionValue->toJsonString();
        if (!Enums::isValid<ParameterPrecision>(precisionString->getString())){
            return logEnumIssue("subpasses", *precisionString, Enums::map<ParameterPrecision>());
        }
    }

    const JsonishValue* formatValue = jsonObject.getValue("format");
    if (formatValue) {
        if (formatValue->getType() != JsonishValue::STRING) {
            std::cerr << "subpasses: format must be a STRING." << std::endl;
            return false;
        }

        auto formatString = formatValue->toJsonString();
        if (!Enums::isValid<SamplerFormat>(formatString->getString())){
            return logEnumIssue("subpasses", *formatString, Enums::map<SamplerFormat>());
        }
    }

    auto typeString = typeValue->toJsonString()->getString();
    auto nameString = nameValue->toJsonString()->getString();

    size_t arraySize = extractArraySize(typeString);

    if (Enums::isValid<SubpassType>(typeString)) {
        if (arraySize > 0) {
            std::cerr << "subpasses: the parameter with name '" << nameString << "'"
                      << " is an array of subpasses of size " << arraySize << ". Arrays of subpasses"
                      << " are currently not supported." << std::endl;
            return false;
        }

        MaterialBuilder::SubpassType type = Enums::toEnum<SubpassType>(typeString);
        if (precisionValue && formatValue) {
            auto format = Enums::toEnum<SamplerFormat>(formatValue->toJsonString()->getString());
            auto precision =
                    Enums::toEnum<ParameterPrecision>(precisionValue->toJsonString()->getString());
            builder.subpass(type, format, precision, nameString.c_str());
        } else if (formatValue) {
            auto format = Enums::toEnum<SamplerFormat>(formatValue->toJsonString()->getString());
            builder.subpass(type, format, nameString.c_str());
        } else if (precisionValue) {
            auto precision =
                    Enums::toEnum<ParameterPrecision>(precisionValue->toJsonString()->getString());
            builder.subpass(type, precision, nameString.c_str());
        } else {
            builder.subpass(type, nameString.c_str());
        }
    } else {
        std::cerr << "subpasses: the type '" << typeString
                  << "' for parameter with name '" << nameString << "' is neither a valid uniform "
                  << "type nor a valid sampler type." << std::endl;
        return false;
    }

    return true;
}

static bool processSubpasses(MaterialBuilder& builder, const JsonishValue& v) {
    auto jsonArray = v.toJsonArray();

    bool ok = true;
    for (auto value : jsonArray->getElements()) {
        if (value->getType() == JsonishValue::Type::OBJECT) {
            ok &= processSubpass(builder, *value->toJsonObject());
            continue;
        }
        std::cerr << "subpasses must be an array of OBJECTs." << std::endl;
        return false;
    }
    return ok;
}

static bool processVariables(MaterialBuilder& builder, const JsonishValue& value) {
    const JsonishArray* jsonArray = value.toJsonArray();
    const auto& elements = jsonArray->getElements();

    if (elements.size() > MaterialBuilder::MATERIAL_VARIABLES_COUNT) {
        std::cerr << "variables: Max array size is " << MaterialBuilder::MATERIAL_VARIABLES_COUNT << "." << std::endl;
        return false;
    }

    for (size_t i = 0; i < elements.size(); i++) {
        ParameterPrecision precision = ParameterPrecision::DEFAULT;
        MaterialBuilder::Variable v = intToVariable(i);
        std::string nameString;

        auto elementValue = elements[i];
        if (elementValue->getType() == JsonishValue::Type::OBJECT) {

            JsonishObject const& jsonObject = *elementValue->toJsonObject();

            const JsonishValue* nameValue = jsonObject.getValue("name");
            if (!nameValue) {
                std::cerr << "variables: entry without 'name' key." << std::endl;
                return false;
            }
            if (nameValue->getType() != JsonishValue::STRING) {
                std::cerr << "variables: name value must be STRING." << std::endl;
                return false;
            }

            const JsonishValue* precisionValue = jsonObject.getValue("precision");
            if (precisionValue) {
                if (precisionValue->getType() != JsonishValue::STRING) {
                    std::cerr << "variables: precision must be a STRING." << std::endl;
                    return false;
                }
                auto precisionString = precisionValue->toJsonString();
                if (!Enums::isValid<ParameterPrecision>(precisionString->getString())){
                    return logEnumIssue("variables", *precisionString, Enums::map<ParameterPrecision>());
                }
            }

            nameString = nameValue->toJsonString()->getString();
            if (precisionValue) {
                precision = Enums::toEnum<ParameterPrecision>(
                        precisionValue->toJsonString()->getString());
            }
            builder.variable(v, nameString.c_str(), precision);
        } else if (elementValue->getType() == JsonishValue::Type::STRING) {
            nameString = elementValue->toJsonString()->getString();
            builder.variable(v, nameString.c_str());
        } else {
            std::cerr << "variables: array index " << i << " is not a STRING. found:" <<
                    JsonishValue::typeToString(elementValue->getType()) << std::endl;
            return false;
        }
    }

    return true;
}

static bool processRequires(MaterialBuilder& builder, const JsonishValue& value) {
    using Attribute = filament::VertexAttribute;
    static const std::unordered_map<std::string, Attribute> strToEnum {
        { "color", Attribute::COLOR },
        { "position", Attribute::POSITION },
        { "tangents", Attribute::TANGENTS },
        { "uv0", Attribute::UV0 },
        { "uv1", Attribute::UV1 },
        { "custom0", Attribute::CUSTOM0 },
        { "custom1", Attribute(Attribute::CUSTOM0 + 1) },
        { "custom2", Attribute(Attribute::CUSTOM0 + 2) },
        { "custom3", Attribute(Attribute::CUSTOM0 + 3) },
        { "custom4", Attribute(Attribute::CUSTOM0 + 4) },
        { "custom5", Attribute(Attribute::CUSTOM0 + 5) },
        { "custom6", Attribute(Attribute::CUSTOM0 + 6) },
        { "custom7", Attribute(Attribute::CUSTOM0 + 7) },
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
        { "custom", MaterialBuilder::BlendingMode::CUSTOM },
    };
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(strToEnum, jsonString->getString())) {
        return logEnumIssue("blending", *jsonString, strToEnum);
    }

    builder.blending(stringToEnum(strToEnum, jsonString->getString()));
    return true;
}

static bool processBlendFunction(MaterialBuilder& builder, const JsonishValue& value) {
    static const std::unordered_map<std::string, MaterialBuilder::BlendFunction> strToEnum{
            { "zero",             MaterialBuilder::BlendFunction::ZERO },
            { "one",              MaterialBuilder::BlendFunction::ONE },
            { "srcColor",         MaterialBuilder::BlendFunction::SRC_COLOR },
            { "oneMinusSrcColor", MaterialBuilder::BlendFunction::ONE_MINUS_SRC_COLOR },
            { "dstColor",         MaterialBuilder::BlendFunction::DST_COLOR },
            { "oneMinusDstColor", MaterialBuilder::BlendFunction::ONE_MINUS_DST_COLOR },
            { "srcAlpha",         MaterialBuilder::BlendFunction::SRC_ALPHA },
            { "oneMinusSrcAlpha", MaterialBuilder::BlendFunction::ONE_MINUS_SRC_ALPHA },
            { "dstAlpha",         MaterialBuilder::BlendFunction::DST_ALPHA },
            { "oneMinusDstAlpha", MaterialBuilder::BlendFunction::ONE_MINUS_DST_ALPHA },
            { "srcAlphaSaturate", MaterialBuilder::BlendFunction::SRC_ALPHA_SATURATE },
    };

    if (value.getType() != JsonishValue::Type::OBJECT) {
        std::cerr << "blendFunction must be an OBJECT." << std::endl;
    }

    JsonishObject const* const jsonObject = value.toJsonObject();

    MaterialBuilder::BlendFunction srcRGB, srcA, dstRGB, dstA;
    std::pair<const char*, MaterialBuilder::BlendFunction*> functions[] = {
            { "srcRGB", &srcRGB },
            { "srcA",   &srcA },
            { "dstRGB", &dstRGB },
            { "dstA",   &dstA },
    };

    for (auto&& entry : functions) {
        const char* key = entry.first;
        const JsonishValue* v = jsonObject->getValue(key);
        if (!v) {
            std::cerr << "blendFunction: entry without '" << key << "' key." << std::endl;
            return false;
        }
        if (v->getType() != JsonishValue::STRING) {
            std::cerr << "blendFunction: '" << key << "' value must be STRING." << std::endl;
            return false;
        }
        *entry.second = stringToEnum(strToEnum, v->toJsonString()->getString());
    }
    builder.customBlendFunctions(srcRGB, srcA, dstRGB, dstA);
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

static bool processQuality(MaterialBuilder& builder, const JsonishValue& value) {
    static const std::unordered_map<std::string, MaterialBuilder::ShaderQuality> strToEnum {
        { "low", MaterialBuilder::ShaderQuality::LOW },
        { "normal", MaterialBuilder::ShaderQuality::NORMAL },
        { "high", MaterialBuilder::ShaderQuality::HIGH },
        { "default", MaterialBuilder::ShaderQuality::DEFAULT },
    };
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(strToEnum, jsonString->getString())) {
        return logEnumIssue("quality", *jsonString, strToEnum);
    }

    builder.quality(stringToEnum(strToEnum, jsonString->getString()));
    return true;
}

static bool processFeatureLevel(MaterialBuilder& builder, const JsonishValue& value) {
    using filament::backend::FeatureLevel;
    JsonishNumber const* const number = value.toJsonNumber();
    FeatureLevel featureLevel;
    if (number->getFloat() == 0.0f) {
        featureLevel = FeatureLevel::FEATURE_LEVEL_0;
    } else if (number->getFloat() == 1.0f) {
        featureLevel = FeatureLevel::FEATURE_LEVEL_1;
    } else if (number->getFloat() == 2.0f) {
        featureLevel = FeatureLevel::FEATURE_LEVEL_2;
    } else if (number->getFloat() == 3.0f) {
        featureLevel = FeatureLevel::FEATURE_LEVEL_3;
    } else {
        std::cerr << "featureLevel: invalid value " << number->getFloat() << std::endl;
        return false;
    }
    builder.featureLevel(featureLevel);
    return true;
}

static bool processGroupSizes(MaterialBuilder& builder, const JsonishValue& v) {
    auto jsonArray = v.toJsonArray();

    filament::math::uint3 groupSize{ 1, 1, 1 };
    size_t index = 0;

    for (auto value : jsonArray->getElements()) {
        if (index >= 3) {
            std::cerr << "groupSize: must be an array no larger than 3" << std::endl;
            return false;
        }
        if (value->getType() == JsonishValue::Type::NUMBER) {
            JsonishNumber const* const number = value->toJsonNumber();
            float aFloat = number->getFloat();
            if (aFloat > 0 && floor(aFloat) == aFloat) {
                groupSize[index] = uint32_t(floor(aFloat));
            } else {
                std::cerr << "groupSize: invalid value " << aFloat << std::endl;
                return false;
            }
            index++;
            continue;
        }
        std::cerr << "groupSize must be an array of NUMBERs." << std::endl;
        return false;
    }
    builder.groupSize(groupSize);
    return true;
}

static bool processStereoscopicType(MaterialBuilder& builder, const JsonishValue& value) {
    static const std::unordered_map<std::string, MaterialBuilder::StereoscopicType> strToEnum{
            { "instanced", MaterialBuilder::StereoscopicType::INSTANCED },
            { "multiview",  MaterialBuilder::StereoscopicType::MULTIVIEW },
    };
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(strToEnum, jsonString->getString())) {
        return logEnumIssue("stereoscopicType", *jsonString, strToEnum);
    }

    builder.stereoscopicType(stringToEnum(strToEnum, jsonString->getString()));
    return true;
}

static bool processOutput(MaterialBuilder& builder, const JsonishObject& jsonObject) noexcept {

    const JsonishValue* nameValue = jsonObject.getValue("name");
    if (!nameValue) {
        std::cerr << "outputs: entry without 'name' key." << std::endl;
        return false;
    }
    if (nameValue->getType() != JsonishValue::STRING) {
        std::cerr << "outputs: name value must be STRING." << std::endl;
        return false;
    }

    const JsonishValue* targetValue = jsonObject.getValue("target");
    if (targetValue) {
        if (targetValue->getType() != JsonishValue::STRING) {
            std::cerr << "outputs: target must be a STRING." << std::endl;
            return false;
        }

        auto targetString = targetValue->toJsonString();
        if (!Enums::isValid<OutputTarget>(targetString->getString())) {
            return logEnumIssue("target", *targetString, Enums::map<OutputTarget>());
        }
    }

    const JsonishValue* precisionValue = jsonObject.getValue("precision");
    if (precisionValue) {
        if (precisionValue->getType() != JsonishValue::STRING) {
            std::cerr << "parameters: precision must be a STRING." << std::endl;
            return false;
        }

        auto precisionString = precisionValue->toJsonString();
        if (!Enums::isValid<ParameterPrecision>(precisionString->getString())){
            return logEnumIssue("parameters", *precisionString, Enums::map<ParameterPrecision>());
        }
    }

    const JsonishValue* typeValue = jsonObject.getValue("type");
    if (typeValue) {
        if (typeValue->getType() != JsonishValue::STRING) {
            std::cerr << "outputs: type must be a STRING." << std::endl;
            return false;
        }

        auto typeString = typeValue->toJsonString();
        if (!Enums::isValid<OutputType>(typeString->getString())) {
            return logEnumIssue("type", *typeString, Enums::map<OutputType>());
        }
    }

    const JsonishValue* qualifierValue = jsonObject.getValue("qualifier");
    if (qualifierValue) {
        if (qualifierValue->getType() != JsonishValue::STRING) {
            std::cerr << "outputs: qualifier must be a STRING." << std::endl;
            return false;
        }

        auto qualifierString = qualifierValue->toJsonString();
        if (!Enums::isValid<OutputQualifier>(qualifierString->getString())) {
            return logEnumIssue("qualifier", *qualifierString, Enums::map<OutputQualifier>());
        }
    }

    const JsonishValue* locationValue = jsonObject.getValue("location");
    if (locationValue) {
        if (locationValue->getType() != JsonishValue::NUMBER) {
            std::cerr << "outputs: location must be a NUMBER." << std::endl;
            return false;
        }
    }

    const char* name = nameValue->toJsonString()->getString().c_str();

    OutputTarget target = OutputTarget::COLOR;
    if (targetValue) {
        target = Enums::toEnum<OutputTarget>(targetValue->toJsonString()->getString());
    }

    ParameterPrecision precision = ParameterPrecision::DEFAULT;
    if (precisionValue) {
        precision = Enums::toEnum<ParameterPrecision>(precisionValue->toJsonString()->getString());
    }

    OutputType type = OutputType::FLOAT4;
    if (target == OutputTarget::DEPTH) {
        // The default type for depth targets is float.
        type = OutputType::FLOAT;
    }
    if (typeValue) {
        type = Enums::toEnum<OutputType>(typeValue->toJsonString()->getString());
    }

    OutputQualifier qualifier = OutputQualifier::OUT;
    if (qualifierValue) {
        qualifier = Enums::toEnum<OutputQualifier>(qualifierValue->toJsonString()->getString());
    }

    int location = -1;
    if (locationValue) {
        location = static_cast<int>(locationValue->toJsonNumber()->getFloat());
    }

    builder.output(qualifier, target, precision, type, name, location);

    return true;
}

static bool processOutputs(MaterialBuilder& builder, const JsonishValue& v) {
    auto jsonArray = v.toJsonArray();

    bool ok = true;
    for (auto value : jsonArray->getElements()) {
        if (value->getType() == JsonishValue::Type::OBJECT) {
            ok &= processOutput(builder, *value->toJsonObject());
            continue;
        }
        std::cerr << "outputs must be an array of OBJECTs." << std::endl;
        return false;
    }
    return ok;
}

static bool processColorWrite(MaterialBuilder& builder, const JsonishValue& value) {
    builder.colorWrite(value.toJsonBool()->getBool());
    return true;
}

static bool processDepthWrite(MaterialBuilder& builder, const JsonishValue& value) {
    builder.depthWrite(value.toJsonBool()->getBool());
    return true;
}

static bool processInstanced(MaterialBuilder& builder, const JsonishValue& value) {
    builder.instanced(value.toJsonBool()->getBool());
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

static bool processAlphaToCoverage(MaterialBuilder& builder, const JsonishValue& value) {
    builder.alphaToCoverage(value.toJsonBool()->getBool());
    return true;
}

static bool processShadowMultiplier(MaterialBuilder& builder, const JsonishValue& value) {
    builder.shadowMultiplier(value.toJsonBool()->getBool());
    return true;
}

static bool processTransparentShadow(MaterialBuilder& builder, const JsonishValue& value) {
    builder.transparentShadow(value.toJsonBool()->getBool());
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

static bool processFramebufferFetch(MaterialBuilder& builder, const JsonishValue& value) {
    if (value.toJsonBool()->getBool()) {
        builder.enableFramebufferFetch();
    }
    return true;
}

static bool processVertexDomainDeviceJittered(MaterialBuilder& builder, const JsonishValue& value) {
    builder.vertexDomainDeviceJittered(value.toJsonBool()->getBool());
    return true;
}

static bool processLegacyMorphing(MaterialBuilder& builder, const JsonishValue& value) {
    if (value.toJsonBool()->getBool()) {
        builder.useLegacyMorphing();
    }
    return true;
}

static bool processCustomSurfaceShading(MaterialBuilder& builder, const JsonishValue& value) {
    builder.customSurfaceShading(value.toJsonBool()->getBool());
    return true;
}

static bool processSpecularAmbientOcclusion(MaterialBuilder& builder, const JsonishValue& value) {
    static const std::unordered_map<std::string, MaterialBuilder::SpecularAmbientOcclusion> strToEnum {
            { "none",        MaterialBuilder::SpecularAmbientOcclusion::NONE },
            { "simple",      MaterialBuilder::SpecularAmbientOcclusion::SIMPLE },
            { "bentNormals", MaterialBuilder::SpecularAmbientOcclusion::BENT_NORMALS },
            // Backward compatibility
            { "false",       MaterialBuilder::SpecularAmbientOcclusion::NONE },
            { "true",        MaterialBuilder::SpecularAmbientOcclusion::SIMPLE },
    };
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(strToEnum, jsonString->getString())) {
        return logEnumIssue("specularAO", *jsonString, strToEnum);
    }

    builder.specularAmbientOcclusion(stringToEnum(strToEnum, jsonString->getString()));
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

static bool processDomain(MaterialBuilder& builder, const JsonishValue& value) {
    static const std::unordered_map<std::string, MaterialBuilder::MaterialDomain> strToEnum {
        { "surface",            MaterialBuilder::MaterialDomain::SURFACE },
        { "postprocess",        MaterialBuilder::MaterialDomain::POST_PROCESS },
        { "compute",            MaterialBuilder::MaterialDomain::COMPUTE },
    };
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(strToEnum, jsonString->getString())) {
        return logEnumIssue("domain", *jsonString, strToEnum);
    }

    builder.materialDomain(stringToEnum(strToEnum, jsonString->getString()));
    return true;
}

static bool processRefractionMode(MaterialBuilder& builder, const JsonishValue& value) {
    static const std::unordered_map<std::string, MaterialBuilder::RefractionMode> strToEnum{
            { "none",        MaterialBuilder::RefractionMode::NONE },
            { "cubemap",     MaterialBuilder::RefractionMode::CUBEMAP },
            { "screenspace", MaterialBuilder::RefractionMode::SCREEN_SPACE },
    };
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(strToEnum, jsonString->getString())) {
        return logEnumIssue("refraction", *jsonString, strToEnum);
    }

    builder.refractionMode(stringToEnum(strToEnum, jsonString->getString()));
    return true;
}

static bool processReflectionMode(MaterialBuilder& builder, const JsonishValue& value) {
    static const std::unordered_map<std::string, MaterialBuilder::ReflectionMode> strToEnum {
            { "default", MaterialBuilder::ReflectionMode ::DEFAULT },
            { "screenspace", MaterialBuilder::ReflectionMode::SCREEN_SPACE },
    };
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(strToEnum, jsonString->getString())) {
        return logEnumIssue("reflection_mode", *jsonString, strToEnum);
    }

    builder.reflectionMode(stringToEnum(strToEnum, jsonString->getString()));
    return true;
}

static bool processRefractionType(MaterialBuilder& builder, const JsonishValue& value) {
    static const std::unordered_map<std::string, MaterialBuilder::RefractionType> strToEnum {
            { "solid", MaterialBuilder::RefractionType::SOLID },
            { "thin",  MaterialBuilder::RefractionType::THIN },
    };
    auto jsonString = value.toJsonString();
    if (!isStringValidEnum(strToEnum, jsonString->getString())) {
        return logEnumIssue("refractionType", *jsonString, strToEnum);
    }

    builder.refractionType(stringToEnum(strToEnum, jsonString->getString()));
    return true;
}

static bool processVariantFilter(MaterialBuilder& builder, const JsonishValue& value) {
    // We avoid using an initializer list for this particular map to avoid build errors that are
    // due to static initialization ordering.
    using filament::Variant;
    static const std::unordered_map<std::string, filament::UserVariantFilterBit> strToEnum  = [] {
        std::unordered_map<std::string, filament::UserVariantFilterBit> strToEnum;
        strToEnum["directionalLighting"]    = filament::UserVariantFilterBit::DIRECTIONAL_LIGHTING;
        strToEnum["dynamicLighting"]        = filament::UserVariantFilterBit::DYNAMIC_LIGHTING;
        strToEnum["shadowReceiver"]         = filament::UserVariantFilterBit::SHADOW_RECEIVER;
        strToEnum["skinning"]               = filament::UserVariantFilterBit::SKINNING;
        strToEnum["vsm"]                    = filament::UserVariantFilterBit::VSM;
        strToEnum["fog"]                    = filament::UserVariantFilterBit::FOG;
        strToEnum["ssr"]                    = filament::UserVariantFilterBit::SSR;
        strToEnum["stereo"]                 = filament::UserVariantFilterBit::STE;
        return strToEnum;
    }();

    filament::UserVariantFilterMask variantFilter = {};
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

        variantFilter |= (uint32_t)strToEnum.at(s);
    }

    builder.variantFilter(variantFilter);
    return true;
}

ParametersProcessor::ParametersProcessor() {
    using Type = JsonishValue::Type;
    mParameters["name"]                          = { &processName, Type::STRING };
    mParameters["interpolation"]                 = { &processInterpolation, Type::STRING };
    mParameters["parameters"]                    = { &processParameters, Type::ARRAY };
    mParameters["constants"]                     = { &processConstants, Type::ARRAY };
    mParameters["buffers"]                       = { &processBuffers, Type::ARRAY };
    mParameters["subpasses"]                     = { &processSubpasses, Type::ARRAY };
    mParameters["variables"]                     = { &processVariables, Type::ARRAY };
    mParameters["requires"]                      = { &processRequires, Type::ARRAY };
    mParameters["blending"]                      = { &processBlending, Type::STRING };
    mParameters["blendFunction"]                 = { &processBlendFunction, Type::OBJECT };
    mParameters["postLightingBlending"]          = { &processPostLightingBlending, Type::STRING };
    mParameters["vertexDomain"]                  = { &processVertexDomain, Type::STRING };
    mParameters["culling"]                       = { &processCulling, Type::STRING };
    mParameters["colorWrite"]                    = { &processColorWrite, Type::BOOL };
    mParameters["depthWrite"]                    = { &processDepthWrite, Type::BOOL };
    mParameters["instanced"]                     = { &processInstanced, Type::BOOL };
    mParameters["depthCulling"]                  = { &processDepthCull, Type::BOOL };
    mParameters["doubleSided"]                   = { &processDoubleSided, Type::BOOL };
    mParameters["transparency"]                  = { &processTransparencyMode, Type::STRING };
    mParameters["reflections"]                   = { &processReflectionMode, Type::STRING };
    mParameters["maskThreshold"]                 = { &processMaskThreshold, Type::NUMBER };
    mParameters["alphaToCoverage"]               = { &processAlphaToCoverage, Type::BOOL };
    mParameters["shadowMultiplier"]              = { &processShadowMultiplier, Type::BOOL };
    mParameters["transparentShadow"]             = { &processTransparentShadow, Type::BOOL };
    mParameters["shadingModel"]                  = { &processShading, Type::STRING };
    mParameters["variantFilter"]                 = { &processVariantFilter, Type::ARRAY };
    mParameters["specularAntiAliasing"]          = { &processSpecularAntiAliasing, Type::BOOL };
    mParameters["specularAntiAliasingVariance"]  = { &processSpecularAntiAliasingVariance, Type::NUMBER };
    mParameters["specularAntiAliasingThreshold"] = { &processSpecularAntiAliasingThreshold, Type::NUMBER };
    mParameters["clearCoatIorChange"]            = { &processClearCoatIorChange, Type::BOOL };
    mParameters["flipUV"]                        = { &processFlipUV, Type::BOOL };
    mParameters["multiBounceAmbientOcclusion"]   = { &processMultiBounceAO, Type::BOOL };
    mParameters["specularAmbientOcclusion"]      = { &processSpecularAmbientOcclusion, Type::STRING };
    mParameters["domain"]                        = { &processDomain, Type::STRING };
    mParameters["refractionMode"]                = { &processRefractionMode, Type::STRING };
    mParameters["refractionType"]                = { &processRefractionType, Type::STRING };
    mParameters["framebufferFetch"]              = { &processFramebufferFetch, Type::BOOL };
    mParameters["vertexDomainDeviceJittered"]    = { &processVertexDomainDeviceJittered, Type::BOOL };
    mParameters["legacyMorphing"]                = { &processLegacyMorphing, Type::BOOL };
    mParameters["outputs"]                       = { &processOutputs, Type::ARRAY };
    mParameters["quality"]                       = { &processQuality, Type::STRING };
    mParameters["customSurfaceShading"]          = { &processCustomSurfaceShading, Type::BOOL };
    mParameters["featureLevel"]                  = { &processFeatureLevel, Type::NUMBER };
    mParameters["groupSize"]                     = { &processGroupSizes, Type::ARRAY };
    mParameters["stereoscopicType"]              = { &processStereoscopicType, Type::STRING };
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
            std::cerr << "Error while processing material json, key:\"" << key << "\"" << std::endl;
            return false;
        }
    }
    return true;
}

bool ParametersProcessor::process(filamat::MaterialBuilder& builder, const std::string& key, const std::string& value) {
    if (mParameters.find(key) == mParameters.end()) {
        std::cerr << "Ignoring config entry (unknown key): \"" << key << "\"" << std::endl;
        return false;
    }

    std::unique_ptr<JsonishValue> var;
    switch (mParameters.at(key).rootAssert) {
        case JsonishValue::Type::BOOL: {
            std::string lower;
            std::transform(value.begin(), value.end(), std::back_inserter(lower), ::tolower);
            if (lower.empty() || lower == "false" || lower == "f" || lower == "0") {
                var = std::make_unique<JsonishBool>(false);
            }
            else {
                var = std::make_unique<JsonishBool>(true);
            }
            break;
        }
        case JsonishValue::Type::NUMBER:
            var = std::make_unique<JsonishNumber>(std::stof(value));
            break;
        case JsonishValue::Type::STRING:
            var = std::make_unique<JsonishString>(value);
            break;
        default:
            std::cerr << "Unsupported type: \""
                      << JsonishValue::typeToString(mParameters.at(key).rootAssert)
                      << "\"" << std::endl;
            return false;
    }

    auto fPointer = mParameters[key].callback;
    bool ok = fPointer(builder, *var);
    if (!ok) {
        std::cerr << "Error while processing material param, key:\"" << key << "\"" << std::endl;
        return false;
    }
    return true;
}

} // namespace matp
