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

#ifndef SRC_TINT_UTILS_GENERATION_ID_H_
#define SRC_TINT_UTILS_GENERATION_ID_H_

#include <stdint.h>

#include "src/tint/utils/rtti/traits.h"

namespace tint {

/// If 1 then checks are enabled that AST nodes are not leaked from one generation to another.
/// It may be worth disabling this in production builds. For now we always check.
#define TINT_CHECK_FOR_CROSS_GENERATION_LEAKS 1

/// A GenerationID is a unique identifier of a generation.
/// GenerationID can be used to ensure that objects referenced by the generation are
/// owned exclusively by that generation and have accidentally not leaked from
/// another generation.
class GenerationID {
  public:
    /// Constructor
    GenerationID();

    /// @returns a new. globally unique GenerationID
    static GenerationID New();

    /// Equality operator
    /// @param rhs the other GenerationID
    /// @returns true if the GenerationIDs are equal
    bool operator==(const GenerationID& rhs) const { return val == rhs.val; }

    /// Inequality operator
    /// @param rhs the other GenerationID
    /// @returns true if the GenerationIDs are not equal
    bool operator!=(const GenerationID& rhs) const { return val != rhs.val; }

    /// @returns the numerical identifier value
    uint32_t Value() const { return val; }

    /// @returns true if this GenerationID is valid
    explicit operator bool() const { return val != 0; }

  private:
    explicit GenerationID(uint32_t);

    uint32_t val = 0;
};

/// A simple pass-through function for GenerationID. Intended to be overloaded for
/// other types.
/// @param id a GenerationID
/// @returns id. Simple pass-through function
inline GenerationID GenerationIDOf(GenerationID id) {
    return id;
}

/// Writes the GenerationID to the stream.
/// @param out the stream to write to
/// @param id the generation identifier to write
/// @returns out so calls can be chained
template <typename STREAM>
    requires(traits::IsOStream<STREAM>)
auto& operator<<(STREAM& out, GenerationID id) {
    out << "Generation<" << id.Value() << ">";
    return out;
}

namespace detail {

/// AssertGenerationIDsEqual is called by TINT_ASSERT_GENERATION_IDS_EQUAL() and
/// TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID() to assert that the GenerationIDs
/// `a` and `b` are equal.
void AssertGenerationIDsEqual(GenerationID a,
                              GenerationID b,
                              bool if_valid,
                              const char* msg,
                              const char* file,
                              size_t line);

}  // namespace detail

/// TINT_ASSERT_GENERATION_IDS_EQUAL(A, B) is a macro that asserts that the
/// generation identifiers for A and B are equal.
///
/// TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(A, B) is a macro that asserts
/// that the generation identifiers for A and B are equal, if both A and B have
/// valid generation identifiers.
#if TINT_CHECK_FOR_CROSS_GENERATION_LEAKS
#define TINT_ASSERT_GENERATION_IDS_EQUAL(a, b)                                                 \
    tint::detail::AssertGenerationIDsEqual(GenerationIDOf(a), GenerationIDOf(b), false,        \
                                           "TINT_ASSERT_GENERATION_IDS_EQUAL(" #a ", " #b ")", \
                                           __FILE__, __LINE__)
#define TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(a, b) \
    tint::detail::AssertGenerationIDsEqual(             \
        GenerationIDOf(a), GenerationIDOf(b), true,     \
        "TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(" #a ", " #b ")", __FILE__, __LINE__)
#else
#define TINT_ASSERT_GENERATION_IDS_EQUAL(a, b) \
    do {                                       \
    } while (false)
#define TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(a, b) \
    do {                                                \
    } while (false)
#endif

}  // namespace tint

#endif  // SRC_TINT_UTILS_GENERATION_ID_H_
