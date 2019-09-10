/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef TNT_UTILS_BITMASKENUM_H
#define TNT_UTILS_BITMASKENUM_H

#include <utils/compiler.h>

#include <type_traits> // for std::false_type

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

namespace utils {
template<typename Enum>
struct EnableBitMaskOperators : public std::false_type { };
} // namespace utils

template<typename Enum, typename std::enable_if_t<
        std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr bool operator!(Enum rhs) noexcept {
    using underlying = std::underlying_type_t<Enum>;
    return underlying(rhs) == 0;
}

template<typename Enum, typename std::enable_if_t<
        std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator~(Enum rhs) noexcept {
    using underlying = std::underlying_type_t<Enum>;
    return Enum(~underlying(rhs));
}

template<typename Enum, typename std::enable_if_t<
        std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator|(Enum lhs, Enum rhs) noexcept {
    using underlying = std::underlying_type_t<Enum>;
    return Enum(underlying(lhs) | underlying(rhs));
}

template<typename Enum, typename std::enable_if_t<
        std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator&(Enum lhs, Enum rhs) noexcept {
    using underlying = std::underlying_type_t<Enum>;
    return Enum(underlying(lhs) & underlying(rhs));
}

template<typename Enum, typename std::enable_if_t<
        std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator^(Enum lhs, Enum rhs) noexcept {
    using underlying = std::underlying_type_t<Enum>;
    return Enum(underlying(lhs) ^ underlying(rhs));
}

template<typename Enum, typename std::enable_if_t<
        std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator|=(Enum& lhs, Enum rhs) noexcept {
    return lhs = lhs | rhs;
}

template<typename Enum, typename std::enable_if_t<
        std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator&=(Enum& lhs, Enum rhs) noexcept {
    return lhs = lhs & rhs;
}

template<typename Enum, typename std::enable_if_t<
        std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr Enum operator^=(Enum& lhs, Enum rhs) noexcept {
    return lhs = lhs ^ rhs;
}

template<typename Enum, typename std::enable_if_t<
        std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr bool none(Enum lhs) noexcept {
    return !lhs;
}

template<typename Enum, typename std::enable_if_t<
        std::is_enum<Enum>::value && utils::EnableBitMaskOperators<Enum>::value, int> = 0>
inline constexpr bool any(Enum lhs) noexcept {
    return !none(lhs);
}


#endif // TNT_UTILS_BITMASKENUM_H
