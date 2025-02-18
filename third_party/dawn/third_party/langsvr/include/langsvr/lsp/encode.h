// Copyright 2024 The langsvr Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimev.
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

#ifndef LANGSVR_LSP_ENCODE_H_
#define LANGSVR_LSP_ENCODE_H_

#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "langsvr/json/builder.h"
#include "langsvr/lsp/primitives.h"
#include "langsvr/one_of.h"
#include "langsvr/optional.h"
#include "langsvr/traits.h"

// Forward declarations
namespace langsvr::lsp {
template <typename T>
Result<const json::Value*> Encode(const Optional<T>& in, json::Builder& b);
template <typename T>
Result<const json::Value*> Encode(const std::vector<T>& in, json::Builder& b);
template <typename... TYPES>
Result<const json::Value*> Encode(const std::tuple<TYPES...>& in, json::Builder& b);
template <typename V>
Result<const json::Value*> Encode(const std::unordered_map<std::string, V>& in, json::Builder& b);
template <typename... TYPES>
Result<const json::Value*> Encode(const OneOf<TYPES...>& in, json::Builder& b);
}  // namespace langsvr::lsp

namespace langsvr::lsp {

Result<const json::Value*> Encode(Null in, json::Builder& b);
Result<const json::Value*> Encode(Boolean in, json::Builder& b);
Result<const json::Value*> Encode(Integer in, json::Builder& b);
Result<const json::Value*> Encode(Uinteger in, json::Builder& b);
Result<const json::Value*> Encode(Decimal in, json::Builder& b);
Result<const json::Value*> Encode(const String& in, json::Builder& b);

template <typename T>
Result<const json::Value*> Encode(const Optional<T>& in, json::Builder& b) {
    return Encode(*in, b);
}

template <typename T>
Result<const json::Value*> Encode(const std::vector<T>& in, json::Builder& b) {
    std::vector<const json::Value*> values;
    values.reserve(in.size());
    for (auto& element : in) {
        auto value = Encode(element, b);
        if (value != Success) {
            return value.Failure();
        }
        values.push_back(value.Get());
    }
    return b.Array(values);
}

template <typename... TYPES>
Result<const json::Value*> Encode(const std::tuple<TYPES...>& in, json::Builder& b) {
    std::string error;
    std::vector<const json::Value*> values;
    values.reserve(sizeof...(TYPES));
    auto encode = [&](auto& el) {
        auto value = Encode(el, b);
        if (value != Success) {
            error = std::move(value.Failure().reason);
            return false;
        }
        values.push_back(value.Get());
        return true;
    };
    std::apply([&](auto&... elements) { (encode(elements) && ...); }, in);
    if (error.empty()) {
        return b.Array(values);
    }
    return Failure{std::move(error)};
}

template <typename V>
Result<const json::Value*> Encode(const std::unordered_map<std::string, V>& in, json::Builder& b) {
    std::vector<json::Builder::Member> members;
    members.reserve(in.size());
    for (auto it : in) {
        auto value = Encode(it.second, b);
        if (value != Success) {
            return value.Failure();
        }
        members.push_back(json::Builder::Member{it.first, value.Get()});
    }
    return b.Object(members);
}

template <typename... TYPES>
Result<const json::Value*> Encode(const OneOf<TYPES...>& in, json::Builder& b) {
    return in.Visit([&](const auto& v) { return Encode(v, b); });
}

}  // namespace langsvr::lsp

#endif  // LANGSVR_LSP_ENCODE_H_
