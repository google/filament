// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_TINT_UTILS_SYMBOL_GENERATION_ID_H_
#define SRC_TINT_UTILS_SYMBOL_GENERATION_ID_H_

#include <stdint.h>

#include "src/tint/utils/rtti/traits.h"

namespace tint {

/// A GenerationID is a unique identifier of a generation.
///
/// GenerationID can be used to ensure that objects referenced by the generation are owned
/// exclusively by that generation and have accidentally not leaked from another generation.
class GenerationID {
  public:
    /// Constructor
    GenerationID();

    /// @returns a new globally unique GenerationID
    static GenerationID New();

    /// Equality operator
    /// @param rhs the other GenerationID
    /// @returns true if the GenerationIDs are equal
    bool operator==(const GenerationID& rhs) const { return val == rhs.val; }

    /// @returns true if this GenerationID is valid
    explicit operator bool() const { return val != 0; }

  private:
    explicit GenerationID(uint32_t);

    uint32_t val = 0;
};

}  // namespace tint

#endif  // SRC_TINT_UTILS_SYMBOL_GENERATION_ID_H_
