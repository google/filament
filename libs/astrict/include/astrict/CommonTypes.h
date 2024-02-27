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

template<typename T>
struct Id {
    int id;

    bool operator==(const Id<T>& o) const {
        return id == o.id;
    }

    bool operator<(const Id<T>& o) const {
        return id < o.id;
    }
};

using StringId = Id<struct StringIdTag>;
using TypeId = Id<struct TypeIdTag>;
using GlobalSymbolId = Id<struct GlobalSymbolIdTag>;
using LocalSymbolId = Id<struct LocalSymbolIdTag>;
using RValueId = Id<struct RValueTag>;
using FunctionId = Id<struct FunctionIdTag>;
using StatementBlockId = Id<struct StatementBlockIdTag>;
using StructId = Id<struct StructIdTag>;

template<class>
inline constexpr bool always_false_v = false;

}  // namespace astrict

namespace std {

template<typename T>
struct hash<astrict::Id<T>> {
    std::size_t operator()(const astrict::Id<T>& o) const {
        return o.id;
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
enum class RValueOperator : uint8_t {
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
    IndexStruct,
    VectorSwizzle,
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

    // Misc
    ConstructStruct,
};

// // Workaround for missing std::expected.
// template<typename T>
// using StatusOr = std::variant<T, int>;

using ValueId = std::variant<GlobalSymbolId, LocalSymbolId, RValueId>;

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

struct Symbol {
    StringId name;
    std::optional<TypeId> type; // Don't record globals' types.

    bool operator==(const Symbol& o) const {
        return name == o.name
                && type == o.type;
    }
};

struct EvaluableRValue {
    std::variant<RValueOperator, FunctionId> op;
    std::vector<ValueId> args;

    bool operator==(const EvaluableRValue& o) const {
        return op == o.op
                && args == o.args;
    }
};

struct LiteralRValue {
    std::variant<
        bool,
        int,
        double,
        unsigned int> value;

    bool operator==(const LiteralRValue& o) const {
        return value == o.value;
    }
};

using RValue = std::variant<
    EvaluableRValue,
    LiteralRValue>;

struct FunctionDefinition {
    FunctionId name;
    TypeId returnType;
    std::vector<LocalSymbolId> parameters;
    StatementBlockId body;
    std::unordered_map<LocalSymbolId, Symbol> localSymbols;

    bool operator==(const FunctionDefinition& o) const {
        return name == o.name
                && returnType == o.returnType
                && parameters == o.parameters
                && body == o.body
                && localSymbols == o.localSymbols;
    }
};

struct IfStatement {
    ValueId condition;
    StatementBlockId thenBlock;
    std::optional<StatementBlockId> elseBlock;

    bool operator==(const IfStatement& o) const {
        return condition == o.condition
                && thenBlock == o.thenBlock
                && elseBlock == o.elseBlock;
    }
};

struct SwitchStatement {
    ValueId condition;
    StatementBlockId body;

    bool operator==(const SwitchStatement& o) const {
        return condition == o.condition
                && body == o.body;
    }
};

struct BranchStatement {
    BranchOperator op;
    std::optional<ValueId> operand;

    bool operator==(const BranchStatement& o) const {
        return op == o.op
                && operand == o.operand;
    }
};

struct LoopStatement {
    ValueId condition;
    std::optional<RValueId> terminal;
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
    RValueId,
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
struct hash<astrict::Symbol> {
    std::size_t operator()(const astrict::Symbol& o) const {
        std::size_t result = 0;
        astrict::hashCombine(result, o.name, o.type);
        return result;
    }
};

template<>
struct hash<astrict::EvaluableRValue> {
    std::size_t operator()(const astrict::EvaluableRValue& o) const {
        std::size_t result = 0;
        astrict::hashCombine(result, o.op, o.args.size());
        for (const auto& arg : o.args) {
            astrict::hashCombine(result, arg);
        }
        return result;
    }
};

template<>
struct hash<astrict::LiteralRValue> {
    std::size_t operator()(const astrict::LiteralRValue& o) const {
        std::size_t result = 0;
        astrict::hashCombine(result, o.value);
        return result;
    }
};

template<>
struct hash<astrict::FunctionDefinition> {
    std::size_t operator()(const astrict::FunctionDefinition& o) const {
        std::size_t result = 0;
        astrict::hashCombine(result, o.name, o.returnType, o.body,
                o.parameters.size(), o.localSymbols.size());
        for (const auto& argument : o.parameters) {
            astrict::hashCombine(result, argument);
        }
        for (const auto& symbol : o.localSymbols) {
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
