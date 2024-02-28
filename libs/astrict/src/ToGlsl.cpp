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

#include <astrict/ToGlsl.h>

#include <astrict/CommonTypes.h>
#include <astrict/DebugCommon.h>
#include <astrict/GlslTypes.h>
#include <sstream>
#include <unordered_set>
#include <utils/Panic.h>

namespace astrict {

constexpr auto kIndentAmount = "  ";
constexpr auto kSpace = " ";
constexpr auto kNewline = "\n";

template<typename T>
void dumpVariableOrExpression(
        const PackFromGlsl& pack, const Function& function,
        T valueId, bool parenthesize, std::ostringstream& out);
void dumpAnyVariableOrExpression(
        const PackFromGlsl& pack, const Function& function,
        VariableOrExpressionId valueId, bool parenthesize, std::ostringstream& out);

template<typename T>
void dumpExpression(
        const PackFromGlsl& pack, const Function& function,
        const T& value, bool parenthesize, std::ostringstream& out);

template<typename T>
void dumpExpressionOperandExpression(
        const PackFromGlsl& pack, const Function& function,
        const T& op, const std::vector<VariableOrExpressionId>& args,
        bool parenthesize, std::ostringstream& out);

template<typename T>
void dumpStatement(
        const PackFromGlsl& pack, const Function& function, StatementBlockId blockId,
        const T& statement, int depth,
        std::ostringstream& out);

void dumpBlock(
        const PackFromGlsl& pack, const Function& function, StatementBlockId blockId,
        int depth, std::ostringstream& out);

void indent(int depth, std::ostringstream& out) {
    for (int i = 0; i < depth; ++i) {
        out << kIndentAmount;
    }
}

void dumpString(const PackFromGlsl& pack, StringId stringId,
        std::ostringstream& out) {
    ASSERT_PRECONDITION(pack.strings.find(stringId) != pack.strings.end(),
            "Missing string");
    const auto& string = pack.strings.at(stringId);
    out << string;
}

void dumpFunctionName(const PackFromGlsl& pack, FunctionId functionId,
        std::ostringstream& out) {
    auto name = pack.functionNames.at(functionId);
    auto indexParenthesis = name.find('(');
    out << name.substr(0, indexParenthesis);
}

void dumpType(const PackFromGlsl& pack, TypeId typeId, std::ostringstream& out) {
    ASSERT_PRECONDITION(pack.types.find(typeId) != pack.types.end(),
            "Missing type definition");
    auto& type = pack.types.at(typeId);
    if (type.qualifiers) {
        dumpString(pack, type.qualifiers.value(), out);
    }
    std::visit([&](auto&& name) {
        using T = std::decay_t<decltype(name)>;
        if constexpr (std::is_same_v<T, StringId>) {
            dumpString(pack, name, out);
        } else if constexpr (std::is_same_v<T, StructId>) {
            // TODO: dump the name
            out << *name;
        } else {
            static_assert(always_false_v<T>, "unreachable");
        }
    }, type.name);
    for (const auto& arraySize : type.arraySizes) {
        out << "[" << arraySize << "]";
    }
}

void dumpBinaryExpressionOperator(
        const PackFromGlsl& pack, const Function& function,
        ExpressionOperator op, const std::vector<VariableOrExpressionId>& args,
        const char* opString,
        std::ostringstream& out) {
    ASSERT_PRECONDITION(args.size() == 2,
            "%s must be a binary operator", rValueOperatorToString(op));
    dumpAnyVariableOrExpression(pack, function, args[0], /*parenthesize=*/true, out);
    out << kSpace << opString << kSpace;
    dumpAnyVariableOrExpression(pack, function, args[1], /*parenthesize=*/true, out);
}

template<>
void dumpExpressionOperandExpression<ExpressionOperator>(
        const PackFromGlsl& pack, const Function& function,
        const ExpressionOperator& op, const std::vector<VariableOrExpressionId>& args,
        bool parenthesize, std::ostringstream& out) {
    const char* lParen = parenthesize ? "(" : "";
    const char* rParen = parenthesize ? ")" : "";
    switch (op) {
        // Unary
        case ExpressionOperator::Negative:
            ASSERT_PRECONDITION(args.size() == 1,
                    "Negative must be a unary operator");
            out << "-";
            dumpAnyVariableOrExpression(pack, function, args[0], /*parenthesize=*/true, out);
            break;
        case ExpressionOperator::LogicalNot:
            ASSERT_PRECONDITION(args.size() == 1,
                    "LogicalNot must be a unary operator");
            out << "!";
            dumpAnyVariableOrExpression(pack, function, args[0], /*parenthesize=*/true, out);
            break;
        case ExpressionOperator::BitwiseNot:
            ASSERT_PRECONDITION(args.size() == 1,
                    "BitwiseNot must be a unary operator");
            out << "~";
            dumpAnyVariableOrExpression(pack, function, args[0], /*parenthesize=*/true, out);
            break;
        case ExpressionOperator::PostIncrement:
            ASSERT_PRECONDITION(args.size() == 1,
                    "PostIncrement must be a unary operator");
            dumpAnyVariableOrExpression(pack, function, args[0], /*parenthesize=*/true, out);
            out << "++";
            break;
        case ExpressionOperator::PostDecrement:
            ASSERT_PRECONDITION(args.size() == 1,
                    "PostDecrement must be a unary operator");
            dumpAnyVariableOrExpression(pack, function, args[0], /*parenthesize=*/true, out);
            out << "--";
            break;
        case ExpressionOperator::PreIncrement:
            ASSERT_PRECONDITION(args.size() == 1,
                    "PreIncrement must be a unary operator");
            out << "++";
            dumpAnyVariableOrExpression(pack, function, args[0], /*parenthesize=*/true, out);
            break;
        case ExpressionOperator::PreDecrement:
            ASSERT_PRECONDITION(args.size() == 1,
                    "PreDecrement must be a unary operator");
            out << "--";
            dumpAnyVariableOrExpression(pack, function, args[0], /*parenthesize=*/true, out);
            break;
        case ExpressionOperator::ArrayLength:
            ASSERT_PRECONDITION(args.size() == 1,
                    "ArrayLength must be a unary operator");
            dumpAnyVariableOrExpression(pack, function, args[0], /*parenthesize=*/true, out);
            out << ".length";
            break;

            // Binary
        case ExpressionOperator::Add:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "+", out);
            out << rParen;
            break;
        case ExpressionOperator::Sub:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "-", out);
            out << rParen;
            break;
        case ExpressionOperator::Mul:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "*", out);
            out << rParen;
            break;
        case ExpressionOperator::Div:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "/", out);
            out << rParen;
            break;
        case ExpressionOperator::Mod:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "%", out);
            out << rParen;
            break;
        case ExpressionOperator::RightShift:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, ">>", out);
            out << rParen;
            break;
        case ExpressionOperator::LeftShift:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "<<", out);
            out << rParen;
            break;
        case ExpressionOperator::And:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "&", out);
            out << rParen;
            break;
        case ExpressionOperator::InclusiveOr:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "|", out);
            out << rParen;
            break;
        case ExpressionOperator::ExclusiveOr:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "^", out);
            out << rParen;
            break;
        case ExpressionOperator::Equal:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "==", out);
            out << rParen;
            break;
        case ExpressionOperator::NotEqual:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "!=", out);
            out << rParen;
            break;
        case ExpressionOperator::LessThan:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "<", out);
            out << rParen;
            break;
        case ExpressionOperator::GreaterThan:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, ">", out);
            out << rParen;
            break;
        case ExpressionOperator::LessThanEqual:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "<=", out);
            out << rParen;
            break;
        case ExpressionOperator::GreaterThanEqual:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, ">=", out);
            out << rParen;
            break;
        case ExpressionOperator::LogicalOr:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "||", out);
            out << rParen;
            break;
        case ExpressionOperator::LogicalXor:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "^^", out);
            out << rParen;
            break;
        case ExpressionOperator::LogicalAnd:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "&&", out);
            out << rParen;
            break;
        case ExpressionOperator::Index:
            ASSERT_PRECONDITION(args.size() == 2,
                    "%s must be a binary operator", rValueOperatorToString(op));
            dumpAnyVariableOrExpression(pack, function, args[0], /*parenthesize=*/true, out);
            out << "[";
            dumpAnyVariableOrExpression(pack, function, args[1], /*parenthesize=*/true, out);
            out << "]";
            break;
        case ExpressionOperator::Assign:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "=", out);
            out << rParen;
            break;
        case ExpressionOperator::AddAssign:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "+=", out);
            out << rParen;
            break;
        case ExpressionOperator::SubAssign:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "-=", out);
            out << rParen;
            break;
        case ExpressionOperator::MulAssign:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "*=", out);
            out << rParen;
            break;
        case ExpressionOperator::DivAssign:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "/=", out);
            out << rParen;
            break;
        case ExpressionOperator::ModAssign:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "%=", out);
            out << rParen;
            break;
        case ExpressionOperator::AndAssign:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "&=", out);
            out << rParen;
            break;
        case ExpressionOperator::InclusiveOrAssign:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "|=", out);
            out << rParen;
            break;
        case ExpressionOperator::ExclusiveOrAssign:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "^=", out);
            out << rParen;
            break;
        case ExpressionOperator::LeftShiftAssign:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, "<<=", out);
            out << rParen;
            break;
        case ExpressionOperator::RightShiftAssign:
            out << lParen;
            dumpBinaryExpressionOperator(pack, function, op, args, ">>=", out);
            out << rParen;
            break;

            // Ternary
        case ExpressionOperator::Ternary:
            ASSERT_PRECONDITION(args.size() == 3,
                    "Ternary must be a ternary operator");
            out << lParen << "(";
            dumpAnyVariableOrExpression(pack, function, args[0], /*parenthesize=*/true, out);
            out << ")" << kSpace << "?" << kSpace << "(";
            dumpAnyVariableOrExpression(pack, function, args[1], /*parenthesize=*/true, out);
            out << ")" << kSpace << ":" << kSpace << "(";
            dumpAnyVariableOrExpression(pack, function, args[2], /*parenthesize=*/true, out);
            out << ")" << rParen;
            break;

            // Variadic
        case ExpressionOperator::Comma:
            ASSERT_PRECONDITION(args.size() >= 2,
                    "Comma operator must have at least two arguments");
            out << lParen;
            dumpAnyVariableOrExpression(pack, function, args[0], /*parenthesize=*/false, out);
            for (int i = 1; i < args.size(); i++) {
                out << "," << kSpace;
                dumpAnyVariableOrExpression(pack, function, args[i], /*parenthesize=*/false, out);
            }
            out << rParen;
            break;
    }
}

