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

#include "src/tint/lang/wgsl/writer/syntax_tree_printer/syntax_tree_printer.h"

#include "src/tint/lang/core/texel_format.h"
#include "src/tint/lang/wgsl/ast/alias.h"
#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/binary_expression.h"
#include "src/tint/lang/wgsl/ast/blend_src_attribute.h"
#include "src/tint/lang/wgsl/ast/bool_literal_expression.h"
#include "src/tint/lang/wgsl/ast/break_if_statement.h"
#include "src/tint/lang/wgsl/ast/break_statement.h"
#include "src/tint/lang/wgsl/ast/call_expression.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/compound_assignment_statement.h"
#include "src/tint/lang/wgsl/ast/const.h"
#include "src/tint/lang/wgsl/ast/continue_statement.h"
#include "src/tint/lang/wgsl/ast/diagnostic_attribute.h"
#include "src/tint/lang/wgsl/ast/diagnostic_rule_name.h"
#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/ast/float_literal_expression.h"
#include "src/tint/lang/wgsl/ast/for_loop_statement.h"
#include "src/tint/lang/wgsl/ast/id_attribute.h"
#include "src/tint/lang/wgsl/ast/identifier.h"
#include "src/tint/lang/wgsl/ast/identifier_expression.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"
#include "src/tint/lang/wgsl/ast/increment_decrement_statement.h"
#include "src/tint/lang/wgsl/ast/index_accessor_expression.h"
#include "src/tint/lang/wgsl/ast/int_literal_expression.h"
#include "src/tint/lang/wgsl/ast/internal_attribute.h"
#include "src/tint/lang/wgsl/ast/interpolate_attribute.h"
#include "src/tint/lang/wgsl/ast/invariant_attribute.h"
#include "src/tint/lang/wgsl/ast/let.h"
#include "src/tint/lang/wgsl/ast/loop_statement.h"
#include "src/tint/lang/wgsl/ast/member_accessor_expression.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/ast/must_use_attribute.h"
#include "src/tint/lang/wgsl/ast/override.h"
#include "src/tint/lang/wgsl/ast/phony_expression.h"
#include "src/tint/lang/wgsl/ast/return_statement.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/ast/stride_attribute.h"
#include "src/tint/lang/wgsl/ast/struct_member_align_attribute.h"
#include "src/tint/lang/wgsl/ast/struct_member_offset_attribute.h"
#include "src/tint/lang/wgsl/ast/struct_member_size_attribute.h"
#include "src/tint/lang/wgsl/ast/switch_statement.h"
#include "src/tint/lang/wgsl/ast/templated_identifier.h"
#include "src/tint/lang/wgsl/ast/unary_op_expression.h"
#include "src/tint/lang/wgsl/ast/var.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/ast/while_statement.h"
#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/wgsl/sem/switch_statement.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/strconv/float_to_string.h"
#include "src/tint/utils/text/string.h"

