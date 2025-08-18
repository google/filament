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

#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"

#include <utility>
#include <variant>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/type/array_count.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/wgsl/ast/accessor_expression.h"
#include "src/tint/lang/wgsl/ast/alias.h"
#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/binary_expression.h"
#include "src/tint/lang/wgsl/ast/block_statement.h"
#include "src/tint/lang/wgsl/ast/break_if_statement.h"
#include "src/tint/lang/wgsl/ast/break_statement.h"
#include "src/tint/lang/wgsl/ast/call_expression.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/compound_assignment_statement.h"
#include "src/tint/lang/wgsl/ast/const.h"
#include "src/tint/lang/wgsl/ast/const_assert.h"
#include "src/tint/lang/wgsl/ast/continue_statement.h"
#include "src/tint/lang/wgsl/ast/diagnostic_directive.h"
#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/ast/enable.h"
#include "src/tint/lang/wgsl/ast/for_loop_statement.h"
#include "src/tint/lang/wgsl/ast/function.h"
#include "src/tint/lang/wgsl/ast/identifier.h"
#include "src/tint/lang/wgsl/ast/identifier_expression.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"
#include "src/tint/lang/wgsl/ast/increment_decrement_statement.h"
#include "src/tint/lang/wgsl/ast/index_accessor_expression.h"
#include "src/tint/lang/wgsl/ast/interpolate_attribute.h"
#include "src/tint/lang/wgsl/ast/invariant_attribute.h"
#include "src/tint/lang/wgsl/ast/let.h"
#include "src/tint/lang/wgsl/ast/literal_expression.h"
#include "src/tint/lang/wgsl/ast/loop_statement.h"
#include "src/tint/lang/wgsl/ast/member_accessor_expression.h"
#include "src/tint/lang/wgsl/ast/override.h"
#include "src/tint/lang/wgsl/ast/phony_expression.h"
#include "src/tint/lang/wgsl/ast/requires.h"
#include "src/tint/lang/wgsl/ast/return_statement.h"
#include "src/tint/lang/wgsl/ast/statement.h"
#include "src/tint/lang/wgsl/ast/struct.h"
#include "src/tint/lang/wgsl/ast/switch_statement.h"
#include "src/tint/lang/wgsl/ast/templated_identifier.h"
#include "src/tint/lang/wgsl/ast/unary_op_expression.h"
#include "src/tint/lang/wgsl/ast/var.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/ast/while_statement.h"
#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"
#include "src/tint/lang/wgsl/ir/builtin_call.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/sem/array_count.h"
#include "src/tint/lang/wgsl/sem/builtin_enum_expression.h"
#include "src/tint/lang/wgsl/sem/builtin_fn.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/index_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/load.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/module.h"
#include "src/tint/lang/wgsl/sem/switch_statement.h"
#include "src/tint/lang/wgsl/sem/type_expression.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"
#include "src/tint/lang/wgsl/sem/value_conversion.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/containers/scope_stack.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/rtti/switch.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::wgsl::reader {
namespace {

using ResultType = diag::Result<core::ir::Module>;

/// Impl is the private-implementation of FromProgram().
class Impl {
  public:
    /// Constructor
    /// @param program the program to convert to IR
    explicit Impl(const Program& program) : program_(program) {}

    /// Builds an IR module from the program passed to the constructor.
    /// @return the IR module or an error.
    ResultType Build() { return EmitModule(); }

  private:
    enum class ControlFlags { kNone, kExcludeSwitch };

    // The input Program
    const Program& program_;

    /// The IR module being built
    core::ir::Module mod;

    /// The IR builder being used by the impl.
    core::ir::Builder builder_{mod};

    // The clone context used to clone data from #program_
    core::constant::CloneContext clone_ctx_{
        /* type_ctx */ core::type::CloneContext{
            /* src */ {&program_.Symbols()},
            /* dst */ {&builder_.ir.symbols, &builder_.ir.Types()},
        },
        /* dst */ {builder_.ir.constant_values},
    };

    /// The stack of flow control instructions.
    Vector<core::ir::ControlInstruction*, 8> control_stack_;

    struct VectorRefElementAccess {
        core::ir::Value* vector = nullptr;
        core::ir::Value* index = nullptr;
    };

    using ValueOrVecElAccess = std::variant<core::ir::Value*, VectorRefElementAccess>;

    /// The current block for expressions.
    core::ir::Block* current_block_ = nullptr;

    /// The current function being processed.
    core::ir::Function* current_function_ = nullptr;

    /// The current stack of scopes being processed.
    ScopeStack<Symbol, core::ir::Value*> scopes_;

    /// The diagnostic that have been raised.
    diag::List diagnostics_;

    class StackScope {
      public:
        explicit StackScope(Impl* impl) : impl_(impl) { impl->scopes_.Push(); }

        ~StackScope() { impl_->scopes_.Pop(); }

      protected:
        Impl* impl_;
    };

    class ControlStackScope : public StackScope {
      public:
        ControlStackScope(Impl* impl, core::ir::ControlInstruction* b) : StackScope(impl) {
            impl_->control_stack_.Push(b);
        }

        ~ControlStackScope() { impl_->control_stack_.Pop(); }
    };

    diag::Diagnostic& AddError(const Source& source) { return diagnostics_.AddError(source); }

    bool NeedTerminator() {
        return (current_block_ != nullptr) && (current_block_->Terminator() == nullptr);
    }

    void SetTerminator(core::ir::Terminator* terminator) {
        TINT_ASSERT(current_block_);
        TINT_ASSERT(!current_block_->Terminator());

        current_block_->Append(terminator);
        current_block_ = nullptr;
    }

    core::ir::Instruction* FindEnclosingControl(ControlFlags flags) {
        for (auto it = control_stack_.rbegin(); it != control_stack_.rend(); ++it) {
            if ((*it)->Is<core::ir::Loop>()) {
                return *it;
            }
            if (flags == ControlFlags::kExcludeSwitch) {
                continue;
            }
            if ((*it)->Is<core::ir::Switch>()) {
                return *it;
            }
        }
        return nullptr;
    }

