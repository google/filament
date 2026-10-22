// Copyright 2026 The Dawn & Tint Authors
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

// Tests for warnings. Note this code should ONLY trigger warnings (no other hard errors),
// otherwise the compiler suppresses the warnings in favor of the errors.

#include <stdint.h>

#include <array>
#include <span>

#include "src/utils/heap_array.h"
#include "src/utils/span.h"

namespace dawn {

// Verify that warnings that are part of -Weverything but not -Wall -Wextra
// are enabled. This test runs only when `dawn_weverything = true`.
void TestWeverything() {
    // -Wzero-as-null-pointer-constant is one such warning.
    [[maybe_unused]] void* p = 0; // expected-error {{zero as null pointer constant}}
}

// Verify that warnings suppressions limited to specific file with --warning-suppression-mappings
// are enabled to other code.
void TestWarningSuppressionMappings() {
    // -Wold-style-cast is one such warning.
    int x = 0;
    [[maybe_unused]] float y = (float)x; // expected-error {{use of old-style cast}}
}

// -Wunsafe-buffer-usage: operator[] on T*
void TestUnsafeBuffersRawPointer() {
    constexpr std::array<int, 4> arr{};

    // Safe to get a pointer to the first element.
    const int* arrPtr = arr.data();

    // But unsafe to use it like an array.
    { [[maybe_unused]] int x = arr[1]; }
    { [[maybe_unused]] int x = std::span(arr)[1]; }
    { [[maybe_unused]] int x = arrPtr[1]; }  // expected-error {{unsafe buffer access}}
}

// -Wunsafe-buffer-usage-in-libc-call: memcpy()
// This is skipped on Android because with Android Bionic's _FORTIFY_SOURCE,
// memcpy is a static inline wrapper (__BIONIC_FORTIFY_INLINE). This causes
// Clang's unsafe-buffer-usage checker to ignore it (as it only matches global
// functions or std:: functions), which then causes the expected error to not
// be raised.
#if !defined(__ANDROID__)
void TestUnsafeBuffersMemcpy() {
    int x = 1;
    int y = 2;
    memcpy(&y, &x, sizeof(int));  // expected-error {{function 'memcpy' is unsafe}}
}
#endif  // !defined(__ANDROID__)

// -Wunsafe-buffer-usage: std::span() constructors
void TestUnsafeBuffersStdSpanConstructors() {
    std::array<int, 4> arr{};
    // std::span is NOT tagged as being unsafe when using the unsafe buffers plugin.
    // (If we use -Wunsafe-buffer-usage without the plugin, it would be. This may be fixable.)
    { [[maybe_unused]] auto s = std::span(arr.data(), arr.size()); }
    { [[maybe_unused]] std::span<int> s(arr.data(), arr.size()); }
}

// -Wunsafe-buffer-usage: dawn::Span() constructors
void TestUnsafeBuffersDawnSpanConstructors() {
    {
        constexpr std::array<int, 5> kArr{};

        DAWN_UNSAFE_BUFFERS(Span<const int>(kArr.data(), kArr.size()));  // Control case.
        Span<const int>(kArr.data(), kArr.size());  // expected-error {{introduces unsafe buffer manipulation}}

        DAWN_UNSAFE_BUFFERS(Span<const int>(kArr.begin(), kArr.end()));  // Control case.
        Span<const int>(kArr.begin(), kArr.end());  // expected-error {{introduces unsafe buffer manipulation}}

        DAWN_UNSAFE_BUFFERS(Span<const int, 5>(kArr.data()));  // Control case.
        Span<const int, 5>(kArr.data());  // expected-error {{introduces unsafe buffer manipulation}}

        DAWN_UNSAFE_BUFFERS(Span<const int, 5>(kArr.begin()));  // Control case.
        Span<const int, 5>(kArr.begin());  // expected-error {{introduces unsafe buffer manipulation}}
    }
}

// -Wunsafe-buffer-usage: dawn::HeapArray::Uninit()
void TestUnsafeBuffersDawnHeapArrayConstructors() {
    { [[maybe_unused]] auto x = DAWN_UNSAFE_BUFFERS(HeapArray<int>::Uninit(4)); }  // Control case.
    { [[maybe_unused]] auto x = HeapArray<int>::Uninit(4); }  // expected-error {{introduces unsafe buffer manipulation}}

    { [[maybe_unused]] auto x = DAWN_UNSAFE_BUFFERS(HeapArray<int>::Uninit(4, std::nothrow)); }  // Control case.
    { [[maybe_unused]] auto x = HeapArray<int>::Uninit(4, std::nothrow); }  // expected-error {{introduces unsafe buffer manipulation}}
}

// -Wunsafe-buffer-usage: dawn::ReinterpretSpan() for non-trivially-copyable types
void TestUnsafeBuffersDawnReinterpretSpan() {
    struct S {
        S() {}
        ~S() {}
        S(const S&) {}
        char i;
    };
    std::array<std::byte, 4> bytes;
    auto s = Span<std::byte>{bytes};

    { [[maybe_unused]] auto x = DAWN_UNSAFE_BUFFERS(ReinterpretSpan<S>(s)); }  // Control case.
    { [[maybe_unused]] auto x = ReinterpretSpan<S>(s); }  // expected-error {{introduces unsafe buffer manipulation}}
}

}
