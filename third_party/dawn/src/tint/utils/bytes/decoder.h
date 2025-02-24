// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_TINT_UTILS_BYTES_DECODER_H_
#define SRC_TINT_UTILS_BYTES_DECODER_H_

#include <bitset>
#include <climits>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/utils/bytes/reader.h"
#include "src/tint/utils/reflection.h"

namespace tint::bytes {

template <typename T, typename = void>
struct Decoder;

/// Decodes T from @p reader.
/// @param reader the byte reader
/// @param args additional arguments used by Decoder<T>::Decode()
/// @returns the decoded object
template <typename T, typename... ARGS>
Result<T> Decode(Reader& reader, ARGS&&... args) {
    return Decoder<T>::Decode(reader, std::forward<ARGS>(args)...);
}

/// Decoder specialization for integer types
template <typename T>
struct Decoder<T, std::enable_if_t<std::is_integral_v<T>>> {
    /// Decode decodes the integer type from @p reader.
    /// @param reader the reader to decode from
    /// @param endianness the endianness of the integer
    /// @returns the decoded integer type, or an error if the stream is too short.
    static Result<T> Decode(Reader& reader, Endianness endianness = Endianness::kLittle) {
        return reader.Int<T>(endianness);
    }
};

/// Decoder specialization for floating point types
template <typename T>
struct Decoder<T, std::enable_if_t<std::is_floating_point_v<T>>> {
    /// Decode decodes the floating point type from @p reader.
    /// @param reader the reader to decode from
    /// @returns the decoded floating point type, or an error if the stream is too short.
    static Result<T> Decode(Reader& reader) { return reader.Float<T>(); }
};

/// Decoder specialization for a uint16_t length prefixed string.
template <>
struct Decoder<std::string, void> {
    /// Decode decodes the string from @p reader.
    /// @param reader the reader to decode from
    /// @returns the decoded string, or an error if the stream is too short.
    static Result<std::string> Decode(Reader& reader) {
        auto len = reader.Int<uint16_t>();
        if (len != Success) {
            return len.Failure();
        }
        return reader.String(len.Get());
    }
};

/// Decoder specialization for bool types
template <>
struct Decoder<bool, void> {
    /// Decode decodes the boolean from @p reader.
    /// @param reader the reader to decode from
    /// @returns the decoded boolean, or an error if the stream is too short.
    static Result<bool> Decode(Reader& reader) { return reader.Bool(); }
};

/// Decoder specialization for types that use TINT_REFLECT
template <typename T>
struct Decoder<T, std::enable_if_t<HasReflection<T>>> {
    /// Decode decodes the reflected type from @p reader.
    /// @param reader the reader to decode from
    /// @returns the decoded reflected type, or an error if the stream is too short.
    static Result<T> Decode(Reader& reader) {
        T object{};
        diag::List errs;
        ForeachField(object, [&](auto& field) {  //
            auto value = bytes::Decode<std::decay_t<decltype(field)>>(reader);
            if (value == Success) {
                field = value.Get();
            } else {
                errs.Add(value.Failure().reason);
            }
        });
        if (errs.empty()) {
            return object;
        }
        return Failure{errs};
    }
};

/// Decoder specialization for std::unordered_map
template <typename K, typename V>
struct Decoder<std::unordered_map<K, V>, void> {
    /// Decode decodes the map from @p reader.
    /// @param reader the reader to decode from
    /// @returns the decoded map, or an error if the stream is too short.
    static Result<std::unordered_map<K, V>> Decode(Reader& reader) {
        std::unordered_map<K, V> out;

        while (!reader.IsEOF()) {
            auto stop = bytes::Decode<bool>(reader);
            if (stop != Success) {
                return stop.Failure();
            }
            if (stop.Get()) {
                break;
            }
            auto key = bytes::Decode<K>(reader);
            if (key != Success) {
                return key.Failure();
            }
            auto val = bytes::Decode<V>(reader);
            if (val != Success) {
                return val.Failure();
            }
            out.emplace(std::move(key.Get()), std::move(val.Get()));
        }

        return out;
    }
};

/// Decoder specialization for std::unordered_set
template <typename V>
struct Decoder<std::unordered_set<V>, void> {
    /// Decode decodes the set from @p reader.
    /// @param reader the reader to decode from
    /// @returns the decoded set, or an error if the stream is too short.
    static Result<std::unordered_set<V>> Decode(Reader& reader) {
        std::unordered_set<V> out;

        while (!reader.IsEOF()) {
            auto stop = bytes::Decode<bool>(reader);
            if (stop != Success) {
                return stop.Failure();
            }
            if (stop.Get()) {
                break;
            }
            auto val = bytes::Decode<V>(reader);
            if (val != Success) {
                return val.Failure();
            }
            out.emplace(std::move(val.Get()));
        }

        return out;
    }
};

/// Decoder specialization for std::vector
template <typename V>
struct Decoder<std::vector<V>, void> {
    /// Decode decodes the vector from @p reader.
    /// @param reader the reader to decode from
    /// @returns the decoded vector, or an error if the stream is too short.
    static Result<std::vector<V>> Decode(Reader& reader) {
        std::vector<V> out;

        while (!reader.IsEOF()) {
            auto stop = bytes::Decode<bool>(reader);
            if (stop != Success) {
                return stop.Failure();
            }
            if (stop.Get()) {
                break;
            }
            auto val = bytes::Decode<V>(reader);
            if (val != Success) {
                return val.Failure();
            }
            out.emplace_back(std::move(val.Get()));
        }

        return out;
    }
};

/// Decoder specialization for std::optional
template <typename T>
struct Decoder<std::optional<T>, void> {
    /// Decode decodes the optional from @p reader.
    /// @param reader the reader to decode from
    /// @returns the decoded optional, or an error if the stream is too short.
    static Result<std::optional<T>> Decode(Reader& reader) {
        auto has_value = bytes::Decode<bool>(reader);
        if (has_value != Success) {
            return has_value.Failure();
        }
        if (!has_value.Get()) {
            return std::optional<T>{std::nullopt};
        }
        auto value = bytes::Decode<T>(reader);
        if (value != Success) {
            return value.Failure();
        }
        return std::optional<T>{value.Get()};
    }
};

/// Decoder specialization for std::bitset
template <std::size_t N>
struct Decoder<std::bitset<N>, void> {
    /// Decode decodes the bitset from @p reader.
    /// @param reader the reader to decode from
    /// @returns the decoded bitset, or an error if the stream is too short.
    static Result<std::bitset<N>> Decode(Reader& reader) {
        Vector<std::byte, 32> vec;
        vec.Resize((N + CHAR_BIT - 1) / CHAR_BIT);

        if (auto len = reader.Read(&vec[0], vec.Length()); len != vec.Length()) {
            return Failure{"EOF"};
        }

        std::bitset<N> out;
        for (std::size_t i = 0; i < N; i++) {
            std::size_t w = i / CHAR_BIT;
            std::size_t b = i - (w * CHAR_BIT);
            out[i] = ((vec[w] >> b) & std::byte{1}) != std::byte{0};
        }
        return out;
    }
};

/// Decoder specialization for std::tuple
template <typename FIRST, typename... OTHERS>
struct Decoder<std::tuple<FIRST, OTHERS...>, void> {
    /// Decode decodes the tuple from @p reader.
    /// @param reader the reader to decode from
    /// @returns the decoded tuple, or an error if the stream is too short.
    static Result<std::tuple<FIRST, OTHERS...>> Decode(Reader& reader) {
        auto first = bytes::Decode<FIRST>(reader);
        if (first != Success) {
            return first.Failure();
        }
        if constexpr (sizeof...(OTHERS) > 0) {
            auto others = bytes::Decode<std::tuple<OTHERS...>>(reader);
            if (others != Success) {
                return others.Failure();
            }
            return std::tuple_cat(std::tuple<FIRST>(first.Get()), others.Get());
        } else {
            return std::tuple<FIRST>(first.Get());
        }
    }
};

/// Decoder specialization for enum types that have a range defined with TINT_REFLECT_ENUM_RANGE
template <typename T>
struct Decoder<T, std::void_t<decltype(tint::EnumRange<T>::kMax)>> {
    /// Decode decodes the enum type from @p reader.
    /// @param reader the reader to decode from
    /// @param endianness the endianness of the enum
    /// @returns the decoded enum type, or an error if the stream is too short.
    static Result<T> Decode(Reader& reader, Endianness endianness = Endianness::kLittle) {
        using Range = tint::EnumRange<T>;
        using U = std::underlying_type_t<T>;
        auto value = reader.Int<U>(endianness);
        if (value != Success) {
            return value.Failure();
        }
        static constexpr U kMin = static_cast<U>(Range::kMin);
        static constexpr U kMax = static_cast<U>(Range::kMax);
        if (value.Get() < kMin || value.Get() > kMax) {
            return Failure{"value " + std::to_string(value.Get()) + " out of range for enum"};
        }
        return static_cast<T>(value.Get());
    }
};

}  // namespace tint::bytes

#endif  // SRC_TINT_UTILS_BYTES_DECODER_H_
