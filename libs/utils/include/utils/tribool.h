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

class tribool {
public:
    enum value_t : uint8_t {
        False = 0,
        True = 1,
        Indeterminate = 2
    };

    constexpr tribool() noexcept : mValue(Indeterminate) {}
    constexpr tribool(bool v) noexcept : mValue(v ? True : False) {}
    constexpr tribool(value_t v) noexcept : mValue(v) {}

    constexpr bool is_true() const noexcept { return mValue == True; }
    constexpr bool is_false() const noexcept { return mValue == False; }
    constexpr bool is_indeterminate() const noexcept { return mValue == Indeterminate; }

    constexpr tribool operator!() const noexcept {
        return mValue == Indeterminate ? Indeterminate : (mValue == True ? False : True);
    }

    constexpr tribool operator&&(tribool rhs) const noexcept {
        if (mValue == False || rhs.mValue == False) return False;
        if (mValue == True) return rhs;
        return Indeterminate;
    }

    constexpr tribool operator||(tribool rhs) const noexcept {
        if (mValue == True || rhs.mValue == True) return True;
        if (mValue == False) return rhs;
        return Indeterminate;
    }

    constexpr bool operator==(tribool rhs) const noexcept { return mValue == rhs.mValue; }
    constexpr bool operator!=(tribool rhs) const noexcept { return mValue != rhs.mValue; }

private:
    value_t mValue;
};

} // namespace utils

#endif // TNT_UTILS_TRIBOOL_H
