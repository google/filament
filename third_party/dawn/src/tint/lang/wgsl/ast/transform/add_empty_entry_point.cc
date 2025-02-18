// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/ast/transform/add_empty_entry_point.h"

#include <utility>

#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::AddEmptyEntryPoint);

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::ast::transform {
namespace {

bool ShouldRun(const Program& program) {
    for (auto* func : program.AST().Functions()) {
        if (func->IsEntryPoint()) {
            return false;
        }
    }
    return true;
}

}  // namespace

AddEmptyEntryPoint::AddEmptyEntryPoint() = default;

AddEmptyEntryPoint::~AddEmptyEntryPoint() = default;

Transform::ApplyResult AddEmptyEntryPoint::Apply(const Program& src,
                                                 const DataMap&,
                                                 DataMap&) const {
    if (!ShouldRun(src)) {
        return SkipTransform;
    }

    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};

    b.Func(b.Symbols().New("unused_entry_point"), {}, b.ty.void_(), {},
           tint::Vector{
               b.Stage(PipelineStage::kCompute),
               b.WorkgroupSize(1_i),
           });

    ctx.Clone();
    return resolver::Resolve(b);
}

}  // namespace tint::ast::transform
