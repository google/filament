/*
 * Copyright (C) 2024 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_ASTRICT_COMMONTYPES_H
#define TNT_ASTRICT_COMMONTYPES_H

#include <variant>
#include <vector>

namespace astrict {

template<typename T, typename Tag>
class Newtype {
public:
    Newtype() : mValue() {}
    Newtype(const Newtype<T, Tag> &other) : mValue(other.mValue) {}
    Newtype(Newtype<T, Tag> &&other) : mValue(other.mValue) {}
    Newtype(T& value) : mValue(value) {}
    Newtype(T&& value) : mValue(value) {}

    Newtype& operator=(const Newtype<T, Tag>& other) {
        mValue = other.mValue;
        return *this;
    }

    Newtype& operator=(Newtype<T, Tag>&& other) {
        mValue = other.mValue;
        return *this;
    }

    const T& operator*() const {
        return mValue;
    }

    bool operator==(const Newtype<T, Tag>& o) const {
        return mValue == o.mValue;
    }

    bool operator<(const Newtype<T, Tag>& o) const {
        return mValue < o.mValue;
    }
private:
    T mValue;
};

using StringId = Newtype<int, struct StringIdTag>;
using TypeId = Newtype<int, struct TypeIdTag>;
using GlobalVariableId = Newtype<int, struct GlobalVariableIdTag>;
using LocalVariableId = Newtype<int, struct LocalVariableIdTag>;
using ExpressionId = Newtype<int, struct ExpressionIdTag>;
using FunctionId = Newtype<int, struct FunctionIdTag>;
using StatementBlockId = Newtype<int, struct StatementBlockIdTag>;
using StructId = Newtype<int, struct StructIdTag>;

template<class>
inline constexpr bool always_false_v = false;

}  // namespace astrict

namespace std {

template<typename Tag>
struct hash<astrict::Newtype<int, Tag>> {
    std::size_t operator()(const astrict::Newtype<int, Tag>& o) const {
        return *o;
    }
};

template<typename Tag>
struct hash<astrict::Newtype<uint16_t, Tag>> {
    std::size_t operator()(const astrict::Newtype<uint16_t, Tag>& o) const {
        return *o;
    }
};

}  // namespace std

namespace astrict {

enum class BranchOperator : uint8_t {
    Discard,             // Fragment only
    TerminateInvocation, // Fragment only
    Demote,              // Fragment only
    TerminateRayEXT,         // Any-hit only
    IgnoreIntersectionEXT,   // Any-hit only
    Return,
    Break,
    Continue,
    Case,
    Default,
};

// Represents an operation whose syntax differs from a function call. For example, addition,
// equality.
enum class ExpressionOperator : uint8_t {
    // Unary
    Negative,
    LogicalNot,
    BitwiseNot,
    PostIncrement,
    PostDecrement,
    PreIncrement,
    PreDecrement,
    ArrayLength,

    // Binary
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    RightShift,
    LeftShift,
    And,
    InclusiveOr,
    ExclusiveOr,
    Equal,
    NotEqual,
    LessThan,
    GreaterThan,
    LessThanEqual,
    GreaterThanEqual,
    LogicalOr,
    LogicalXor,
    LogicalAnd,
    Index,
    Assign,
    AddAssign,
    SubAssign,
    MulAssign,
    DivAssign,
    ModAssign,
    AndAssign,
    InclusiveOrAssign,
    ExclusiveOrAssign,
    LeftShiftAssign,
    RightShiftAssign,

    // Ternary
    Ternary,

    // Variadic
    Comma,
};

// // Workaround for missing std::expected.
// template<typename T>
// using StatusOr = std::variant<T, int>;

using VariableOrExpressionId = std::variant<GlobalVariableId, LocalVariableId, ExpressionId>;

struct StructMember {
    StringId name;
    TypeId type;

    bool operator==(const StructMember& o) const {
        return name == o.name
                && type == o.type;
    }
};

struct Struct {
    StringId name;
    std::vector<StructMember> members;

    bool operator==(const Struct& o) const {
        return name == o.name
                && members == o.members;
    }
};

struct Type {
    std::variant<StringId, StructId> name;
    std::optional<StringId> qualifiers;
    std::vector<std::size_t> arraySizes;

    bool operator==(const Type& o) const {
        return name == o.name
                && qualifiers == o.qualifiers
                && arraySizes == o.arraySizes;
    }
};

struct Variable {
    StringId name;
    std::optional<TypeId> type; // Don't record globals' types.

    bool operator==(const Variable& o) const {
        return name == o.name
                && type == o.type;
    }
};

struct ExpressionOperandExpression {
    std::variant<ExpressionOperator, FunctionId, StructId> op;
    std::vector<VariableOrExpressionId> args;

    bool operator==(const ExpressionOperandExpression& o) const {
        return op == o.op
                && args == o.args;
    }
};

using Swizzle = Newtype<uint16_t, struct SwizzleTag>;
using IndexStruct = Newtype<uint16_t, struct StructIndexTag>;

struct LiteralOperandExpression {
    VariableOrExpressionId lhs;
    std::variant<Swizzle, IndexStruct> rhs;

    bool operator==(const LiteralOperandExpression& o) const {
        return lhs == o.lhs
                && rhs == o.rhs;
    }
};

struct LiteralExpression {
    std::variant<
        bool,
        int,
        double,
        unsigned int> value;

    bool operator==(const LiteralExpression& o) const {
        return value == o.value;
    }
};

using Expression = std::variant<
    ExpressionOperandExpression,
    LiteralOperandExpression,
    LiteralExpression>;

struct Function {
    FunctionId name;
    TypeId returnType;
    std::vector<LocalVariableId> parameters;
    StatementBlockId body;
    std::unordered_map<LocalVariableId, Variable> localVariables;

    bool operator==(const Function& o) const {
        return name == o.name
                && returnType == o.returnType
                && parameters == o.parameters
                && body == o.body
                && localVariables == o.localVariables;
    }
};

struct IfStatement {
    VariableOrExpressionId condition;
    StatementBlockId thenBlock;
    std::optional<StatementBlockId> elseBlock;

    bool operator==(const IfStatement& o) const {
        return condition == o.condition
                && thenBlock == o.thenBlock
                && elseBlock == o.elseBlock;
    }
};

struct SwitchStatement {
    VariableOrExpressionId condition;
    StatementBlockId body;

    bool operator==(const SwitchStatement& o) const {
        return condition == o.condition
                && body == o.body;
    }
};

struct BranchStatement {
    BranchOperator op;
    std::optional<VariableOrExpressionId> operand;

    bool operator==(const BranchStatement& o) const {
        return op == o.op
                && operand == o.operand;
    }
};

struct LoopStatement {
    VariableOrExpressionId condition;
    std::optional<ExpressionId> terminal;
    bool testFirst;
    StatementBlockId body;

    bool operator==(const LoopStatement& o) const {
        return condition == o.condition
                && terminal == o.terminal
                && testFirst == o.testFirst
                && body == o.body;
    }
};

using Statement = std::variant<
    ExpressionId,
    IfStatement,
    SwitchStatement,
    BranchStatement,
    LoopStatement>;

template <typename T, typename... Rest>
void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hashCombine(seed, rest), ...);
}

} // namespace astrict

namespace std {

template<>
struct hash<astrict::StructMember> {
    std::size_t operator()(const astrict::StructMember& o) const {
        std::size_t result = 0;
        astrict::hashCombine(result, o.name, o.type);
        return result;
    }
};

template<>
struct hash<astrict::Struct> {
    std::size_t operator()(const astrict::Struct& o) const {
        std::size_t result = 0;
        astrict::hashCombine(result, o.name, o.members.size());
        for (const auto& member : o.members) {
            astrict::hashCombine(result, member);
        }
        return result;
    }
};

template<>
struct hash<astrict::Type> {
    std::size_t operator()(const astrict::Type& o) const {
        std::size_t result = 0;
        astrict::hashCombine(result, o.name, o.qualifiers, o.arraySizes.size());
        for (const auto& size : o.arraySizes) {
            astrict::hashCombine(result, size);
        }
        return result;
    }
};

template<>
struct hash<astrict::Variable> {
    std::size_t operator()(const astrict::Variable& o) const {
        std::size_t result = 0;
        astrict::hashCombine(result, o.name, o.type);
        return result;
    }
};

template<>
struct hash<astrict::ExpressionOperandExpression> {
    std::size_t operator()(const astrict::ExpressionOperandExpression& o) const {
        std::size_t result = 0;
        astrict::hashCombine(result, o.op, o.args.size());
        for (const auto& arg : o.args) {
            astrict::hashCombine(result, arg);
        }
        return result;
    }
};

template<>
struct hash<astrict::LiteralOperandExpression> {
    std::size_t operator()(const astrict::LiteralOperandExpression& o) const {
        std::size_t result = 0;
        astrict::hashCombine(result, o.lhs, o.rhs);
        return result;
    }
};

template<>
struct hash<astrict::LiteralExpression> {
    std::size_t operator()(const astrict::LiteralExpression& o) const {
        std::size_t result = 0;
        astrict::hashCombine(result, o.value);
        return result;
    }
};

template<>
struct hash<astrict::Function> {
    std::size_t operator()(const astrict::Function& o) const {
        std::size_t result = 0;
        astrict::hashCombine(result, o.name, o.returnType, o.body,
                o.parameters.size(), o.localVariables.size());
        for (const auto& argument : o.parameters) {
            astrict::hashCombine(result, argument);
        }
        for (const auto& symbol : o.localVariables) {
            astrict::hashCombine(result, symbol.first, symbol.second);
        }
        return result;
    }
};

template<>
struct hash<astrict::IfStatement> {
    std::size_t operator()(const astrict::IfStatement& o) const {
        std::size_t result = 0;
        astrict::hashCombine(result, o.condition, o.thenBlock, o.elseBlock);
        return result;
    }
};

template<>
struct hash<astrict::SwitchStatement> {
    std::size_t operator()(const astrict::SwitchStatement& o) const {
        std::size_t result = 0;
        astrict::hashCombine(result, o.condition, o.body);
        return result;
    }
};

template<>
struct hash<astrict::BranchStatement> {
    std::size_t operator()(const astrict::BranchStatement& o) const {
        std::size_t result = 0;
        astrict::hashCombine(result, o.op, o.operand);
        return result;
    }
};

template<>
struct hash<astrict::LoopStatement> {
    std::size_t operator()(const astrict::LoopStatement& o) const {
        std::size_t result = 0;
        astrict::hashCombine(result, o.condition, o.terminal, o.testFirst, o.body);
        return result;
    }
};

template<>
struct hash<std::vector<astrict::Statement>> {
    std::size_t operator()(const std::vector<astrict::Statement>& o) const {
        std::size_t result = o.size();
        for (const auto& statement : o) {
            astrict::hashCombine(result, statement);
        }
        return result;
    }
};

}  // namespace std

#endif  // TNT_ASTRICT_COMMONTYPES_H