    ResultType EmitModule() {
        auto* sem = program_.Sem().Module();

        for (auto* decl : sem->DependencyOrderedDeclarations()) {
            tint::Switch(
                decl,  //
                [&](const ast::Struct*) {
                    // Will be encoded into the `type::Struct` when used. We will then hoist all
                    // used structs up to module scope when converting IR.
                },
                [&](const ast::Alias*) {
                    // Folded away and doesn't appear in the IR.
                },
                [&](const ast::Variable* var) {
                    // Setup the current block to be the root block for the module. The builder
                    // will handle creating it if it doesn't exist already.
                    TINT_SCOPED_ASSIGNMENT(current_block_, mod.root_block);
                    EmitVariable(var);
                },
                [&](const ast::Function* func) { EmitFunction(func); },
                [&](const ast::Enable*) {
                    // TODO(dsinclair): Implement? I think these need to be passed along so further
                    // stages know what is enabled.
                },
                [&](const ast::ConstAssert*) {
                    // Evaluated by the resolver, drop from the IR.
                },
                [&](const ast::DiagnosticDirective*) {
                    // Ignored for now.
                },  //
                [&](const ast::Requires*) {
                    // Ignored for now.
                },  //
                TINT_ICE_ON_NO_MATCH);
        }

        if (diagnostics_.ContainsErrors()) {
            return diag::Failure{std::move(diagnostics_)};
        }

        return std::move(mod);
    }

    void EmitFunction(const ast::Function* ast_func) {
        // The flow stack should have been emptied when the previous function finished building.
        TINT_ASSERT(control_stack_.IsEmpty());

        const auto* sem = program_.Sem().Get(ast_func);

        auto* ir_func = builder_.Function(ast_func->name->symbol.NameView(),
                                          sem->ReturnType()->Clone(clone_ctx_.type_ctx));
        current_function_ = ir_func;
        scopes_.Set(ast_func->name->symbol, ir_func);

        if (ast_func->IsEntryPoint()) {
            switch (ast_func->PipelineStage()) {
                case ast::PipelineStage::kVertex:
                    ir_func->SetStage(core::ir::Function::PipelineStage::kVertex);
                    break;
                case ast::PipelineStage::kFragment:
                    ir_func->SetStage(core::ir::Function::PipelineStage::kFragment);
                    break;
                case ast::PipelineStage::kCompute: {
                    ir_func->SetStage(core::ir::Function::PipelineStage::kCompute);

                    auto attr = ast::GetAttribute<ast::WorkgroupAttribute>(ast_func->attributes);
                    if (attr) {
                        TINT_SCOPED_ASSIGNMENT(current_block_, mod.root_block);

                        // The x size is always required (y, z are optional).
                        auto value_x = EmitValueExpression(attr->x);
                        bool is_unsigned = value_x->Type()->IsUnsignedIntegerScalar();

                        auto* one_const =
                            is_unsigned ? builder_.Constant(1_u) : builder_.Constant(1_i);

                        ir_func->SetWorkgroupSize(
                            value_x, attr->y ? EmitValueExpression(attr->y) : one_const,
                            attr->z ? EmitValueExpression(attr->z) : one_const);
                    } else {
                        TINT_ICE() << "Missing workgroup attribute for compute entry point.";
                    }
                    break;
                }
                default: {
                    TINT_ICE() << "Invalid pipeline stage";
                }
            }

            for (auto* attr : ast_func->return_type_attributes) {
                tint::Switch(
                    attr,  //
                    [&](const ast::InterpolateAttribute* interp) {
                        ir_func->SetReturnInterpolation(interp->interpolation);
                    },
                    [&](const ast::InvariantAttribute*) { ir_func->SetReturnInvariant(true); },
                    [&](const ast::BuiltinAttribute* b) { ir_func->SetReturnBuiltin(b->builtin); });
            }
            ir_func->SetReturnLocation(sem->ReturnLocation());
        }

        scopes_.Push();
        TINT_DEFER(scopes_.Pop());

        Vector<core::ir::FunctionParam*, 1> params;
        for (auto* p : ast_func->params) {
            const auto* param_sem = program_.Sem().Get(p)->As<sem::Parameter>();
            auto* ty = param_sem->Type()->Clone(clone_ctx_.type_ctx);
            auto* param = builder_.FunctionParam(p->name->symbol.NameView(), ty);

            for (auto* attr : p->attributes) {
                tint::Switch(
                    attr,  //
                    [&](const ast::InterpolateAttribute* interp) {
                        param->SetInterpolation(interp->interpolation);
                    },
                    [&](const ast::InvariantAttribute*) { param->SetInvariant(true); },
                    [&](const ast::BuiltinAttribute* b) { param->SetBuiltin(b->builtin); });

                param->SetLocation(param_sem->Attributes().location);
                param->SetColor(param_sem->Attributes().color);
            }

            scopes_.Set(p->name->symbol, param);
            params.Push(param);
        }
        ir_func->SetParams(params);

        TINT_SCOPED_ASSIGNMENT(current_block_, ir_func->Block());
        EmitBlock(ast_func->body);

        // Add a terminator if one was not already created.
        if (NeedTerminator()) {
            if (!program_.Sem().Get(ast_func->body)->Behaviors().Contains(sem::Behavior::kNext)) {
                SetTerminator(builder_.Unreachable());
            } else {
                SetTerminator(builder_.Return(current_function_));
            }
        }

        TINT_ASSERT(control_stack_.IsEmpty());
        current_block_ = nullptr;
        current_function_ = nullptr;
    }

    void EmitStatements(VectorRef<const ast::Statement*> stmts) {
        for (auto* s : stmts) {
            EmitStatement(s);

            if (auto* sem = program_.Sem().Get(s);
                sem && !sem->Behaviors().Contains(sem::Behavior::kNext)) {
                break;  // Unreachable statement.
            }
        }
    }

