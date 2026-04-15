// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_SEM_ARRAY_H_
#define SRC_TINT_LANG_WGSL_SEM_ARRAY_H_

#include "src/tint/lang/core/type/array.h"
#include "src/tint/utils/containers/unique_vector.h"

/// Forward declarations
namespace tint::sem {
class GlobalVariable;
}

namespace tint::sem {

/// Array holds the semantic information for Arrays.
class Array final : public Castable<Array, core::type::Array> {
  public:
    /// Constructor
    /// @param element the array element type
    /// @param count the number of elements in the array.
    /// @param size the byte size of the array. The size will be 0 if the array element count is
    ///        pipeline overridable.
    Array(core::type::Type const* element, const core::type::ArrayCount* count, uint32_t size);

    /// Destructor
    ~Array() override;

    /// Records that this variable (transitively) references the given override variable.
    /// @param var the module-scope override variable
    void AddTransitivelyReferencedOverride(const GlobalVariable* var);

    /// @returns all transitively referenced override variables
    VectorRef<const GlobalVariable*> TransitivelyReferencedOverrides() const {
        return transitively_referenced_overrides_;
    }

  private:
    UniqueVector<const GlobalVariable*, 4> transitively_referenced_overrides_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_ARRAY_H_
