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

#ifndef TNT_UTILS_TRIBOOL_H
#define TNT_UTILS_TRIBOOL_H

#include <stdint.h>

namespace utils {

/**
 * A 3-state boolean type representing true, false, or indeterminate.
 *
 * This class provides standard logical operators (!, &&, ||) and observer methods
 * that correctly implement Kleene three-valued logic.
 */
class tribool {
public:
    enum class Value : uint8_t {
        kFalse = 0,
        kTrue = 1,
        kIndeterminate = 2
    };

    using value_t = Value;

    static constexpr Value kFalse = Value::kFalse;
    static constexpr Value kTrue = Value::kTrue;
    static constexpr Value kIndeterminate = Value::kIndeterminate;

    constexpr tribool() noexcept : mValue(Value::kIndeterminate) {}
    constexpr tribool(bool const v) noexcept : mValue(v ? Value::kTrue : Value::kFalse) {}
    constexpr tribool(Value const v) noexcept : mValue(v) {}

    constexpr bool is_true() const noexcept { return mValue == Value::kTrue; }
    constexpr bool is_false() const noexcept { return mValue == Value::kFalse; }
    constexpr bool is_indeterminate() const noexcept { return mValue == Value::kIndeterminate; }

    constexpr tribool operator!() const noexcept {
        return mValue == Value::kIndeterminate
                       ? Value::kIndeterminate
                       : (mValue == Value::kTrue ? Value::kFalse : Value::kTrue);
    }

    constexpr tribool operator&&(tribool const rhs) const noexcept {
        if (mValue == Value::kFalse || rhs.mValue == Value::kFalse) return Value::kFalse;
        if (mValue == Value::kTrue) return rhs;
        return Value::kIndeterminate;
    }

    constexpr tribool operator||(tribool const rhs) const noexcept {
        if (mValue == Value::kTrue || rhs.mValue == Value::kTrue) return Value::kTrue;
        if (mValue == Value::kFalse) return rhs;
        return Value::kIndeterminate;
    }

    constexpr bool operator==(tribool const rhs) const noexcept { return mValue == rhs.mValue; }
    constexpr bool operator!=(tribool const rhs) const noexcept { return mValue != rhs.mValue; }

private:
    Value mValue;
};

} // namespace utils

#endif // TNT_UTILS_TRIBOOL_H
