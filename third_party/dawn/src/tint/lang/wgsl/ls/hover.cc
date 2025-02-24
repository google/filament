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

#include <string_view>
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/constant/value.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/lang/wgsl/ast/const.h"
#include "src/tint/lang/wgsl/ast/identifier.h"
#include "src/tint/lang/wgsl/ast/identifier_expression.h"
#include "src/tint/lang/wgsl/ast/let.h"
#include "src/tint/lang/wgsl/ast/member_accessor_expression.h"
#include "src/tint/lang/wgsl/ast/override.h"
#include "src/tint/lang/wgsl/ast/templated_identifier.h"
#include "src/tint/lang/wgsl/ast/var.h"
#include "src/tint/lang/wgsl/ls/server.h"
#include "src/tint/lang/wgsl/ls/utils.h"
#include "src/tint/lang/wgsl/sem/behavior.h"
#include "src/tint/lang/wgsl/sem/builtin_enum_expression.h"
#include "src/tint/lang/wgsl/sem/builtin_fn.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/call_target.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/function_expression.h"
#include "src/tint/lang/wgsl/sem/load.h"
#include "src/tint/lang/wgsl/sem/materialize.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/wgsl/sem/type_expression.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string_stream.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace lsp = langsvr::lsp;

namespace tint::wgsl::ls {

namespace {

/// @returns a lsp::MarkedStringWithLanguage with the content @p wgsl, using the language `wgsl`
lsp::MarkedStringWithLanguage WGSL(std::string wgsl) {
    lsp::MarkedStringWithLanguage str;
    str.language = "wgsl";
    str.value = wgsl;
    return str;
}

/// PrintConstant() writes the constant value @p val as a WGSL value to the StringStream @p ss
void PrintConstant(const core::constant::Value* val, StringStream& ss) {
    Switch(
        val,  //
        [&](const core::constant::Scalar<AInt>* s) { ss << s->value; },
        [&](const core::constant::Scalar<AFloat>* s) { ss << s->value; },
        [&](const core::constant::Scalar<bool>* s) { ss << s->value; },
        [&](const core::constant::Scalar<f16>* s) { ss << s->value << "h"; },
        [&](const core::constant::Scalar<f32>* s) { ss << s->value << "f"; },
        [&](const core::constant::Scalar<i32>* s) { ss << s->value << "i"; },
        [&](const core::constant::Scalar<u32>* s) { ss << s->value << "u"; },
        [&](const core::constant::Splat* s) {
            ss << s->Type()->FriendlyName() << "(";
            PrintConstant(s->el, ss);
            ss << ")";
        },
        [&](const core::constant::Composite* s) {
            ss << s->Type()->FriendlyName() << "(";
            for (size_t i = 0, n = s->elements.Length(); i < n; i++) {
                if (i > 0) {
                    ss << ", ";
                }
                PrintConstant(s->elements[i], ss);
            }
            ss << ")";
        });
}

/// Variable() writes the hover information for the variable @p v to @p out
void Variable(const sem::Variable* v, std::vector<lsp::MarkedString>& out) {
    StringStream ss;
    auto* kind = Switch(
        v->Declaration(),                            //
        [&](const ast::Var*) { return "var"; },      //
        [&](const ast::Let*) { return "let"; },      //
        [&](const ast::Const*) { return "const"; },  //
        [&](const ast::Override*) { return "override"; });
    if (kind) {
        ss << kind << " ";
    }
    ss << v->Declaration()->name->symbol.NameView() << " : "
       << v->Type()->UnwrapRef()->FriendlyName();

    if (auto* init = v->Initializer()) {
        if (auto* val = init->ConstantValue()) {
            ss << " = ";
            PrintConstant(val, ss);
        }
    }

    out.push_back(WGSL(ss.str()));
}

/// Function() writes the hover information for the function @p f to @p out
void Function(const sem::Function* f, std::vector<lsp::MarkedString>& out) {
    StringStream ss;
    ss << "fn " << f->Declaration()->name->symbol.NameView();
    ss << "(";
    bool first = true;
    for (auto* param : f->Parameters()) {
        if (!first) {
            ss << ", ";
        }
        first = false;
        ss << param->Declaration()->name->symbol.NameView() << " : "
           << param->Type()->FriendlyName();
    }
    ss << ")";
    if (auto* ret = f->ReturnType(); !ret->Is<core::type::Void>()) {
        ss << " -> " << ret->FriendlyName();
    }
    out.push_back(WGSL(ss.str()));
}

/// Call() writes the hover information for a call to the function with the name @p name with
/// semantic info @p c, to @p out
void Call(std::string_view name, const sem::Call* c, std::vector<lsp::MarkedString>& out) {
    StringStream ss;
    ss << name << "(";
    bool first = true;
    for (auto* param : c->Target()->Parameters()) {
        if (!first) {
            ss << ", ";
        }
        first = false;
        if (auto* decl = param->Declaration()) {
            ss << decl->name->symbol.NameView() << " : ";
        } else if (auto usage = param->Usage(); usage != core::ParameterUsage::kNone) {
            ss << param->Usage() << " : ";
        }
        ss << param->Type()->FriendlyName();
    }
    ss << ")";
    if (auto* ret = c->Target()->ReturnType(); !ret->Is<core::type::Void>()) {
        ss << " -> " << ret->FriendlyName();
    }
    out.push_back(WGSL(ss.str()));
}

/// Constant() writes the hover information for the constant value @p val to @p out
void Constant(const core::constant::Value* val, std::vector<lsp::MarkedString>& out) {
    StringStream ss;
    PrintConstant(val, ss);
    out.push_back(WGSL(ss.str()));
}

}  // namespace

typename lsp::TextDocumentHoverRequest::ResultType  //
Server::Handle(const lsp::TextDocumentHoverRequest& r) {
    auto file = files_.Get(r.text_document.uri);
    if (!file) {
        return lsp::Null{};
    }

    auto* node =
        (*file)->NodeAt<CastableBase, File::UnwrapMode::kNoUnwrap>((*file)->Conv(r.position));
    if (!node) {
        return lsp::Null{};
    }

    std::vector<lsp::MarkedString> strings;

    if (auto* materialize = node->As<sem::Materialize>()) {
        if (auto* val = materialize->ConstantValue()) {
            Constant(val, strings);
            lsp::Hover hover;
            hover.contents = std::move(strings);
            hover.range = (*file)->Conv(materialize->Declaration()->source.range);
            return hover;
        }
    }

    langsvr::Optional<lsp::Range> range;
    Switch(
        Unwrap(node),  //
        [&](const sem::VariableUser* user) {
            Variable(user->Variable(), strings);
            range = (*file)->Conv(user->Declaration()->source.range);
        },
        [&](const sem::Variable* v) {
            Variable(v, strings);
            range = (*file)->Conv(v->Declaration()->name->source.range);
        },
        [&](const sem::Call* c) {
            Call(c->Declaration()->target->identifier->symbol.NameView(), c, strings);
            range = (*file)->Conv(c->Declaration()->target->source.range);
        },
        [&](const sem::FunctionExpression* expr) {
            Function(expr->Function(), strings);
            range = (*file)->Conv(expr->Declaration()->source.range);
        },
        [&](const sem::BuiltinEnumExpression<wgsl::BuiltinFn>* fn) {
            if (auto* call = (*file)->NodeAt<sem::Call>((*file)->Conv(r.position))) {
                Call(str(fn->Value()), call, strings);
            } else {
                strings.push_back(WGSL(str(fn->Value())));
            }
            range = (*file)->Conv(fn->Declaration()->source.range);
        },
        [&](const sem::TypeExpression* expr) {
            Switch(
                expr->Type(),  //
                [&](const sem::Struct* str) {
                    strings.push_back(WGSL("struct " + str->Name().Name()));
                },
                [&](Default) { strings.push_back(WGSL(expr->Type()->FriendlyName())); });
            range = (*file)->Conv(expr->Declaration()->source.range);
        },
        [&](const sem::StructMemberAccess* access) {
            if (auto* member = access->Member()->As<sem::StructMember>()) {
                StringStream ss;
                ss << member->Declaration()->name->symbol.NameView() << " : "
                   << member->Type()->FriendlyName();
                strings.push_back(WGSL(ss.str()));
                range = (*file)->Conv(access->Declaration()->member->source.range);
            }
        },
        [&](const sem::ValueExpression* expr) {
            if (auto* val = expr->ConstantValue()) {
                Constant(val, strings);
                range = (*file)->Conv(expr->Declaration()->source.range);
            }
        });

    if (strings.empty()) {
        return lsp::Null{};
    }

    lsp::Hover hover;
    hover.contents = std::move(strings);
    hover.range = std::move(range);
    return hover;
}

}  // namespace tint::wgsl::ls
