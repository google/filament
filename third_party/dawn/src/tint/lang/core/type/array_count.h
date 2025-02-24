// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_CORE_TYPE_ARRAY_COUNT_H_
#define SRC_TINT_LANG_CORE_TYPE_ARRAY_COUNT_H_

#include <functional>
#include <string>

#include "src/tint/lang/core/type/clone_context.h"
#include "src/tint/lang/core/type/unique_node.h"
#include "src/tint/utils/symbol/symbol_table.h"

namespace tint::core::type {

/// An array count
class ArrayCount : public Castable<ArrayCount, UniqueNode> {
  public:
    ~ArrayCount() override;

    /// @returns the friendly name for this array count
    virtual std::string FriendlyName() const = 0;

    /// @param ctx the clone context
    /// @returns a clone of this type
    virtual ArrayCount* Clone(CloneContext& ctx) const = 0;

  protected:
    /// Constructor
    /// @param hash the unique hash of the node
    explicit ArrayCount(size_t hash);
};

/// The variant of an ArrayCount when the array is a const-expression.
/// Example:
/// ```
/// const N = 123;
/// type arr = array<i32, N>
/// ```
class ConstantArrayCount final : public Castable<ConstantArrayCount, ArrayCount> {
  public:
    /// Constructor
    /// @param val the constant-expression value
    explicit ConstantArrayCount(uint32_t val);
    ~ConstantArrayCount() override;

    /// @param other the other object
    /// @returns true if this array count is equal to other
    bool Equals(const UniqueNode& other) const override;

    /// @returns the friendly name for this array count
    std::string FriendlyName() const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    ConstantArrayCount* Clone(CloneContext& ctx) const override;

    /// The array count constant-expression value.
    uint32_t value;
};

/// The variant of an ArrayCount when the array is is runtime-sized.
/// Example:
/// ```
/// type arr = array<i32>
/// ```
class RuntimeArrayCount final : public Castable<RuntimeArrayCount, ArrayCount> {
  public:
    /// Constructor
    RuntimeArrayCount();
    ~RuntimeArrayCount() override;

    /// @param other the other object
    /// @returns true if this array count is equal to other
    bool Equals(const UniqueNode& other) const override;

    /// @returns the friendly name for this array count
    std::string FriendlyName() const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    RuntimeArrayCount* Clone(CloneContext& ctx) const override;
};

}  // namespace tint::core::type

#endif  // SRC_TINT_LANG_CORE_TYPE_ARRAY_COUNT_H_
