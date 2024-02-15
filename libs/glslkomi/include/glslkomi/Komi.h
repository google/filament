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

#ifndef TNT_GLSLKOMI_KOMI_H
#define TNT_GLSLKOMI_KOMI_H

#include <utils/compiler.h>
#include <vector>
#include <variant>
#include "intermediate.h"
#include <backend/DriverEnums.h>
#include <ShaderLang.h>

namespace glslkomi {

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
// equality. GLSL is not Lisp, unfortunately.
enum class RValueOperator : uint16_t {
    // Unary
    Negative,
    LogicalNot,
    BitwiseNot,

    PostIncrement,
    PostDecrement,
    PreIncrement,
    PreDecrement,

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
    VectorEqual,
    VectorNotEqual,
    LessThan,
    GreaterThan,
    LessThanEqual,
    GreaterThanEqual,
    Comma,
    LogicalOr,
    LogicalXor,
    LogicalAnd,
    IndexDirect,
    IndexIndirect,
    IndexDirectStruct,
    VectorSwizzle,
    Assign,
    AddAssign,
    SubAssign,
    MulAssign,
    VectorTimesMatrixAssign,
    VectorTimesScalarAssign,
    MatrixTimesScalarAssign,
    MatrixTimesMatrixAssign,
    DivAssign,
    ModAssign,
    AndAssign,
    InclusiveOrAssign,
    ExclusiveOrAssign,
    LeftShiftAssign,
    RightShiftAssign,
    ArrayLength,

    // Ternary
    Ternary,

    // Misc
    ConstructStruct,
};

// The order of the fields in this enum is very important due to a hack in glslangTypeToType.
enum class BuiltInType : uint8_t {
    Void,
    Struct,
    Block,
    Sampler2DArray,
    // Float
    Float,
    Vec2,
    Vec3,
    Vec4,
    Mat2,
    Mat2x3,
    Mat2x4,
    Mat3x2,
    Mat3,
    Mat3x4,
    Mat4x2,
    Mat4x3,
    Mat4,
    // Double
    Double,
    Dvec2,
    Dvec3,
    Dvec4,
    Dmat2,
    Dmat2x3,
    Dmat2x4,
    Dmat3x2,
    Dmat3,
    Dmat3x4,
    Dmat4x2,
    Dmat4x3,
    Dmat4,
    // Int
    Int,
    IVec2,
    IVec3,
    IVec4,
    // UInt
    Uint,
    Uvec2,
    Uvec3,
    Uvec4,
    // Bool
    Bool,
    Bvec2,
    Bvec3,
    Bvec4,
    // AtomicUInt
    AtomicUint,
};

// // Workaround for missing std::expected.
// template<typename T>
// using StatusOr = std::variant<T, int>;

using Precision = filament::backend::Precision;
using ElementType = filament::backend::ElementType;

template<typename T>
struct Id {
    std::size_t id;

    bool operator==(const Id<T>& o) const {
        return id == o.id;
    };
};

using TypeId = Id<struct TypeIdTag>;
using LValueId = Id<struct LValueTag>;
using RValueId = Id<struct RValueTag>;
using ValueId = std::variant<LValueId, RValueId>;
using FunctionId = Id<struct FunctionIdTag>;
using StatementBlockId = Id<struct StatementBlockIdTag>;

template <typename T, typename... Rest>
void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hashCombine(seed, rest), ...);
}

// TODO: qualifiers
struct Type {
    std::string_view name;
    std::string_view precision;
    std::vector<std::size_t> arraySizes;

    bool operator==(const Type& o) const {
        return name == o.name
                && precision == o.precision
                && arraySizes == o.arraySizes;
    }
};

struct LValue {
    std::string_view name;

    bool operator==(const LValue& o) const {
        return name == o.name;
    }
};

struct FunctionCallRValue {
    FunctionId function;
    std::vector<ValueId> args;

    bool operator==(const FunctionCallRValue& o) const {
        return function == o.function
                && args == o.args;
    }
};

struct OperatorRValue {
    RValueOperator op;
    std::vector<ValueId> args;

