// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_UTILS_MATH_HASH_H_
#define SRC_TINT_UTILS_MATH_HASH_H_

#include <stdint.h>

#include <cstdio>
#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "src/tint/utils/math/crc32.h"

namespace tint {

namespace detail {

template <typename T, typename = void>
struct HasHashCodeMember : std::false_type {};

template <typename T>
struct HasHashCodeMember<
    T,
    std::enable_if_t<std::is_member_function_pointer_v<decltype(&T::HashCode)>>> : std::true_type {
};

}  // namespace detail

/// The type of a hash code
using HashCode = uint32_t;

/// Forward declarations (see below)
template <typename... ARGS>
HashCode Hash(const ARGS&... values);

template <typename... ARGS>
HashCode HashCombine(HashCode hash, const ARGS&... values);

/// A STL-compatible hasher that does a more thorough job than most implementations of std::hash.
/// Hasher has been optimized for a better quality hash at the expense of increased computation
/// costs.
/// Hasher is specialized for various core Tint data types. The default implementation will use a
/// `HashCode HashCode()` method on the `T` type, and will fallback to `std::hash<T>` if
/// `T::HashCode` is missing.
template <typename T>
struct Hasher {
    /// @param value the value to hash
    /// @returns a hash of the value
    HashCode operator()(const T& value) const {
        if constexpr (detail::HasHashCodeMember<T>::value) {
            auto hash = value.HashCode();
            static_assert(std::is_same_v<decltype(hash), HashCode>,
                          "T::HashCode() must return HashCode");
            return hash;
        } else {
            return static_cast<HashCode>(std::hash<T>()(value));
        }
    }
};

/// Hasher specialization for pointers
template <typename T>
struct Hasher<T*> {
    /// @param ptr the pointer to hash
    /// @returns a hash of the pointer
    HashCode operator()(T* ptr) const {
        auto hash = reinterpret_cast<uintptr_t>(ptr);
#ifdef TINT_HASH_SEED
        hash ^= static_cast<uint32_t>(TINT_HASH_SEED);
#endif
        if constexpr (sizeof(hash) > 4) {
            return static_cast<HashCode>(hash >> 4 | hash >> 32);
        } else {
            return static_cast<HashCode>(hash >> 4);
        }
    }
};

/// Hasher specialization for std::vector
template <typename T>
struct Hasher<std::vector<T>> {
    /// @param vector the vector to hash
    /// @returns a hash of the vector
    HashCode operator()(const std::vector<T>& vector) const {
        auto hash = Hash(vector.size());
        for (auto& el : vector) {
            hash = HashCombine(hash, el);
        }
        return hash;
    }
};

/// Hasher specialization for std::tuple
template <typename... TYPES>
struct Hasher<std::tuple<TYPES...>> {
    /// @param tuple the tuple to hash
    /// @returns a hash of the tuple
    HashCode operator()(const std::tuple<TYPES...>& tuple) const {
        return std::apply(Hash<TYPES...>, tuple);
    }
};

/// Hasher specialization for std::pair
template <typename A, typename B>
struct Hasher<std::pair<A, B>> {
    /// @param tuple the tuple to hash
    /// @returns a hash of the tuple
    HashCode operator()(const std::pair<A, B>& tuple) const {
        return std::apply(Hash<A, B>, tuple);
    }
};

/// Hasher specialization for std::optional
template <typename T>
struct Hasher<std::optional<T>> {
    /// @param optional the std::optional to hash
    /// @returns a hash of the std::optional
    HashCode operator()(const std::optional<T>& optional) const {
        auto hash = Hash(optional.has_value());
        if (optional.has_value()) {
            hash = HashCombine(hash, optional.value());
        }
        return hash;
    }
};

/// Hasher specialization for std::variant
template <typename... TYPES>
struct Hasher<std::variant<TYPES...>> {
    /// @param variant the variant to hash
    /// @returns a hash of the tuple
    HashCode operator()(const std::variant<TYPES...>& variant) const {
        return std::visit([](auto&& val) { return Hash(val); }, variant);
    }
};

/// Hasher specialization for std::string, which also supports hashing of const char* and
/// std::string_view without first constructing a std::string.
template <>
struct Hasher<std::string> {
    /// @param str the string to hash
    /// @returns a hash of the string
    HashCode operator()(const std::string& str) const {
        return static_cast<HashCode>(std::hash<std::string_view>()(std::string_view(str)));
    }

    /// @param str the string to hash
    /// @returns a hash of the string
    HashCode operator()(const char* str) const {
        return static_cast<HashCode>(std::hash<std::string_view>()(std::string_view(str)));
    }

