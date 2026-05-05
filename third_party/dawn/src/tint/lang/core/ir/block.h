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

#ifndef SRC_TINT_LANG_CORE_IR_BLOCK_H_
#define SRC_TINT_LANG_CORE_IR_BLOCK_H_

#include <utility>

#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/terminator.h"
#include "src/tint/utils/containers/vector.h"

// Forward declarations
namespace tint::core::ir {
class ControlInstruction;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// A block of statements. The instructions in the block are a linear list of instructions to
/// execute. The block will terminate with a Terminator instruction at the end.
/// The instructions are held in a doubly-linked list with explicit first and last pointer,
/// and with an explicit count.
class Block : public Castable<Block> {
  public:
    /// Constructor
    Block();
    ~Block() override;

    /// @param ctx the CloneContext used to clone this block
    /// @returns a clone of this block
    virtual Block* Clone(CloneContext& ctx);

    /// Clones the block contents into the given block
    /// @param ctx the CloneContext used to clone
    /// @param out the block to clone into
    virtual void CloneInto(CloneContext& ctx, Block* out);

    /// @return the terminator instruction for this block, or nullptr if this block does not end in
    /// a terminator.
    ir::Terminator* Terminator() { return tint::As<ir::Terminator>(instructions_.last); }

    /// @return the terminator instruction for this block, or nullptr if this block does not end in
    /// a terminator.
    const ir::Terminator* Terminator() const {
        return tint::As<ir::Terminator>(instructions_.last);
    }

    /// @returns the instructions in the block
    Instruction* Instructions() { return instructions_.first; }

    /// @returns the instructions in the block
    const Instruction* Instructions() const { return instructions_.first; }

    /// Iterator for the instructions inside a block
    template <typename T>
    class Iterator {
      public:
        /// Constructor
        /// @param inst the instruction to start iterating from
        explicit Iterator(T* inst) : inst_(inst) {}
        ~Iterator() = default;

        /// Dereference operator
        /// @returns the instruction for this iterator
        T* operator*() const { return inst_; }

        /// Comparison operator
        /// @param itr to compare against
        /// @returns true if this iterator and @p itr point to the same instruction
        bool operator==(const Iterator& itr) const { return itr.inst_ == inst_; }

        /// Not equal operator
        /// @param itr to compare against
        /// @returns true if this iterator and @p itr point to different instructions
        bool operator!=(const Iterator& itr) const { return !(*this == itr); }

        /// Increment operator
        /// @returns this iterator advanced to the next element
        Iterator& operator++() {
            inst_ = inst_->next;
            return *this;
        }

      private:
        T* inst_ = nullptr;
    };

    /// @returns the iterator pointing to the start of the instruction list
    Iterator<Instruction> begin() { return Iterator<Instruction>{instructions_.first}; }

    /// @returns the ending iterator
    Iterator<Instruction> end() { return Iterator<Instruction>{nullptr}; }

    /// @returns the iterator pointing to the start of the instruction list
    Iterator<const Instruction> begin() const {
        return Iterator<const Instruction>{instructions_.first};
    }

    /// @returns the ending iterator
    Iterator<const Instruction> end() const { return Iterator<const Instruction>{nullptr}; }

    /// @returns the first instruction in the instruction list
    Instruction* Front() { return instructions_.first; }

    /// @returns the first instruction in the instruction list
    const Instruction* Front() const { return instructions_.first; }

    /// @returns the last instruction in the instruction list
    Instruction* Back() { return instructions_.last; }

    /// @returns the last instruction in the instruction list
    const Instruction* Back() const { return instructions_.last; }

    /// Adds the instruction to the beginning of the block
    /// @param inst the instruction to add
    /// @returns the instruction to allow calls to be chained
    Instruction* Prepend(Instruction* inst);
    /// Adds the instruction to the end of the block
    /// @param inst the instruction to add
    /// @returns the instruction to allow calls to be chained
    Instruction* Append(Instruction* inst);
    /// Adds the new instruction before the given instruction
    /// @param before the instruction to insert before
    /// @param inst the instruction to insert
    void InsertBefore(Instruction* before, Instruction* inst);
    /// Adds the new instruction after the given instruction
    /// @param after the instruction to insert after
    /// @param inst the instruction to insert
    void InsertAfter(Instruction* after, Instruction* inst);
    /// Replaces the target instruction with the new instruction
    /// @param target the instruction to replace
    /// @param inst the instruction to insert
    void Replace(Instruction* target, Instruction* inst);
    /// Removes the target instruction
    /// @param inst the instruction to remove
    void Remove(Instruction* inst);

    /// @returns true if the block contains no instructions
    bool IsEmpty() const { return Length() == 0; }

    /// @returns the number of instructions in the block
    size_t Length() const { return instructions_.count; }

    /// @return the parent instruction that owns this block
    ControlInstruction* Parent() { return parent_; }

    /// @return the parent instruction that owns this block
    const ControlInstruction* Parent() const { return parent_; }

    /// @param parent the parent instruction that owns this block
    void SetParent(ControlInstruction* parent) { parent_ = parent; }

    /// Destroys the block and all of its instructions.
    void Destroy();

  private:
    // The first and last pointers are null if and only if the list is empty.
    struct {
        Instruction* first = nullptr;
        Instruction* last = nullptr;
        size_t count = 0;
    } instructions_;

    ControlInstruction* parent_ = nullptr;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_BLOCK_H_
