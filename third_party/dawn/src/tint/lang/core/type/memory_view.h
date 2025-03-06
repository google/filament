// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_TYPE_MEMORY_VIEW_H_
#define SRC_TINT_LANG_CORE_TYPE_MEMORY_VIEW_H_

#include <string>

#include "src/tint/lang/core/access.h"
#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/type/type.h"

namespace tint::core::type {

/// Base class for memory view types: pointers and references
class MemoryView : public Castable<MemoryView, Type> {
  public:
    /// Constructor
    /// @param hash the unique hash of the node
    /// @param address_space the address space of the memory view
    /// @param store_type the store type
    /// @param access the resolved access control of the reference
    MemoryView(size_t hash,
               core::AddressSpace address_space,
               const Type* store_type,
               core::Access access);

    /// Destructor
    ~MemoryView() override;

    /// @returns the store type of the memory view
    const Type* StoreType() const { return store_type_; }

    /// @returns the address space of the memory view
    core::AddressSpace AddressSpace() const { return address_space_; }

    /// @returns the access control of the memory view
    core::Access Access() const { return access_; }

  private:
    Type const* const store_type_;
    core::AddressSpace const address_space_;
    core::Access const access_;
};

}  // namespace tint::core::type

#endif  // SRC_TINT_LANG_CORE_TYPE_MEMORY_VIEW_H_