    bool operator==(const OperatorRValue& o) const {
        return op == o.op
                && args == o.args;
    }
};

// TODO: value
struct LiteralRValue {
    bool operator==(const LiteralRValue& o) const {
        return true;
    }
};

using RValue = std::variant<
    FunctionCallRValue,
    OperatorRValue,
    LiteralRValue>;

// TODO: annotation
struct FunctionParameter {
    LValueId name;
    TypeId type;

    bool operator==(const FunctionParameter& o) const {
        return name == o.name
                && type == o.type;
    }
};

// TODO: type, local variables
struct FunctionDefinition {
    FunctionId name;
    TypeId returnType;
    std::vector<FunctionParameter> parameters;
    std::optional<StatementBlockId> body;

    bool operator==(const FunctionDefinition& o) const {
        return name == o.name
                && returnType == o.returnType
                && parameters == o.parameters
                && body == o.body;
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

class Komi {
public:
    void slurp(const glslang::TIntermediate& intermediate);
};

} // namespace glslkomi

template<typename T>
struct ::std::hash<glslkomi::Id<T>> {
    std::size_t operator()(const glslkomi::Id<T>& o) const {
        return o.id;
    }
};


template<>
struct ::std::hash<glslkomi::Type> {
    std::size_t operator()(const glslkomi::Type& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.name, o.precision, o.arraySizes.size());
        for (const auto& size : o.arraySizes) {
            glslkomi::hashCombine(result, size);
        }
        return result;
    }
};

template<>
struct ::std::hash<glslkomi::LValue> {
    std::size_t operator()(const glslkomi::LValue& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.name);
        return result;
    }
};

template<>
struct ::std::hash<glslkomi::FunctionCallRValue> {
    std::size_t operator()(const glslkomi::FunctionCallRValue& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.function, o.args.size());
        for (const auto& arg : o.args) {
            glslkomi::hashCombine(result, arg);
        }
        return result;
    }
};

template<>
struct ::std::hash<glslkomi::OperatorRValue> {
    std::size_t operator()(const glslkomi::OperatorRValue& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.op, o.args.size());
        for (const auto& arg : o.args) {
            glslkomi::hashCombine(result, arg);
        }
        return result;
    }
};

// TODO
template<>
struct ::std::hash<glslkomi::LiteralRValue> {
    std::size_t operator()(const glslkomi::LiteralRValue& o) const {
        return 0;
    }
};

template<>
struct ::std::hash<glslkomi::FunctionParameter> {
    std::size_t operator()(const glslkomi::FunctionParameter& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.name, o.type);
        return result;
    }
};

template<>
struct ::std::hash<glslkomi::FunctionDefinition> {
    std::size_t operator()(const glslkomi::FunctionDefinition& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.name, o.returnType, o.body, o.parameters.size());
        for (const auto& argument : o.parameters) {
            glslkomi::hashCombine(result, argument);
        }
        return result;
    }
};

template<>
struct ::std::hash<glslkomi::IfStatement> {
    std::size_t operator()(const glslkomi::IfStatement& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.condition, o.thenBlock, o.elseBlock);
        return result;
    }
};

template<>
struct ::std::hash<glslkomi::SwitchStatement> {
    std::size_t operator()(const glslkomi::SwitchStatement& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.condition, o.body);
        return result;
    }
};

template<>
struct ::std::hash<glslkomi::BranchStatement> {
    std::size_t operator()(const glslkomi::BranchStatement& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.op, o.operand);
        return result;
    }
};

template<>
struct ::std::hash<glslkomi::LoopStatement> {
    std::size_t operator()(const glslkomi::LoopStatement& o) const {
        std::size_t result = 0;
        glslkomi::hashCombine(result, o.condition, o.terminal, o.testFirst, o.body);
        return result;
    }
};

template<>
struct ::std::hash<std::vector<glslkomi::Statement>> {
    std::size_t operator()(const std::vector<glslkomi::Statement>& o) const {
        std::size_t result = o.size();
        for (const auto& statement : o) {
            glslkomi::hashCombine(result, statement);
        }
        return result;
    }
};

#endif
