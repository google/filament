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

#ifndef LANGSVR_OPTIONAL_H_
#define LANGSVR_OPTIONAL_H_

#include <cassert>
#include <type_traits>
#include <utility>

namespace langsvr {

/// Optional is similar to std::optional, but internally uses a pointer for the value. This allows
/// Optional to use forward-declared type lists, which is required as the LSP has cyclic
/// dependencies.
template <typename T>
struct Optional {
    /// Constructor
    /// The optional is constructed with no value
    Optional() = default;

    /// Destructor
    ~Optional() { Reset(); }

    /// Copy constructor
    Optional(const Optional& other) { *this = other; }

    /// Move constructor
    Optional(Optional&& other) { *this = std::move(other); }

    /// Copy constructor with value
    Optional(const T& other) { *this = other; }

    /// Move constructor with value
    Optional(T&& other) { *this = std::move(other); }

    /// Reset clears the value from the optional
    void Reset() {
        if (ptr) {
            delete ptr;
            ptr = nullptr;
        }
    }

    /// Copy assignment operator
    Optional& operator=(const Optional& other) {
        Reset();
        if (other.ptr) {
            ptr = new T(*other);
        }
        return *this;
    }

    /// Move assignment operator
    Optional& operator=(Optional&& other) {
        Reset();
        ptr = other.ptr;
        other.ptr = nullptr;
        return *this;
    }

    /// Copy assignment operator
    Optional& operator=(const T& value) {
        Reset();
        if (!ptr) {
            ptr = new T(value);
        }
        return *this;
    }

    /// Move assignment operator
    Optional& operator=(T&& value) {
        Reset();
        if (!ptr) {
            ptr = new T(std::move(value));
        }
        return *this;
    }

    /// @returns true if the optional holds a value
    operator bool() const { return ptr != nullptr; }

    /// @returns false if the optional holds a value
    bool operator!() const { return ptr == nullptr; }

    T* operator->() { return &Get(); }
    const T* operator->() const { return &Get(); }

    T& operator*() { return Get(); }
    const T& operator*() const { return Get(); }

    /// Equality operator
    template <typename V>
    bool operator==(V&& value) const {
        if constexpr (std::is_same_v<Optional, std::decay_t<V>>) {
            return (!*this && !value) || (*this && value && (Get() == value.Get()));
        } else {
            if (!ptr) {
                return false;
            }
            return Get() == std::forward<V>(value);
        }
    }

    /// Inequality operator
    template <typename V>
    bool operator!=(V&& value) const {
        return !(*this == std::forward<V>(value));
    }

  private:
    T& Get() {
        assert(ptr);
        return *ptr;
    }

    const T& Get() const {
        assert(ptr);
        return *ptr;
    }

    T* ptr = nullptr;
};

}  // namespace langsvr

#endif  // LANGSVR_OPTIONAL_H_
