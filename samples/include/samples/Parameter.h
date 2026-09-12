/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_SAMPLES_PARAMETER_H
#define TNT_SAMPLES_PARAMETER_H

#include <utils/CString.h>
#include <utils/FixedCapacityVector.h>

#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>
#include <utility>
#include <variant>

namespace samples {

enum class ParameterType : uint8_t {
    BOOL,
    INT,
    FLOAT,
    STRING,
    ENUM
};

struct IntRange {
    int min = std::numeric_limits<int>::min();
    int max = std::numeric_limits<int>::max();
    int step = 1;
};

struct FloatRange {
    float min = -std::numeric_limits<float>::infinity();
    float max = std::numeric_limits<float>::infinity();
    float step = 0.01f;
};

using ParameterValue = std::variant<bool, int, float, utils::CString>;

/**
 * Represents a typed command-line configuration parameter for a sample application.
 *
 * Defines parameter metadata, type information, default values, optional numerical
 * ranges or discrete enum choices, and command-line flags (both long name and optional
 * single-character shorthand). Used to declaratively configure, validate, parse, and
 * document sample-specific and common options across Filament samples.
 */
struct Parameter {
    utils::CString name;
    char shorthand = '\0';
    utils::CString description;
    ParameterType type = ParameterType::BOOL;

    ParameterValue value = false;
    ParameterValue defaultValue = false;

    std::optional<IntRange> intRange;
    std::optional<FloatRange> floatRange;
    utils::FixedCapacityVector<utils::CString> choices;
    bool required = false;

    static bool isValidName(std::string_view name) {
        if (name.empty() || name.front() < 'a' || name.front() > 'z' || name.back() == '-') {
            return false;
        }
        bool prevHyphen = false;
        for (char c : name) {
            if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
                prevHyphen = false;
            } else if (c == '-') {
                if (prevHyphen) return false;
                prevHyphen = true;
            } else {
                return false;
            }
        }
        return true;
    }

    bool isValidValue(const ParameterValue& val) const {
        switch (type) {
            case ParameterType::BOOL:
                return std::holds_alternative<bool>(val);
            case ParameterType::INT: {
                if (!std::holds_alternative<int>(val)) return false;
                int v = std::get<int>(val);
                return !intRange || (v >= intRange->min && v <= intRange->max);
            }
            case ParameterType::FLOAT: {
                if (!std::holds_alternative<float>(val)) return false;
                float v = std::get<float>(val);
                return !floatRange || (v >= floatRange->min && v <= floatRange->max);
            }
            case ParameterType::STRING:
                return std::holds_alternative<utils::CString>(val);
            case ParameterType::ENUM: {
                if (!std::holds_alternative<utils::CString>(val)) return false;
                const auto& v = std::get<utils::CString>(val);
                for (const auto& choice : choices) {
                    if (choice == v) return true;
                }
                return false;
            }
        }
        return false;
    }

    static Parameter makeBool(utils::CString name, char shorthand, utils::CString description,
            bool defaultValue = false, bool required = false) {
        return Parameter{
            .name = std::move(name),
            .shorthand = shorthand,
            .description = std::move(description),
            .type = ParameterType::BOOL,
            .value = defaultValue,
            .defaultValue = defaultValue,
            .intRange = std::nullopt,
            .floatRange = std::nullopt,
            .choices = {},
            .required = required
        };
    }

    static Parameter makeInt(utils::CString name, char shorthand, utils::CString description,
            int defaultValue, std::optional<int> min = std::nullopt,
            std::optional<int> max = std::nullopt, int step = 1, bool required = false) {
        std::optional<IntRange> range;
        if (min || max) {
            range = IntRange{
                min.value_or(std::numeric_limits<int>::min()),
                max.value_or(std::numeric_limits<int>::max()),
                step
            };
        }
        return Parameter{
            .name = std::move(name),
            .shorthand = shorthand,
            .description = std::move(description),
            .type = ParameterType::INT,
            .value = defaultValue,
            .defaultValue = defaultValue,
            .intRange = range,
            .floatRange = std::nullopt,
            .choices = {},
            .required = required
        };
    }

    static Parameter makeFloat(utils::CString name, char shorthand, utils::CString description,
            float defaultValue, std::optional<float> min = std::nullopt,
            std::optional<float> max = std::nullopt, float step = 0.01f, bool required = false) {
        std::optional<FloatRange> range;
        if (min || max) {
            range = FloatRange{
                min.value_or(-std::numeric_limits<float>::infinity()),
                max.value_or(std::numeric_limits<float>::infinity()),
                step
            };
        }
        return Parameter{
            .name = std::move(name),
            .shorthand = shorthand,
            .description = std::move(description),
            .type = ParameterType::FLOAT,
            .value = defaultValue,
            .defaultValue = defaultValue,
            .intRange = std::nullopt,
            .floatRange = range,
            .choices = {},
            .required = required
        };
    }

    static Parameter makeString(utils::CString name, char shorthand, utils::CString description,
            utils::CString defaultValue = "", bool required = false) {
        return Parameter{
            .name = std::move(name),
            .shorthand = shorthand,
            .description = std::move(description),
            .type = ParameterType::STRING,
            .value = defaultValue,
            .defaultValue = defaultValue,
            .intRange = std::nullopt,
            .floatRange = std::nullopt,
            .choices = {},
            .required = required
        };
    }

    static Parameter makeEnum(utils::CString name, char shorthand, utils::CString description,
            utils::CString defaultValue, utils::FixedCapacityVector<utils::CString> choices,
            bool required = false) {
        return Parameter{
            .name = std::move(name),
            .shorthand = shorthand,
            .description = std::move(description),
            .type = ParameterType::ENUM,
            .value = defaultValue,
            .defaultValue = defaultValue,
            .intRange = std::nullopt,
            .floatRange = std::nullopt,
            .choices = std::move(choices),
            .required = required
        };
    }
};

using SampleParameters = utils::FixedCapacityVector<Parameter>;

} // namespace samples

#endif // TNT_SAMPLES_PARAMETER_H
