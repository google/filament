/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_ENUMMANAGER_H
#define TNT_ENUMMANAGER_H

#include <algorithm>
#include <string>
#include <unordered_map>

#include <filamat/MaterialBuilder.h>

namespace filamat {

using Property = MaterialBuilder::Property;
using UniformType = MaterialBuilder::UniformType;
using SamplerType = MaterialBuilder::SamplerType;
using SubpassType = MaterialBuilder::SubpassType;
using SamplerFormat = MaterialBuilder::SamplerFormat;
using ParameterPrecision = MaterialBuilder::ParameterPrecision;
using OutputTarget = MaterialBuilder::OutputTarget;
using OutputQualifier = MaterialBuilder::VariableQualifier;
using OutputType = MaterialBuilder::OutputType;
using ConstantType = MaterialBuilder::ConstantType;

// Convenience methods to convert std::string to Enum and also iterate over Enum values.
class Enums {
public:

    // Returns true if string "s" is a valid string representation of an element of enum T.
    template<typename T>
    static bool isValid(const std::string& s) noexcept {
        std::unordered_map<std::string, T>& map = getMap<T>();
        return map.find(s) != map.end();
    }

    // Return enum matching its string representation. Returns undefined if s is not a valid enum T
    // value. You should always call isValid() first to validate a string before calling toEnum().
    template<typename T>
    static T toEnum(const std::string& s) noexcept {
        std::unordered_map<std::string, T>& map = getMap<T>();
        return map.at(s);
    }

    template<typename T>
    static std::string toString(T t) noexcept;

    // Return a map of all values in an enum with their string representation.
    template<typename T>
    static std::unordered_map<std::string, T>& map() noexcept {
        std::unordered_map<std::string, T>& map = getMap<T>();
        return map;
    };

private:
    template<typename T>
    static std::unordered_map<std::string, T>& getMap() noexcept;

    static std::unordered_map<std::string, Property> mStringToProperty;
    static std::unordered_map<std::string, UniformType> mStringToUniformType;
    static std::unordered_map<std::string, SamplerType> mStringToSamplerType;
    static std::unordered_map<std::string, SubpassType> mStringToSubpassType;
    static std::unordered_map<std::string, SamplerFormat> mStringToSamplerFormat;
    static std::unordered_map<std::string, ParameterPrecision> mStringToSamplerPrecision;
    static std::unordered_map<std::string, OutputTarget> mStringToOutputTarget;
    static std::unordered_map<std::string, OutputQualifier> mStringToOutputQualifier;
    static std::unordered_map<std::string, OutputType> mStringToOutputType;
    static std::unordered_map<std::string, ConstantType> mStringToConstantType;
};

template<typename T>
std::string Enums::toString(T t) noexcept {
    std::unordered_map<std::string, T>& map = getMap<T>();
    auto result = std::find_if(map.begin(), map.end(), [t](auto& pair) {
        return pair.second == t;
    });
    if (result != map.end()) {
        return result->first;
    }
    return "";
}

} // namespace filamat

#endif //TNT_ENUMMANAGER_H
