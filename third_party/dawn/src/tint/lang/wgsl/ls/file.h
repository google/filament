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

#ifndef SRC_TINT_LANG_WGSL_LS_FILE_H_
#define SRC_TINT_LANG_WGSL_LS_FILE_H_

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "src/tint/lang/wgsl/ast/node.h"
#include "src/tint/lang/wgsl/ls/utils.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/sem/expression.h"
#include "src/tint/lang/wgsl/sem/load.h"
#include "src/tint/lang/wgsl/sem/materialize.h"
#include "src/tint/utils/diagnostic/source.h"

namespace tint::wgsl::ls {

/// File represents an open language-server WGSL file ("document").
class File {
  public:
    /// The source file
    std::unique_ptr<Source::File> source;
    /// The current version of the file. Incremented with each change.
    int64_t version = 0;
    /// The parsed and resolved Program
    Program program;
    /// A source-ordered list of AST nodes.
    std::vector<const ast::Node*> nodes;

    /// Constructor
    File(std::unique_ptr<Source::File>&& source_, int64_t version_, Program program_);

    /// @returns all the references to the symbol at the location @p l in the file.
    /// @param l the source location to lookup the symbol.
    /// @param include_declaration if true, the declaration of @p l will be included in the returned
    /// list.
    std::vector<Source::Range> References(Source::Location l, bool include_declaration);

    /// The result of Definition
    struct DefinitionResult {
        // The identifier text
        std::string text;
        // The source range of the definition identifier
        Source::Range definition;
        // The source range of the reference identifier
        Source::Range reference;
    };

    /// @returns the definition of the symbol at the location @p l in the file.
    std::optional<DefinitionResult> Definition(Source::Location l);

    /// Behaviour of NodeAt()
    enum class UnwrapMode {
        /// NodeAt will Unwrap() the semantic node when searching for the template type.
        kNoUnwrap,
        /// NodeAt will not Unwrap() the semantic node when searching for the template type.
        kUnwrap
    };

    // Default UnwrapMode is to unwrap, unless searching for a sem::Materialize or sem::Load
    template <typename T>
    static constexpr UnwrapMode DefaultUnwrapMode =
        (std::is_same_v<T, sem::Materialize> || std::is_same_v<T, sem::Load>)
            ? UnwrapMode::kNoUnwrap
            : UnwrapMode::kUnwrap;

    /// @returns the inner-most semantic node at the location @p l in the file.
    /// @tparam T the type or subtype of the node to scan for.
    template <typename T = sem::Node, UnwrapMode UNWRAP_MODE = DefaultUnwrapMode<T>>
    const T* NodeAt(Source::Location l) const {
        // TODO(crbug.com/tint/2127): This is a brute-force search. Optimize.
        // Suggested optimization: bin the intersecting ranges per-line.
        size_t best_len = std::numeric_limits<size_t>::max();
        const T* best_node = nullptr;
        for (auto* node : nodes) {
            if (node->source.range.begin <= l && node->source.range.end >= l) {
                auto* sem = program.Sem().Get(node);
                if constexpr (UNWRAP_MODE == UnwrapMode::kUnwrap) {
                    sem = Unwrap(sem);
                }
                if (auto* cast = As<T, CastFlags::kDontErrorOnImpossibleCast>(sem)) {
                    size_t len = node->source.range.Length(source->content);
                    if (len < best_len) {
                        best_len = len;
                        best_node = cast;
                    }
                }
            }
        }
        return best_node;
    }

    /// @return the zero-based langsvr::lsp::Position @p pos in utf-16 code points converted to a
    /// one-based tint::Source::Location in utf-8 code points.
    Source::Location Conv(langsvr::lsp::Position pos) const;

    /// @return the one-based tint::Source::Position @p loc in utf-8 code points converted to a
    /// zero-based langsvr::lsp::Position in utf-16 code points.
    langsvr::lsp::Position Conv(Source::Location loc) const;

    /// @return the one-based tint::Source::Range @p rng in utf-8 code points converted to a
    /// zero-based langsvr::lsp::Range in utf-16 code points.
    langsvr::lsp::Range Conv(Source::Range rng) const;

    /// @return the zero-based langsvr::lsp::Range @p rng in utf-16 code points converted to a
    /// one-based Source::Range in utf-8 code points.
    Source::Range Conv(langsvr::lsp::Range rng) const;
};

}  // namespace tint::wgsl::ls

#endif  // SRC_TINT_LANG_WGSL_LS_FILE_H_
