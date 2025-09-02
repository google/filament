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

#ifndef SRC_TINT_LANG_WGSL_PROGRAM_CLONE_CONTEXT_H_
#define SRC_TINT_LANG_WGSL_PROGRAM_CLONE_CONTEXT_H_

#include <utility>

#include "src/tint/lang/wgsl/ast/clone_context.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/program/program_builder.h"

// Forward declarations
namespace tint::program {

/// CloneContext holds the state used while cloning Programs.
class CloneContext {
  public:
    /// SymbolTransform is a function that takes a symbol and returns a new
    /// symbol.
    using SymbolTransform = std::function<Symbol(Symbol)>;

    /// Constructor for cloning objects from `from` into `to`.
    /// @param to the target Builder to clone into
    /// @param from the source Program to clone from
    /// @param auto_clone_symbols clone all symbols in `from` before returning
    CloneContext(ProgramBuilder* to, Program const* from, bool auto_clone_symbols = true);

    /// Destructor
    ~CloneContext();

    /// @copybrief ast::CloneContext::Clone
    /// @param args the arguments to forward to ast::CloneContext::Clone
    /// @return the cloned node
    template <typename... ARGS>
    auto Clone(ARGS&&... args) {
        return ctx_.Clone(std::forward<ARGS>(args)...);
    }

    /// @copybrief ast::CloneContext::CloneWithoutTransform
    /// @param args the arguments to forward to ast::CloneContext::CloneWithoutTransform
    /// @return the cloned node
    template <typename... ARGS>
    auto CloneWithoutTransform(ARGS&&... args) {
        return ctx_.CloneWithoutTransform(std::forward<ARGS>(args)...);
    }

    /// @copybrief ast::CloneContext::ReplaceAll
    /// @param args the arguments to forward to ast::CloneContext::ReplaceAll
    /// @return this CloneContext so calls can be chained
    template <typename... ARGS>
    CloneContext& ReplaceAll(ARGS&&... args) {
        ctx_.ReplaceAll(std::forward<ARGS>(args)...);
        return *this;
    }

    /// @copydoc ast::CloneContext::Replace
    template <typename WHAT, typename WITH>
        requires(traits::IsTypeOrDerived<WITH, ast::Node>)
    CloneContext& Replace(const WHAT* what, const WITH* with) {
        ctx_.Replace<WHAT, WITH>(what, with);
        return *this;
    }

    /// @copydoc ast::CloneContext::Replace
    template <typename WHAT, typename WITH, typename = std::invoke_result_t<WITH>>
    CloneContext& Replace(const WHAT* what, WITH&& with) {
        ctx_.Replace<WHAT, WITH>(what, std::forward<WITH>(with));
        return *this;
    }

    /// @copybrief ast::CloneContext::Remove
    /// @param args the arguments to forward to ast::CloneContext::Remove
    /// @return this CloneContext so calls can be chained
    template <typename... ARGS>
    CloneContext& Remove(ARGS&&... args) {
        ctx_.Remove(std::forward<ARGS>(args)...);
        return *this;
    }

    /// @copybrief ast::CloneContext::InsertFront
    /// @param args the arguments to forward to ast::CloneContext::InsertFront
    /// @return this CloneContext so calls can be chained
    template <typename... ARGS>
    CloneContext& InsertFront(ARGS&&... args) {
        ctx_.InsertFront(std::forward<ARGS>(args)...);
        return *this;
    }

    /// @copybrief ast::CloneContext::InsertBack
    /// @param args the arguments to forward to ast::CloneContext::InsertBack
    /// @return this CloneContext so calls can be chained
    template <typename... ARGS>
    CloneContext& InsertBack(ARGS&&... args) {
        ctx_.InsertBack(std::forward<ARGS>(args)...);
        return *this;
    }

    /// @copybrief ast::CloneContext::InsertBefore
    /// @param args the arguments to forward to ast::CloneContext::InsertBefore
    /// @return this CloneContext so calls can be chained
    template <typename... ARGS>
    CloneContext& InsertBefore(ARGS&&... args) {
        ctx_.InsertBefore(std::forward<ARGS>(args)...);
        return *this;
    }

    /// @copybrief ast::CloneContext::InsertAfter
    /// @param args the arguments to forward to ast::CloneContext::InsertAfter
    /// @return this CloneContext so calls can be chained
    template <typename... ARGS>
    CloneContext& InsertAfter(ARGS&&... args) {
        ctx_.InsertAfter(std::forward<ARGS>(args)...);
        return *this;
    }

    /// Clone performs the clone of the Program's AST nodes, types and symbols
    /// from #src to #dst. Semantic nodes are not cloned, as these will be rebuilt
    /// when the Builder #dst builds its Program.
    void Clone();

    /// @returns the ast::CloneContext
    inline operator ast::CloneContext&() { return ctx_; }

    /// The target Builder to clone into.
    ProgramBuilder* const dst;

    /// The source Program to clone from.
    Program const* const src;

  private:
    ast::CloneContext ctx_;
};

}  // namespace tint::program

#endif  // SRC_TINT_LANG_WGSL_PROGRAM_CLONE_CONTEXT_H_