template <>
void dumpExpressionOperandExpression<FunctionId>(
        const PackFromGlsl& pack, const Function& function,
        const FunctionId& op, const std::vector<VariableOrExpressionId>& args,
        bool parenthesize, std::ostringstream& out) {
    dumpFunctionName(pack, op, out);
    out << "(";
    bool firstArg = true;
    for (auto& arg : args) {
        if (firstArg) {
            firstArg = false;
        } else {
            out << "," << kSpace;
        }
        dumpAnyVariableOrExpression(pack, function, arg, /*parenthesize=*/false, out);
    }
    out << ")";
}

template <>
void dumpExpressionOperandExpression<StructId>(
        const PackFromGlsl& pack, const Function& function,
        const StructId& op, const std::vector<VariableOrExpressionId>& args,
        bool parenthesize, std::ostringstream& out) {
    // TODO
}

template<>
void dumpExpression<ExpressionOperandExpression>(
        const PackFromGlsl& pack, const Function& function,
        const ExpressionOperandExpression& value, bool parenthesize,
        std::ostringstream& out) {
    std::visit([&](auto&& op) {
        dumpExpressionOperandExpression(pack, function, op, value.args, parenthesize, out);
    }, value.op);
}

template<>
void dumpExpression<IndexStructExpression>(
        const PackFromGlsl& pack, const Function& function,
        const IndexStructExpression& value, bool parenthesize,
        std::ostringstream& out) {
    dumpAnyVariableOrExpression(pack, function, value.operand, /*parenthesize=*/true, out);
    out << ".";
    ASSERT_PRECONDITION(pack.structs.find(value.strukt) != pack.structs.end(),
            "Missing struct definition");
    auto& strukt = pack.structs.at(value.strukt);
    ASSERT_PRECONDITION(value.index < strukt.members.size(),
            "Struct member index out of bounds");
    dumpString(pack, strukt.members[value.index].name, out);
}

