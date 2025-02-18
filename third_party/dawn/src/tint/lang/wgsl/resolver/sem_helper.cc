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

#include "src/tint/lang/wgsl/resolver/sem_helper.h"

#include "src/tint/lang/wgsl/resolver/incomplete_type.h"
#include "src/tint/lang/wgsl/resolver/unresolved_identifier.h"
#include "src/tint/lang/wgsl/sem/builtin_enum_expression.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/function_expression.h"
#include "src/tint/lang/wgsl/sem/type_expression.h"
#include "src/tint/lang/wgsl/sem/value_expression.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/text_style.h"

namespace tint::resolver {

SemHelper::SemHelper(ProgramBuilder* builder) : builder_(builder) {}

SemHelper::~SemHelper() = default;

std::string SemHelper::TypeNameOf(const core::type::Type* ty) const {
    return RawTypeNameOf(ty->UnwrapRef());
}

std::string SemHelper::RawTypeNameOf(const core::type::Type* ty) const {
    return ty->FriendlyName();
}

core::type::Type* SemHelper::TypeOf(const ast::Expression* expr) const {
    auto* sem = GetVal(expr);
    return sem ? const_cast<core::type::Type*>(sem->Type()) : nullptr;
}

sem::TypeExpression* SemHelper::AsTypeExpression(sem::Expression* expr) const {
    if (DAWN_UNLIKELY(!expr)) {
        return nullptr;
    }

    auto* ty_expr = expr->As<sem::TypeExpression>();
    if (DAWN_UNLIKELY(!ty_expr)) {
        ErrorUnexpectedExprKind(expr, "type");
        return nullptr;
    }

    auto* type = ty_expr->Type();
    if (auto* incomplete = type->As<IncompleteType>(); DAWN_UNLIKELY(incomplete)) {
        AddError(expr->Declaration()->source.End())
            << "expected " << style::Code("<") << " for " << style::Type(incomplete->builtin);
        return nullptr;
    }

    return ty_expr;
}

StyledText SemHelper::Describe(const sem::Expression* expr) const {
    StyledText text;

    Switch(
        expr,  //
        [&](const sem::VariableUser* var_expr) {
            auto* variable = var_expr->Variable()->Declaration();
            auto name = variable->name->symbol.NameView();
            Switch(
                variable,                                                                         //
                [&](const ast::Var*) { text << style::Keyword("var") << style::Code(" "); },      //
                [&](const ast::Let*) { text << style::Keyword("let") << style::Code(" "); },      //
                [&](const ast::Const*) { text << style::Keyword("const") << style::Code(" "); },  //
                [&](const ast::Parameter*) { text << "parameter "; },                             //
                [&](const ast::Override*) {
                    text << style::Keyword("override") << style::Code(" ");
                },  //
                [&](Default) { text << "variable "; });
            text << style::Variable(name);
        },
        [&](const sem::ValueExpression* val_expr) {
            text << "value of type " << style::Type(val_expr->Type()->FriendlyName());
        },
        [&](const sem::TypeExpression* ty_expr) {
            text << "type " << style::Type(ty_expr->Type()->FriendlyName());
        },
        [&](const sem::FunctionExpression* fn_expr) {
            auto* fn = fn_expr->Function()->Declaration();
            text << "function " << style::Function(fn->name->symbol.NameView());
        },
        [&](const sem::BuiltinEnumExpression<wgsl::BuiltinFn>* fn) {
            text << "builtin function " << style::Function(fn->Value());
        },
        [&](const sem::BuiltinEnumExpression<core::Access>* access) {
            text << "access " << style::Enum(access->Value());
        },
        [&](const sem::BuiltinEnumExpression<core::AddressSpace>* addr) {
            text << "address space " << style::Enum(addr->Value());
        },
        [&](const sem::BuiltinEnumExpression<core::TexelFormat>* fmt) {
            text << "texel format " << style::Enum(fmt->Value());
        },
        [&](const UnresolvedIdentifier* ui) {
            auto name = ui->Identifier()->identifier->symbol.NameView();
            text << "unresolved identifier " << style::Code(name);
        },  //
        TINT_ICE_ON_NO_MATCH);

    return text;
}

void SemHelper::ErrorUnexpectedIdent(
    const ast::Identifier* ident,
    std::string_view wanted,
    tint::Slice<const std::string_view> suggestions /* = Empty */) const {
    auto name = ident->symbol.NameView();
    AddError(ident->source) << "unresolved " << wanted << " " << style::Code(name);
    if (!suggestions.IsEmpty()) {
        // Filter out suggestions that have a leading underscore.
        Vector<std::string_view, 8> filtered;
        for (auto str : suggestions) {
            if (str[0] != '_') {
                filtered.Push(str);
            }
        }
        auto& note = AddNote(ident->source);
        SuggestAlternativeOptions opts;
        opts.alternatives_style = style::Enum;
        SuggestAlternatives(name, filtered.Slice(), note.message, opts);
    }
}

void SemHelper::ErrorUnexpectedExprKind(
    const sem::Expression* expr,
    std::string_view wanted,
    tint::Slice<const std::string_view> suggestions /* = Empty */) const {
    if (auto* ui = expr->As<UnresolvedIdentifier>()) {
        return ErrorUnexpectedIdent(ui->Identifier()->identifier, wanted, suggestions);
    }

    AddError(expr->Declaration()->source) << "cannot use " << Describe(expr) << " as " << wanted;
    NoteDeclarationSource(expr->Declaration());
}

void SemHelper::ErrorExpectedValueExpr(const sem::Expression* expr) const {
    ErrorUnexpectedExprKind(expr, "value");
    if (auto* ident = expr->Declaration()->As<ast::IdentifierExpression>()) {
        if (expr->IsAnyOf<sem::FunctionExpression, sem::TypeExpression,
                          sem::BuiltinEnumExpression<wgsl::BuiltinFn>>()) {
            AddNote(ident->source.End())
                << "are you missing " << style::Code("()") << style::Plain("?");
        }
    }
}

void SemHelper::NoteDeclarationSource(const ast::Node* node) const {
    if (!node) {
        return;
    }

    Switch(
        Get(node),  //
        [&](const sem::VariableUser* var_expr) { node = var_expr->Variable()->Declaration(); },
        [&](const sem::TypeExpression* ty_expr) {
            Switch(ty_expr->Type(),  //
                   [&](const sem::Struct* s) { node = s->Declaration(); });
        },
        [&](const sem::FunctionExpression* fn_expr) { node = fn_expr->Function()->Declaration(); });

    Switch(
        node,
        [&](const ast::Struct* n) {
            AddNote(n->source) << style::Keyword("struct ")
                               << style::Type(n->name->symbol.NameView()) << " declared here";
        },
        [&](const ast::Alias* n) {
            AddNote(n->source) << style::Keyword("alias ")
                               << style::Type(n->name->symbol.NameView()) << " declared here";
        },
        [&](const ast::Var* n) {
            AddNote(n->source) << style::Keyword("var ")
                               << style::Variable(n->name->symbol.NameView()) << " declared here";
        },
        [&](const ast::Let* n) {
            AddNote(n->source) << style::Keyword("let ")
                               << style::Variable(n->name->symbol.NameView()) << " declared here";
        },
        [&](const ast::Override* n) {
            AddNote(n->source) << style::Keyword("override ")
                               << style::Variable(n->name->symbol.NameView()) << " declared here";
        },
        [&](const ast::Const* n) {
            AddNote(n->source) << style::Keyword("const ")
                               << style::Variable(n->name->symbol.NameView()) << " declared here";
        },
        [&](const ast::Parameter* n) {
            AddNote(n->source) << "parameter " << style::Variable(n->name->symbol.NameView())
                               << " declared here";
        },
        [&](const ast::Function* n) {
            AddNote(n->source) << "function " << style::Function(n->name->symbol.NameView())
                               << " declared here";
        });
}

diag::Diagnostic& SemHelper::AddError(const Source& source) const {
    return builder_->Diagnostics().AddError(source);
}

diag::Diagnostic& SemHelper::AddWarning(const Source& source) const {
    return builder_->Diagnostics().AddWarning(source);
}

diag::Diagnostic& SemHelper::AddNote(const Source& source) const {
    return builder_->Diagnostics().AddNote(source);
}
}  // namespace tint::resolver