    /// @param str the string to hash
    /// @returns a hash of the string
    HashCode operator()(const std::string_view& str) const {
        return static_cast<HashCode>(std::hash<std::string_view>()(str));
    }
};

/// Hasher specialization for std::unordered_map
template <typename K, typename V>
struct Hasher<std::unordered_map<K, V>> {
    /// @param map the std::unordered_map to hash
    /// @returns a hash of the map
    HashCode operator()(const std::unordered_map<K, V>& map) const {
        auto hash = Hash(map.size());
        for (const auto& [key, value] : map) {
            // Use an XOR to ensure that the non-deterministic ordering of the map still produces
            // the same hash value for the same entries.
            hash ^= Hash(key, value);
        }
        return hash;
    }
};

/// Hasher specialization for std::unordered_set
template <typename K>
struct Hasher<std::unordered_set<K>> {
    /// @param set the std::unordered_set to hash
    /// @returns a hash of the set
    HashCode operator()(const std::unordered_set<K>& set) const {
        auto hash = Hash(set.size());
        for (const auto& key : set) {
            // Use an XOR to ensure that the non-deterministic ordering of the map still produces
            // the same hash value for the same entries.
            hash ^= Hash(key);
        }
        return hash;
    }
};

/// @param args the arguments to hash
/// @returns a hash of the variadic list of arguments.
///          The returned hash is dependent on the order of the arguments.
template <typename... ARGS>
HashCode Hash(const ARGS&... args) {
    if constexpr (sizeof...(ARGS) == 0) {
        return 0;
    } else if constexpr (sizeof...(ARGS) == 1) {
        using T = std::tuple_element_t<0, std::tuple<ARGS...>>;
        return Hasher<T>()(args...);
    } else {
        HashCode hash = 102931;  // seed with an arbitrary prime
        return HashCombine(hash, args...);
    }
}

/// @param hash the hash value to combine with
/// @param values the values to hash
/// @returns a hash of the variadic list of arguments.
///          The returned hash is dependent on the order of the arguments.
template <typename... ARGS>
HashCode HashCombine(HashCode hash, const ARGS&... values) {
#ifdef TINT_HASH_SEED
    constexpr uint32_t offset = 0x7f4a7c16 ^ static_cast<uint32_t>(TINT_HASH_SEED);
#else
    constexpr uint32_t offset = 0x7f4a7c16;
#endif

    ((hash ^= Hash(values) + (offset ^ (hash >> 2))), ...);
    return hash;
}

/// A STL-compatible equal_to implementation that specializes for types.
template <typename T>
struct EqualTo {
    /// @param lhs the left hand side value
    /// @param rhs the right hand side value
    /// @returns true if the two values are equal
    constexpr bool operator()(const T& lhs, const T& rhs) const {
        return std::equal_to<T>()(lhs, rhs);
    }
};

/// A specialization for EqualTo for std::string, which supports additional comparision with
/// std::string_view and const char*.
template <>
struct EqualTo<std::string> {
    /// @param lhs the left hand side value
    /// @param rhs the right hand side value
    /// @returns true if the two values are equal
    bool operator()(const std::string& lhs, const std::string& rhs) const { return lhs == rhs; }

    /// @param lhs the left hand side value
    /// @param rhs the right hand side value
    /// @returns true if the two values are equal
    bool operator()(const std::string& lhs, const char* rhs) const { return lhs == rhs; }

    /// @param lhs the left hand side value
    /// @param rhs the right hand side value
    /// @returns true if the two values are equal
    bool operator()(const std::string& lhs, std::string_view rhs) const { return lhs == rhs; }

    /// @param lhs the left hand side value
    /// @param rhs the right hand side value
    /// @returns true if the two values are equal
    bool operator()(const char* lhs, const std::string& rhs) const { return lhs == rhs; }

    /// @param lhs the left hand side value
    /// @param rhs the right hand side value
    /// @returns true if the two values are equal
    bool operator()(std::string_view lhs, const std::string& rhs) const { return lhs == rhs; }
};

/// Wrapper for a hashable type enabling the wrapped value to be used as a key
/// for an unordered_map or unordered_set.
template <typename T>
struct UnorderedKeyWrapper {
    /// The wrapped value
    T value;
    /// The hash of value
    HashCode hash;

    /// Constructor
    /// @param v the value to wrap
    explicit UnorderedKeyWrapper(const T& v) : value(v), hash(Hash(v)) {}

    /// Move constructor
    /// @param v the value to wrap
    explicit UnorderedKeyWrapper(T&& v) : value(std::move(v)), hash(Hash(value)) {}

    /// @returns true if this wrapper comes before other
    /// @param other the RHS of the operator
    bool operator<(const UnorderedKeyWrapper& other) const { return hash < other.hash; }

    /// @returns true if this wrapped value is equal to the other wrapped value
    /// @param other the RHS of the operator
    bool operator==(const UnorderedKeyWrapper& other) const { return value == other.value; }
};

}  // namespace tint

namespace std {

/// Custom std::hash specialization for tint::UnorderedKeyWrapper
template <typename T>
class hash<tint::UnorderedKeyWrapper<T>> {
  public:
    /// @param w the UnorderedKeyWrapper
    /// @return the hash value
    inline std::size_t operator()(const tint::UnorderedKeyWrapper<T>& w) const { return w.hash; }
};

}  // namespace std

#endif  // SRC_TINT_UTILS_MATH_HASH_H_
