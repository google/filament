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

#ifndef SRC_TINT_LANG_CORE_IR_VALUE_H_
#define SRC_TINT_LANG_CORE_IR_VALUE_H_

#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/containers/hashset.h"
#include "src/tint/utils/rtti/castable.h"

// Forward declarations
namespace tint::core::ir {
class CloneContext;
class Instruction;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// A specific usage of a Value in the IR.
struct Usage {
    /// The instruction that is using the value;
    Instruction* instruction = nullptr;
    /// The index of the operand that is the value being used.
    size_t operand_index = 0u;

    /// @returns the hash code of the Usage
    tint::HashCode HashCode() const { return Hash(instruction, operand_index); }

    /// An equality helper for Usage.
    /// @param other the usage to compare against
    /// @returns true if the two usages are equal
    bool operator==(const Usage& other) const {
        return instruction == other.instruction && operand_index == other.operand_index;
    }

    /// A comparison helper for Usage.
    /// @param other the usage to compare against
    /// @returns true if `this` is less then `other`.
    bool operator<(const Usage& other) const {
        if (instruction == nullptr && other.instruction != nullptr) {
            return false;
        }
        if (instruction != nullptr && other.instruction == nullptr) {
            return true;
        }
        if (instruction == other.instruction) {
            return operand_index < other.operand_index;
        }
        return instruction < other.instruction;
    }
};

/// Value in the IR.
class Value : public Castable<Value> {
  public:
    /// Destructor
    ~Value() override;

    /// @returns the type of the value
    virtual const core::type::Type* Type() const { return nullptr; }

    /// Destroys the Value. Once called, the Value must not be used again.
    /// The Value must not be in use by any instruction.
    virtual void Destroy();

    /// @param ctx the CloneContext used to clone this value
    /// @returns a clone of this value
    virtual Value* Clone(CloneContext& ctx) = 0;

    /// @returns true if the Value has not been destroyed with Destroy()
    bool Alive() const { return !flags_.Contains(Flag::kDead); }

    /// Adds a usage of this value.
    /// @param u the usage
    void AddUsage(Usage u) { uses_.Add(u); }

    /// Remove a usage of this value.
    /// @param u the usage
    void RemoveUsage(Usage u) { uses_.Remove(u); }

    /// @returns the set of usages of this value. An instruction may appear multiple times if it
    /// uses the value for multiple different operands.
    const Hashset<Usage, 4>& UsagesUnsorted() { return uses_; }

    /// @returns a sorted list of usages of this value. The usages are in the order of
    /// <instruction, operand index> where the instructions are ordered earliest instruction to
    /// latest and the operand indices from lowest to highest.
    Vector<Usage, 4> UsagesSorted() const;

    /// @returns true if this Value has any usages
    bool IsUsed() const { return !uses_.IsEmpty(); }

    /// @returns the number of usages of this Value
    size_t NumUsages() const { return uses_.Count(); }

    /// @returns true if the usages contains the instruction and operand index pair.
    /// @param instruction the instruction
    /// @param operand_index the in
    bool HasUsage(const Instruction* instruction, size_t operand_index) const {
        return uses_.Contains(Usage{const_cast<Instruction*>(instruction), operand_index});
    }

    /// Apply a function to all uses of the value that exist prior to calling this method. The uses
    /// are in unsorted ordered.
    /// @param func the function will be applied to each use
    void ForEachUseUnsorted(std::function<void(Usage use)> func) const;

    /// Apply a function to all uses of the value that exist prior to calling this method. The uses
    /// are sorted in (instruction,operand) order
    /// @param func the function will be applied to each use
    void ForEachUseSorted(std::function<void(Usage use)> func) const;

    /// Replace all uses of the value.
    /// @param replacer a function which returns a replacement for a given use
    void ReplaceAllUsesWith(std::function<Value*(Usage use)> replacer);

    /// Replace all uses of the value.
    /// @param replacement the replacement value
    void ReplaceAllUsesWith(Value* replacement);

  protected:
    /// Constructor
    Value();

  private:
    /// Flags applied to an Value
    enum class Flag {
        /// The value has been destroyed
        kDead,
    };

    Hashset<Usage, 4> uses_;

    /// Bitset of value flags
    tint::EnumSet<Flag> flags_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_VALUE_H_
