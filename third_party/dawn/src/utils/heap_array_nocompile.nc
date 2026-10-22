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

// Test the type checks of dawn::Span.
// Note: all unsafe buffer warning tests are in warning_nocompile.nc.

#include <array>

#include "src/utils/heap_array.h"
#include "src/utils/typed_integer.h"

namespace dawn {

static constexpr std::array<int, 5> kSpanData = {1, 2, 3, 4, 5};

using Index = TypedInteger<struct IndexT, uint32_t>;
using IndexSizeT = TypedInteger<struct IndexT, size_t>;
enum class Val {};

using TypedHeapArray = ityp::HeapArray<Index, Val>;
using UntypedHeapArray = HeapArray<Val>;
using SmallUntypedHeapArray = ityp::HeapArray<uint8_t, Val>;
using TypedHeapArrayPOD = ityp::HeapArray<Index, int>;

void TestConstValue() {
    { HeapArray<const int> x; }  // expected-error@heap_array.h:* {{Array contents cannot be constant}}
}

// Uninit can only be used when the value type is capable of being uninitialized.
void TestUninit() {
    (void)HeapArray<int>::Uninit(4);
    (void)HeapArray<Val>::Uninit(4);
    (void)HeapArray<Index>::Uninit(4);  // expected-error {{no matching function for call to 'Uninit'}}
}

void TestConversionToSpan() {
    TypedHeapArray arr{Index{10u}};

    // dawn::Span cannot construct from a range with a different index type.
    // This probably isn't important and could be changed.
    (void)ityp::span<Index, Val>(arr);
    (void)dawn::Span<Val>(arr);  // expected-error {{no matching conversion for functional-style cast from}}
}

void TestIndexing() {
    {
        TypedHeapArray arr(Index{4u});
        arr[Index{0u}];  // Control case.
        arr[0u];         // expected-error {{no viable overloaded operator[]}}
    }
    {
        UntypedHeapArray arr(4);
        arr[0u];         // Control case.
        // TODO(https://crbug.com/42240462): This should be an invalid implicit integer conversion.
        arr[0];          // Control case.
        arr[Index{0u}];  // expected-error {{no viable overloaded operator[]}}
    }
    {
        SmallUntypedHeapArray arr(4);
        arr[255];  // Control case.
        arr[256];  // expected-error {{implicit conversion from 'int' to 'unsigned char' changes value}}

        arr[uint8_t{0}];   // Control case.
        // TODO(https://crbug.com/42240462): These should be invalid implicit integer conversions.
        arr[uint32_t{0}];
        arr[int8_t{0}];
    }
}

}  // namespace dawn
