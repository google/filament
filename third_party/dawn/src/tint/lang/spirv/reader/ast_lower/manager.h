// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_SPIRV_READER_AST_LOWER_MANAGER_H_
#define SRC_TINT_LANG_SPIRV_READER_AST_LOWER_MANAGER_H_

#include <memory>
#include <utility>
#include <vector>

#include "src/tint/lang/spirv/reader/ast_lower/data.h"
#include "src/tint/lang/spirv/reader/ast_lower/transform.h"
#include "src/tint/lang/wgsl/program/program.h"

namespace tint::ast::transform {

/// A collection of Transforms that act as a single Transform.
/// The inner transforms will execute in the appended order.
/// If any inner transform fails the manager will return immediately and
/// the error can be retrieved with the Output's diagnostics.
class Manager {
  public:
    /// Constructor
    Manager();
    ~Manager();

    /// Add pass to the manager
    /// @param transform the transform to append
    void append(std::unique_ptr<Transform> transform) {
        transforms_.push_back(std::move(transform));
    }

    /// Add pass to the manager of type `T`, constructed with the provided
    /// arguments.
    /// @param args the arguments to forward to the `T` initializer
    template <typename T, typename... ARGS>
    void Add(ARGS&&... args) {
        transforms_.emplace_back(std::make_unique<T>(std::forward<ARGS>(args)...));
    }

    /// Runs the transforms on @p program, returning the transformed clone of @p program.
    /// @param program the source program to transform
    /// @param inputs optional extra transform-specific input data
    /// @param outputs optional extra transform-specific output data
    /// @returns the transformed program
    Program Run(const Program& program, const DataMap& inputs, DataMap& outputs) const;

  private:
    std::vector<std::unique_ptr<Transform>> transforms_;
};

}  // namespace tint::ast::transform

#endif  // SRC_TINT_LANG_SPIRV_READER_AST_LOWER_MANAGER_H_