template<>
void dumpExpression<VectorSwizzleExpression>(
        const PackFromGlsl& pack, const Function& function,
        const VectorSwizzleExpression& value, bool parenthesize,
        std::ostringstream& out) {
    static const char COMPONENTS[] = {'x', 'y', 'z', 'w'};
    dumpAnyVariableOrExpression(pack, function, value.operand, /*parenthesize=*/true, out);
    out << ".";
    for (int i = 0; i < 4; i++) {
        int component = (value.swizzle >> (i * 3)) & 0x7;
        if (component == 0) {
            break;
        }
        out << COMPONENTS[component - 1];
    }
}

template<>
void dumpExpression<LiteralExpression>(
        const PackFromGlsl& pack, const Function& function,
        const LiteralExpression& value, bool parenthesize,
        std::ostringstream& out) {
    std::visit([&](auto&& value) {
        out << value;
    }, value.value);
}

template<>
void dumpVariableOrExpression<ExpressionId>(
        const PackFromGlsl& pack, const Function& function, ExpressionId valueId,
        bool parenthesize, std::ostringstream& out) {
    if (*valueId == 0) {
        out << "INVALID_EXPRESSION";
        return;
    }
    ASSERT_PRECONDITION(pack.expressions.find(valueId) != pack.expressions.end(),
            "Missing Expression");
    std::visit([&](auto&& value) {
        dumpExpression(pack, function, value, parenthesize, out);
    }, pack.expressions.at(valueId));
}

