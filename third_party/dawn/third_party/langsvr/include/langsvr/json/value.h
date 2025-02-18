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

#ifndef LANGSVR_JSON_VALUE_H_
#define LANGSVR_JSON_VALUE_H_

#include <memory>
#include <string>
#include <vector>

#include "langsvr/json/types.h"
#include "langsvr/result.h"

namespace langsvr::json {

/// Value is a structured interface to read a JSON message
class Value {
  public:
    /// Destructor
    virtual ~Value();

    /// @returns the kind of the current JSON value
    virtual json::Kind Kind() const = 0;

    /// @returns the JSON string of the current value
    virtual std::string Json() const = 0;

    /// @returns success if the JSON value is null
    virtual Result<SuccessType> Null() const = 0;

    /// @returns the current JSON value as a Bool
    virtual Result<json::Bool> Bool() const = 0;

    /// @returns the current JSON value as a I64
    virtual Result<json::I64> I64() const = 0;

    /// @returns the current JSON value as a U64
    virtual Result<json::U64> U64() const = 0;

    /// @returns the current JSON value as a F64
    virtual Result<json::F64> F64() const = 0;

    /// @returns the current JSON value as a String
    virtual Result<json::String> String() const = 0;

    /// @returns a const Value* to the element if the JSON value is an array, the index is in
    /// bounds.
    virtual Result<const Value*> Get(size_t index) const = 0;

    /// @returns a const Value* to the element if the JSON value is an object, a member with the
    /// given name exists.
    virtual Result<const Value*> Get(std::string_view name) const = 0;

    /// @returns the number of JSON elements if the value is an array, the number of members if this
    /// is an object, otherwise 0.
    virtual size_t Count() const = 0;

    /// @returns the member names of this JSON object.
    virtual Result<std::vector<std::string>> MemberNames() const = 0;

    /// @returns true if the JSON value is an object and has a member with the given name
    virtual bool Has(std::string_view name) const = 0;

    template <typename T, typename I>
    Result<T> Get(I&& index) const {
        auto element = Get(std::forward<I>(index));
        if (element != Success) {
            return element.Failure();
        }

        static constexpr bool is_bool = std::is_same_v<T, json::Bool>;
        static constexpr bool is_i64 = std::is_same_v<T, json::I64>;
        static constexpr bool is_u64 = std::is_same_v<T, json::U64>;
        static constexpr bool is_f64 = std::is_same_v<T, json::F64>;
        static constexpr bool is_string = std::is_same_v<T, json::String>;
        static_assert(is_bool || is_i64 || is_u64 || is_f64 || is_string);

        if constexpr (is_bool) {
            return element.Get()->Bool();
        } else if constexpr (is_i64) {
            return element.Get()->I64();
        } else if constexpr (is_u64) {
            return element.Get()->U64();
        } else if constexpr (is_f64) {
            return element.Get()->F64();
        } else if constexpr (is_string) {
            return element.Get()->String();
        }
    }
};

}  // namespace langsvr::json

#endif  // LANGSVR_JSON_VALUE_H_
