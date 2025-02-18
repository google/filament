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

#ifndef LANGSVR_ONE_OF_H_
#define LANGSVR_ONE_OF_H_

#include <memory>
#include <type_traits>
#include <utility>

#include "langsvr/traits.h"

namespace langsvr {

/// OneOf is similar to std::variant with an implicit std::monostate, but internally uses a pointer
/// for the value. This allows OneOf to use forward-declared type lists, which is required as the
/// LSP has cyclic dependencies.
template <typename... TYPES>
struct OneOf {
    template <typename T>
    static constexpr bool IsValidType = TypeIsIn<std::decay_t<T>, TYPES...>;

    /// Constructor
    /// The OneOf is constructed with no initial value.
    OneOf() = default;

    /// Destructor
    ~OneOf() { Reset(); }

    /// Constructor
    /// @param value the initial value of the OneOf
    template <typename T, typename = std::enable_if_t<IsValidType<T>>>
    OneOf(T&& value) {
        Set(std::forward<T>(value));
    }

    /// Copy constructor
    OneOf(const OneOf& other) { *this = other; }

    /// Move constructor
    OneOf(OneOf&& other) {
        ptr = other.ptr;
        kind = other.kind;
        other.ptr = nullptr;
    }

    /// Copy assignment operator
    OneOf& operator=(const OneOf& other) {
        Reset();
        auto copy = [this](auto* p) {
            if (p) {
                this->ptr = new std::decay_t<decltype(*p)>(*p);
                return true;
            }
            return false;
        };
        (copy(other.Get<TYPES>()) || ...);
        kind = other.kind;
        return *this;
    }

    /// Move assignment constructor
    template <typename T, typename = std::enable_if_t<IsValidType<T>>>
    OneOf& operator=(T&& value) {
        Set(std::forward<T>(value));
        return *this;
    }

    /// Equality operator
    bool operator==(const OneOf& other) const {
        return Visit([&](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            if (auto* other_value = other.Get<T>()) {
                return value == *other_value;
            }
            return false;
        });
    }

    /// Inequality operator
    bool operator!=(const OneOf& other) const { return !(*this == other); }

    /// Reset clears the value of the OneOf
    void Reset() {
        (delete Get<TYPES>(), ...);
        ptr = nullptr;
        kind = kNoKind;
    }

    /// Set sets the value of the OneOf to @p value
    template <typename T, typename = std::enable_if_t<IsValidType<T>>>
    void Set(T&& value) {
        using D = std::decay_t<T>;
        Reset();
        kind = static_cast<uint8_t>(TypeIndex<D, TYPES...>);
        ptr = new D(std::forward<T>(value));
    }

    /// @returns true if the OneOf holds a value of the type T
    template <typename T>
    bool Is() const {
        return kind == TypeIndex<T, TYPES...>;
    }

    /// @returns a pointer to the value if the value is of type T
    template <typename T>
    T* Get() {
        return this->Is<T>() ? static_cast<T*>(ptr) : nullptr;
    }

    /// @returns a pointer to the value if the value is of type T
    template <typename T>
    const T* Get() const {
        return this->Is<T>() ? static_cast<const T*>(ptr) : nullptr;
    }

    /// Visit calls @p cb passing the value held by the OneOf as the single argument.
    /// @returns the value returned by @p cb
    template <typename F>
    auto Visit(F&& cb) const {
        using FIRST = typename std::tuple_element<0, std::tuple<TYPES...>>::type;
        using RET = decltype(cb(std::declval<FIRST&>()));
        if constexpr (std::is_void_v<RET>) {
            auto call = [&](auto* p) {
                if (p) {
                    cb(*p);
                }
            };
            (call(Get<TYPES>()), ...);
        } else {
            RET ret{};
            auto call = [&](auto* p) {
                if (p) {
                    ret = cb(*p);
                }
            };
            (call(Get<TYPES>()), ...);
            return ret;
        }
    }

  private:
    static_assert(sizeof...(TYPES) < 255);
    static constexpr uint8_t kNoKind = 0xff;
    void* ptr = nullptr;
    uint8_t kind = 0xff;
};

}  // namespace langsvr

#endif  // LANGSVR_ONE_OF_H_