template<>
void dumpVariableOrExpression<GlobalVariableId>(
        const PackFromGlsl& pack, const Function& function, GlobalVariableId valueId,
        bool parenthesize, std::ostringstream& out) {
    if (*valueId == 0) {
        out << "INVALID_GLOBAL_SYMBOL";
        return;
    }
    ASSERT_PRECONDITION(pack.globalVariables.find(valueId) != pack.globalVariables.end(),
            "Missing global symbol");

    auto globalSymbol = pack.globalVariables.at(valueId);
    dumpString(pack, globalSymbol.name, out);
}

template<>
void dumpVariableOrExpression<LocalVariableId>(
        const PackFromGlsl& pack, const Function& function, LocalVariableId valueId,
        bool parenthesize, std::ostringstream& out) {
    if (*valueId == 0) {
        out << "INVALID_LOCAL_SYMBOL";
        return;
    }
    ASSERT_PRECONDITION(function.localVariables.find(valueId) != function.localVariables.end(),
            "Missing local symbol");

    auto& localSymbol = function.localVariables.at(valueId);
    dumpString(pack, localSymbol.name, out);
}

template<>
void dumpStatement<ExpressionId>(
        const PackFromGlsl& pack, const Function& function, StatementBlockId blockId,
        const ExpressionId& statement, int depth,
        std::ostringstream& out) {
    indent(depth, out);
    dumpAnyVariableOrExpression(pack, function, statement, /*parenthesize=*/false, out);
    out << ";" << kNewline;
}

template<>
void dumpStatement<IfStatement>(
        const PackFromGlsl& pack, const Function& function, StatementBlockId blockId,
        const IfStatement& statement, int depth,
        std::ostringstream& out) {
    indent(depth, out);
    out << "if" << kSpace << "(";
    dumpAnyVariableOrExpression(pack, function, statement.condition, /*parenthesize=*/false, out);
    out << ")" << kSpace << "{" << kNewline;
    dumpBlock(pack, function, statement.thenBlock, depth + 1, out);
    if (statement.elseBlock) {
        indent(depth, out);
        out << "}" << kSpace << "else" << kSpace << "{" << kNewline;
        dumpBlock(pack, function, statement.elseBlock.value(), depth + 1, out);
    }
    indent(depth, out);
    out << "}" << kNewline;
}

