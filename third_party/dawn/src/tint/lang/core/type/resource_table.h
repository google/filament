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

#ifndef SRC_TINT_LANG_CORE_TYPE_RESOURCE_TABLE_H_
#define SRC_TINT_LANG_CORE_TYPE_RESOURCE_TABLE_H_

#include <string>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/core/type/type.h"

namespace tint::core::type {

/// ResourceTable represents an OpTypeRuntimeArray of resources
class ResourceTable final : public Castable<ResourceTable, core::type::Type> {
  public:
    /// Constructor
    /// @param binding_type the type of the table
    explicit ResourceTable(const core::type::Type* binding_type);

    /// @param other the other node to compare against
    /// @returns true if the this type is equal to @p other
    bool Equals(const UniqueNode& other) const override;

    const core::type::Type* GetBindingType() const { return binding_type_; }

    /// @copydoc core::type::Type::Elements
    core::type::TypeAndCount Elements(const core::type::Type* type_if_invalid = nullptr,
                                      uint32_t count_if_invalid = 0) const override;

    /// @copydoc core::type::Type::Element
    const core::type::Type* Element(uint32_t index) const override;

    /// @returns the friendly name for this type
    std::string FriendlyName() const override;

    bool IsHandle() const override { return true; }

    /// @param ctx the clone context
    /// @returns a clone of this type
    ResourceTable* Clone(core::type::CloneContext& ctx) const override;

  private:
    const core::type::Type* binding_type_;
};

}  // namespace tint::core::type

#endif  // SRC_TINT_LANG_CORE_TYPE_RESOURCE_TABLE_H_
