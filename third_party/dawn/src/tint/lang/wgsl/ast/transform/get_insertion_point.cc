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

#include "src/tint/lang/wgsl/ast/transform/get_insertion_point.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/sem/for_loop_statement.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::ast::transform::utils {

InsertionPoint GetInsertionPoint(program::CloneContext& ctx, const Statement* stmt) {
    auto& sem = ctx.src->Sem();
    using RetType = std::pair<const sem::BlockStatement*, const Statement*>;

    if (auto* sem_stmt = sem.Get(stmt)) {
        auto* parent = sem_stmt->Parent();
        return Switch(
            parent,
            [&](const sem::BlockStatement* block) -> RetType {
                // Common case, can insert in the current block above/below the input
                // statement.
                return {block, stmt};
            },
            [&](const sem::ForLoopStatement* fl) -> RetType {
                // `stmt` is either the for loop initializer or the continuing
                // statement of a for-loop.
                if (fl->Declaration()->initializer == stmt) {
                    // For loop init, can insert above the for loop itself.
                    return {fl->Block(), fl->Declaration()};
                }

                // Cannot insert before or after continuing statement of a for-loop
                return {};
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    return {};
}

}  // namespace tint::ast::transform::utils
