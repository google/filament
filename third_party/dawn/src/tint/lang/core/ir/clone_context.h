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

#ifndef SRC_TINT_LANG_CORE_IR_CLONE_CONTEXT_H_
#define SRC_TINT_LANG_CORE_IR_CLONE_CONTEXT_H_

#include "src/tint/utils/containers/const_propagating_ptr.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/rtti/traits.h"

namespace tint::core::ir {
class Block;
class Instruction;
class Module;
class Value;
}  // namespace tint::core::ir

namespace tint::core::ir {

/// Constant in the IR.
class CloneContext {
  public:
    /// @param module the IR module
    explicit CloneContext(Module& module);

    /// The IR module
    Module& ir;

    /// Performs a clone of @p what.
    /// @param what the item to clone
    /// @return the cloned item
    template <typename T>
    T* Clone(T* what) {
        if (auto replacement = replacements_.Get(what)) {
            return (*replacement)->template As<T>();
        }
        T* result = what->Clone(*this);
        Replace(what, result);
        return result;
    }

    /// Performs a clone of @p what.
    /// @param what the item to clone
    /// @return the cloned item
    template <typename T>
    T* Clone(ConstPropagatingPtr<T>& what) {
        return Clone(what.Get());
    }

    /// Performs a clone of all the elements in @p what.
    /// @param what the elements to clone
    /// @return the cloned elements
    template <size_t N, typename T>
    Vector<T*, N> Clone(Slice<T* const> what) {
        return Transform<N>(what, [&](T* const p) { return Clone(p); });
    }

    /// Performs a clone of all the elements in @p what.
    /// @param what the elements to clone
    /// @return the cloned elements
    template <size_t N, typename T>
    Vector<T*, N> Clone(Slice<T*> what) {
        return Transform<N>(what, [&](T* p) { return Clone(p); });
    }

    /// Performs a clone of all the elements in @p what.
    /// @param what the elements to clone
    /// @return the cloned elements
    template <size_t N, typename T>
    Vector<T*, N> Clone(Vector<T*, N> what) {
        return Transform(what, [&](T* p) { return Clone(p); });
    }

    /// Obtains the (potentially) remapped pointer to @p what
    /// @param what the item
    /// @return the cloned item for @p what, or the original pointer if @p what has not been cloned.
    template <typename T>
    T* Remap(T* what) {
        if (auto replacement = replacements_.Get(what)) {
            return (*replacement)->template As<T>();
        }
        return what;
    }

    /// Obtains the (potentially) remapped pointer to @p what
    /// @param what the item
    /// @return the cloned item for @p what, or the original pointer if @p what has not been cloned.
    template <typename T>
    T* Remap(ConstPropagatingPtr<T>& what) {
        return Remap(what.Get());
    }

    /// Obtains the (potentially) remapped pointer of all the elements in @p what.
    /// @param what the item
    /// @return the remapped elements
    template <size_t N, typename T>
    Vector<T*, N> Remap(Slice<T* const> what) {
        return Transform<N>(what, [&](T* const p) { return Remap(p); });
    }

    /// Obtains the (potentially) remapped pointer of all the elements in @p what.
    /// @param what the item
    /// @return the remapped elements
    template <size_t N, typename T>
    Vector<T*, N> Remap(Slice<T*> what) {
        return Transform<N>(what, [&](T* p) { return Remap(p); });
    }

    /// Obtains the (potentially) remapped pointer of all the elements in @p what.
    /// @param what the item
    /// @return the remapped elements
    template <size_t N, typename T>
    Vector<T*, N> Remap(Vector<T*, N> what) {
        return Transform(what, [&](T* p) { return Remap(p); });
    }

    /// Registers the replacement of `what` with `with`
    /// @param what the value or instruction to replace
    /// @param with a pointer to a replacement value or instruction
    template <typename WHAT, typename WITH>
    void Replace(WHAT* what, WITH* with) {
        static_assert(traits::IsTypeOrDerived<WHAT, ir::Instruction> ||
                      traits::IsTypeOrDerived<WHAT, ir::Value>);
        static_assert(traits::IsTypeOrDerived<WITH, WHAT>);
        TINT_ASSERT(with);
        replacements_.Add(what, with);
    }

  private:
    Hashmap<CastableBase*, CastableBase*, 8> replacements_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_CLONE_CONTEXT_H_
