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

#include "src/tint/lang/wgsl/writer/ast_printer/ast_printer.h"

#include <string>

#include "src/tint/lang/core/enums.h"
#include "src/tint/lang/wgsl/ast/accessor_expression.h"
#include "src/tint/lang/wgsl/ast/alias.h"
#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/binary_expression.h"
#include "src/tint/lang/wgsl/ast/blend_src_attribute.h"
#include "src/tint/lang/wgsl/ast/bool_literal_expression.h"
#include "src/tint/lang/wgsl/ast/break_if_statement.h"
#include "src/tint/lang/wgsl/ast/break_statement.h"
#include "src/tint/lang/wgsl/ast/call_expression.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/color_attribute.h"
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
#include "src/tint/lang/wgsl/ast/input_attachment_index_attribute.h"
#include "src/tint/lang/wgsl/ast/int_literal_expression.h"
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
#include "src/tint/lang/wgsl/ast/struct_member_align_attribute.h"
#include "src/tint/lang/wgsl/ast/struct_member_size_attribute.h"
#include "src/tint/lang/wgsl/ast/subgroup_size_attribute.h"
#include "src/tint/lang/wgsl/ast/switch_statement.h"
#include "src/tint/lang/wgsl/ast/templated_identifier.h"
#include "src/tint/lang/wgsl/ast/unary_op_expression.h"
#include "src/tint/lang/wgsl/ast/var.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/ast/while_statement.h"
#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/reserved_words.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/wgsl/sem/switch_statement.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/math/math.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/strconv/float_to_string.h"
#include "src/tint/utils/text/string.h"

