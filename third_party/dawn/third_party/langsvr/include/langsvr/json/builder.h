// Copyright 2024 The langsvr Authors
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

#ifndef LANGSVR_JSON_BUILDER_
#define LANGSVR_JSON_BUILDER_

#include <memory>
#include <type_traits>

#include "langsvr/json/value.h"
#include "langsvr/span.h"
#include "langsvr/traits.h"

// Forward declarations
namespace langsvr::json {
class Value;
}

namespace langsvr::json {

/// Builder is a structured interface to build a JSON message.
/// Value objects returned by methods of the Builder are owned by the Builder, and will be freed
/// when the Builder is destructed.
class Builder {
  public:
    /// Destructor
    virtual ~Builder();

    /// @return a new Builder
    static std::unique_ptr<Builder> Create();

    /// @param json the JSON string to parse
    /// @returns a new Value from the JSON string
    virtual Result<const Value*> Parse(std::string_view json) = 0;

    /// Creates a Null JSON value
    virtual const Value* Null() = 0;

    /// Creates a Bool JSON value
    /// @param value the new JSON value
    virtual const Value* Bool(json::Bool value) = 0;

    /// Creates a I64 JSON value
    /// @param value the new JSON value
    virtual const Value* I64(json::I64 value) = 0;

    /// Creates a U64 JSON value
    /// @param value the new JSON value
    virtual const Value* U64(json::U64 value) = 0;

    /// Creates a F64 JSON value
    /// @param value the new JSON value
    virtual const Value* F64(json::F64 value) = 0;

    /// Creates a String JSON value
    /// @param value the new JSON value
    virtual const Value* String(json::String value) = 0;

    /// Creates a String JSON value
    /// @param value the new JSON value
    const Value* String(std::string_view value) { return String(json::String(value)); }

    /// Creates a String JSON value
    /// @param value the new JSON value
    const Value* String(const char* value) { return String(json::String(value)); }

    /// Creates an array JSON value
    /// @param elements the elements of the array
    virtual const Value* Array(Span<const Value*> elements) = 0;

    /// Member represents a single member of a JSON object
    struct Member {
        /// The member name
        json::String name;
        /// The member value
        const Value* value;
    };

    /// Creates an object JSON value
    /// @param members the members of the array
    virtual const Value* Object(Span<Member> members) = 0;

    template <typename T>
    auto Create(T&& value) {
        static constexpr bool is_bool = std::is_same_v<T, json::Bool>;
        static constexpr bool is_i64 = std::is_integral_v<T> && std::is_signed_v<T>;
        static constexpr bool is_u64 = std::is_integral_v<T> && std::is_unsigned_v<T>;
        static constexpr bool is_f64 = std::is_floating_point_v<T>;
        static constexpr bool is_string = IsStringLike<T>;
        static_assert(is_bool || is_i64 || is_u64 || is_f64 || is_string);
        if constexpr (is_bool) {
            return Bool(std::forward<T>(value));
        } else if constexpr (is_i64) {
            return I64(static_cast<json::I64>(std::forward<T>(value)));
        } else if constexpr (is_u64) {
            return U64(static_cast<json::U64>(std::forward<T>(value)));
        } else if constexpr (is_f64) {
            return F64(static_cast<json::F64>(std::forward<T>(value)));
        } else if constexpr (is_string) {
            return String(std::forward<T>(value));
        }
    }
};

}  // namespace langsvr::json

#endif  // LANGSVR_JSON_BUILDER_
