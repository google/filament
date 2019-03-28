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

#ifndef TNT_PARAMETERSPROCESSOR_H
#define TNT_PARAMETERSPROCESSOR_H

#include <unordered_map>
#include <string>
#include <iostream>

#include "JsonishLexeme.h"
#include "JsonishParser.h"

#include <filamat/MaterialBuilder.h>

namespace matc {

class ParametersProcessor {

public:
    ParametersProcessor();
    ~ParametersProcessor() = default;
    bool process(filamat::MaterialBuilder& builder, const JsonishObject& jsonObject);

private:
    bool processName(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processInterpolation(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processParameters(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processVariables(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processRequires(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processBlending(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processPostLightingBlending(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processVertexDomain(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processCulling(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processColorWrite(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processDepthWrite(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processDepthCull(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processDoubleSided(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processTransparencyMode(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processMaskThreshold(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processShadowMultiplier(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processCurvatureToRoughness(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processLimitOverInterpolation(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processClearCoatIorChange(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processFlipUV(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processShading(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processVariantFilter(filamat::MaterialBuilder &builder, const JsonishValue &value);
    bool processParameter(filamat::MaterialBuilder& builder, const JsonishObject& value) const
    noexcept;

    template <class T>
    bool logEnumIssue(const std::string& key, const JsonishString& value,
            const std::unordered_map<std::string, T>& map) const noexcept {
        std::cerr << "Error while processing key '" << key << "' value." << std::endl;
        std::cerr << "Value '" << value.getString() << "' is invalid. Valid values are:"
                << std::endl;
        for (auto entries : map) {
            std::cerr << "    " << entries.first  << std::endl;
        }
        return false;
    }

    template <class T>
    bool isStringValidEnum(const std::unordered_map<std::string, T>& map,
            const std::string& s) const noexcept {
        return map.find(s) != map.end();
    }

    template <class T>
    T stringToEnum(const std::unordered_map<std::string, T>& map,
            const std::string& s) const noexcept {
        return map.at(s);
    }

    filamat::MaterialBuilder::Variable intToVariable(size_t i) const noexcept;

    using MaterialConfigProcessor =
            bool (ParametersProcessor::*)(filamat::MaterialBuilder& builder, const JsonishValue& value);
    std::unordered_map<std::string, MaterialConfigProcessor> mConfigProcessor;

    std::unordered_map<std::string, JsonishValue::Type> mRootAsserts;
    std::unordered_map<std::string, filamat::MaterialBuilder::Interpolation>
            mStringToInterpolation;
    std::unordered_map<std::string, filamat::MaterialBuilder::CullingMode> mStringToCullingMode;
    std::unordered_map<std::string, filamat::MaterialBuilder::TransparencyMode>
            mStringToTransparencyMode;
    std::unordered_map<std::string, filamat::MaterialBuilder::VertexDomain>
            mStringToVertexDomain;
    std::unordered_map<std::string, filamat::MaterialBuilder::BlendingMode>
            mStringToBlendingMode;
    std::unordered_map<std::string, filament::VertexAttribute> mStringToAttributeIndex;
    std::unordered_map<std::string, filamat::MaterialBuilder::Shading> mStringToShading;
    std::unordered_map<std::string, uint8_t> mStringToVariant;
};

} // namespace matc

#endif //TNT_PARAMETERSPROCESSOR_H