template<>
void dumpStatement<SwitchStatement>(
        const PackFromGlsl& pack, const Function& function, StatementBlockId blockId,
        const SwitchStatement& statement, int depth,
        std::ostringstream& out) {
    indent(depth, out);
    out << "switch" << kSpace << "(";
    dumpAnyVariableOrExpression(pack, function, statement.condition, /*parenthesize=*/false, out);
    out << ")" << kSpace << "{" << kNewline;
    dumpBlock(pack, function, statement.body, depth + 1, out);
    indent(depth, out);
    out << "}" << kNewline;
}

template<>
void dumpStatement<BranchStatement>(
        const PackFromGlsl& pack, const Function& function, StatementBlockId blockId,
        const BranchStatement& statement, int depth,
        std::ostringstream& out) {
    switch (statement.op) {
        case BranchOperator::Discard:
            indent(depth, out);
            out << "discard";
            break;
        case BranchOperator::TerminateInvocation:
            indent(depth, out);
            out << "terminateInvocation";
            break;
        case BranchOperator::Demote:
            indent(depth, out);
            out << "demote";
            break;
        case BranchOperator::TerminateRayEXT:
            indent(depth, out);
            out << "terminateRayEXT";
            break;
        case BranchOperator::IgnoreIntersectionEXT:
            indent(depth, out);
            out << "terminateIntersectionEXT";
            break;
        case BranchOperator::Return:
            indent(depth, out);
            out << "return";
            break;
        case BranchOperator::Break:
            indent(depth, out);
            out << "break";
            break;
        case BranchOperator::Continue:
            indent(depth, out);
            out << "continue";
            break;
        case BranchOperator::Case:
            indent(depth - 1, out);
            out << "case";
            break;
        case BranchOperator::Default:
            indent(depth - 1, out);
            out << "default";
            break;
    }
    if (statement.operand) {
        out << " ";
        dumpAnyVariableOrExpression(pack, function, statement.operand.value(),
                /*parenthesize=*/false, out);
    }
    switch (statement.op) {
        case BranchOperator::Case:
        case BranchOperator::Default:
            out << ":" << kNewline;
            break;
        default:
            out << ";" << kNewline;
            break;
    }
}

template<>
void dumpStatement<LoopStatement>(
        const PackFromGlsl& pack, const Function& function, StatementBlockId blockId,
        const LoopStatement& statement, int depth,
        std::ostringstream& out) {
    if (statement.testFirst) {
        if (statement.terminal) {
            indent(depth, out);
            out << "for" << kSpace << "(;" << kSpace;
            dumpAnyVariableOrExpression(pack, function, statement.condition,
                    /*parenthesize=*/false, out);
            out << ";" << kSpace;
            dumpAnyVariableOrExpression(pack, function, statement.terminal.value(),
                    /*parenthesize=*/false, out);
        } else {
            indent(depth, out);
            out << "while" << kSpace << "(";
            dumpAnyVariableOrExpression(pack, function, statement.condition,
                    /*parenthesize=*/false, out);
        }
        out << ")" << kSpace << "{" << kNewline;
        dumpBlock(pack, function, statement.body, depth + 1, out);
        indent(depth, out);
        out << "}" << kNewline;
    } else {
        indent(depth, out);
        out << "do" << kSpace << "{" << kNewline;
        dumpBlock(pack, function, statement.body, depth + 1, out);
        indent(depth, out);
        out << "}" << kSpace << "while" << kSpace << "(";
        dumpAnyVariableOrExpression(pack, function, statement.condition, /*parenthesize=*/false, out);
        out << ");" << kNewline;
    }
}

void dumpAnyVariableOrExpression(
        const PackFromGlsl& pack, const Function& function, VariableOrExpressionId valueId,
        bool parenthesize, std::ostringstream& out) {
    std::visit([&](auto&& variantValueId){
        dumpVariableOrExpression(pack, function, variantValueId, parenthesize, out);
    }, valueId);
}

void dumpBlock(
        const PackFromGlsl& pack, const Function& function, StatementBlockId blockId,
        int depth, std::ostringstream& out) {
    ASSERT_PRECONDITION(pack.statementBlocks.find(blockId) != pack.statementBlocks.end(),
            "Missing block definition");
    for (auto statement : pack.statementBlocks.at(blockId)) {
        std::visit([&](auto&& variantStatement){
            dumpStatement(pack, function, blockId, variantStatement, depth, out);
        }, statement);
    }
}