    void EmitStatement(const ast::Statement* stmt) {
        tint::Switch(
            stmt,  //
            [&](const ast::AssignmentStatement* a) { EmitAssignment(a); },
            [&](const ast::BlockStatement* b) { EmitBlock(b); },
            [&](const ast::BreakStatement* b) { EmitBreak(b); },
            [&](const ast::BreakIfStatement* b) { EmitBreakIf(b); },
            [&](const ast::CallStatement* c) { EmitCall(c); },
            [&](const ast::CompoundAssignmentStatement* c) { EmitCompoundAssignment(c); },
            [&](const ast::ContinueStatement* c) { EmitContinue(c); },
            [&](const ast::DiscardStatement* d) { EmitDiscard(d); },
            [&](const ast::IfStatement* i) { EmitIf(i); },
            [&](const ast::LoopStatement* l) { EmitLoop(l); },
            [&](const ast::ForLoopStatement* l) { EmitForLoop(l); },
            [&](const ast::WhileStatement* l) { EmitWhile(l); },
            [&](const ast::ReturnStatement* r) { EmitReturn(r); },
            [&](const ast::SwitchStatement* s) { EmitSwitch(s); },
            [&](const ast::VariableDeclStatement* v) { EmitVariable(v->variable); },
            [&](const ast::IncrementDecrementStatement* i) { EmitIncrementDecrement(i); },
            [&](const ast::ConstAssert*) {
                // Not emitted
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    void EmitAssignment(const ast::AssignmentStatement* stmt) {
        // If assigning to a phony, and the expression evaluation stage is runtime or override, just
        // generate the RHS and we're done. Note that, because this isn't used, a subsequent
        // transform could remove it due to it being dead code. This could then change the interface
        // for the program (i.e. a global var no longer used). If that happens we have to either fix
        // this to store to a phony value, or make sure we pull the interface before doing the dead
        // code elimination.
        if (stmt->lhs->Is<ast::PhonyExpression>()) {
            const auto* sem = program_.Sem().GetVal(stmt->rhs);
            switch (sem->Stage()) {
                case core::EvaluationStage::kRuntime:
                case core::EvaluationStage::kOverride: {
                    auto* value = EmitValueExpression(stmt->rhs);
                    auto* result = value->As<core::ir::InstructionResult>();
                    // Overrides need to be referenced if they are to be retained in single entry
                    // point in the case of phony assignment.
                    if (!result || result->Instruction()->Block() == mod.root_block) {
                        current_block_->Append(builder_.Let(value));
                    }
                } break;
                case core::EvaluationStage::kNotEvaluated:
                case core::EvaluationStage::kConstant:
                    // Don't emit.
                    break;
            }
            return;
        }

        auto lhs = EmitExpression(stmt->lhs);

        auto rhs = EmitValueExpression(stmt->rhs);
        if (!rhs) {
            return;
        }

        auto b = builder_.Append(current_block_);
        if (auto* v = std::get_if<core::ir::Value*>(&lhs)) {
            b.Store(*v, rhs);
        } else if (auto ref = std::get_if<VectorRefElementAccess>(&lhs)) {
            b.StoreVectorElement(ref->vector, ref->index, rhs);
        }
    }

    void EmitIncrementDecrement(const ast::IncrementDecrementStatement* stmt) {
        auto lhs = EmitExpression(stmt->lhs);

        auto* one = program_.TypeOf(stmt->lhs)->UnwrapRef()->IsSignedIntegerScalar()
                        ? builder_.Constant(1_i)
                        : builder_.Constant(1_u);

        EmitCompoundAssignment(lhs, one,
                               stmt->increment ? core::BinaryOp::kAdd : core::BinaryOp::kSubtract);
    }

    void EmitCompoundAssignment(const ast::CompoundAssignmentStatement* stmt) {
        auto lhs = EmitExpression(stmt->lhs);

        auto rhs = EmitValueExpression(stmt->rhs);
        if (!rhs) {
            return;
        }

        EmitCompoundAssignment(lhs, rhs, stmt->op);
    }

    void EmitCompoundAssignment(ValueOrVecElAccess lhs, core::ir::Value* rhs, core::BinaryOp op) {
        auto b = builder_.Append(current_block_);
        if (auto* v = std::get_if<core::ir::Value*>(&lhs)) {
            auto* load = b.Load(*v);
            auto* ty = load->Result()->Type();
            auto* inst = current_block_->Append(BinaryOp(ty, load->Result(), rhs, op));
            b.Store(*v, inst);
        } else if (auto ref = std::get_if<VectorRefElementAccess>(&lhs)) {
            auto* load = b.LoadVectorElement(ref->vector, ref->index);
            auto* ty = load->Result()->Type();
            auto* inst = b.Append(BinaryOp(ty, load->Result(), rhs, op));
            b.StoreVectorElement(ref->vector, ref->index, inst);
        }
    }

    void EmitBlock(const ast::BlockStatement* block) {
        scopes_.Push();
        TINT_DEFER(scopes_.Pop());

        // Note, this doesn't need to emit a Block as the current block should be sufficient as the
        // blocks all get flattened. Each flow control node will inject the basic blocks it
        // requires.
        EmitStatements(block->statements);
    }

    void EmitIf(const ast::IfStatement* stmt) {
        // Emit the if condition into the end of the preceding block
        auto reg = EmitValueExpression(stmt->condition);
        if (!reg) {
            return;
        }
        auto* if_inst = builder_.If(reg);
        current_block_->Append(if_inst);

        {
            ControlStackScope scope(this, if_inst);

            {
                TINT_SCOPED_ASSIGNMENT(current_block_, if_inst->True());
                EmitBlock(stmt->body);

                // If the true block did not terminate, then emit an exit_if
                if (NeedTerminator()) {
                    SetTerminator(builder_.ExitIf(if_inst));
                }
            }

            if (stmt->else_statement) {
                TINT_SCOPED_ASSIGNMENT(current_block_, if_inst->False());
                EmitStatement(stmt->else_statement);

                // If the false block did not terminate, then emit an exit_if
                if (NeedTerminator()) {
                    SetTerminator(builder_.ExitIf(if_inst));
                }
            }
        }
    }

    void EmitLoop(const ast::LoopStatement* stmt) {
        auto* loop_inst = builder_.Loop();
        current_block_->Append(loop_inst);

        // Note: The loop doesn't use EmitBlock because it needs the scope stack to not get popped
        // until after the continuing block.

        ControlStackScope scope(this, loop_inst);

        auto& body_behaviors = program_.Sem().Get(stmt->body)->Behaviors();
        {
            TINT_SCOPED_ASSIGNMENT(current_block_, loop_inst->Body());

            EmitStatements(stmt->body->statements);

            // The current block didn't `break`, `return` or `continue`, go to the continuing block
            // or mark the end of the block as unreachable.
            if (NeedTerminator()) {
                if (body_behaviors.Contains(sem::Behavior::kNext)) {
                    SetTerminator(builder_.Continue(loop_inst));
                } else {
                    SetTerminator(builder_.Unreachable());
                }
            }
        }

        // Emit a continuing block if it is reachable.
        if (body_behaviors.Contains(sem::Behavior::kNext) ||
            body_behaviors.Contains(sem::Behavior::kContinue)) {
            TINT_SCOPED_ASSIGNMENT(current_block_, loop_inst->Continuing());
            if (stmt->continuing) {
                EmitBlock(stmt->continuing);
            }

            // Branch back to the start block if the continue target didn't terminate already
            if (NeedTerminator()) {
                SetTerminator(builder_.NextIteration(loop_inst));
            }
        }
    }

    void EmitWhile(const ast::WhileStatement* stmt) {
        auto* loop_inst = builder_.Loop();
        current_block_->Append(loop_inst);

        ControlStackScope scope(this, loop_inst);

        // Continue is always empty, just go back to the start
        {
            TINT_SCOPED_ASSIGNMENT(current_block_, loop_inst->Continuing());
            SetTerminator(builder_.NextIteration(loop_inst));
        }

        {
            TINT_SCOPED_ASSIGNMENT(current_block_, loop_inst->Body());

            // Emit the while condition into the Start().target of the loop
            auto reg = EmitValueExpression(stmt->condition);
            if (!reg) {
                return;
            }

            // Create an `if (cond) {} else {break;}` control flow
            auto* if_inst = builder_.If(reg);
            current_block_->Append(if_inst);

            {
                TINT_SCOPED_ASSIGNMENT(current_block_, if_inst->True());
                SetTerminator(builder_.ExitIf(if_inst));
            }

            {
                TINT_SCOPED_ASSIGNMENT(current_block_, if_inst->False());
                SetTerminator(builder_.ExitLoop(loop_inst));
            }

            EmitStatements(stmt->body->statements);

            // The current block didn't `break`, `return` or `continue`, go to the continuing block.
            if (NeedTerminator()) {
                SetTerminator(builder_.Continue(loop_inst));
            }
        }
    }

    void EmitForLoop(const ast::ForLoopStatement* stmt) {
        auto* loop_inst = builder_.Loop();
        current_block_->Append(loop_inst);

        ControlStackScope scope(this, loop_inst);

        if (stmt->initializer) {
            TINT_SCOPED_ASSIGNMENT(current_block_, loop_inst->Initializer());

            // Emit the for initializer before branching to the loop body
            EmitStatement(stmt->initializer);

            if (NeedTerminator()) {
                SetTerminator(builder_.NextIteration(loop_inst));
            }
        }

        TINT_SCOPED_ASSIGNMENT(current_block_, loop_inst->Body());

        if (stmt->condition) {
            // Emit the condition into the target target of the loop body
            auto reg = EmitValueExpression(stmt->condition);
            if (!reg) {
                return;
            }

            // Create an `if (cond) {} else {break;}` control flow
            auto* if_inst = builder_.If(reg);
            current_block_->Append(if_inst);

            {
                TINT_SCOPED_ASSIGNMENT(current_block_, if_inst->True());
                SetTerminator(builder_.ExitIf(if_inst));
            }

            {
                TINT_SCOPED_ASSIGNMENT(current_block_, if_inst->False());
                SetTerminator(builder_.ExitLoop(loop_inst));
            }
        }

        EmitBlock(stmt->body);
        if (NeedTerminator()) {
            SetTerminator(builder_.Continue(loop_inst));
        }

        if (stmt->continuing) {
            TINT_SCOPED_ASSIGNMENT(current_block_, loop_inst->Continuing());
            EmitStatement(stmt->continuing);
            SetTerminator(builder_.NextIteration(loop_inst));
        }
    }

    void EmitSwitch(const ast::SwitchStatement* stmt) {
        // Emit the condition into the preceding block
        auto reg = EmitValueExpression(stmt->condition);
        if (!reg) {
            return;
        }
        auto* switch_inst = builder_.Switch(reg);
        current_block_->Append(switch_inst);

        ControlStackScope scope(this, switch_inst);

        const auto* sem = program_.Sem().Get(stmt);
        for (const auto* c : sem->Cases()) {
            Vector<core::ir::Constant*, 4> selectors;
            for (const auto* selector : c->Selectors()) {
                if (selector->IsDefault()) {
                    selectors.Push(nullptr);
                } else {
                    selectors.Push(builder_.Constant(selector->Value()->Clone(clone_ctx_)));
                }
            }

            TINT_SCOPED_ASSIGNMENT(current_block_, builder_.Case(switch_inst, selectors));
            EmitBlock(c->Body()->Declaration());

            if (NeedTerminator()) {
                SetTerminator(builder_.ExitSwitch(switch_inst));
            }
        }
    }

    void EmitReturn(const ast::ReturnStatement* stmt) {
        core::ir::Value* ret_value = nullptr;
        if (stmt->value) {
            auto ret = EmitValueExpression(stmt->value);
            if (!ret) {
                return;
            }
            ret_value = ret;
        }
        if (ret_value) {
            SetTerminator(builder_.Return(current_function_, ret_value));
        } else {
            SetTerminator(builder_.Return(current_function_));
        }
    }

    void EmitBreak(const ast::BreakStatement*) {
        auto* current_control = FindEnclosingControl(ControlFlags::kNone);
        TINT_ASSERT(current_control);

        if (auto* c = current_control->As<core::ir::Loop>()) {
            SetTerminator(builder_.ExitLoop(c));
        } else if (auto* s = current_control->As<core::ir::Switch>()) {
            SetTerminator(builder_.ExitSwitch(s));
        } else {
            TINT_UNREACHABLE();
        }
    }

    void EmitContinue(const ast::ContinueStatement*) {
        auto* current_control = FindEnclosingControl(ControlFlags::kExcludeSwitch);
        TINT_ASSERT(current_control);

        if (auto* c = current_control->As<core::ir::Loop>()) {
            SetTerminator(builder_.Continue(c));
        } else {
            TINT_UNREACHABLE();
        }
    }

    // Discard is being treated as an instruction. The semantics in WGSL is demote_to_helper, so
    // the code has to continue as before it just predicates writes. If WGSL grows some kind of
    // terminating discard that would probably make sense as a Block but would then require
    // figuring out the multi-level exit that is triggered.
    void EmitDiscard(const ast::DiscardStatement*) { current_block_->Append(builder_.Discard()); }

    void EmitBreakIf(const ast::BreakIfStatement* stmt) {
        auto* current_control = FindEnclosingControl(ControlFlags::kExcludeSwitch);

        // Emit the break-if condition into the end of the preceding block
        auto cond = EmitValueExpression(stmt->condition);
        if (!cond) {
            return;
        }
        SetTerminator(builder_.BreakIf(current_control->As<core::ir::Loop>(), cond));
    }

    ValueOrVecElAccess EmitExpression(const ast::Expression* root) {
        struct Emitter {
            explicit Emitter(Impl& i) : impl(i) {}

            ValueOrVecElAccess Emit(const ast::Expression* root) {
                // Process the root expression. This will likely add tasks.
                Process(root);

                // Execute all the tasks until all expressions have been resolved.
                while (!tasks.IsEmpty()) {
                    auto task = tasks.Pop();
                    task();
                }

                // Get the resolved root expression.
                return Get(root);
            }

          private:
            Impl& impl;
            Vector<core::ir::Block*, 8> blocks;
            Vector<std::function<void()>, 64> tasks;
            Hashmap<const ast::Expression*, ValueOrVecElAccess, 64> bindings_;

            void Bind(const ast::Expression* expr, core::ir::Value* value) {
                // If this expression maps to sem::Load, insert a load instruction to get the result
                if (impl.program_.Sem().Get<sem::Load>(expr)) {
                    auto* load = impl.builder_.Load(value);
                    impl.current_block_->Append(load);
                    value = load->Result();
                }
                bindings_.Add(expr, value);
            }

            void Bind(const ast::Expression* expr, const VectorRefElementAccess& access) {
                // If this expression maps to sem::Load, insert a load instruction to get the result
                if (impl.program_.Sem().Get<sem::Load>(expr)) {
                    auto* load = impl.builder_.LoadVectorElement(access.vector, access.index);
                    impl.current_block_->Append(load);
                    bindings_.Add(expr, load->Result());
                } else {
                    bindings_.Add(expr, access);
                }
            }

            ValueOrVecElAccess Get(const ast::Expression* expr) {
                auto val = bindings_.Get(expr);
                if (!val) {
                    return nullptr;
                }
                return *val;
            }

            core::ir::Value* GetValue(const ast::Expression* expr) {
                auto res = Get(expr);
                if (auto** val = std::get_if<core::ir::Value*>(&res)) {
                    return *val;
                }
                TINT_ICE() << "expression did not resolve to a value";
            }

            void PushBlock(core::ir::Block* block) {
                blocks.Push(impl.current_block_);
                impl.current_block_ = block;
            }

            void PopBlock() { impl.current_block_ = blocks.Pop(); }

            core::ir::Value* EmitConstant(const ast::Expression* expr) {
                if (auto* sem = impl.program_.Sem().GetVal(expr)) {
                    if (auto* v = sem->ConstantValue()) {
                        if (auto* cv = v->Clone(impl.clone_ctx_)) {
                            auto* val = impl.builder_.Constant(cv);
                            bindings_.Add(expr, val);
                            return val;
                        }
                    }
                }
                return nullptr;
            }

            void EmitAccess(const ast::AccessorExpression* expr) {
                if (auto vec_access = AsVectorRefElementAccess(expr)) {
                    Bind(expr, vec_access.value());
                    return;
                }

                auto* obj = GetValue(expr->object);
                if (!obj) {
                    TINT_UNREACHABLE() << "no object result";
                }

                auto* sem = impl.program_.Sem().Get(expr)->Unwrap();

                // The access result type should match the source result type. If the source is a
                // pointer, we generate a pointer.
                const core::type::Type* ty =
                    sem->Type()->UnwrapRef()->Clone(impl.clone_ctx_.type_ctx);
                if (auto* ptr = obj->Type()->As<core::type::Pointer>();
                    ptr && !ty->Is<core::type::Pointer>()) {
                    ty = impl.builder_.ir.Types().ptr(ptr->AddressSpace(), ty, ptr->Access());
                }

                auto index = tint::Switch(
                    sem,
                    [&](const sem::IndexAccessorExpression* idx) -> core::ir::Value* {
                        if (auto* v = idx->Index()->ConstantValue()) {
                            if (auto* cv = v->Clone(impl.clone_ctx_)) {
                                return impl.builder_.Constant(cv);
                            }
                            TINT_UNREACHABLE() << "constant clone failed";
                        }
                        return GetValue(idx->Index()->Declaration());
                    },
                    [&](const sem::StructMemberAccess* access) -> core::ir::Value* {
                        return impl.builder_.Constant(u32((access->Member()->Index())));
                    },
                    [&](const sem::Swizzle* swizzle) -> core::ir::Value* {
                        auto& indices = swizzle->Indices();

                        // A single element swizzle is just treated as an accessor.
                        if (indices.Length() == 1) {
                            return impl.builder_.Constant(u32(indices[0]));
                        }
                        auto* val = impl.builder_.Swizzle(ty, obj, std::move(indices));
                        impl.current_block_->Append(val);
                        Bind(expr, val->Result());
                        return nullptr;
                    },  //
                    TINT_ICE_ON_NO_MATCH);

                if (!index) {
                    return;
                }

                // If the object is an unnamed value (a subexpression, not a let) and is the result
                // of another access, then we can just append the index to that access.
                if (!impl.mod.NameOf(obj).IsValid()) {
                    if (auto* inst_res = obj->As<core::ir::InstructionResult>()) {
                        if (auto* access = inst_res->Instruction()->As<core::ir::Access>()) {
                            access->AddIndex(index);
                            access->Result()->SetType(ty);
                            bindings_.Remove(expr->object);
                            // Move the access after the index expression.
                            if (impl.current_block_->Back() != access) {
                                impl.current_block_->Remove(access);
                                impl.current_block_->Append(access);
                            }
                            Bind(expr, access->Result());
                            return;
                        }
                    }
                }

                // Create a new access
                auto* access = impl.builder_.Access(ty, obj, index);
                impl.current_block_->Append(access);
                Bind(expr, access->Result());
            }

            void EmitBinary(const ast::BinaryExpression* b) {
                auto* b_sem = impl.program_.Sem().Get(b);
                auto* ty = b_sem->Type()->Clone(impl.clone_ctx_.type_ctx);
                auto lhs = GetValue(b->lhs);
                if (!lhs) {
                    return;
                }
                auto rhs = GetValue(b->rhs);
                if (!rhs) {
                    return;
                }
                auto* inst = impl.BinaryOp(ty, lhs, rhs, b->op);
                if (!inst) {
                    return;
                }
                impl.current_block_->Append(inst);
                Bind(b, inst->Result());
            }

            void EmitUnary(const ast::UnaryOpExpression* expr) {
                auto val = GetValue(expr->expr);
                if (!val) {
                    return;
                }
                core::ir::Instruction* inst = nullptr;
                auto* sem = impl.program_.Sem().Get(expr);
                switch (expr->op) {
                    case core::UnaryOp::kAddressOf:
                    case core::UnaryOp::kIndirection:
                        // 'address-of' and 'indirection' just fold away and we propagate the
                        // pointer.
                        Bind(expr, val);
                        return;
                    case core::UnaryOp::kComplement: {
                        auto* ty = sem->Type()->Clone(impl.clone_ctx_.type_ctx);
                        inst = impl.builder_.Complement(ty, val);
                        break;
                    }
                    case core::UnaryOp::kNegation: {
                        auto* ty = sem->Type()->Clone(impl.clone_ctx_.type_ctx);
                        inst = impl.builder_.Negation(ty, val);
                        break;
                    }
                    case core::UnaryOp::kNot: {
                        auto* ty = sem->Type()->Clone(impl.clone_ctx_.type_ctx);
                        inst = impl.builder_.Not(ty, val);
                        break;
                    }
                }
                impl.current_block_->Append(inst);
                Bind(expr, inst->Result());
            }

            void EmitCall(const ast::CallExpression* expr) {
                // If this is a materialized semantic node, just use the constant value.
                if (auto* mat = impl.program_.Sem().Get(expr)) {
                    if (mat->ConstantValue()) {
                        auto* cv = mat->ConstantValue()->Clone(impl.clone_ctx_);
                        if (!cv) {
                            impl.AddError(expr->source) << "failed to get constant value for call "
                                                        << expr->TypeInfo().name;
                            return;
                        }
                        Bind(expr, impl.builder_.Constant(cv));
                        return;
                    }
                }
                Vector<core::ir::Value*, 8> args;
                args.Reserve(expr->args.Length());
                // Emit the arguments
                for (const auto* arg : expr->args) {
                    auto value = GetValue(arg);
                    if (!value) {
                        impl.AddError(arg->source) << "failed to convert arguments";
                        return;
                    }
                    args.Push(value);
                }
                auto* sem = impl.program_.Sem().Get<sem::Call>(expr);
                if (!sem) {
                    impl.AddError(expr->source)
                        << "failed to get semantic information for call " << expr->TypeInfo().name;
                    return;
                }
                auto* ty = sem->Target()->ReturnType()->Clone(impl.clone_ctx_.type_ctx);
                core::ir::Instruction* inst = nullptr;
                // If this is a builtin function, emit the specific builtin value
                if (auto* b = sem->Target()->As<sem::BuiltinFn>()) {
                    if (b->Fn() == wgsl::BuiltinFn::kBitcast) {
                        inst = impl.builder_.Bitcast(ty, args[0]);
                    } else {
                        auto* call =
                            impl.builder_.Call<wgsl::ir::BuiltinCall>(ty, b->Fn(), std::move(args));

                        // Set explicit template parameters if present.
                        if (b->Overload().num_explicit_templates > 0) {
                            auto* tmpl = expr->target->identifier->As<ast::TemplatedIdentifier>();
                            TINT_ASSERT(tmpl);
                            Vector<const core::type::Type*, 1> explicit_types;
                            for (uint32_t i = 0; i < b->Overload().num_explicit_templates; i++) {
                                auto* tmpl_sem = impl.program_.Sem().Get(tmpl->arguments[i]);
                                auto* tmpl_ty = tmpl_sem->As<sem::TypeExpression>();
                                TINT_ASSERT(tmpl_ty);
                                auto* cloned_ty = tmpl_ty->Type()->Clone(impl.clone_ctx_.type_ctx);
                                explicit_types.Push(cloned_ty);
                            }
                            call->SetExplicitTemplateParams(std::move(explicit_types));
                        }

                        inst = call;
                    }
                } else if (sem->Target()->As<sem::ValueConstructor>()) {
                    inst = impl.builder_.Construct(ty, std::move(args));
                } else if (sem->Target()->Is<sem::ValueConversion>()) {
                    inst = impl.builder_.Convert(ty, args[0]);
                } else if (expr->target->identifier->Is<ast::TemplatedIdentifier>()) {
                    TINT_UNIMPLEMENTED() << "missing templated ident support";
                } else {
                    // Not a builtin and not a templated call, so this is a user function.
                    inst = impl.builder_.Call(ty,
                                              impl.scopes_.Get(expr->target->identifier->symbol)
                                                  ->As<core::ir::Function>(),
                                              std::move(args));
                }
                if (inst == nullptr) {
                    return;
                }
                impl.current_block_->Append(inst);
                Bind(expr, inst->Result());
            }

            void EmitIdentifier(const ast::IdentifierExpression* i) {
                auto* v = impl.scopes_.Get(i->identifier->symbol);
                if (DAWN_UNLIKELY(!v)) {
                    impl.AddError(i->source)
                        << "unable to find identifier " << i->identifier->symbol.Name();
                    return;
                }
                Bind(i, v);
            }

            void EmitLiteral(const ast::LiteralExpression* lit) {
                auto* sem = impl.program_.Sem().Get(lit);
                if (!sem) {
                    impl.AddError(lit->source)
                        << "failed to get semantic information for node " << lit->TypeInfo().name;
                    return;
                }
                auto* cv = sem->ConstantValue()->Clone(impl.clone_ctx_);
                if (!cv) {
                    impl.AddError(lit->source)
                        << "failed to get constant value for node " << lit->TypeInfo().name;
                    return;
                }
                auto* val = impl.builder_.Constant(cv);
                Bind(lit, val);
            }

            std::optional<VectorRefElementAccess> AsVectorRefElementAccess(
                const ast::Expression* expr) {
                return AsVectorRefElementAccess(
                    impl.program_.Sem().Get<sem::ValueExpression>(expr)->UnwrapLoad());
            }

            std::optional<VectorRefElementAccess> AsVectorRefElementAccess(
                const sem::ValueExpression* expr) {
                auto* access = As<sem::AccessorExpression>(expr);
                if (!access) {
                    return std::nullopt;
                }

                auto* memory_view = access->Object()->Type()->As<core::type::MemoryView>();
                if (!memory_view) {
                    return std::nullopt;
                }

                if (!memory_view->StoreType()->Is<core::type::Vector>()) {
                    return std::nullopt;
                }
                return tint::Switch(
                    access,
                    [&](const sem::Swizzle* s) -> std::optional<VectorRefElementAccess> {
                        if (auto vec = GetValue(access->Object()->Declaration())) {
                            return VectorRefElementAccess{
                                vec, impl.builder_.Constant(u32(s->Indices()[0]))};
                        }
                        return std::nullopt;
                    },
                    [&](const sem::IndexAccessorExpression* i)
                        -> std::optional<VectorRefElementAccess> {
                        if (auto vec = GetValue(access->Object()->Declaration())) {
                            if (auto idx = GetValue(i->Index()->Declaration())) {
                                return VectorRefElementAccess{vec, idx};
                            }
                        }
                        return std::nullopt;
                    });
            }

            void BeginShortCircuit(const ast::BinaryExpression* expr) {
                auto lhs = GetValue(expr->lhs);
                if (!lhs) {
                    return;
                }

                const auto* sem = impl.program_.Sem().GetVal(expr->lhs);
                const bool is_const_eval = sem->Stage() == core::EvaluationStage::kOverride;
                auto& b = impl.builder_;
                core::ir::If* if_inst = nullptr;
                if (is_const_eval) {
                    if_inst = b.ConstExprIf(lhs);
                } else {
                    if_inst = b.If(lhs);
                }

                impl.current_block_->Append(if_inst);

                auto* result = b.InstructionResult(b.ir.Types().bool_());
                if_inst->SetResult(result);

                if (expr->op == core::BinaryOp::kLogicalAnd) {
                    if_inst->False()->Append(b.ExitIf(if_inst, b.Constant(false)));
                    PushBlock(if_inst->True());
                } else {
                    if_inst->True()->Append(b.ExitIf(if_inst, b.Constant(true)));
                    PushBlock(if_inst->False());
                }

                Bind(expr, result);
            }

            void EndShortCircuit(const ast::BinaryExpression* b) {
                auto res = GetValue(b);
                auto* src = res->As<core::ir::InstructionResult>()->Instruction();
                auto* if_ = src->As<core::ir::If>();
                TINT_ASSERT(if_);
                auto rhs = GetValue(b->rhs);
                if (!rhs) {
                    return;
                }
                impl.current_block_->Append(impl.builder_.ExitIf(if_, rhs));
                PopBlock();
            }

            void Process(const ast::Expression* expr) {
                if (EmitConstant(expr)) {
                    // If this is a value that has been const-eval'd, then no need to traverse
                    // deeper.
                    return;
                }

                tint::Switch(
                    expr,  //
                    [&](const ast::BinaryExpression* e) {
                        if (e->op == core::BinaryOp::kLogicalAnd ||
                            e->op == core::BinaryOp::kLogicalOr) {
                            tasks.Push([this, e] { EndShortCircuit(e); });
                            tasks.Push([this, e] { Process(e->rhs); });
                            tasks.Push([this, e] { BeginShortCircuit(e); });
                            tasks.Push([this, e] { Process(e->lhs); });
                        } else {
                            tasks.Push([this, e] { EmitBinary(e); });
                            tasks.Push([this, e] { Process(e->rhs); });
                            tasks.Push([this, e] { Process(e->lhs); });
                        }
                    },
                    [&](const ast::IndexAccessorExpression* e) {
                        tasks.Push([this, e] { EmitAccess(e); });
                        tasks.Push([this, e] { Process(e->index); });
                        tasks.Push([this, e] { Process(e->object); });
                    },
                    [&](const ast::MemberAccessorExpression* e) {
                        tasks.Push([this, e] { EmitAccess(e); });
                        tasks.Push([this, e] { Process(e->object); });
                    },
                    [&](const ast::UnaryOpExpression* e) {
                        tasks.Push([this, e] { EmitUnary(e); });
                        tasks.Push([this, e] { Process(e->expr); });
                    },
                    [&](const ast::CallExpression* e) {
                        tasks.Push([this, e] { EmitCall(e); });
                        for (auto* arg : tint::Reverse(e->args)) {
                            tasks.Push([this, arg] { Process(arg); });
                        }
                    },
                    [&](const ast::LiteralExpression* e) { EmitLiteral(e); },
                    [&](const ast::IdentifierExpression* e) { EmitIdentifier(e); },  //
                    TINT_ICE_ON_NO_MATCH);
            }
        };

        return Emitter(*this).Emit(root);
    }

    core::ir::Value* EmitValueExpression(const ast::Expression* root) {
        auto res = EmitExpression(root);
        if (auto** val = std::get_if<core::ir::Value*>(&res)) {
            builder_.ir.SetSource(*val, root->source);
            return *val;
        }
        TINT_ICE() << "expression did not resolve to a value";
    }

    void EmitCall(const ast::CallStatement* stmt) { (void)EmitValueExpression(stmt->expr); }

    void EmitVariable(const ast::Variable* var) {
        auto* sem = program_.Sem().Get(var);

        return tint::Switch(  //
            var,
            [&](const ast::Var* v) {
                auto* ref = sem->Type()->As<core::type::Reference>();
                const core::type::Type* store_ty = nullptr;

                const auto* ary = ref->StoreType()->As<core::type::Array>();
                // If the array has an override count
                if (ary && !ary->Count()
                                ->IsAnyOf<core::type::RuntimeArrayCount,
                                          core::type::ConstantArrayCount>()) {
                    core::ir::Value* count = tint::Switch(
                        ary->Count(),  //
                        [&](const sem::UnnamedOverrideArrayCount* u) {
                            return EmitValueExpression(u->expr->Declaration());
                        },
                        [&](const sem::NamedOverrideArrayCount* n) {
                            return scopes_.Get(n->variable->Declaration()->name->symbol);
                        },
                        TINT_ICE_ON_NO_MATCH);

                    if (!count) {
                        return;
                    }

                    auto* ary_count =
                        builder_.ir.Types().Get<core::ir::type::ValueArrayCount>(count);
                    store_ty = builder_.ir.Types().Get<core::type::Array>(
                        ary->ElemType()->Clone(clone_ctx_.type_ctx), ary_count, ary->Align(),
                        ary->Size(), ary->Stride(), ary->ImplicitStride());
                } else {
                    store_ty = ref->StoreType()->Clone(clone_ctx_.type_ctx);
                }

                auto* ty = builder_.ir.Types().Get<core::type::Pointer>(ref->AddressSpace(),
                                                                        store_ty, ref->Access());

                auto* val = builder_.Var(ty);
                if (v->initializer) {
                    auto init = EmitValueExpression(v->initializer);
                    if (!init) {
                        return;
                    }
                    val->SetInitializer(init);
                }
                current_block_->Append(val);

                if (auto* gv = sem->As<sem::GlobalVariable>(); gv) {
                    if (var->HasBindingPoint()) {
                        val->SetBindingPoint(gv->Attributes().binding_point->group,
                                             gv->Attributes().binding_point->binding);
                    }
                    if (var->HasInputAttachmentIndex()) {
                        val->SetInputAttachmentIndex(
                            gv->Attributes().input_attachment_index.value());
                    }
                }

                // Store the declaration so we can get the instruction to store too
                scopes_.Set(v->name->symbol, val->Result());

                // Record the original name and source of the var
                builder_.ir.SetName(val, v->name->symbol.Name());
                builder_.ir.SetSource(val, v->source);
            },
            [&](const ast::Let* l) {
                auto init = EmitValueExpression(l->initializer);
                if (!init) {
                    return;
                }

                auto* let = current_block_->Append(builder_.Let(l->name->symbol.Name(), init));

                // Store the results of the initialization
                scopes_.Set(l->name->symbol, let->Result());
                builder_.ir.SetSource(let, l->source);
            },
            [&](const ast::Override* o) {
                auto* o_sem = program_.Sem().Get(o);
                auto* ty = sem->Type()->Clone(clone_ctx_.type_ctx);

                auto* override = builder_.Override(ty);
                if (o->initializer) {
                    auto init = EmitValueExpression(o->initializer);
                    if (!init) {
                        return;
                    }
                    override->SetInitializer(init);
                }
                current_block_->Append(override);

                TINT_ASSERT(o_sem->Attributes().override_id.has_value());
                override->SetOverrideId(o_sem->Attributes().override_id.value());

                // Store the declaration so we can get the instruction to store too
                scopes_.Set(o->name->symbol, override->Result());

                // Record the original name and source of the var
                builder_.ir.SetName(override, o->name->symbol.Name());
                builder_.ir.SetSource(override, o->source);
            },
            [&](const ast::Const*) {
                // Skip. This should be handled by const-eval already, so the const will be a
                // `core::constant::` value at the usage sites. Can just ignore the `const` variable
                // as it should never be used.
            },
            TINT_ICE_ON_NO_MATCH);
    }

    core::ir::CoreBinary* BinaryOp(const core::type::Type* ty,
                                   core::ir::Value* lhs,
                                   core::ir::Value* rhs,
                                   core::BinaryOp op) {
        switch (op) {
            case core::BinaryOp::kAnd:
                return builder_.And(ty, lhs, rhs);
            case core::BinaryOp::kOr:
                return builder_.Or(ty, lhs, rhs);
            case core::BinaryOp::kXor:
                return builder_.Xor(ty, lhs, rhs);
            case core::BinaryOp::kEqual:
                return builder_.Equal(ty, lhs, rhs);
            case core::BinaryOp::kNotEqual:
                return builder_.NotEqual(ty, lhs, rhs);
            case core::BinaryOp::kLessThan:
                return builder_.LessThan(ty, lhs, rhs);
            case core::BinaryOp::kGreaterThan:
                return builder_.GreaterThan(ty, lhs, rhs);
            case core::BinaryOp::kLessThanEqual:
                return builder_.LessThanEqual(ty, lhs, rhs);
            case core::BinaryOp::kGreaterThanEqual:
                return builder_.GreaterThanEqual(ty, lhs, rhs);
            case core::BinaryOp::kShiftLeft:
                return builder_.ShiftLeft(ty, lhs, rhs);
            case core::BinaryOp::kShiftRight:
                return builder_.ShiftRight(ty, lhs, rhs);
            case core::BinaryOp::kAdd:
                return builder_.Add(ty, lhs, rhs);
            case core::BinaryOp::kSubtract:
                return builder_.Subtract(ty, lhs, rhs);
            case core::BinaryOp::kMultiply:
                return builder_.Multiply(ty, lhs, rhs);
            case core::BinaryOp::kDivide:
                return builder_.Divide(ty, lhs, rhs);
            case core::BinaryOp::kModulo:
                return builder_.Modulo(ty, lhs, rhs);
            case core::BinaryOp::kLogicalAnd:
            case core::BinaryOp::kLogicalOr:
                TINT_ICE() << "short circuit op should have already been handled";
        }
        TINT_UNREACHABLE();
    }
};

}  // namespace

Result<core::ir::Module> ProgramToIR(const Program& program) {
    if (!program.IsValid()) {
        return Failure{program.Diagnostics().Str()};
    }

    Impl b(program);
    auto r = b.Build();
    if (r != Success) {
        diag::List err = std::move(r.Failure().reason);
        err.AddNote(Source{}) << "AST:\n" + Program::printer(program);
        return Failure{err.Str()};
    }

    return r.Move();
}

}  // namespace tint::wgsl::reader
