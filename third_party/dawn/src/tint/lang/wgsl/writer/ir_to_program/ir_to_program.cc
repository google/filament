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

#include "src/tint/lang/wgsl/writer/ir_to_program/ir_to_program.h"

#include <string>
#include <tuple>
#include <utility>

#include "src/tint/lang/core/builtin_type.h"
#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/bitcast.h"
#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/break_if.h"
#include "src/tint/lang/core/ir/call.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/continue.h"
#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/discard.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/override.h"
#include "src/tint/lang/core/ir/phony.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/store_vector_element.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/unary.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/texel_format.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/wgsl/ast/type.h"
#include "src/tint/lang/wgsl/common/reserved_words.h"
#include "src/tint/lang/wgsl/ir/builtin_call.h"
#include "src/tint/lang/wgsl/ir/unary.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/predicates.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/math/math.h"
#include "src/tint/utils/rtti/switch.h"

TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

// Helper for incrementing nesting_depth_ and then decrementing nesting_depth_ at the end
// of the scope that holds the call.
#define SCOPED_NESTING() \
    nesting_depth_++;    \
    TINT_DEFER(nesting_depth_--)

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::wgsl::writer {
namespace {

class State {
  public:
    explicit State(const core::ir::Module& m) : mod(m) {}

    Program Run(const ProgramOptions& options) {
        core::ir::Capabilities caps{core::ir::Capability::kAllowRefTypes,
                                    core::ir::Capability::kAllowOverrides,
                                    core::ir::Capability::kAllowPhonyInstructions};
        if (auto res = core::ir::Validate(mod, caps); res != Success) {
            // IR module failed validation.
            b.Diagnostics().AddError(Source{}) << res.Failure();
            return Program{resolver::Resolve(b)};
        }

        // Clone all symbols before we start.
        // This ensures that we preserve the names of named values and prevents unnamed values
        // receiving names that would conflict with named values that are emitted later than them.
        mod.symbols.Foreach([&](Symbol s) {  //
            b.Symbols().New(s.Name());
        });

        RootBlock(mod.root_block);

        // TODO(crbug.com/tint/1902): Emit user-declared types
        for (auto& fn : mod.functions) {
            Fn(fn);
        }

        if (options.allow_non_uniform_derivatives) {
            // Suppress errors regarding non-uniform derivative operations if requested, by adding a
            // diagnostic directive to the module.
            b.DiagnosticDirective(wgsl::DiagnosticSeverity::kOff, "derivative_uniformity");
        }

        return Program{resolver::Resolve(b, options.allowed_features)};
    }

  private:
    /// The source IR module
    const core::ir::Module& mod;

    /// The target ProgramBuilder
    ProgramBuilder b;

    /// The structure for a value held by a 'let', 'var' or parameter.
    struct VariableValue {
        Symbol name;  // Name of the variable
    };

    /// The structure for an inlined value
    struct InlinedValue {
        const ast::Expression* expr = nullptr;
    };

    /// Empty struct used as a sentinel value to indicate that an ast::Value has been consumed by
    /// its single place of usage. Attempting to use this value a second time should result in an
    /// ICE.
    struct ConsumedValue {};

    using ValueBinding = std::variant<VariableValue, InlinedValue, ConsumedValue>;

    /// IR values to their representation
    Hashmap<const core::ir::Value*, ValueBinding, 32> bindings_;

    /// Names for values
    Hashmap<const core::ir::Value*, Symbol, 32> names_;

    /// The nesting depth of the currently generated AST
    /// 0  is module scope
    /// 1  is root-level function scope
    /// 2+ is within control flow
    uint32_t nesting_depth_ = 0;

    using StatementList =
        Vector<const ast::Statement*, decltype(ast::BlockStatement::statements)::static_length>;
    StatementList* statements_ = nullptr;

    /// The current switch case block
    const core::ir::Block* current_switch_case_ = nullptr;

    /// Set of enable directives emitted.
    Hashset<wgsl::Extension, 4> enables_;

    /// Map of struct to output program name.
    Hashmap<const core::type::Struct*, Symbol, 8> structs_;

    /// True if 'diagnostic(off, derivative_uniformity)' has been emitted
    bool disabled_derivative_uniformity_ = false;

    void RootBlock(const core::ir::Block* root) {
        for (auto* inst : *root) {
            tint::Switch(
                inst,                                                               //
                [&](const core::ir::Var* var) { Var(var); },                        //
                [&](const core::ir::Override* override_) { Override(override_); },  //
                [&](const core::ir::Binary* binary) { Binary(binary); },            //
                TINT_ICE_ON_NO_MATCH);
        }
    }
    const ast::Function* Fn(const core::ir::Function* fn) {
        SCOPED_NESTING();

        // Emit parameters.
        static constexpr size_t N = decltype(ast::Function::params)::static_length;
        auto params = tint::Transform<N>(fn->Params(), [&](const core::ir::FunctionParam* param) {
            auto ty = Type(param->Type());
            auto name = NameFor(param);
            Vector<const ast::Attribute*, 1> attrs{};
            Bind(param, name);

            // Emit parameter attributes.
            if (auto builtin = param->Builtin()) {
                switch (builtin.value()) {
                    case core::BuiltinValue::kVertexIndex:
                        attrs.Push(b.Builtin(core::BuiltinValue::kVertexIndex));
                        break;
                    case core::BuiltinValue::kInstanceIndex:
                        attrs.Push(b.Builtin(core::BuiltinValue::kInstanceIndex));
                        break;
                    case core::BuiltinValue::kPosition:
                        attrs.Push(b.Builtin(core::BuiltinValue::kPosition));
                        break;
                    case core::BuiltinValue::kFrontFacing:
                        attrs.Push(b.Builtin(core::BuiltinValue::kFrontFacing));
                        break;
                    case core::BuiltinValue::kLocalInvocationId:
                        attrs.Push(b.Builtin(core::BuiltinValue::kLocalInvocationId));
                        break;
                    case core::BuiltinValue::kLocalInvocationIndex:
                        attrs.Push(b.Builtin(core::BuiltinValue::kLocalInvocationIndex));
                        break;
                    case core::BuiltinValue::kGlobalInvocationId:
                        attrs.Push(b.Builtin(core::BuiltinValue::kGlobalInvocationId));
                        break;
                    case core::BuiltinValue::kWorkgroupId:
                        attrs.Push(b.Builtin(core::BuiltinValue::kWorkgroupId));
                        break;
                    case core::BuiltinValue::kNumWorkgroups:
                        attrs.Push(b.Builtin(core::BuiltinValue::kNumWorkgroups));
                        break;
                    case core::BuiltinValue::kSampleIndex:
                        attrs.Push(b.Builtin(core::BuiltinValue::kSampleIndex));
                        break;
                    case core::BuiltinValue::kSampleMask:
                        attrs.Push(b.Builtin(core::BuiltinValue::kSampleMask));
                        break;
                    case core::BuiltinValue::kSubgroupInvocationId:
                        Enable(wgsl::Extension::kSubgroups);
                        attrs.Push(b.Builtin(core::BuiltinValue::kSubgroupInvocationId));
                        break;
                    case core::BuiltinValue::kSubgroupSize:
                        Enable(wgsl::Extension::kSubgroups);
                        attrs.Push(b.Builtin(core::BuiltinValue::kSubgroupSize));
                        break;
                    case core::BuiltinValue::kClipDistances:
                        Enable(wgsl::Extension::kClipDistances);
                        attrs.Push(b.Builtin(core::BuiltinValue::kClipDistances));
                        break;
                    default:
                        TINT_UNIMPLEMENTED() << builtin.value();
                }
            }
            if (auto loc = param->Location()) {
                attrs.Push(b.Location(u32(loc.value())));
            }
            if (auto color = param->Color()) {
                Enable(wgsl::Extension::kChromiumExperimentalFramebufferFetch);
                attrs.Push(b.Color(u32(color.value())));
            }
            if (auto interp = param->Interpolation()) {
                attrs.Push(b.Interpolate(interp->type, interp->sampling));
            }
            if (param->Invariant()) {
                attrs.Push(b.Invariant());
            }

            return b.Param(name, ty, std::move(attrs));
        });

        auto name = NameFor(fn);
        auto ret_ty = Type(fn->ReturnType());
        auto* body = Block(fn->Block());
        Vector<const ast::Attribute*, 1> attrs{};
        Vector<const ast::Attribute*, 1> ret_attrs{};

        // Emit entry point attributes.
        switch (fn->Stage()) {
            case core::ir::Function::PipelineStage::kUndefined:
                break;
            case core::ir::Function::PipelineStage::kCompute: {
                auto wgsize = fn->WorkgroupSize().value();
                attrs.Push(b.Stage(ast::PipelineStage::kCompute));
                attrs.Push(b.WorkgroupSize(Expr(wgsize[0]), Expr(wgsize[1]), Expr(wgsize[2])));
                break;
            }
            case core::ir::Function::PipelineStage::kFragment:
                attrs.Push(b.Stage(ast::PipelineStage::kFragment));
                break;
            case core::ir::Function::PipelineStage::kVertex:
                attrs.Push(b.Stage(ast::PipelineStage::kVertex));
                break;
        }

        // Emit return type attributes.
        if (auto builtin = fn->ReturnBuiltin()) {
            switch (builtin.value()) {
                case core::BuiltinValue::kPosition:
                    ret_attrs.Push(b.Builtin(core::BuiltinValue::kPosition));
                    break;
                case core::BuiltinValue::kFragDepth:
                    ret_attrs.Push(b.Builtin(core::BuiltinValue::kFragDepth));
                    break;
                case core::BuiltinValue::kSampleMask:
                    ret_attrs.Push(b.Builtin(core::BuiltinValue::kSampleMask));
                    break;
                default:
                    TINT_UNIMPLEMENTED() << builtin.value();
            }
        }
        if (auto loc = fn->ReturnLocation()) {
            ret_attrs.Push(b.Location(u32(loc.value())));
        }
        if (auto interp = fn->ReturnInterpolation()) {
            ret_attrs.Push(b.Interpolate(interp->type, interp->sampling));
        }
        if (fn->ReturnInvariant()) {
            ret_attrs.Push(b.Invariant());
        }

        return b.Func(name, std::move(params), ret_ty, body, std::move(attrs),
                      std::move(ret_attrs));
    }

    const ast::BlockStatement* Block(const core::ir::Block* block) {
        // TODO(crbug.com/tint/1902): Handle block arguments.
        return b.Block(Statements(block));
    }

    StatementList Statements(const core::ir::Block* block) {
        StatementList stmts;
        if (block) {
            TINT_SCOPED_ASSIGNMENT(statements_, &stmts);
            for (auto* inst : *block) {
                Instruction(inst);
            }
        }
        return stmts;
    }

    void Append(const ast::Statement* inst) { statements_->Push(inst); }

    void Instruction(const core::ir::Instruction* inst) {
        tint::Switch(
            inst,                                                                   //
            [&](const core::ir::Access* i) { Access(i); },                          //
            [&](const core::ir::Binary* i) { Binary(i); },                          //
            [&](const core::ir::BreakIf* i) { BreakIf(i); },                        //
            [&](const core::ir::Call* i) { Call(i); },                              //
            [&](const core::ir::Continue*) {},                                      //
            [&](const core::ir::ExitIf*) {},                                        //
            [&](const core::ir::ExitLoop* i) { ExitLoop(i); },                      //
            [&](const core::ir::ExitSwitch* i) { ExitSwitch(i); },                  //
            [&](const core::ir::If* i) { If(i); },                                  //
            [&](const core::ir::Let* i) { Let(i); },                                //
            [&](const core::ir::Load* l) { Load(l); },                              //
            [&](const core::ir::LoadVectorElement* i) { LoadVectorElement(i); },    //
            [&](const core::ir::Loop* l) { Loop(l); },                              //
            [&](const core::ir::NextIteration*) {},                                 //
            [&](const core::ir::Phony* i) { Phony(i); },                            //
            [&](const core::ir::Return* i) { Return(i); },                          //
            [&](const core::ir::Store* i) { Store(i); },                            //
            [&](const core::ir::StoreVectorElement* i) { StoreVectorElement(i); },  //
            [&](const core::ir::Switch* i) { Switch(i); },                          //
            [&](const core::ir::Swizzle* i) { Swizzle(i); },                        //
            [&](const core::ir::Unary* i) { Unary(i); },                            //
            [&](const core::ir::Unreachable*) {},                                   //
            [&](const core::ir::Var* i) { Var(i); },                                //
            TINT_ICE_ON_NO_MATCH);
    }

    void If(const core::ir::If* if_) {
        SCOPED_NESTING();

        auto true_stmts = Statements(if_->True());
        auto false_stmts = Statements(if_->False());
        if (AsShortCircuit(if_, true_stmts, false_stmts)) {
            return;
        }

        auto* cond = Expr(if_->Condition());
        auto* true_block = b.Block(std::move(true_stmts));

        switch (false_stmts.Length()) {
            case 0:
                Append(b.If(cond, true_block));
                return;
            case 1:
                if (auto* else_if = false_stmts.Front()->As<ast::IfStatement>()) {
                    Append(b.If(cond, true_block, b.Else(else_if)));
                    return;
                }
                break;
        }

        auto* false_block = b.Block(std::move(false_stmts));
        Append(b.If(cond, true_block, b.Else(false_block)));
    }

    void Loop(const core::ir::Loop* l) {
        SCOPED_NESTING();

        // Build all the initializer statements
        auto init_stmts = Statements(l->Initializer());

        // If there's a single initializer statement and meets the WGSL 'for_init' pattern, then
        // this can be used as the initializer for a for-loop.
        // @see https://www.w3.org/TR/WGSL/#syntax-for_init
        auto* init = (init_stmts.Length() == 1) &&
                             init_stmts.Front()
                                 ->IsAnyOf<ast::VariableDeclStatement, ast::AssignmentStatement,
                                           ast::CompoundAssignmentStatement,
                                           ast::IncrementDecrementStatement, ast::CallStatement>()
                         ? init_stmts.Front()
                         : nullptr;

        // Build the loop body statements. If the loop body starts with a if with the following
        // pattern, then treat it as the loop condition:
        //   if cond {
        //     block { exit_if   }
        //     block { exit_loop }
        //   }
        const ast::Expression* cond = nullptr;
        StatementList body_stmts;
        {
            TINT_SCOPED_ASSIGNMENT(statements_, &body_stmts);
            for (auto* inst : *l->Body()) {
                if (body_stmts.IsEmpty() && !cond) {
                    if (auto* if_ = inst->As<core::ir::If>()) {
                        if (if_->Results().IsEmpty() &&                          //
                            if_->True()->Length() == 1 &&                        //
                            if_->False()->Length() == 1 &&                       //
                            tint::Is<core::ir::ExitIf>(if_->True()->Front()) &&  //
                            tint::Is<core::ir::ExitLoop>(if_->False()->Front())) {
                            // Matched the loop condition.
                            cond = Expr(if_->Condition());
                            continue;  // Don't emit this as an instruction in the body.
                        }
                    }
                }

                // Process the loop body instruction. Append to 'body_stmts'
                Instruction(inst);
            }
        }

        // Build any continuing statements
        auto cont_stmts = Statements(l->Continuing());
        // If there's a single continuing statement and meets the WGSL 'for_update' pattern then
        // this can be used as the continuing for a for-loop.
        // @see https://www.w3.org/TR/WGSL/#syntax-for_update
        auto* cont =
            (cont_stmts.Length() == 1) &&
                    cont_stmts.Front()
                        ->IsAnyOf<ast::AssignmentStatement, ast::CompoundAssignmentStatement,
                                  ast::IncrementDecrementStatement, ast::CallStatement>()
                ? cont_stmts.Front()
                : nullptr;

        // Depending on 'init', 'cond' and 'cont', build a 'for', 'while' or 'loop'
        const ast::Statement* loop = nullptr;
        if ((!cont && !cont_stmts.IsEmpty())  // Non-trivial continuing
            || !cond                          // or non-trivial or no condition
        ) {
            // Build a loop
            if (cond) {
                body_stmts.Insert(0, b.If(b.Not(cond), b.Block(b.Break())));
            }
            auto* body = b.Block(std::move(body_stmts));
            loop = cont_stmts.IsEmpty() ? b.Loop(body)  //
                                        : b.Loop(body, b.Block(std::move(cont_stmts)));
            if (!init_stmts.IsEmpty()) {
                init_stmts.Push(loop);
                loop = b.Block(std::move(init_stmts));
            }
        } else if (init || cont) {
            // Build a for-loop
            auto* body = b.Block(std::move(body_stmts));
            loop = b.For(init, cond, cont, body);
            if (!init && !init_stmts.IsEmpty()) {
                init_stmts.Push(loop);
                loop = b.Block(std::move(init_stmts));
            }
        } else {
            // Build a while-loop
            auto* body = b.Block(std::move(body_stmts));
            loop = b.While(cond, body);
            if (!init_stmts.IsEmpty()) {
                init_stmts.Push(loop);
                loop = b.Block(std::move(init_stmts));
            }
        }
        statements_->Push(loop);
    }

    void Switch(const core::ir::Switch* s) {
        SCOPED_NESTING();

        auto* cond = Expr(s->Condition());

        auto cases = tint::Transform<4>(
            s->Cases(),  //
            [&](const core::ir::Switch::Case& c) -> const tint::ast::CaseStatement* {
                SCOPED_NESTING();

                const ast::BlockStatement* body = nullptr;
                {
                    TINT_SCOPED_ASSIGNMENT(current_switch_case_, c.block);
                    body = Block(c.block);
                }

                auto selectors = tint::Transform(c.selectors,  //
                                                 [&](const core::ir::Switch::CaseSelector& cs) {
                                                     return cs.IsDefault()
                                                                ? b.DefaultCaseSelector()
                                                                : b.CaseSelector(Expr(cs.val));
                                                 });
                return b.Case(std::move(selectors), body);
            });

        Append(b.Switch(cond, std::move(cases)));
    }

    void ExitSwitch(const core::ir::ExitSwitch* e) {
        if (current_switch_case_ && current_switch_case_->Terminator() == e) {
            return;  // No need to emit
        }
        Append(b.Break());
    }

    void ExitLoop(const core::ir::ExitLoop*) { Append(b.Break()); }

    void BreakIf(const core::ir::BreakIf* i) { Append(b.BreakIf(Expr(i->Condition()))); }

    void Return(const core::ir::Return* ret) {
        if (ret->Args().IsEmpty()) {
            // Return has no arguments.
            // If this block is nested withing some control flow, then we must
            // emit a 'return' statement, otherwise we've just naturally reached
            // the end of the function where the 'return' is redundant.
            if (nesting_depth_ > 1) {
                Append(b.Return());
            }
            return;
        }

        // Return has arguments - this is the return value.
        if (ret->Args().Length() != 1) {
            TINT_ICE() << "expected 1 value for return, got " << ret->Args().Length();
        }

        Append(b.Return(Expr(ret->Args().Front())));
    }

    void Var(const core::ir::Var* var) {
        auto* val = var->Result();
        auto* ref = As<core::type::Reference>(val->Type());
        TINT_ASSERT(ref /* converted by PtrToRef */);
        auto ty = Type(ref->StoreType());
        Symbol name = NameFor(var->Result());
        Bind(var->Result(), name);

        Vector<const ast::Attribute*, 4> attrs;
        if (auto bp = var->BindingPoint()) {
            attrs.Push(b.Group(u32(bp->group)));
            attrs.Push(b.Binding(u32(bp->binding)));
        }

        if (auto ii = var->InputAttachmentIndex()) {
            attrs.Push(b.InputAttachmentIndex(u32(ii.value())));
        }

        const ast::Expression* init = nullptr;
        if (var->Initializer()) {
            init = Expr(var->Initializer());
        }
        switch (ref->AddressSpace()) {
            case core::AddressSpace::kFunction:
                Append(b.Decl(b.Var(name, ty, init, std::move(attrs))));
                return;
            case core::AddressSpace::kStorage:
                b.GlobalVar(name, ty, init, ref->Access(), ref->AddressSpace(), std::move(attrs));
                return;
            case core::AddressSpace::kHandle:
                b.GlobalVar(name, ty, init, std::move(attrs));
                return;
            case core::AddressSpace::kPixelLocal:
                Enable(wgsl::Extension::kChromiumExperimentalPixelLocal);
                b.GlobalVar(name, ty, init, ref->AddressSpace(), std::move(attrs));
                return;
            case core::AddressSpace::kPushConstant:
                Enable(wgsl::Extension::kChromiumExperimentalPushConstant);
                b.GlobalVar(name, ty, init, ref->AddressSpace(), std::move(attrs));
                return;
            default:
                b.GlobalVar(name, ty, init, ref->AddressSpace(), std::move(attrs));
                return;
        }
    }

    void Override(const core::ir::Override* override_) {
        auto* val = override_->Result();
        Symbol name = NameFor(override_->Result());
        Bind(override_->Result(), name);

        Vector<const ast::Attribute*, 4> attrs;
        if (override_->OverrideId().has_value()) {
            attrs.Push(b.Id(override_->OverrideId().value()));
        }

        auto ty = Type(val->Type());
        const ast::Expression* init = nullptr;
        if (override_->Initializer()) {
            init = Expr(override_->Initializer());
        }
        b.Override(name, ty, init, attrs);
    }

    void Let(const core::ir::Let* let) {
        auto* result = let->Result();
        Symbol name = NameFor(result);
        Append(b.Decl(b.Let(name, Expr(let->Value()))));
        Bind(result, name);
    }

    void Phony(const core::ir::Phony* phony) { Append(b.Assign(b.Phony(), Expr(phony->Value()))); }

    void Store(const core::ir::Store* store) {
        auto* dst = Expr(store->To());
        auto* src = Expr(store->From());
        Append(b.Assign(dst, src));
    }

    void StoreVectorElement(const core::ir::StoreVectorElement* store) {
        auto* ptr = Expr(store->To());
        auto* val = Expr(store->Value());
        Append(b.Assign(VectorMemberAccess(ptr, store->Index()), val));
    }

    void Call(const core::ir::Call* call) {
        auto args = tint::Transform<4>(call->Args(), [&](const core::ir::Value* arg) {
            // Pointer-like arguments are passed by pointer, never reference.
            return Expr(arg);
        });
        tint::Switch(
            call,  //
            [&](const core::ir::UserCall* c) {
                auto* expr = b.Call(NameFor(c->Target()), std::move(args));
                if (call->Results().IsEmpty() || !call->Result()->IsUsed()) {
                    Append(b.CallStmt(expr));
                    return;
                }
                Bind(c->Result(), expr);
            },
            [&](const wgsl::ir::BuiltinCall* c) {
                if (!disabled_derivative_uniformity_ && RequiresDerivativeUniformity(c->Func())) {
                    // TODO(crbug.com/tint/1985): Be smarter about disabling derivative uniformity.
                    b.DiagnosticDirective(wgsl::DiagnosticSeverity::kOff,
                                          wgsl::CoreDiagnosticRule::kDerivativeUniformity);
                    disabled_derivative_uniformity_ = true;
                }

                switch (c->Func()) {
                    case wgsl::BuiltinFn::kSubgroupBallot:
                    case wgsl::BuiltinFn::kSubgroupElect:
                    case wgsl::BuiltinFn::kSubgroupBroadcast:
                    case wgsl::BuiltinFn::kSubgroupBroadcastFirst:
                    case wgsl::BuiltinFn::kSubgroupShuffle:
                    case wgsl::BuiltinFn::kSubgroupShuffleXor:
                    case wgsl::BuiltinFn::kSubgroupShuffleUp:
                    case wgsl::BuiltinFn::kSubgroupShuffleDown:
                    case wgsl::BuiltinFn::kSubgroupAdd:
                    case wgsl::BuiltinFn::kSubgroupInclusiveAdd:
                    case wgsl::BuiltinFn::kSubgroupExclusiveAdd:
                    case wgsl::BuiltinFn::kSubgroupMul:
                    case wgsl::BuiltinFn::kSubgroupInclusiveMul:
                    case wgsl::BuiltinFn::kSubgroupExclusiveMul:
                    case wgsl::BuiltinFn::kSubgroupAnd:
                    case wgsl::BuiltinFn::kSubgroupOr:
                    case wgsl::BuiltinFn::kSubgroupXor:
                    case wgsl::BuiltinFn::kSubgroupMin:
                    case wgsl::BuiltinFn::kSubgroupMax:
                    case wgsl::BuiltinFn::kSubgroupAny:
                    case wgsl::BuiltinFn::kSubgroupAll:
                    case wgsl::BuiltinFn::kQuadBroadcast:
                    case wgsl::BuiltinFn::kQuadSwapX:
                    case wgsl::BuiltinFn::kQuadSwapY:
                    case wgsl::BuiltinFn::kQuadSwapDiagonal:
                        Enable(wgsl::Extension::kF16);
                        Enable(wgsl::Extension::kSubgroups);
                        break;
                    default:
                        break;
                }

                const ast::CallExpression* expr = nullptr;
                if (!c->ExplicitTemplateParams().IsEmpty()) {
                    Vector<const ast::Expression*, 4> tmpl_args;
                    for (auto* e : c->ExplicitTemplateParams()) {
                        tmpl_args.Push(Type(e).expr);
                    }
                    expr = b.Call(b.Ident(c->Func(), std::move(tmpl_args)), std::move(args));
                } else {
                    expr = b.Call(c->Func(), std::move(args));
                }

                if (call->Results().IsEmpty() || !call->Result()->IsUsed()) {
                    Append(b.CallStmt(expr));
                    return;
                }
                Bind(c->Result(), expr);
            },
            [&](const core::ir::Construct* c) {
                auto ty = Type(c->Result()->Type());
                Bind(c->Result(), b.Call(ty, std::move(args)));
            },
            [&](const core::ir::Convert* c) {
                auto ty = Type(c->Result()->Type());
                Bind(c->Result(), b.Call(ty, std::move(args)));
            },
            [&](const core::ir::Bitcast* c) {
                auto ty = Type(c->Result()->Type());
                Bind(c->Result(), b.Bitcast(ty, args[0]));
            },
            [&](const core::ir::Discard*) { Append(b.Discard()); },  //
            TINT_ICE_ON_NO_MATCH);
    }

    void Load(const core::ir::Load* l) { Bind(l->Result(), Expr(l->From())); }

    void LoadVectorElement(const core::ir::LoadVectorElement* l) {
        auto* vec = Expr(l->From());
        Bind(l->Result(), VectorMemberAccess(vec, l->Index()));
    }

    void Unary(const core::ir::Unary* u) {
        const ast::Expression* expr = nullptr;
        switch (u->Op()) {
            case core::UnaryOp::kComplement:
                expr = b.Complement(Expr(u->Val()));
                break;
            case core::UnaryOp::kNegation:
                expr = b.Negation(Expr(u->Val()));
                break;
            case core::UnaryOp::kNot:
                expr = b.Not(Expr(u->Val()));
                break;
            case core::UnaryOp::kAddressOf:
                expr = b.AddressOf(Expr(u->Val()));
                break;
            case core::UnaryOp::kIndirection:
                expr = b.Deref(Expr(u->Val()));
                break;
        }
        Bind(u->Result(), expr);
    }

    void Access(const core::ir::Access* a) {
        auto* expr = Expr(a->Object());
        auto* obj_ty = a->Object()->Type()->UnwrapRef();
        for (auto* index : a->Indices()) {
            tint::Switch(
                obj_ty,
                [&](const core::type::Vector* vec) {
                    TINT_DEFER(obj_ty = vec->Type());
                    expr = VectorMemberAccess(expr, index);
                },
                [&](const core::type::Matrix* mat) {
                    obj_ty = mat->ColumnType();
                    expr = b.IndexAccessor(expr, Expr(index));
                },
                [&](const core::type::Array* arr) {
                    obj_ty = arr->ElemType();
                    expr = b.IndexAccessor(expr, Expr(index));
                },
                [&](const core::type::Struct* s) {
                    if (auto* c = index->As<core::ir::Constant>()) {
                        auto i = c->Value()->ValueAs<uint32_t>();
                        TINT_ASSERT(i < s->Members().Length());
                        auto* member = s->Members()[i];
                        obj_ty = member->Type();
                        expr = b.MemberAccessor(expr, member->Name().NameView());
                    } else {
                        TINT_ICE() << "invalid index for struct type: " << index->TypeInfo().name;
                    }
                },  //
                TINT_ICE_ON_NO_MATCH);
        }
        Bind(a->Result(), expr);
    }

    void Swizzle(const core::ir::Swizzle* s) {
        auto* vec = Expr(s->Object());
        Vector<char, 4> components;
        for (uint32_t i : s->Indices()) {
            if (DAWN_UNLIKELY(i >= 4)) {
                TINT_ICE() << "invalid swizzle index: " << i;
            }
            components.Push("xyzw"[i]);
        }
        auto* swizzle =
            b.MemberAccessor(vec, std::string_view(components.begin(), components.Length()));
        Bind(s->Result(), swizzle);
    }

    void Binary(const core::ir::Binary* e) {
        if (e->Op() == core::BinaryOp::kEqual) {
            auto* rhs = e->RHS()->As<core::ir::Constant>();
            if (rhs && rhs->Type()->Is<core::type::Bool>() &&
                rhs->Value()->ValueAs<bool>() == false) {
                // expr == false
                Bind(e->Result(), b.Not(Expr(e->LHS())));
                return;
            }
        }
        auto* lhs = Expr(e->LHS());
        auto* rhs = Expr(e->RHS());
        const ast::Expression* expr = nullptr;
        switch (e->Op()) {
            case core::BinaryOp::kAdd:
                expr = b.Add(lhs, rhs);
                break;
            case core::BinaryOp::kSubtract:
                expr = b.Sub(lhs, rhs);
                break;
            case core::BinaryOp::kMultiply:
                expr = b.Mul(lhs, rhs);
                break;
            case core::BinaryOp::kDivide:
                expr = b.Div(lhs, rhs);
                break;
            case core::BinaryOp::kModulo:
                expr = b.Mod(lhs, rhs);
                break;
            case core::BinaryOp::kAnd:
                expr = b.And(lhs, rhs);
                break;
            case core::BinaryOp::kOr:
                expr = b.Or(lhs, rhs);
                break;
            case core::BinaryOp::kXor:
                expr = b.Xor(lhs, rhs);
                break;
            case core::BinaryOp::kEqual:
                expr = b.Equal(lhs, rhs);
                break;
            case core::BinaryOp::kNotEqual:
                expr = b.NotEqual(lhs, rhs);
                break;
            case core::BinaryOp::kLessThan:
                expr = b.LessThan(lhs, rhs);
                break;
            case core::BinaryOp::kGreaterThan:
                expr = b.GreaterThan(lhs, rhs);
                break;
            case core::BinaryOp::kLessThanEqual:
                expr = b.LessThanEqual(lhs, rhs);
                break;
            case core::BinaryOp::kGreaterThanEqual:
                expr = b.GreaterThanEqual(lhs, rhs);
                break;
            case core::BinaryOp::kShiftLeft:
                expr = b.Shl(lhs, rhs);
                break;
            case core::BinaryOp::kShiftRight:
                expr = b.Shr(lhs, rhs);
                break;
            case core::BinaryOp::kLogicalAnd:
                expr = b.LogicalAnd(lhs, rhs);
                break;
            case core::BinaryOp::kLogicalOr:
                expr = b.LogicalOr(lhs, rhs);
                break;
        }
        Bind(e->Result(), expr);
    }

    TINT_BEGIN_DISABLE_WARNING(UNREACHABLE_CODE);

    const ast::Expression* Expr(const core::ir::Value* value) {
        auto expr = tint::Switch(
            value,  //
            [&](const core::ir::Constant* c) { return Constant(c); },
            [&](Default) -> const ast::Expression* {
                auto lookup = bindings_.Get(value);
                if (DAWN_UNLIKELY(!lookup)) {
                    TINT_ICE() << "Expr(" << (value ? value->TypeInfo().name : "null")
                               << ") value has no expression";
                }
                return std::visit(
                    [&](auto&& got) -> const ast::Expression* {
                        using T = std::decay_t<decltype(got)>;

                        if constexpr (std::is_same_v<T, VariableValue>) {
                            return b.Expr(got.name);
                        }

                        if constexpr (std::is_same_v<T, InlinedValue>) {
                            auto result = got.expr;
                            // Single use (inlined) expression.
                            // Mark the bindings_ map entry as consumed.
                            *lookup = ConsumedValue{};
                            return result;
                        }

                        if constexpr (std::is_same_v<T, ConsumedValue>) {
                            TINT_ICE() << "Expr(" << value->TypeInfo().name
                                       << ") called twice on the same value";
                        } else {
                            TINT_ICE()
                                << "Expr(" << value->TypeInfo().name << ") has unhandled value";
                        }
                        return nullptr;
                    },
                    *lookup);
            });

        if (!expr) {
            return b.Expr("<error>");
        }

        return expr;
    }

    TINT_END_DISABLE_WARNING(UNREACHABLE_CODE);

    const ast::Expression* Constant(const core::ir::Constant* c) { return Constant(c->Value()); }

    const ast::Expression* Constant(const core::constant::Value* c) {
        auto composite = [&](bool can_splat) {
            auto ty = Type(c->Type());
            if (c->AllZero()) {
                return b.Call(ty);
            }
            if (can_splat && c->Is<core::constant::Splat>()) {
                return b.Call(ty, Constant(c->Index(0)));
            }

            Vector<const ast::Expression*, 8> els;
            for (size_t i = 0, n = c->NumElements(); i < n; i++) {
                els.Push(Constant(c->Index(i)));
            }
            return b.Call(ty, std::move(els));
        };
        return tint::Switch(
            c->Type(),  //
            [&](const core::type::I32*) { return b.Expr(c->ValueAs<i32>()); },
            [&](const core::type::U32*) { return b.Expr(c->ValueAs<u32>()); },
            [&](const core::type::F32*) { return b.Expr(c->ValueAs<f32>()); },
            [&](const core::type::F16*) {
                Enable(wgsl::Extension::kF16);
                return b.Expr(c->ValueAs<f16>());
            },
            [&](const core::type::Bool*) { return b.Expr(c->ValueAs<bool>()); },
            [&](const core::type::Array*) { return composite(/* can_splat */ false); },
            [&](const core::type::Vector*) { return composite(/* can_splat */ true); },
            [&](const core::type::Matrix*) { return composite(/* can_splat */ false); },
            [&](const core::type::Struct*) { return composite(/* can_splat */ false); },  //
            TINT_ICE_ON_NO_MATCH);
    }

    void Enable(wgsl::Extension ext) {
        if (enables_.Add(ext)) {
            b.Enable(ext);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Types
    //
    // The the case of an error:
    // * The types generating methods must return a non-null ast type, which may not be semantically
    //   legal, but is enough to populate the AST.
    // * A diagnostic error must be added to the ast::ProgramBuilder.
    // This prevents littering the ToProgram logic with expensive error checking code.
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /// @param ty the type::Type
    /// @return an ast::Type from @p ty.
    /// @note May be a semantically-invalid placeholder type on error.
    ast::Type Type(const core::type::Type* ty) {
        return tint::Switch(
            ty,                                                    //
            [&](const core::type::Void*) { return ast::Type{}; },  //
            [&](const core::type::I32*) { return b.ty.i32(); },    //
            [&](const core::type::U32*) { return b.ty.u32(); },    //
            [&](const core::type::F16*) {
                Enable(wgsl::Extension::kF16);
                return b.ty.f16();
            },
            [&](const core::type::F32*) { return b.ty.f32(); },  //
            [&](const core::type::Bool*) { return b.ty.bool_(); },
            [&](const core::type::Matrix* m) {
                return b.ty.mat(Type(m->Type()), m->Columns(), m->Rows());
            },
            [&](const core::type::Vector* v) {
                auto el = Type(v->Type());
                return b.ty.vec(el, v->Width());
            },
            [&](const core::type::Array* a) {
                if (ContainsBuiltinStruct(a)) {
                    // The element type is untypeable, so we need to infer it instead.
                    return ast::Type{b.Expr(b.Ident("array"))};
                }

                auto el = Type(a->ElemType());
                Vector<const ast::Attribute*, 1> attrs;
                if (!a->IsStrideImplicit()) {
                    attrs.Push(b.Stride(a->Stride()));
                }
                if (a->Count()->Is<core::type::RuntimeArrayCount>()) {
                    return b.ty.array(el, std::move(attrs));
                }
                auto count = a->ConstantCount();
                if (DAWN_UNLIKELY(!count)) {
                    TINT_ICE() << core::type::Array::kErrExpectedConstantCount;
                }
                return b.ty.array(el, u32(count.value()), std::move(attrs));
            },
            [&](const core::type::Struct* s) { return Struct(s); },
            [&](const core::type::Atomic* a) { return b.ty.atomic(Type(a->Type())); },
            [&](const core::type::DepthTexture* t) { return b.ty.depth_texture(t->Dim()); },
            [&](const core::type::DepthMultisampledTexture* t) {
                return b.ty.depth_multisampled_texture(t->Dim());
            },
            [&](const core::type::ExternalTexture*) { return b.ty.external_texture(); },
            [&](const core::type::MultisampledTexture* t) {
                auto el = Type(t->Type());
                return b.ty.multisampled_texture(t->Dim(), el);
            },
            [&](const core::type::SampledTexture* t) {
                auto el = Type(t->Type());
                return b.ty.sampled_texture(t->Dim(), el);
            },
            [&](const core::type::StorageTexture* t) {
                if (RequiresChromiumInternalGraphite(t)) {
                    Enable(wgsl::Extension::kChromiumInternalGraphite);
                }

                return b.ty.storage_texture(t->Dim(), t->TexelFormat(), t->Access());
            },
            [&](const core::type::Sampler* s) { return b.ty.sampler(s->Kind()); },
            [&](const core::type::Pointer* p) {
                // Note: type::Pointer always has an inferred access, but WGSL only allows an
                // explicit access in the 'storage' address space.
                auto el = Type(p->StoreType());
                auto address_space = p->AddressSpace();
                auto access = address_space == core::AddressSpace::kStorage
                                  ? p->Access()
                                  : core::Access::kUndefined;
                return b.ty.ptr(address_space, el, access);
            },
            [&](const core::type::Reference*) -> ast::Type {
                TINT_ICE() << "reference types should never appear in the IR";
            },
            [&](const core::type::InputAttachment* i) {
                Enable(wgsl::Extension::kChromiumInternalInputAttachments);
                auto el = Type(i->Type());
                return b.ty.input_attachment(el);
            },  //
            [&](const core::type::SubgroupMatrix* m) {
                Enable(wgsl::Extension::kChromiumExperimentalSubgroupMatrix);
                auto el = Type(m->Type());
                return b.ty.subgroup_matrix(m->Kind(), el, m->Columns(), m->Rows());
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    ast::Type Struct(const core::type::Struct* s) {
        // Skip builtin structures.
        if (ContainsBuiltinStruct(s)) {
            return ast::Type{};
        }

        auto n = structs_.GetOrAdd(s, [&] {
            auto members = tint::Transform<8>(s->Members(), [&](const core::type::StructMember* m) {
                auto ty = Type(m->Type());
                const auto& ir_attrs = m->Attributes();
                Vector<const ast::Attribute*, 4> ast_attrs;
                if (m->Type()->Align() != m->Align()) {
                    ast_attrs.Push(b.MemberAlign(u32(m->Align())));
                }
                if (m->Type()->Size() != m->Size()) {
                    ast_attrs.Push(b.MemberSize(u32(m->Size())));
                }
                if (auto location = ir_attrs.location) {
                    ast_attrs.Push(b.Location(u32(*location)));
                }
                if (auto blend_src = ir_attrs.blend_src) {
                    Enable(wgsl::Extension::kDualSourceBlending);
                    ast_attrs.Push(b.BlendSrc(u32(*blend_src)));
                }
                if (auto color = ir_attrs.color) {
                    Enable(wgsl::Extension::kChromiumExperimentalFramebufferFetch);
                    ast_attrs.Push(b.Color(u32(*color)));
                }
                if (auto builtin = ir_attrs.builtin) {
                    if (RequiresSubgroups(*builtin)) {
                        Enable(wgsl::Extension::kSubgroups);
                    } else if (*builtin == core::BuiltinValue::kClipDistances) {
                        Enable(wgsl::Extension::kClipDistances);
                    }
                    ast_attrs.Push(b.Builtin(*builtin));
                }
                if (auto interpolation = ir_attrs.interpolation) {
                    ast_attrs.Push(b.Interpolate(interpolation->type, interpolation->sampling));
                }
                if (ir_attrs.invariant) {
                    ast_attrs.Push(b.Invariant());
                }
                return b.Member(m->Name().NameView(), ty, std::move(ast_attrs));
            });

            // TODO(crbug.com/tint/1902): Emit structure attributes
            Vector<const ast::Attribute*, 2> attrs;

            auto name = b.Symbols().Register(s->Name().NameView());
            b.Structure(name, std::move(members), std::move(attrs));
            return name;
        });

        return b.ty(n);
    }

    bool ContainsBuiltinStruct(const core::type::Type* ty) {
        if (auto* s = ty->As<core::type::Struct>()) {
            // Note: We don't need to check the members of the struct, as builtin structures cannot
            // be nested inside other structures.
            if (s->IsWgslInternal()) {
                return true;
            }
        } else if (auto* a = ty->As<core::type::Array>()) {
            return ContainsBuiltinStruct(a->ElemType());
        }
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Bindings
    ////////////////////////////////////////////////////////////////////////////////////////////////

    bool IsKeyword(std::string_view s) {
        return s == "alias" || s == "break" || s == "case" || s == "const" || s == "const_assert" ||
               s == "continue" || s == "continuing" || s == "default" || s == "diagnostic" ||
               s == "discard" || s == "else" || s == "enable" || s == "false" || s == "fn" ||
               s == "for" || s == "if" || s == "let" || s == "loop" || s == "override" ||
               s == "requires" || s == "return" || s == "struct" || s == "switch" || s == "true" ||
               s == "var" || s == "while";
    }

    bool IsEnumName(std::string_view s) {
        return s == "read" || s == "write" || s == "read_write" || s == "function" ||
               s == "private" || s == "workgroup" || s == "uniform" || s == "storage" ||
               s == "rgba8unorm" || s == "rgba8snorm" || s == "rgba8uint" || s == "rgba8sint" ||
               s == "rgba16uint" || s == "rgba16sint" || s == "rgba16float" || s == "r32uint" ||
               s == "r32sint" || s == "r32float" || s == "rg32uint" || s == "rg32sint" ||
               s == "rg32float" || s == "rgba32uint" || s == "rgba32sint" || s == "rgba32float" ||
               s == "bgra8unorm";
    }

    bool IsTypeName(std::string_view s) {
        return s == "bool" || s == "void" || s == "i32" || s == "u32" || s == "f32" || s == "f16" ||
               s == "vec" || s == "vec2" || s == "vec3" || s == "vec4" || s == "vec2f" ||
               s == "vec3f" || s == "vec4f" || s == "vec2h" || s == "vec3h" || s == "vec4h" ||
               s == "vec2i" || s == "vec3i" || s == "vec4i" || s == "vec2u" || s == "vec3u" ||
               s == "vec4u" || s == "mat2x2" || s == "mat2x3" || s == "mat2x4" || s == "mat3x2" ||
               s == "mat3x3" || s == "mat3x4" || s == "mat4x2" || s == "mat4x3" || s == "mat4x4" ||
               s == "mat2x2f" || s == "mat2x3f" || s == "mat2x4f" || s == "mat3x2f" ||
               s == "mat3x3f" || s == "mat3x4f" || s == "mat4x2f" || s == "mat4x3f" ||
               s == "mat4x4f" || s == "mat2x2h" || s == "mat2x3h" || s == "mat2x4h" ||
               s == "mat3x2h" || s == "mat3x3h" || s == "mat3x4h" || s == "mat4x2h" ||
               s == "mat4x3h" || s == "mat4x4h" || s == "atomic" || s == "array" || s == "ptr" ||
               s == "texture_1d" || s == "texture_2d" || s == "texture_2d_array" ||
               s == "texture_3d" || s == "texture_cube" || s == "texture_cube_array" ||
               s == "texture_multisampled_2d" || s == "texture_depth_multisampled_2d" ||
               s == "texture_external" || s == "texture_storage_1d" || s == "texture_storage_2d" ||
               s == "texture_storage_2d_array" || s == "texture_storage_3d" ||
               s == "texture_depth_2d" || s == "texture_depth_2d_array" ||
               s == "texture_depth_cube" || s == "texture_depth_cube_array" || s == "sampler" ||
               s == "sampler_comparison";
    }

    bool IsWGSLSafe(std::string_view name) {
        return !IsReserved(name) && !IsKeyword(name) && !IsEnumName(name) && !IsTypeName(name);
    }

    /// @returns the AST name for the given value, creating and returning a new name on the first
    /// call.
    Symbol NameFor(const core::ir::Value* value, std::string_view suggested = {}) {
        return names_.GetOrAdd(value, [&] {
            if (!suggested.empty()) {
                if (IsWGSLSafe(suggested)) {
                    return b.Symbols().Register(suggested);
                }
            } else if (auto sym = mod.NameOf(value)) {
                if (IsWGSLSafe(sym.NameView())) {
                    return b.Symbols().Register(sym.NameView());
                }
            }
            return b.Symbols().New("v");
        });
    }

    /// Associates the IR value @p value with the AST expression @p expr if it is used, otherwise
    /// creates a phony assignment with @p expr.
    void Bind(const core::ir::Value* value, const ast::Expression* expr) {
        TINT_ASSERT(value);
        if (value->IsUsed()) {
            // Value will be inlined at its place of usage.
            if (DAWN_UNLIKELY(!bindings_.Add(value, InlinedValue{expr}))) {
                TINT_ICE() << "Bind(" << value->TypeInfo().name << ") called twice for same value";
            }
        } else {
            Append(b.Assign(b.Phony(), expr));
        }
    }

    /// Associates the IR value @p value with the AST 'var', 'let' or parameter with the name @p
    /// name.
    void Bind(const core::ir::Value* value, Symbol name) {
        TINT_ASSERT(value);
        if (DAWN_UNLIKELY(!bindings_.Add(value, VariableValue{name}))) {
            TINT_ICE() << "Bind(" << value->TypeInfo().name << ") called twice for same value";
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Helpers
    ////////////////////////////////////////////////////////////////////////////////////////////////
    bool AsShortCircuit(const core::ir::If* i,
                        const StatementList& true_stmts,
                        const StatementList& false_stmts) {
        if (i->Results().IsEmpty()) {
            return false;
        }
        auto* result = i->Result();
        if (!result->Type()->Is<core::type::Bool>()) {
            return false;  // Wrong result type
        }
        if (i->Exits().Count() != 2) {
            return false;  // Doesn't have two exits
        }
        if (!true_stmts.IsEmpty() || !false_stmts.IsEmpty()) {
            return false;  // True or False blocks contain statements
        }

        auto* cond = i->Condition();
        auto* true_val = i->True()->Back()->Operands().Front();
        auto* false_val = i->False()->Back()->Operands().Front();
        if (IsConstant(false_val, false)) {
            //  %res = if %cond {
            //     block {  # true
            //       exit_if %true_val;
            //     }
            //     block {  # false
            //       exit_if false;
            //     }
            //  }
            //
            // transform into:
            //
            //   res = cond && true_val;
            //
            auto* lhs = Expr(cond);
            auto* rhs = Expr(true_val);
            Bind(result, b.LogicalAnd(lhs, rhs));
            return true;
        }
        if (IsConstant(true_val, true)) {
            //  %res = if %cond {
            //     block {  # true
            //       exit_if true;
            //     }
            //     block {  # false
            //       exit_if %false_val;
            //     }
            //  }
            //
            // transform into:
            //
            //   res = cond || false_val;
            //
            auto* lhs = Expr(cond);
            auto* rhs = Expr(false_val);
            Bind(result, b.LogicalOr(lhs, rhs));
            return true;
        }
        return false;
    }

    bool IsConstant(const core::ir::Value* val, bool value) {
        if (auto* c = val->As<core::ir::Constant>()) {
            if (c->Type()->Is<core::type::Bool>()) {
                return c->Value()->ValueAs<bool>() == value;
            }
        }
        return false;
    }

    const ast::Expression* VectorMemberAccess(const ast::Expression* expr,
                                              const core::ir::Value* index) {
        if (auto* c = index->As<core::ir::Constant>()) {
            switch (c->Value()->ValueAs<int>()) {
                case 0:
                    return b.MemberAccessor(expr, "x");
                case 1:
                    return b.MemberAccessor(expr, "y");
                case 2:
                    return b.MemberAccessor(expr, "z");
                case 3:
                    return b.MemberAccessor(expr, "w");
            }
        }
        return b.IndexAccessor(expr, Expr(index));
    }

    bool RequiresDerivativeUniformity(wgsl::BuiltinFn fn) {
        switch (fn) {
            case wgsl::BuiltinFn::kDpdxCoarse:
            case wgsl::BuiltinFn::kDpdyCoarse:
            case wgsl::BuiltinFn::kFwidthCoarse:
            case wgsl::BuiltinFn::kDpdxFine:
            case wgsl::BuiltinFn::kDpdyFine:
            case wgsl::BuiltinFn::kFwidthFine:
            case wgsl::BuiltinFn::kDpdx:
            case wgsl::BuiltinFn::kDpdy:
            case wgsl::BuiltinFn::kFwidth:
            case wgsl::BuiltinFn::kTextureSample:
            case wgsl::BuiltinFn::kTextureSampleBias:
            case wgsl::BuiltinFn::kTextureSampleCompare:
                return true;
            default:
                return false;
        }
    }

    /// @returns true if the builtin value requires the kSubgroups extension to be enabled.
    bool RequiresSubgroups(core::BuiltinValue builtin) {
        switch (builtin) {
            case core::BuiltinValue::kSubgroupInvocationId:
            case core::BuiltinValue::kSubgroupSize:
                return true;
            default:
                return false;
        }
    }

    /// @returns true if the storage texture type requires the kChromiumInternalGraphite extension
    /// to be enabled.
    bool RequiresChromiumInternalGraphite(const core::type::StorageTexture* tex) {
        return tex->TexelFormat() == core::TexelFormat::kR8Unorm;
    }
};

}  // namespace

Program IRToProgram(const core::ir::Module& i, const ProgramOptions& options) {
    return State{i}.Run(options);
}

}  // namespace tint::wgsl::writer

TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);
