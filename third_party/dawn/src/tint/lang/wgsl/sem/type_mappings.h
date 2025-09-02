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

#ifndef SRC_TINT_LANG_WGSL_SEM_TYPE_MAPPINGS_H_
#define SRC_TINT_LANG_WGSL_SEM_TYPE_MAPPINGS_H_

#include <type_traits>

// Forward declarations
namespace tint {
class CastableBase;
}  // namespace tint
namespace tint::ast {
class AccessorExpression;
class BinaryExpression;
class BitcastExpression;
class BlockStatement;
class BuiltinAttribute;
class CallExpression;
class Expression;
class ForLoopStatement;
class Function;
class IfStatement;
class LiteralExpression;
class Node;
class Override;
class Parameter;
class PhonyExpression;
class Statement;
class Struct;
class StructMember;
class SwitchStatement;
class TypeDecl;
class UnaryOpExpression;
class Variable;
class WhileStatement;
}  // namespace tint::ast
namespace tint::core {
enum class BuiltinValue : uint8_t;
}
namespace tint::sem {
class BlockStatement;
class Expression;
class ForLoopStatement;
class Function;
class GlobalVariable;
class IfStatement;
class Parameter;
class Statement;
class Struct;
class StructMember;
class SwitchStatement;
class ValueExpression;
class Variable;
class WhileStatement;
}  // namespace tint::sem
namespace tint::core::type {
class Array;
class Type;
}  // namespace tint::core::type

namespace tint::sem {

/// TypeMappings is a struct that holds undefined `operator()` methods that's
/// used by SemanticNodeTypeFor to map AST / type node types to their
/// corresponding semantic node types. The standard operator overload resolving
/// rules will be used to infer the return type based on the argument type.
struct TypeMappings {
    //! @cond Doxygen_Suppress
    BlockStatement* operator()(ast::BlockStatement*);
    CastableBase* operator()(ast::Node*);
    Expression* operator()(ast::Expression*);
    ForLoopStatement* operator()(ast::ForLoopStatement*);
    Function* operator()(ast::Function*);
    GlobalVariable* operator()(ast::Override*);
    IfStatement* operator()(ast::IfStatement*);
    Parameter* operator()(ast::Parameter*);
    Statement* operator()(ast::Statement*);
    Struct* operator()(ast::Struct*);
    StructMember* operator()(ast::StructMember*);
    SwitchStatement* operator()(ast::SwitchStatement*);
    core::type::Type* operator()(ast::TypeDecl*);
    ValueExpression* operator()(ast::AccessorExpression*);
    ValueExpression* operator()(ast::BinaryExpression*);
    ValueExpression* operator()(ast::CallExpression*);
    ValueExpression* operator()(ast::LiteralExpression*);
    ValueExpression* operator()(ast::PhonyExpression*);
    ValueExpression* operator()(ast::UnaryOpExpression*);
    Variable* operator()(ast::Variable*);
    WhileStatement* operator()(ast::WhileStatement*);
    //! @endcond
};

/// SemanticNodeTypeFor resolves to the appropriate sem::Node type for the
/// AST node `AST`.
template <typename AST>
using SemanticNodeTypeFor =
    typename std::remove_pointer<decltype(TypeMappings()(std::declval<AST*>()))>::type;

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_TYPE_MAPPINGS_H_
