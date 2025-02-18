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

#ifndef LANGSVR_DECODE_H_
#define LANGSVR_DECODE_H_

#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "langsvr/json/value.h"
#include "langsvr/lsp/primitives.h"
#include "langsvr/one_of.h"
#include "langsvr/optional.h"
#include "langsvr/traits.h"

// Forward declarations
namespace langsvr::lsp {
template <typename T>
Result<SuccessType> Decode(const json::Value& v, Optional<T>& out);
template <typename T>
Result<SuccessType> Decode(const json::Value& v, std::vector<T>& out);
template <typename... TYPES>
Result<SuccessType> Decode(const json::Value& v, std::tuple<TYPES...>& out);
template <typename V>
Result<SuccessType> Decode(const json::Value& v, std::unordered_map<std::string, V>& out);
template <typename... TYPES>
Result<SuccessType> Decode(const json::Value& v, OneOf<TYPES...>& out);
}  // namespace langsvr::lsp

namespace langsvr::lsp {

Result<SuccessType> Decode(const json::Value& v, Null& out);
Result<SuccessType> Decode(const json::Value& v, Boolean& out);
Result<SuccessType> Decode(const json::Value& v, Integer& out);
Result<SuccessType> Decode(const json::Value& v, Uinteger& out);
Result<SuccessType> Decode(const json::Value& v, Decimal& out);
Result<SuccessType> Decode(const json::Value& v, String& out);

template <typename T>
Result<SuccessType> Decode(const json::Value& v, Optional<T>& out) {
    return Decode(v, *out);
}

template <typename T>
Result<SuccessType> Decode(const json::Value& v, std::vector<T>& out) {
    if (v.Kind() != json::Kind::kArray) {
        return Failure{"JSON value is not an array"};
    }
    out.resize(v.Count());
    for (size_t i = 0, n = out.size(); i < n; i++) {
        auto element = v.Get(i);
        if (element != Success) {
            return element.Failure();
        }
        if (auto res = Decode(*element.Get(), out[i]); res != Success) {
            return res.Failure();
        }
    }
    return Success;
}

template <typename... TYPES>
Result<SuccessType> Decode(const json::Value& v, std::tuple<TYPES...>& out) {
    if (v.Kind() != json::Kind::kArray) {
        return Failure{"JSON value is not an array"};
    }
    if (v.Count() != sizeof...(TYPES)) {
        return Failure{"JSON array does not match tuple length"};
    }

    std::string error;
    size_t idx = 0;

    auto decode = [&](auto& el) {
        auto element = v.Get(idx);
        if (element != Success) {
            error = std::move(element.Failure().reason);
            return false;
        }
        if (auto res = Decode(*element.Get(), el); res != Success) {
            error = std::move(res.Failure().reason);
            return false;
        }
        idx++;
        return true;
    };
    std::apply([&](auto&... elements) { (decode(elements) && ...); }, out);
    if (error.empty()) {
        return Success;
    }
    return Failure{std::move(error)};
}

template <typename V>
Result<SuccessType> Decode(const json::Value& v, std::unordered_map<std::string, V>& out) {
    if (v.Kind() != json::Kind::kObject) {
        return Failure{"JSON value is not an object"};
    }
    auto names = v.MemberNames();
    if (names != Success) {
        return names.Failure();
    }
    out.reserve(names->size());
    for (auto& name : names.Get()) {
        auto element = v.Get(name);
        if (element != Success) {
            return element.Failure();
        }
        if (auto res = Decode(*element.Get(), out[name]); res != Success) {
            return res.Failure();
        }
    }
    return Success;
}

template <typename... TYPES>
Result<SuccessType> Decode(const json::Value& v, OneOf<TYPES...>& out) {
    auto try_type = [&](auto* p) {
        using T = std::remove_pointer_t<decltype(p)>;
        T val;
        if (auto res = Decode(v, val); res == Success) {
            out = std::move(val);
            return true;
        }
        return false;
    };

    bool ok = (try_type(static_cast<TYPES*>(nullptr)) || ...);
    if (ok) {
        return Success;
    }

    return Failure{"no types matched the OneOf"};
}

}  // namespace langsvr::lsp

#endif  // LANGSVR_DECODE_H_