void dumpSymbolDefinition(
        const PackFromGlsl& pack, const Variable& symbol,
        std::ostringstream& out) {
    ASSERT_PRECONDITION(symbol.type.has_value(),
            "Symbol definition must have type");
    dumpType(pack, symbol.type.value(), out);
    out << " "; // required space
    dumpString(pack, symbol.name, out);
}

void dumpFunction(
        const PackFromGlsl& pack, const FunctionId& functionId, bool dumpBody,
        std::ostringstream& out) {
    if (!dumpBody && pack.functions.find(functionId) == pack.functions.end()) {
        // TODO: prune function prototypes with no actual definition.
        return;
    }
    ASSERT_PRECONDITION(pack.functions.find(functionId) != pack.functions.end(),
            "Missing function definition");
    auto& function = pack.functions.at(functionId);
    dumpType(pack, function.returnType, out);
    out << " ";
    dumpFunctionName(pack, function.name, out);
    out << "(";
    std::unordered_set<LocalVariableId> parameterSymbolIds;
    bool firstParameter = true;
    for (const auto& parameterId : function.parameters) {
        if (firstParameter) {
            firstParameter = false;
        } else {
            out << "," << kSpace;
        }
        parameterSymbolIds.insert(parameterId);
        const auto& parameter = function.localVariables.at(parameterId);
        dumpSymbolDefinition(pack, parameter, out);
    }
    out << ")";
    if (dumpBody) {
        out << kSpace << "{" << kNewline;
        for (const auto& localSymbol : function.localVariables) {
            if (parameterSymbolIds.find(localSymbol.first) == parameterSymbolIds.end()) {
                out << kIndentAmount;
                dumpSymbolDefinition(pack, localSymbol.second, out);
                out << ";" << kNewline;
            }
        }
        dumpBlock(pack, function, function.body, 1, out);
        out << "}" << kNewline;
    } else {
        out << ";" << kNewline;
    }
}

void dumpStruct(const PackFromGlsl& pack, StructId structId, std::ostringstream& out) {
    ASSERT_PRECONDITION(pack.structs.find(structId) != pack.structs.end(),
            "Missing struct definition");
    auto& strukt = pack.structs.at(structId);
    out << "struct ";
    dumpString(pack, strukt.name, out);
    out << kSpace << "{" << kNewline;
    for (const auto& member : strukt.members) {
        indent(1, out);
        dumpType(pack, member.type, out);
        out << " "; // non-optional
        dumpString(pack, member.name, out);
        out << ";" << kNewline;
    }
    out << "};" << kNewline;
}

void toGlsl(const PackFromGlsl& pack, std::ostringstream& out) {
    const Function emptyFunction{};
    for (auto structId : pack.structsInOrder) {
        dumpStruct(pack, structId, out);
    }
    std::unordered_set<GlobalVariableId> globalSymbolIdsWithValues;
    for (auto globalSymbolPair : pack.globalSymbolsInOrder) {
        globalSymbolIdsWithValues.insert(globalSymbolPair.first);
    }
    for (auto globalSymbolPair : pack.globalVariables) {
        if (globalSymbolIdsWithValues.find(globalSymbolPair.first)
                == globalSymbolIdsWithValues.end()) {
            const auto& globalSymbol = pack.globalVariables.at(globalSymbolPair.first);
            if (globalSymbol.type) {
                dumpSymbolDefinition(pack, globalSymbol, out);
                out << ";" << kNewline;
            }
        }
    }
    for (auto globalSymbolPair : pack.globalSymbolsInOrder) {
        const auto& globalSymbol = pack.globalVariables.at(globalSymbolPair.first);
        dumpSymbolDefinition(pack, globalSymbol, out);
        out << kSpace << "=" << kSpace;
        dumpAnyVariableOrExpression(pack, emptyFunction, globalSymbolPair.second,
                /*parenthesize=*/false, out);
        out << ";" << kNewline;
    }
    for (auto functionId : pack.functionPrototypes) {
        dumpFunction(pack, functionId, /*dumpBody=*/false, out);
    }
    for (auto functionId : pack.functionsInOrder) {
        dumpFunction(pack, functionId, /*dumpBody=*/true, out);
    }
}

} // namespace astrict