namespace tint::wgsl::writer {

ASTPrinter::ASTPrinter(const Program& program, const Options& options)
    : program_(program), options_(options) {}

ASTPrinter::~ASTPrinter() = default;

bool ASTPrinter::Generate() {
    if (options_.minify) {
        BuildUnrenameableNames();
    }

    // Generate directives before any other global declarations.
    bool has_directives = false;
    for (auto enable : program_.AST().Enables()) {
        EmitEnable(enable);
        has_directives = true;
    }
    for (auto req : program_.AST().Requires()) {
        EmitRequires(req);
        has_directives = true;
    }
    for (auto diagnostic : program_.AST().DiagnosticDirectives()) {
        auto out = Line();
        EmitDiagnosticControl(out, diagnostic->control);
        out << ";";
        has_directives = true;
    }
    if (has_directives) {
        Line();
    }
    // Generate global declarations in the order they appear in the module.
    bool has_declaration = false;
    for (auto* decl : program_.AST().GlobalDeclarations()) {
        if (decl->IsAnyOf<ast::DiagnosticDirective, ast::Enable, ast::Requires>()) {
            continue;
        }
        if (has_declaration) {
            Line();
        }
        has_declaration = true;
        Switch(
            decl,  //
            [&](const ast::TypeDecl* td) { return EmitTypeDecl(td); },
            [&](const ast::Function* func) { return EmitFunction(func); },
            [&](const ast::Variable* var) { return EmitVariable(Line(), var); },
            [&](const ast::ConstAssert* ca) { return EmitConstAssert(ca); },  //
            TINT_ICE_ON_NO_MATCH);
    }

    return !diagnostics_.ContainsErrors();
}

std::string ASTPrinter::Result() const {
    if (options_.minify) {
        return main_buffer_.MinifiedString();
    }
    return main_buffer_.String();
}

void ASTPrinter::EmitDiagnosticControl(StringStream& out,
                                       const ast::DiagnosticControl& diagnostic) {
    out << "diagnostic(" << diagnostic.severity << ", " << diagnostic.rule_name->String() << ")";
}

void ASTPrinter::EmitEnable(const ast::Enable* enable) {
    auto out = Line();
    out << "enable ";
    for (auto* ext : enable->extensions) {
        if (ext != enable->extensions.Front()) {
            out << ", ";
        }
        out << ext->name;
    }
    out << ";";
}

void ASTPrinter::EmitRequires(const ast::Requires* req) {
    auto out = Line();
    out << "requires ";
    bool first = true;
    for (auto feature : req->features) {
        if (!first) {
            out << ", ";
        }
        out << wgsl::ToString(feature);
        first = false;
    }
    out << ";";
}

void ASTPrinter::EmitTypeDecl(const ast::TypeDecl* ty) {
    Switch(
        ty,  //
        [&](const ast::Alias* alias) {
            auto out = Line();
            out << "alias " << GetSymbolName(alias->name->symbol) << " = ";
            EmitExpression(out, alias->type);
            out << ";";
        },
        [&](const ast::Struct* str) { EmitStructType(str); },  //
        TINT_ICE_ON_NO_MATCH);
}

void ASTPrinter::EmitExpression(StringStream& out, const ast::Expression* expr) {
    Switch(
        expr,  //
        [&](const ast::IndexAccessorExpression* a) { EmitIndexAccessor(out, a); },
        [&](const ast::BinaryExpression* b) { EmitBinary(out, b); },
        [&](const ast::CallExpression* c) { EmitCall(out, c); },
        [&](const ast::IdentifierExpression* i) { EmitIdentifier(out, i); },
        [&](const ast::LiteralExpression* l) { EmitLiteral(out, l); },
        [&](const ast::MemberAccessorExpression* m) { EmitMemberAccessor(out, m); },
        [&](const ast::PhonyExpression*) { out << "_"; },
        [&](const ast::UnaryOpExpression* u) { EmitUnaryOp(out, u); },  //
        TINT_ICE_ON_NO_MATCH);
}

void ASTPrinter::EmitIndexAccessor(StringStream& out, const ast::IndexAccessorExpression* expr) {
    bool paren_lhs =
        !expr->object
             ->IsAnyOf<ast::AccessorExpression, ast::CallExpression, ast::IdentifierExpression>();
    if (paren_lhs) {
        out << "(";
    }
    EmitExpression(out, expr->object);
    if (paren_lhs) {
        out << ")";
    }
    out << "[";

    EmitExpression(out, expr->index);
    out << "]";
}

void ASTPrinter::EmitMemberAccessor(StringStream& out, const ast::MemberAccessorExpression* expr) {
    bool paren_lhs =
        !expr->object
             ->IsAnyOf<ast::AccessorExpression, ast::CallExpression, ast::IdentifierExpression>();
    if (paren_lhs) {
        out << "(";
    }
    EmitExpression(out, expr->object);
    if (paren_lhs) {
        out << ")";
    }

    auto* sem = program_.Sem().GetVal(expr->object);
    auto* str = sem ? sem->Type()->UnwrapPtrOrRef()->As<core::type::Struct>() : nullptr;
    if (str && !str->IsWgslInternal()) {
        // If we are accessing a non-builtin struct, we can minify the member name.
        out << "." << GetSymbolName(expr->member->symbol);
    } else {
        // Builtin structure members and vector accessors are never renamed.
        out << "." << expr->member->symbol.NameView();
    }
}

void ASTPrinter::EmitCall(StringStream& out, const ast::CallExpression* expr) {
    EmitExpression(out, expr->target);
    out << "(";

    bool first = true;
    const auto& args = expr->args;
    for (auto* arg : args) {
        if (!first) {
            out << ", ";
        }
        first = false;

        EmitExpression(out, arg);
    }
    out << ")";
}

void ASTPrinter::EmitLiteral(StringStream& out, const ast::LiteralExpression* lit) {
    Switch(
        lit,  //
        [&](const ast::BoolLiteralExpression* l) { out << (l->value ? "true" : "false"); },
        [&](const ast::FloatLiteralExpression* l) {
            // f16 literals are also emitted as float value with suffix "h".
            // Note that all normal and subnormal f16 values are normal f32 values, and since NaN
            // and Inf are not allowed to be spelled in literal, it should be fine to emit f16
            // literals in this way.
            if (l->suffix == ast::FloatLiteralExpression::Suffix::kNone) {
                out << tint::strconv::DoubleToBitPreservingString(l->value);
            } else {
                out << tint::strconv::FloatToBitPreservingString(static_cast<float>(l->value))
                    << l->suffix;
            }
        },
        [&](const ast::IntLiteralExpression* l) { out << l->value << l->suffix; },  //
        TINT_ICE_ON_NO_MATCH);
}

void ASTPrinter::EmitIdentifier(StringStream& out, const ast::IdentifierExpression* expr) {
    EmitIdentifier(out, expr->identifier);
}

void ASTPrinter::EmitIdentifier(StringStream& out, const ast::Identifier* ident) {
    if (auto* tmpl_ident = ident->As<ast::TemplatedIdentifier>()) {
        out << GetSymbolName(ident->symbol) << "<";
        TINT_DEFER(out << ">");
        for (auto* expr : tmpl_ident->arguments) {
            if (expr != tmpl_ident->arguments.Front()) {
                out << ", ";
            }
            EmitExpression(out, expr);
        }
    } else {
        out << GetSymbolName(ident->symbol);
    }
}

void ASTPrinter::EmitFunction(const ast::Function* func) {
    if (func->attributes.Length()) {
        EmitAttributes(Line(), func->attributes);
    }
    {
        auto out = Line();
        out << "fn " << GetSymbolName(func->name->symbol) << "(";

        bool first = true;
        for (auto* v : func->params) {
            if (!first) {
                out << ", ";
            }
            first = false;

            if (!v->attributes.IsEmpty()) {
                EmitAttributes(out, v->attributes);
                out << " ";
            }

            out << GetSymbolName(v->name->symbol) << " : ";

            EmitExpression(out, v->type);
        }

        out << ")";

        if (func->return_type || !func->return_type_attributes.IsEmpty()) {
            out << " -> ";

            if (!func->return_type_attributes.IsEmpty()) {
                EmitAttributes(out, func->return_type_attributes);
                out << " ";
            }

            EmitExpression(out, func->return_type);
        }

        if (func->body) {
            out << " ";
            EmitBlockHeader(out, func->body);
        }
    }

    if (func->body) {
        EmitStatementsWithIndent(func->body->statements);
        Line() << "}";
    }
}

void ASTPrinter::EmitImageFormat(StringStream& out, const core::TexelFormat fmt) {
    switch (fmt) {
        case core::TexelFormat::kUndefined:
            diagnostics_.AddError(Source{}) << "unknown image format";
            break;
        default:
            out << fmt;
            break;
    }
}

void ASTPrinter::EmitStructType(const ast::Struct* str) {
    if (str->attributes.Length()) {
        EmitAttributes(Line(), str->attributes);
    }
    Line() << "struct " << GetSymbolName(str->name->symbol) << " {";

    Hashset<std::string, 8> member_names;
    for (auto* mem : str->members) {
        member_names.Add(std::string(GetSymbolName(mem->name->symbol)));
    }
    size_t padding_idx = 0;
    auto new_padding_name = [&] {
        while (true) {
            auto name = "padding_" + tint::ToString(padding_idx++);
            if (member_names.Add(name)) {
                return name;
            }
        }
    };

    auto add_padding = [&](uint32_t size) {
        Line() << "@size(" << size << ")";

        // Note: u32 is the smallest primitive we currently support. When WGSL
        // supports smaller types, this will need to be updated.
        Line() << new_padding_name() << " : u32,";
    };

    IncrementIndent();
    uint32_t offset = 0;
    for (auto* mem : str->members) {
        // Emit comments to show non-default struct member offsets.
        if (auto* mem_sem = program_.Sem().Get(mem)) {
            offset = tint::RoundUp(mem_sem->Align(), offset);
            if (mem_sem->Offset() != offset) {
                Line() << "/* offset(" << mem_sem->Offset() << ") */";
            }

            if (uint32_t padding = mem_sem->Offset() - offset) {
                add_padding(padding);
                offset += padding;
            }

            offset += mem_sem->Size();
        }

        if (!mem->attributes.IsEmpty()) {
            EmitAttributes(Line(), mem->attributes);
        }

        auto out = Line();
        out << GetSymbolName(mem->name->symbol) << " : ";
        EmitExpression(out, mem->type);
        out << ",";
    }
    DecrementIndent();

    Line() << "}";
}

void ASTPrinter::EmitVariable(StringStream& out, const ast::Variable* v) {
    if (!v->attributes.IsEmpty()) {
        EmitAttributes(out, v->attributes);
        out << " ";
    }

    Switch(
        v,  //
        [&](const ast::Var* var) {
            out << "var";
            if (var->declared_address_space || var->declared_access) {
                out << "<";
                TINT_DEFER(out << ">");
                EmitExpression(out, var->declared_address_space);
                if (var->declared_access) {
                    out << ", ";
                    EmitExpression(out, var->declared_access);
                }
            }
        },
        [&](const ast::Let*) { out << "let"; }, [&](const ast::Override*) { out << "override"; },
        [&](const ast::Const*) { out << "const"; },  //
        TINT_ICE_ON_NO_MATCH);

    out << " " << GetSymbolName(v->name->symbol);

    if (auto ty = v->type) {
        out << " : ";
        EmitExpression(out, ty);
    }

    if (v->initializer != nullptr) {
        out << " = ";
        EmitExpression(out, v->initializer);
    }
    out << ";";
}

void ASTPrinter::EmitAttributes(StringStream& out, VectorRef<const ast::Attribute*> attrs) {
    bool first = true;
    for (auto* attr : attrs) {
        if (!first) {
            out << " ";
        }
        first = false;
        out << "@";
        Switch(
            attr,  //
            [&](const ast::WorkgroupAttribute* workgroup) {
                auto values = workgroup->Values();
                out << "workgroup_size(";
                for (size_t i = 0; i < 3; i++) {
                    if (values[i]) {
                        if (i > 0) {
                            out << ", ";
                        }
                        EmitExpression(out, values[i]);
                    }
                }
                out << ")";
            },
            [&](const ast::StageAttribute* stage) { out << stage->stage; },
            [&](const ast::BindingAttribute* binding) {
                out << "binding(";
                EmitExpression(out, binding->expr);
                out << ")";
            },
            [&](const ast::GroupAttribute* group) {
                out << "group(";
                EmitExpression(out, group->expr);
                out << ")";
            },
            [&](const ast::LocationAttribute* location) {
                out << "location(";
                EmitExpression(out, location->expr);
                out << ")";
            },
            [&](const ast::ColorAttribute* color) {
                out << "color(";
                EmitExpression(out, color->expr);
                out << ")";
            },
            [&](const ast::BlendSrcAttribute* blend_src) {
                out << "blend_src(";
                EmitExpression(out, blend_src->expr);
                out << ")";
            },
            [&](const ast::BuiltinAttribute* builtin) {
                out << "builtin(";
                out << core::ToString(builtin->builtin);
                out << ")";
            },
            [&](const ast::DiagnosticAttribute* diagnostic) {
                EmitDiagnosticControl(out, diagnostic->control);
            },
            [&](const ast::InterpolateAttribute* interpolate) {
                out << "interpolate(";
                out << core::ToString(interpolate->interpolation.type);
                if (interpolate->interpolation.sampling !=
                    core::InterpolationSampling::kUndefined) {
                    out << ", ";
                    out << core::ToString(interpolate->interpolation.sampling);
                }
                out << ")";
            },
            [&](const ast::InvariantAttribute*) { out << "invariant"; },
            [&](const ast::IdAttribute* override_deco) {
                out << "id(";
                EmitExpression(out, override_deco->expr);
                out << ")";
            },
            [&](const ast::MustUseAttribute*) { out << "must_use"; },
            [&](const ast::StructMemberSizeAttribute* size) {
                out << "size(";
                EmitExpression(out, size->expr);
                out << ")";
            },
            [&](const ast::StructMemberAlignAttribute* align) {
                out << "align(";
                EmitExpression(out, align->expr);
                out << ")";
            },
            [&](const ast::InputAttachmentIndexAttribute* index) {
                out << "input_attachment_index(";
                EmitExpression(out, index->expr);
                out << ")";
            },
            [&](const ast::SubgroupSizeAttribute* subgroup_size) {
                out << "subgroup_size(";
                EmitExpression(out, subgroup_size->subgroup_size);
                out << ")";
            },  //
            TINT_ICE_ON_NO_MATCH);
    }
}

void ASTPrinter::EmitBinary(StringStream& out, const ast::BinaryExpression* expr) {
    out << "(";

    EmitExpression(out, expr->lhs);
    out << " ";
    EmitBinaryOp(out, expr->op);
    out << " ";

    EmitExpression(out, expr->rhs);
    out << ")";
}

void ASTPrinter::EmitBinaryOp(StringStream& out, const core::BinaryOp op) {
    switch (op) {
        case core::BinaryOp::kAnd:
            out << "&";
            return;
        case core::BinaryOp::kOr:
            out << "|";
            return;
        case core::BinaryOp::kXor:
            out << "^";
            return;
        case core::BinaryOp::kLogicalAnd:
            out << "&&";
            return;
        case core::BinaryOp::kLogicalOr:
            out << "||";
            return;
        case core::BinaryOp::kEqual:
            out << "==";
            return;
        case core::BinaryOp::kNotEqual:
            out << "!=";
            return;
        case core::BinaryOp::kLessThan:
            out << "<";
            return;
        case core::BinaryOp::kGreaterThan:
            out << ">";
            return;
        case core::BinaryOp::kLessThanEqual:
            out << "<=";
            return;
        case core::BinaryOp::kGreaterThanEqual:
            out << ">=";
            return;
        case core::BinaryOp::kShiftLeft:
            out << "<<";
            return;
        case core::BinaryOp::kShiftRight:
            out << ">>";
            return;
        case core::BinaryOp::kAdd:
            out << "+";
            return;
        case core::BinaryOp::kSubtract:
            out << "-";
            return;
        case core::BinaryOp::kMultiply:
            out << "*";
            return;
        case core::BinaryOp::kDivide:
            out << "/";
            return;
        case core::BinaryOp::kModulo:
            out << "%";
            return;
    }
    TINT_ICE() << "invalid binary op " << op;
}

void ASTPrinter::EmitUnaryOp(StringStream& out, const ast::UnaryOpExpression* expr) {
    switch (expr->op) {
        case core::UnaryOp::kAddressOf:
            out << "&";
            break;
        case core::UnaryOp::kComplement:
            out << "~";
            break;
        case core::UnaryOp::kIndirection:
            out << "*";
            break;
        case core::UnaryOp::kNot:
            out << "!";
            break;
        case core::UnaryOp::kNegation:
            out << "-";
            break;
    }
    out << "(";
    EmitExpression(out, expr->expr);
    out << ")";
}

void ASTPrinter::EmitBlock(const ast::BlockStatement* stmt) {
    {
        auto out = Line();
        EmitBlockHeader(out, stmt);
    }
    EmitStatementsWithIndent(stmt->statements);
    Line() << "}";
}

void ASTPrinter::EmitBlockHeader(StringStream& out, const ast::BlockStatement* stmt) {
    if (!stmt->attributes.IsEmpty()) {
        EmitAttributes(out, stmt->attributes);
        out << " ";
    }
    out << "{";
}

void ASTPrinter::EmitStatement(const ast::Statement* stmt) {
    Switch(
        stmt,  //
        [&](const ast::AssignmentStatement* a) { EmitAssign(a); },
        [&](const ast::BlockStatement* b) { EmitBlock(b); },
        [&](const ast::BreakStatement* b) { EmitBreak(b); },
        [&](const ast::BreakIfStatement* b) { EmitBreakIf(b); },
        [&](const ast::CallStatement* c) {
            auto out = Line();
            EmitCall(out, c->expr);
            out << ";";
        },
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
        [&](const ast::VariableDeclStatement* v) { EmitVariable(Line(), v->variable); },  //
        TINT_ICE_ON_NO_MATCH);
}

void ASTPrinter::EmitStatements(VectorRef<const ast::Statement*> stmts) {
    for (auto* s : stmts) {
        EmitStatement(s);
    }
}

void ASTPrinter::EmitStatementsWithIndent(VectorRef<const ast::Statement*> stmts) {
    ScopedIndent si(this);
    EmitStatements(stmts);
}

void ASTPrinter::EmitAssign(const ast::AssignmentStatement* stmt) {
    auto out = Line();

    EmitExpression(out, stmt->lhs);
    out << " = ";
    EmitExpression(out, stmt->rhs);
    out << ";";
}

void ASTPrinter::EmitBreak(const ast::BreakStatement*) {
    Line() << "break;";
}

void ASTPrinter::EmitBreakIf(const ast::BreakIfStatement* b) {
    auto out = Line();

    out << "break if ";
    EmitExpression(out, b->condition);
    out << ";";
}

void ASTPrinter::EmitCase(const ast::CaseStatement* stmt) {
    if (stmt->selectors.Length() == 1 && stmt->ContainsDefault()) {
        auto out = Line();
        out << "default: ";
        EmitBlockHeader(out, stmt->body);
    } else {
        auto out = Line();
        out << "case ";

        bool first = true;
        for (auto* sel : stmt->selectors) {
            if (!first) {
                out << ", ";
            }

            first = false;

            if (sel->IsDefault()) {
                out << "default";
            } else {
                EmitExpression(out, sel->expr);
            }
        }
        out << ": ";
        EmitBlockHeader(out, stmt->body);
    }
    EmitStatementsWithIndent(stmt->body->statements);
    Line() << "}";
}

void ASTPrinter::EmitCompoundAssign(const ast::CompoundAssignmentStatement* stmt) {
    auto out = Line();

    EmitExpression(out, stmt->lhs);
    out << " ";
    EmitBinaryOp(out, stmt->op);
    out << "= ";
    EmitExpression(out, stmt->rhs);
    out << ";";
}

void ASTPrinter::EmitContinue(const ast::ContinueStatement*) {
    Line() << "continue;";
}

void ASTPrinter::EmitIf(const ast::IfStatement* stmt) {
    {
        auto out = Line();

        if (!stmt->attributes.IsEmpty()) {
            EmitAttributes(out, stmt->attributes);
            out << " ";
        }

        out << "if (";
        EmitExpression(out, stmt->condition);
        out << ") ";
        EmitBlockHeader(out, stmt->body);
    }

    EmitStatementsWithIndent(stmt->body->statements);

    const ast::Statement* e = stmt->else_statement;
    while (e) {
        if (auto* elseif = e->As<ast::IfStatement>()) {
            {
                auto out = Line();
                out << "} else if (";
                EmitExpression(out, elseif->condition);
                out << ") ";
                EmitBlockHeader(out, elseif->body);
            }
            EmitStatementsWithIndent(elseif->body->statements);
            e = elseif->else_statement;
        } else {
            auto* body = e->As<ast::BlockStatement>();
            {
                auto out = Line();
                out << "} else ";
                EmitBlockHeader(out, body);
            }
            EmitStatementsWithIndent(body->statements);
            break;
        }
    }

    Line() << "}";
}

void ASTPrinter::EmitIncrementDecrement(const ast::IncrementDecrementStatement* stmt) {
    auto out = Line();
    EmitExpression(out, stmt->lhs);
    out << (stmt->increment ? "++" : "--") << ";";
}

void ASTPrinter::EmitDiscard(const ast::DiscardStatement*) {
    Line() << "discard;";
}

void ASTPrinter::EmitLoop(const ast::LoopStatement* stmt) {
    {
        auto out = Line();

        if (!stmt->attributes.IsEmpty()) {
            EmitAttributes(out, stmt->attributes);
            out << " ";
        }

        out << "loop ";
        EmitBlockHeader(out, stmt->body);
    }
    IncrementIndent();

    EmitStatements(stmt->body->statements);

    if (stmt->continuing && !stmt->continuing->Empty()) {
        Line();
        {
            auto out = Line();
            out << "continuing ";
            if (!stmt->continuing->attributes.IsEmpty()) {
                EmitAttributes(out, stmt->continuing->attributes);
                out << " ";
            }
            out << "{";
        }
        EmitStatementsWithIndent(stmt->continuing->statements);
        Line() << "}";
    }

    DecrementIndent();
    Line() << "}";
}

void ASTPrinter::EmitForLoop(const ast::ForLoopStatement* stmt) {
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

    {
        auto out = Line();

        if (!stmt->attributes.IsEmpty()) {
            EmitAttributes(out, stmt->attributes);
            out << " ";
        }

        out << "for";
        {
            ScopedParen sp(out);
            switch (init_buf.lines.size()) {
                case 0:  // No initializer
                    break;
                case 1:  // Single line initializer statement
                    out << tint::TrimSuffix(init_buf.lines[0].content, ";");
                    break;
                default:  // Block initializer statement
                    for (size_t i = 1; i < init_buf.lines.size(); i++) {
                        // Indent all by the first line
                        init_buf.lines[i].indent += current_buffer_->current_indent;
                    }
                    out << tint::TrimSuffix(init_buf.String(), "\n");
                    break;
            }

            out << "; ";

            if (auto* cond = stmt->condition) {
                EmitExpression(out, cond);
            }

            out << "; ";

            switch (cont_buf.lines.size()) {
                case 0:  // No continuing
                    break;
                case 1:  // Single line continuing statement
                    out << tint::TrimSuffix(cont_buf.lines[0].content, ";");
                    break;
                default:  // Block continuing statement
                    for (size_t i = 1; i < cont_buf.lines.size(); i++) {
                        // Indent all by the first line
                        cont_buf.lines[i].indent += current_buffer_->current_indent;
                    }
                    out << tint::TrimSuffix(cont_buf.String(), "\n");
                    break;
            }
        }
        out << " ";
        EmitBlockHeader(out, stmt->body);
    }

    EmitStatementsWithIndent(stmt->body->statements);

    Line() << "}";
}

void ASTPrinter::EmitWhile(const ast::WhileStatement* stmt) {
    {
        auto out = Line();

        if (!stmt->attributes.IsEmpty()) {
            EmitAttributes(out, stmt->attributes);
            out << " ";
        }

        out << "while";
        {
            ScopedParen sp(out);

            auto* cond = stmt->condition;
            EmitExpression(out, cond);
        }
        out << " ";
        EmitBlockHeader(out, stmt->body);
    }

    EmitStatementsWithIndent(stmt->body->statements);

    Line() << "}";
}

void ASTPrinter::EmitReturn(const ast::ReturnStatement* stmt) {
    auto out = Line();

    out << "return";
    if (stmt->value) {
        out << " ";
        EmitExpression(out, stmt->value);
    }
    out << ";";
}

void ASTPrinter::EmitConstAssert(const ast::ConstAssert* stmt) {
    auto out = Line();
    out << "const_assert ";
    EmitExpression(out, stmt->condition);
    out << ";";
}

void ASTPrinter::EmitSwitch(const ast::SwitchStatement* stmt) {
    {
        auto out = Line();

        if (!stmt->attributes.IsEmpty()) {
            EmitAttributes(out, stmt->attributes);
            out << " ";
        }

        out << "switch(";
        EmitExpression(out, stmt->condition);
        out << ") ";

        if (!stmt->body_attributes.IsEmpty()) {
            EmitAttributes(out, stmt->body_attributes);
            out << " ";
        }

        out << "{";
    }

    {
        ScopedIndent si(this);
        for (auto* s : stmt->body) {
            EmitCase(s);
        }
    }

    Line() << "}";
}

void ASTPrinter::BuildUnrenameableNames() {
    // Find all the entry points and overrides and mark their names as unrenameable.
    for (auto* func : program_.AST().Functions()) {
        if (func->IsEntryPoint()) {
            unrenameable_.Add(func->name->symbol.NameView());
        }
    }
    for (auto* var : program_.AST().GlobalVariables()) {
        if (var->Is<ast::Override>()) {
            unrenameable_.Add(var->name->symbol.NameView());
        }
    }
}

std::string ASTPrinter::NextMinifiedName() {
    constexpr std::string_view kMinifiedIdentifierChars =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    constexpr auto kNumMinifiedIdentifierChars = kMinifiedIdentifierChars.length();

    // Three characters is 140K names.
    std::string name;
    name.reserve(3);

    auto idx = next_minified_name_idx_++;
    while (true) {
        // Add the next character based on the modulo of the index.
        name += kMinifiedIdentifierChars[idx % kNumMinifiedIdentifierChars];

        // Reduce the index by a factor of `kNumMinifiedIdentifierChars`.
        idx /= kNumMinifiedIdentifierChars;
        if (idx == 0) {
            break;
        }
        idx--;
    }
    return name;
}

std::string_view ASTPrinter::GetSymbolName(tint::Symbol sym) {
    if (!options_.minify) {
        return sym.NameView();
    }

    // Returns `true` if `name` can be renamed, or can be used as a minified identifier.
    auto can_rename_to_or_from = [&](std::string_view name) {
        if (unrenameable_.Contains(name)) {
            return false;
        }
        if (IsReserved(name) || IsKeyword(name) || IsEnumName(name) || IsTypeName(name) ||
            wgsl::ParseBuiltinFn(name) != wgsl::BuiltinFn::kNone) {
            return false;
        }
        return true;
    };

    return symbol_to_name_.GetOrAdd(sym, [&] {
        if (!can_rename_to_or_from(sym.NameView())) {
            return sym.Name();
        }

        // Generate minified names until we find one that we can use.
        while (true) {
            auto name = NextMinifiedName();
            if (can_rename_to_or_from(name)) {
                return name;
            }
        }
    });
}

}  // namespace tint::wgsl::writer