namespace tint::wgsl::writer {

SyntaxTreePrinter::SyntaxTreePrinter(const Program& program) : program_(program) {}

SyntaxTreePrinter::~SyntaxTreePrinter() = default;

bool SyntaxTreePrinter::Generate() {
    // Generate global declarations in the order they appear in the module.
    for (auto* decl : program_.AST().GlobalDeclarations()) {
        Switch(
            decl,  //
            [&](const ast::DiagnosticDirective* dd) { EmitDiagnosticControl(dd->control); },
            [&](const ast::Enable* e) { EmitEnable(e); },
            [&](const ast::TypeDecl* td) { EmitTypeDecl(td); },
            [&](const ast::Function* func) { EmitFunction(func); },
            [&](const ast::Variable* var) { EmitVariable(var); },
            [&](const ast::ConstAssert* ca) { EmitConstAssert(ca); },  //
            TINT_ICE_ON_NO_MATCH);

        if (decl != program_.AST().GlobalDeclarations().Back()) {
            Line();
        }
    }

    return true;
}

void SyntaxTreePrinter::EmitDiagnosticControl(const ast::DiagnosticControl& diagnostic) {
    Line() << "DiagnosticControl [severity: " << diagnostic.severity
           << ", rule: " << diagnostic.rule_name->String() << "]";
}

void SyntaxTreePrinter::EmitEnable(const ast::Enable* enable) {
    auto l = Line();
    l << "Enable [";
    for (auto* ext : enable->extensions) {
        if (ext != enable->extensions.Front()) {
            l << ", ";
        }
        l << ext->name;
    }
    l << "]";
}

void SyntaxTreePrinter::EmitTypeDecl(const ast::TypeDecl* ty) {
    Switch(
        ty,  //
        [&](const ast::Alias* alias) {
            Line() << "Alias [";
            {
                ScopedIndent ai(this);

                Line() << "name: " << alias->name->symbol.Name();
                Line() << "expr: ";
                {
                    ScopedIndent ex(this);
                    EmitExpression(alias->type);
                }
            }
            Line() << "]";
        },
        [&](const ast::Struct* str) { EmitStructType(str); },  //
        TINT_ICE_ON_NO_MATCH);
}

void SyntaxTreePrinter::EmitExpression(const ast::Expression* expr) {
    Switch(
        expr,  //
        [&](const ast::IndexAccessorExpression* a) { EmitIndexAccessor(a); },
        [&](const ast::BinaryExpression* b) { EmitBinary(b); },
        [&](const ast::CallExpression* c) { EmitCall(c); },
        [&](const ast::IdentifierExpression* i) { EmitIdentifier(i); },
        [&](const ast::LiteralExpression* l) { EmitLiteral(l); },
        [&](const ast::MemberAccessorExpression* m) { EmitMemberAccessor(m); },
        [&](const ast::PhonyExpression*) { Line() << "[PhonyExpression]"; },
        [&](const ast::UnaryOpExpression* u) { EmitUnaryOp(u); },  //
        TINT_ICE_ON_NO_MATCH);
}

void SyntaxTreePrinter::EmitIndexAccessor(const ast::IndexAccessorExpression* expr) {
    Line() << "IndexAccessorExpression [";
    {
        ScopedIndent iae(this);
        Line() << "object: ";
        {
            ScopedIndent obj(this);
            EmitExpression(expr->object);
        }

        Line() << "index: ";
        {
            ScopedIndent idx(this);
            EmitExpression(expr->index);
        }
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitMemberAccessor(const ast::MemberAccessorExpression* expr) {
    Line() << "MemberAccessorExpression [";
    {
        ScopedIndent mae(this);

        Line() << "object: ";
        {
            ScopedIndent obj(this);
            EmitExpression(expr->object);
        }
        Line() << "member: " << expr->member->symbol.Name();
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitCall(const ast::CallExpression* expr) {
    Line() << "Call [";
    {
        ScopedIndent cl(this);

        Line() << "target: [";
        {
            ScopedIndent tgt(this);
            EmitExpression(expr->target);
        }
        Line() << "]";

        if (!expr->args.IsEmpty()) {
            Line() << "args: [";
            {
                ScopedIndent args(this);
                for (auto* arg : expr->args) {
                    Line() << "arg: [";
                    {
                        ScopedIndent arg_val(this);
                        EmitExpression(arg);
                    }
                    Line() << "]";
                }
            }
            Line() << "]";
        }
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitLiteral(const ast::LiteralExpression* lit) {
    Line() << "LiteralExpression [";
    {
        ScopedIndent le(this);
        Switch(
            lit,  //
            [&](const ast::BoolLiteralExpression* l) { Line() << (l->value ? "true" : "false"); },
            [&](const ast::FloatLiteralExpression* l) {
                // f16 literals are also emitted as float value with suffix "h".
                // Note that all normal and subnormal f16 values are normal f32 values, and since
                // NaN and Inf are not allowed to be spelled in literal, it should be fine to emit
                // f16 literals in this way.
                if (l->suffix == ast::FloatLiteralExpression::Suffix::kNone) {
                    Line() << tint::strconv::DoubleToBitPreservingString(l->value);
                } else {
                    Line() << tint::strconv::FloatToBitPreservingString(
                                  static_cast<float>(l->value))
                           << l->suffix;
                }
            },
            [&](const ast::IntLiteralExpression* l) { Line() << l->value << l->suffix; },  //
            TINT_ICE_ON_NO_MATCH);
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitIdentifier(const ast::IdentifierExpression* expr) {
    Line() << "IdentifierExpression [";
    {
        ScopedIndent ie(this);
        EmitIdentifier(expr->identifier);
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitIdentifier(const ast::Identifier* ident) {
    Line() << "Identifier [";
    {
        ScopedIndent id(this);
        if (auto* tmpl_ident = ident->As<ast::TemplatedIdentifier>()) {
            Line() << "Templated [";
            {
                ScopedIndent tmpl(this);
                if (!tmpl_ident->attributes.IsEmpty()) {
                    Line() << "attrs: [";
                    {
                        ScopedIndent attrs(this);
                        EmitAttributes(tmpl_ident->attributes);
                    }
                    Line() << "]";
                }
                Line() << "name: " << ident->symbol.Name();
                if (!tmpl_ident->arguments.IsEmpty()) {
                    Line() << "args: [";
                    {
                        ScopedIndent args(this);
                        for (auto* expr : tmpl_ident->arguments) {
                            EmitExpression(expr);
                        }
                    }
                    Line() << "]";
                }
            }
            Line() << "]";
        } else {
            Line() << ident->symbol.Name();
        }
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitFunction(const ast::Function* func) {
    Line() << "Function [";
    {
        ScopedIndent funct(this);

        if (func->attributes.Length()) {
            Line() << "attrs: [";
            {
                ScopedIndent attrs(this);
                EmitAttributes(func->attributes);
            }
            Line() << "]";
        }
        Line() << "name: " << func->name->symbol.Name();

        if (!func->params.IsEmpty()) {
            Line() << "params: [";
            {
                ScopedIndent args(this);
                for (auto* v : func->params) {
                    Line() << "param: [";
                    {
                        ScopedIndent param(this);
                        Line() << "name: " << v->name->symbol.Name();
                        if (!v->attributes.IsEmpty()) {
                            Line() << "attrs: [";
                            {
                                ScopedIndent attrs(this);
                                EmitAttributes(v->attributes);
                            }
                            Line() << "]";
                        }
                        Line() << "type: [";
                        {
                            ScopedIndent ty(this);
                            EmitExpression(v->type);
                        }
                        Line() << "]";
                    }
                    Line() << "]";
                }
            }
            Line() << "]";
        }

        Line() << "return: [";
        {
            ScopedIndent ret(this);

            if (func->return_type || !func->return_type_attributes.IsEmpty()) {
                if (!func->return_type_attributes.IsEmpty()) {
                    Line() << "attrs: [";
                    {
                        ScopedIndent attrs(this);
                        EmitAttributes(func->return_type_attributes);
                    }
                    Line() << "]";
                }

                Line() << "type: [";
                {
                    ScopedIndent ty(this);
                    EmitExpression(func->return_type);
                }
                Line() << "]";
            } else {
                Line() << "void";
            }
        }
        Line() << "]";
        Line() << "body: [";
        {
            ScopedIndent bdy(this);
            if (func->body) {
                EmitBlockHeader(func->body);
                EmitStatementsWithIndent(func->body->statements);
            }
        }
        Line() << "]";
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitImageFormat(const core::TexelFormat fmt) {
    Line() << "core::TexelFormat [" << fmt << "]";
}

void SyntaxTreePrinter::EmitStructType(const ast::Struct* str) {
    Line() << "Struct [";
    {
        ScopedIndent strct(this);

        if (str->attributes.Length()) {
            Line() << "attrs: [";
            {
                ScopedIndent attrs(this);
                EmitAttributes(str->attributes);
            }
            Line() << "]";
        }
        Line() << "name: " << str->name->symbol.Name();
        Line() << "members: [";
        {
            ScopedIndent membs(this);

            for (auto* mem : str->members) {
                Line() << "StructMember[";
                {
                    ScopedIndent m(this);
                    if (!mem->attributes.IsEmpty()) {
                        Line() << "attrs: [";
                        {
                            ScopedIndent attrs(this);
                            EmitAttributes(mem->attributes);
                        }
                        Line() << "]";
                    }

                    Line() << "name: " << mem->name->symbol.Name();
                    Line() << "type: [";
                    {
                        ScopedIndent ty(this);
                        EmitExpression(mem->type);
                    }
                    Line() << "]";
                }
            }
            Line() << "]";
        }
        Line() << "]";
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitVariable(const ast::Variable* v) {
    Line() << "Variable [";
    {
        ScopedIndent variable(this);
        if (!v->attributes.IsEmpty()) {
            Line() << "attrs: [";
            {
                ScopedIndent attr(this);
                EmitAttributes(v->attributes);
            }
            Line() << "]";
        }

        Switch(
            v,  //
            [&](const ast::Var* var) {
                if (var->declared_address_space || var->declared_access) {
                    Line() << "Var [";
                    {
                        ScopedIndent vr(this);
                        Line() << "address_space: [";
                        {
                            ScopedIndent addr(this);
                            EmitExpression(var->declared_address_space);
                        }
                        Line() << "]";
                        if (var->declared_access) {
                            Line() << "access: [";
                            {
                                ScopedIndent acs(this);
                                EmitExpression(var->declared_access);
                            }
                            Line() << "]";
                        }
                    }
                    Line() << "]";
                } else {
                    Line() << "Var []";
                }
            },
            [&](const ast::Let*) { Line() << "Let []"; },
            [&](const ast::Override*) { Line() << "Override []"; },
            [&](const ast::Const*) { Line() << "Const []"; },  //
            TINT_ICE_ON_NO_MATCH);

        Line() << "name: " << v->name->symbol.Name();

        if (auto ty = v->type) {
            Line() << "type: [";
            {
                ScopedIndent vty(this);
                EmitExpression(ty);
            }
            Line() << "]";
        }

        if (v->initializer != nullptr) {
            Line() << "initializer: [";
            {
                ScopedIndent init(this);
                EmitExpression(v->initializer);
            }
            Line() << "]";
        }
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitAttributes(VectorRef<const ast::Attribute*> attrs) {
    for (auto* attr : attrs) {
        Switch(
            attr,  //
            [&](const ast::WorkgroupAttribute* workgroup) {
                auto values = workgroup->Values();
                Line() << "WorkgroupAttribute [";
                {
                    ScopedIndent wg(this);
                    for (size_t i = 0; i < 3; i++) {
                        if (values[i]) {
                            EmitExpression(values[i]);
                        }
                    }
                }
                Line() << "]";
            },
            [&](const ast::StageAttribute* stage) {
                Line() << "StageAttribute [" << stage->stage << "]";
            },
            [&](const ast::BindingAttribute* binding) {
                Line() << "BindingAttribute [";
                {
                    ScopedIndent ba(this);
                    EmitExpression(binding->expr);
                }
                Line() << "]";
            },
            [&](const ast::GroupAttribute* group) {
                Line() << "GroupAttribute [";
                {
                    ScopedIndent ga(this);
                    EmitExpression(group->expr);
                }
                Line() << "]";
            },
            [&](const ast::LocationAttribute* location) {
                Line() << "LocationAttribute [";
                {
                    ScopedIndent la(this);
                    EmitExpression(location->expr);
                }
                Line() << "]";
            },
            [&](const ast::BlendSrcAttribute* index) {
                Line() << "BlendSrcAttribute [";
                {
                    ScopedIndent idx(this);
                    EmitExpression(index->expr);
                }
                Line() << "]";
            },
            [&](const ast::BuiltinAttribute* builtin) {
                Line() << "BuiltinAttribute [" << core::ToString(builtin->builtin) << "]";
            },
            [&](const ast::DiagnosticAttribute* diagnostic) {
                EmitDiagnosticControl(diagnostic->control);
            },
            [&](const ast::InterpolateAttribute* interpolate) {
                Line() << "InterpolateAttribute [";
                {
                    ScopedIndent ia(this);
                    Line() << "type: [";
                    {
                        ScopedIndent ty(this);
                        Line() << core::ToString(interpolate->interpolation.type);
                    }
                    Line() << "]";
                    if (interpolate->interpolation.sampling !=
                        core::InterpolationSampling::kUndefined) {
                        Line() << "sampling: [";
                        {
                            ScopedIndent sa(this);
                            Line() << core::ToString(interpolate->interpolation.sampling);
                        }
                        Line() << "]";
                    }
                }
                Line() << "]";
            },
            [&](const ast::InvariantAttribute*) { Line() << "InvariantAttribute []"; },
            [&](const ast::IdAttribute* override_deco) {
                Line() << "IdAttribute [";
                {
                    ScopedIndent id(this);
                    EmitExpression(override_deco->expr);
                }
                Line() << "]";
            },
            [&](const ast::MustUseAttribute*) { Line() << "MustUseAttribute []"; },
            [&](const ast::StructMemberOffsetAttribute* offset) {
                Line() << "StructMemberOffsetAttribute [";
                {
                    ScopedIndent smoa(this);
                    EmitExpression(offset->expr);
                }
                Line() << "]";
            },
            [&](const ast::StructMemberSizeAttribute* size) {
                Line() << "StructMemberSizeAttribute [";
                {
                    ScopedIndent smsa(this);
                    EmitExpression(size->expr);
                }
                Line() << "]";
            },
            [&](const ast::StructMemberAlignAttribute* align) {
                Line() << "StructMemberAlignAttribute [";
                {
                    ScopedIndent smaa(this);
                    EmitExpression(align->expr);
                }
                Line() << "]";
            },
            [&](const ast::StrideAttribute* stride) {
                Line() << "StrideAttribute [" << stride->stride << "]";
            },
            [&](const ast::InternalAttribute* internal) {
                Line() << "InternalAttribute [" << internal->InternalName() << "]";
            },  //
            TINT_ICE_ON_NO_MATCH);
    }
}

void SyntaxTreePrinter::EmitBinary(const ast::BinaryExpression* expr) {
    Line() << "BinaryExpression [";
    {
        ScopedIndent be(this);
        Line() << "lhs: [";
        {
            ScopedIndent lhs(this);

            EmitExpression(expr->lhs);
        }
        Line() << "]";
        Line() << "op: [";
        {
            ScopedIndent op(this);
            EmitBinaryOp(expr->op);
        }
        Line() << "]";
        Line() << "rhs: [";
        {
            ScopedIndent rhs(this);
            EmitExpression(expr->rhs);
        }
        Line() << "]";
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitBinaryOp(const core::BinaryOp op) {
    switch (op) {
        case core::BinaryOp::kAnd:
            Line() << "&";
            break;
        case core::BinaryOp::kOr:
            Line() << "|";
            break;
        case core::BinaryOp::kXor:
            Line() << "^";
            break;
        case core::BinaryOp::kLogicalAnd:
            Line() << "&&";
            break;
        case core::BinaryOp::kLogicalOr:
            Line() << "||";
            break;
        case core::BinaryOp::kEqual:
            Line() << "==";
            break;
        case core::BinaryOp::kNotEqual:
            Line() << "!=";
            break;
        case core::BinaryOp::kLessThan:
            Line() << "<";
            break;
        case core::BinaryOp::kGreaterThan:
            Line() << ">";
            break;
        case core::BinaryOp::kLessThanEqual:
            Line() << "<=";
            break;
        case core::BinaryOp::kGreaterThanEqual:
            Line() << ">=";
            break;
        case core::BinaryOp::kShiftLeft:
            Line() << "<<";
            break;
        case core::BinaryOp::kShiftRight:
            Line() << ">>";
            break;
        case core::BinaryOp::kAdd:
            Line() << "+";
            break;
        case core::BinaryOp::kSubtract:
            Line() << "-";
            break;
        case core::BinaryOp::kMultiply:
            Line() << "*";
            break;
        case core::BinaryOp::kDivide:
            Line() << "/";
            break;
        case core::BinaryOp::kModulo:
            Line() << "%";
            break;
    }
}

void SyntaxTreePrinter::EmitUnaryOp(const ast::UnaryOpExpression* expr) {
    Line() << "UnaryOpExpression [";
    {
        ScopedIndent uoe(this);
        Line() << "op: [";
        {
            ScopedIndent op(this);
            switch (expr->op) {
                case core::UnaryOp::kAddressOf:
                    Line() << "&";
                    break;
                case core::UnaryOp::kComplement:
                    Line() << "~";
                    break;
                case core::UnaryOp::kIndirection:
                    Line() << "*";
                    break;
                case core::UnaryOp::kNot:
                    Line() << "!";
                    break;
                case core::UnaryOp::kNegation:
                    Line() << "-";
                    break;
            }
        }
        Line() << "]";
        Line() << "expr: [";
        {
            ScopedIndent ex(this);
            EmitExpression(expr->expr);
        }
        Line() << "]";
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitBlock(const ast::BlockStatement* stmt) {
    EmitBlockHeader(stmt);
    EmitStatementsWithIndent(stmt->statements);
}

void SyntaxTreePrinter::EmitBlockHeader(const ast::BlockStatement* stmt) {
    if (!stmt->attributes.IsEmpty()) {
        Line() << "attrs: [";
        {
            ScopedIndent attrs(this);
            EmitAttributes(stmt->attributes);
        }
        Line() << "]";
    }
}

void SyntaxTreePrinter::EmitStatement(const ast::Statement* stmt) {
    Switch(
        stmt,  //
        [&](const ast::AssignmentStatement* a) { EmitAssign(a); },
        [&](const ast::BlockStatement* b) { EmitBlock(b); },
        [&](const ast::BreakStatement* b) { EmitBreak(b); },
        [&](const ast::BreakIfStatement* b) { EmitBreakIf(b); },
        [&](const ast::CallStatement* c) { EmitCall(c->expr); },
        [&](const ast::CompoundAssignmentStatement* c) { EmitCompoundAssign(c); },
        [&](const ast::ContinueStatement* c) { EmitContinue(c); },
        [&](const ast::DiscardStatement* d) { EmitDiscard(d); },
        [&](const ast::IfStatement* i) { EmitIf(i); },
        [&](const ast::IncrementDecrementStatement* l) { EmitIncrementDecrement(l); },
        [&](const ast::LoopStatement* l) { EmitLoop(l); },
        [&](const ast::ForLoopStatement* l) { EmitForLoop(l); },
        [&](const ast::WhileStatement* l) { EmitWhile(l); },
        [&](const ast::ReturnStatement* r) { EmitReturn(r); },
        [&](const ast::ConstAssert* c) { EmitConstAssert(c); },
        [&](const ast::SwitchStatement* s) { EmitSwitch(s); },
        [&](const ast::VariableDeclStatement* v) { EmitVariable(v->variable); },  //
        TINT_ICE_ON_NO_MATCH);
}

void SyntaxTreePrinter::EmitStatements(VectorRef<const ast::Statement*> stmts) {
    for (auto* s : stmts) {
        EmitStatement(s);
    }
}

void SyntaxTreePrinter::EmitStatementsWithIndent(VectorRef<const ast::Statement*> stmts) {
    ScopedIndent si(this);
    EmitStatements(stmts);
}

void SyntaxTreePrinter::EmitAssign(const ast::AssignmentStatement* stmt) {
    Line() << "AssignmentStatement [";
    {
        ScopedIndent as(this);
        Line() << "lhs: [";
        {
            ScopedIndent lhs(this);
            EmitExpression(stmt->lhs);
        }
        Line() << "]";
        Line() << "rhs: [";
        {
            ScopedIndent rhs(this);
            EmitExpression(stmt->rhs);
        }
        Line() << "]";
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitBreak(const ast::BreakStatement*) {
    Line() << "BreakStatement []";
}

void SyntaxTreePrinter::EmitBreakIf(const ast::BreakIfStatement* b) {
    Line() << "BreakIfStatement [";
    {
        ScopedIndent bis(this);
        EmitExpression(b->condition);
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitCase(const ast::CaseStatement* stmt) {
    Line() << "CaseStatement [";
    {
        ScopedIndent cs(this);
        if (stmt->selectors.Length() == 1 && stmt->ContainsDefault()) {
            Line() << "selector: default";
            EmitBlockHeader(stmt->body);
        } else {
            Line() << "selectors: [";
            {
                ScopedIndent sels(this);
                for (auto* sel : stmt->selectors) {
                    if (sel->IsDefault()) {
                        Line() << "default []";
                    } else {
                        EmitExpression(sel->expr);
                    }
                }
            }
            Line() << "]";
            EmitBlockHeader(stmt->body);
        }
        EmitStatementsWithIndent(stmt->body->statements);
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitCompoundAssign(const ast::CompoundAssignmentStatement* stmt) {
    Line() << "CompoundAssignmentStatement [";
    {
        ScopedIndent cas(this);
        Line() << "lhs: [";
        {
            ScopedIndent lhs(this);
            EmitExpression(stmt->lhs);
        }
        Line() << "]";

        Line() << "op: [";
        {
            ScopedIndent op(this);
            EmitBinaryOp(stmt->op);
        }
        Line() << "]";
        Line() << "rhs: [";
        {
            ScopedIndent rhs(this);

            EmitExpression(stmt->rhs);
        }
        Line() << "]";
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitContinue(const ast::ContinueStatement*) {
    Line() << "ContinueStatement []";
}

void SyntaxTreePrinter::EmitIf(const ast::IfStatement* stmt) {
    {
        Line() << "IfStatement [";
        {
            ScopedIndent ifs(this);
            Line() << "condition: [";
            {
                ScopedIndent cond(this);
                EmitExpression(stmt->condition);
            }
            Line() << "]";
            EmitBlockHeader(stmt->body);
        }
        Line() << "] ";
    }
    EmitStatementsWithIndent(stmt->body->statements);

    const ast::Statement* e = stmt->else_statement;
    while (e) {
        if (auto* elseif = e->As<ast::IfStatement>()) {
            {
                Line() << "Else IfStatement [";
                {
                    ScopedIndent ifs(this);
                    Line() << "condition: [";
                    EmitExpression(elseif->condition);
                }
                Line() << "]";
                EmitBlockHeader(elseif->body);
            }
            Line() << "]";
            EmitStatementsWithIndent(elseif->body->statements);
            e = elseif->else_statement;
        } else {
            auto* body = e->As<ast::BlockStatement>();
            {
                Line() << "Else [";
                {
                    ScopedIndent els(this);
                    EmitBlockHeader(body);
                }
                Line() << "]";
            }
            EmitStatementsWithIndent(body->statements);
            break;
        }
    }
}

void SyntaxTreePrinter::EmitIncrementDecrement(const ast::IncrementDecrementStatement* stmt) {
    Line() << "IncrementDecrementStatement [";
    {
        ScopedIndent ids(this);
        Line() << "expr: [";
        EmitExpression(stmt->lhs);
        Line() << "]";
        Line() << "dir: " << (stmt->increment ? "++" : "--");
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitDiscard(const ast::DiscardStatement*) {
    Line() << "DiscardStatement []";
}

void SyntaxTreePrinter::EmitLoop(const ast::LoopStatement* stmt) {
    Line() << "LoopStatement [";
    {
        ScopedIndent ls(this);
        EmitStatements(stmt->body->statements);

        if (stmt->continuing && !stmt->continuing->Empty()) {
            Line() << "Continuing [";
            {
                ScopedIndent cont(this);
                EmitStatementsWithIndent(stmt->continuing->statements);
            }
            Line() << "]";
        }
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitForLoop(const ast::ForLoopStatement* stmt) {
    TextBuffer init_buf;
    if (auto* init = stmt->initializer) {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &init_buf);
        EmitStatement(init);
    }

    TextBuffer cont_buf;
    if (auto* cont = stmt->continuing) {
        TINT_SCOPED_ASSIGNMENT(current_buffer_, &cont_buf);
        EmitStatement(cont);
    }

    Line() << "ForLoopStatement [";
    {
        ScopedIndent fs(this);

        Line() << "initializer: [";
        {
            ScopedIndent init(this);
            switch (init_buf.lines.size()) {
                case 0:  // No initializer
                    break;
                case 1:  // Single line initializer statement
                    Line() << tint::TrimSuffix(init_buf.lines[0].content, ";");
                    break;
                default:  // Block initializer statement
                    for (size_t i = 1; i < init_buf.lines.size(); i++) {
                        // Indent all by the first line
                        init_buf.lines[i].indent += current_buffer_->current_indent;
                    }
                    Line() << tint::TrimSuffix(init_buf.String(), "\n");
                    break;
            }
        }
        Line() << "]";
        Line() << "condition: [";
        {
            ScopedIndent con(this);
            if (auto* cond = stmt->condition) {
                EmitExpression(cond);
            }
        }

        Line() << "]";
        Line() << "continuing: [";
        {
            ScopedIndent cont(this);
            switch (cont_buf.lines.size()) {
                case 0:  // No continuing
                    break;
                case 1:  // Single line continuing statement
                    Line() << tint::TrimSuffix(cont_buf.lines[0].content, ";");
                    break;
                default:  // Block continuing statement
                    for (size_t i = 1; i < cont_buf.lines.size(); i++) {
                        // Indent all by the first line
                        cont_buf.lines[i].indent += current_buffer_->current_indent;
                    }
                    Line() << tint::TrimSuffix(cont_buf.String(), "\n");
                    break;
            }
        }
        EmitBlockHeader(stmt->body);
        EmitStatementsWithIndent(stmt->body->statements);
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitWhile(const ast::WhileStatement* stmt) {
    Line() << "WhileStatement [";
    {
        ScopedIndent ws(this);
        EmitExpression(stmt->condition);
        EmitBlockHeader(stmt->body);
        EmitStatementsWithIndent(stmt->body->statements);
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitReturn(const ast::ReturnStatement* stmt) {
    Line() << "ReturnStatement [";
    {
        ScopedIndent ret(this);
        if (stmt->value) {
            EmitExpression(stmt->value);
        }
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitConstAssert(const ast::ConstAssert* stmt) {
    Line() << "ConstAssert [";
    {
        ScopedIndent ca(this);
        EmitExpression(stmt->condition);
    }
    Line() << "]";
}

void SyntaxTreePrinter::EmitSwitch(const ast::SwitchStatement* stmt) {
    Line() << "SwitchStatement [";
    {
        ScopedIndent ss(this);
        Line() << "condition: [";
        {
            ScopedIndent cond(this);
            EmitExpression(stmt->condition);
        }
        Line() << "]";

        {
            ScopedIndent si(this);
            for (auto* s : stmt->body) {
                EmitCase(s);
            }
        }
    }
    Line() << "]";
}

}  // namespace tint::wgsl::writer
