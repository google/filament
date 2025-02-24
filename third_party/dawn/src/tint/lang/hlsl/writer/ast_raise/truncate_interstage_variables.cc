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

#include "src/tint/lang/hlsl/writer/ast_raise/truncate_interstage_variables.h"

#include <memory>
#include <string>
#include <utility>

#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/text/unicode.h"

TINT_INSTANTIATE_TYPEINFO(tint::hlsl::writer::TruncateInterstageVariables);
TINT_INSTANTIATE_TYPEINFO(tint::hlsl::writer::TruncateInterstageVariables::Config);

namespace tint::hlsl::writer {

namespace {

struct TruncatedStructAndConverter {
    /// The symbol of the truncated structure.
    Symbol truncated_struct;
    /// The symbol of the helper function that takes the original structure as a single argument and
    /// returns the truncated structure type.
    Symbol truncate_fn;
};

}  // anonymous namespace

TruncateInterstageVariables::TruncateInterstageVariables() = default;
TruncateInterstageVariables::~TruncateInterstageVariables() = default;

ast::transform::Transform::ApplyResult TruncateInterstageVariables::Apply(
    const Program& src,
    const ast::transform::DataMap& config,
    ast::transform::DataMap&) const {
    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};

    const auto* data = config.Get<Config>();
    if (data == nullptr) {
        b.Diagnostics().AddError(Source{})
            << "missing transform data for "
            << tint::TypeInfo::Of<TruncateInterstageVariables>().name;
        return resolver::Resolve(b);
    }

    auto& sem = ctx.src->Sem();

    bool should_run = false;

    Hashmap<const sem::Function*, Symbol, 4u> entry_point_functions_to_truncate_functions;
    Hashmap<const sem::Struct*, TruncatedStructAndConverter, 4u>
        old_shader_io_structs_to_new_struct_and_truncate_functions;

    for (auto* func_ast : ctx.src->AST().Functions()) {
        if (!func_ast->IsEntryPoint()) {
            continue;
        }

        if (func_ast->PipelineStage() != ast::PipelineStage::kVertex) {
            // Currently only vertex stage could have interstage output variables that need
            // truncated.
            continue;
        }

        auto* func_sem = sem.Get(func_ast);
        auto* str = func_sem->ReturnType()->As<sem::Struct>();

        // This transform is run after CanonicalizeEntryPointIO transform,
        // So it is guaranteed that entry point inputs are already grouped in a struct.
        if (DAWN_UNLIKELY(!str)) {
            TINT_ICE() << "Entrypoint function return type is non-struct.\n"
                       << "TruncateInterstageVariables transform needs to run after "
                          "CanonicalizeEntryPointIO transform.";
        }

        // A prepass to check if any interstage variable locations in the entry point needs
        // truncating. If not we don't really need to handle this entry point.
        Hashset<const sem::StructMember*, 16u> omit_members;

        for (auto* member : str->Members()) {
            if (auto location = member->Attributes().location) {
                const size_t kMaxLocation = data->interstage_locations.size() - 1u;
                if (location.value() > kMaxLocation) {
                    b.Diagnostics().AddError(Source{})
                        << "The location (" << location.value() << ") of " << member->Name().Name()
                        << " in " << str->Name().Name() << " exceeds the maximum value ("
                        << kMaxLocation << ").";
                    return resolver::Resolve(b);
                }
                if (!data->interstage_locations.test(location.value())) {
                    omit_members.Add(member);
                }
            }
        }

        if (omit_members.IsEmpty()) {
            continue;
        }

        // Now we are sure the transform needs to be run.
        should_run = true;

        // Get or create a new truncated struct/truncate function for the interstage inputs &
        // outputs.
        auto entry = old_shader_io_structs_to_new_struct_and_truncate_functions.GetOrAdd(str, [&] {
            auto new_struct_sym = b.Symbols().New();

            Vector<const ast::StructMember*, 20> truncated_members;
            Vector<const ast::Expression*, 20> initializer_exprs;

            for (auto* member : str->Members()) {
                if (omit_members.Contains(member)) {
                    continue;
                }

                truncated_members.Push(ctx.Clone(member->Declaration()));
                initializer_exprs.Push(b.MemberAccessor("io", ctx.Clone(member->Name())));
            }

            // Create the new shader io struct.
            b.Structure(new_struct_sym, std::move(truncated_members));

            // Create the mapping function to truncate the shader io.
            auto mapping_fn_sym = b.Symbols().New("truncate_shader_output");
            b.Func(mapping_fn_sym, Vector{b.Param("io", ctx.Clone(func_ast->return_type))},
                   b.ty(new_struct_sym),
                   Vector{
                       b.Return(b.Call(new_struct_sym, std::move(initializer_exprs))),
                   });
            return TruncatedStructAndConverter{new_struct_sym, mapping_fn_sym};
        });

        ctx.Replace(func_ast->return_type.expr, b.Expr(entry.truncated_struct));

        entry_point_functions_to_truncate_functions.Add(func_sem, entry.truncate_fn);
    }

    if (!should_run) {
        return SkipTransform;
    }

    // Replace return statements with new truncated shader IO struct
    ctx.ReplaceAll(
        [&](const ast::ReturnStatement* return_statement) -> const ast::ReturnStatement* {
            auto* return_sem = sem.Get(return_statement);
            if (auto mapping_fn_sym =
                    entry_point_functions_to_truncate_functions.Get(return_sem->Function())) {
                return b.Return(return_statement->source,
                                b.Call(*mapping_fn_sym, ctx.Clone(return_statement->value)));
            }
            return nullptr;
        });

    // Remove IO attributes from old shader IO struct which is not used as entry point output
    // anymore.
    for (auto& it : old_shader_io_structs_to_new_struct_and_truncate_functions) {
        const ast::Struct* struct_ty = it.key->Declaration();
        for (auto* member : struct_ty->members) {
            for (auto* attr : member->attributes) {
                if (attr->IsAnyOf<ast::BuiltinAttribute, ast::LocationAttribute,
                                  ast::InterpolateAttribute, ast::InvariantAttribute>()) {
                    ctx.Remove(member->attributes, attr);
                }
            }
        }
    }

    ctx.Clone();
    return resolver::Resolve(b);
}

TruncateInterstageVariables::Config::Config() = default;

TruncateInterstageVariables::Config::Config(const Config&) = default;

TruncateInterstageVariables::Config::~Config() = default;

TruncateInterstageVariables::Config& TruncateInterstageVariables::Config::operator=(const Config&) =
    default;

}  // namespace tint::hlsl::writer
