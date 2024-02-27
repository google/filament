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
void dumpValue(
        const PackFromGlsl& pack, const FunctionDefinition& function,
        T valueId, bool parenthesize, std::ostringstream& out);
void dumpAnyValue(
        const PackFromGlsl& pack, const FunctionDefinition& function,
        ValueId valueId, bool parenthesize, std::ostringstream& out);

template<typename T>
void dumpRValue(
        const PackFromGlsl& pack, const FunctionDefinition& function,
        const T& value, bool parenthesize, std::ostringstream& out);

template<typename T>
void dumpEvaluableRValue(
        const PackFromGlsl& pack, const FunctionDefinition& function,
        const T& op, const std::vector<ValueId>& args,
        bool parenthesize, std::ostringstream& out);

template<typename T>
void dumpStatement(
        const PackFromGlsl& pack, const FunctionDefinition& function, StatementBlockId blockId,
        const T& statement, int depth,
        std::ostringstream& out);

void dumpBlock(
        const PackFromGlsl& pack, const FunctionDefinition& function, StatementBlockId blockId,
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
    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, StringId>) {
            dumpString(pack, arg, out);
        } else if constexpr (std::is_same_v<T, StructId>) {
            out << arg.id;
        } else {
            static_assert(always_false_v<T>, "unreachable");
        }
    }, type.name);
    for (const auto& arraySize : type.arraySizes) {
        out << "[" << arraySize << "]";
    }
}

void dumpBinaryRValueOperator(
        const PackFromGlsl& pack, const FunctionDefinition& function,
        RValueOperator op, const std::vector<ValueId>& args,
        const char* opString,
        std::ostringstream& out) {
    ASSERT_PRECONDITION(args.size() == 2,
            "%s must be a binary operator", rValueOperatorToString(op));
    dumpAnyValue(pack, function, args[0], /*parenthesize=*/true, out);
    out << kSpace << opString << kSpace;
    dumpAnyValue(pack, function, args[1], /*parenthesize=*/true, out);
}

template<>
void dumpEvaluableRValue<RValueOperator>(
        const PackFromGlsl& pack, const FunctionDefinition& function,
        const RValueOperator& op, const std::vector<ValueId>& args,
        bool parenthesize, std::ostringstream& out) {
    const char* lParen = parenthesize ? "(" : "";
    const char* rParen = parenthesize ? ")" : "";
    switch (op) {
        // Unary
        case RValueOperator::Negative:
            ASSERT_PRECONDITION(args.size() == 1,
                    "Negative must be a unary operator");
            out << "-";
            dumpAnyValue(pack, function, args[0], /*parenthesize=*/true, out);
            break;
        case RValueOperator::LogicalNot:
            ASSERT_PRECONDITION(args.size() == 1,
                    "LogicalNot must be a unary operator");
            out << "!";
            dumpAnyValue(pack, function, args[0], /*parenthesize=*/true, out);
            break;
        case RValueOperator::BitwiseNot:
            ASSERT_PRECONDITION(args.size() == 1,
                    "BitwiseNot must be a unary operator");
            out << "~";
            dumpAnyValue(pack, function, args[0], /*parenthesize=*/true, out);
            break;
        case RValueOperator::PostIncrement:
            ASSERT_PRECONDITION(args.size() == 1,
                    "PostIncrement must be a unary operator");
            dumpAnyValue(pack, function, args[0], /*parenthesize=*/true, out);
            out << "++";
            break;
        case RValueOperator::PostDecrement:
            ASSERT_PRECONDITION(args.size() == 1,
                    "PostDecrement must be a unary operator");
            dumpAnyValue(pack, function, args[0], /*parenthesize=*/true, out);
            out << "--";
            break;
        case RValueOperator::PreIncrement:
            ASSERT_PRECONDITION(args.size() == 1,
                    "PreIncrement must be a unary operator");
            out << "++";
            dumpAnyValue(pack, function, args[0], /*parenthesize=*/true, out);
            break;
        case RValueOperator::PreDecrement:
            ASSERT_PRECONDITION(args.size() == 1,
                    "PreDecrement must be a unary operator");
            out << "--";
            dumpAnyValue(pack, function, args[0], /*parenthesize=*/true, out);
            break;
        case RValueOperator::ArrayLength:
            ASSERT_PRECONDITION(args.size() == 1,
                    "ArrayLength must be a unary operator");
            dumpAnyValue(pack, function, args[0], /*parenthesize=*/true, out);
            out << ".length";
            break;

            // Binary
        case RValueOperator::Add:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "+", out);
            out << rParen;
            break;
        case RValueOperator::Sub:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "-", out);
            out << rParen;
            break;
        case RValueOperator::Mul:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "*", out);
            out << rParen;
            break;
        case RValueOperator::Div:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "/", out);
            out << rParen;
            break;
        case RValueOperator::Mod:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "%", out);
            out << rParen;
            break;
        case RValueOperator::RightShift:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, ">>", out);
            out << rParen;
            break;
        case RValueOperator::LeftShift:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "<<", out);
            out << rParen;
            break;
        case RValueOperator::And:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "&", out);
            out << rParen;
            break;
        case RValueOperator::InclusiveOr:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "|", out);
            out << rParen;
            break;
        case RValueOperator::ExclusiveOr:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "^", out);
            out << rParen;
            break;
        case RValueOperator::Equal:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "==", out);
            out << rParen;
            break;
        case RValueOperator::NotEqual:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "!=", out);
            out << rParen;
            break;
        case RValueOperator::LessThan:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "<", out);
            out << rParen;
            break;
        case RValueOperator::GreaterThan:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, ">", out);
            out << rParen;
            break;
        case RValueOperator::LessThanEqual:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "<=", out);
            out << rParen;
            break;
        case RValueOperator::GreaterThanEqual:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, ">=", out);
            out << rParen;
            break;
        case RValueOperator::LogicalOr:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "||", out);
            out << rParen;
            break;
        case RValueOperator::LogicalXor:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "^^", out);
            out << rParen;
            break;
        case RValueOperator::LogicalAnd:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "&&", out);
            out << rParen;
            break;
        case RValueOperator::Index:
            ASSERT_PRECONDITION(args.size() == 2,
                    "%s must be a binary operator", rValueOperatorToString(op));
            dumpAnyValue(pack, function, args[0], /*parenthesize=*/true, out);
            out << "[";
            dumpAnyValue(pack, function, args[1], /*parenthesize=*/true, out);
            out << "]";
            break;
            // case RValueOperator::IndexStruct:
            //     break;
            // case RValueOperator::VectorSwizzle:
            //     break;
        case RValueOperator::Assign:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "=", out);
            out << rParen;
            break;
        case RValueOperator::AddAssign:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "+=", out);
            out << rParen;
            break;
        case RValueOperator::SubAssign:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "-=", out);
            out << rParen;
            break;
        case RValueOperator::MulAssign:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "*=", out);
            out << rParen;
            break;
        case RValueOperator::DivAssign:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "/=", out);
            out << rParen;
            break;
        case RValueOperator::ModAssign:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "%=", out);
            out << rParen;
            break;
        case RValueOperator::AndAssign:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "&=", out);
            out << rParen;
            break;
        case RValueOperator::InclusiveOrAssign:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "|=", out);
            out << rParen;
            break;
        case RValueOperator::ExclusiveOrAssign:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "^=", out);
            out << rParen;
            break;
        case RValueOperator::LeftShiftAssign:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, "<<=", out);
            out << rParen;
            break;
        case RValueOperator::RightShiftAssign:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, ">>=", out);
            out << rParen;
            break;

            // Ternary
        case RValueOperator::Ternary:
            ASSERT_PRECONDITION(args.size() == 3,
                    "Ternary must be a ternary operator");
            out << lParen << "(";
            dumpAnyValue(pack, function, args[0], /*parenthesize=*/true, out);
            out << ")" << kSpace << "?" << kSpace << "(";
            dumpAnyValue(pack, function, args[1], /*parenthesize=*/true, out);
            out << ")" << kSpace << ":" << kSpace << "(";
            dumpAnyValue(pack, function, args[2], /*parenthesize=*/true, out);
            out << ")" << rParen;
            break;

            // Variadic
        case RValueOperator::Comma:
            ASSERT_PRECONDITION(args.size() >= 2,
                    "Comma operator must have at least two arguments");
            out << lParen;
            dumpAnyValue(pack, function, args[0], /*parenthesize=*/false, out);
            for (int i = 1; i < args.size(); i++) {
                out << "," << kSpace;
                dumpAnyValue(pack, function, args[i], /*parenthesize=*/false, out);
            }
            out << rParen;
            break;

            // Misc
        case RValueOperator::ConstructStruct:
        default:
            out << "(" << rValueOperatorToString(op);
            for (auto& arg : args) {
                out << kSpace;
                dumpAnyValue(pack, function, arg, /*parenthesize=*/true, out);
            }
            out << ")";
            break;
    }
}

template <>
void dumpEvaluableRValue<FunctionId>(
        const PackFromGlsl& pack, const FunctionDefinition& function,
        const FunctionId& functionId, const std::vector<ValueId>& args,
        bool parenthesize, std::ostringstream& out) {
    dumpFunctionName(pack, functionId, out);
    out << "(";
    bool firstArg = true;
    for (auto& arg : args) {
        if (firstArg) {
            firstArg = false;
        } else {
            out << "," << kSpace;
        }
        dumpAnyValue(pack, function, arg, /*parenthesize=*/false, out);
    }
    out << ")";
}

template<>
void dumpRValue<EvaluableRValue>(
        const PackFromGlsl& pack, const FunctionDefinition& function,
        const EvaluableRValue& value, bool parenthesize,
        std::ostringstream& out) {
    std::visit([&](auto&& op) {
        dumpEvaluableRValue(pack, function, op, value.args, parenthesize, out);
    }, value.op);
}

template<>
void dumpRValue<LiteralRValue>(
        const PackFromGlsl& pack, const FunctionDefinition& function,
        const LiteralRValue& value, bool parenthesize,
        std::ostringstream& out) {
    std::visit([&](auto&& value) {
        out << value;
    }, value.value);
}

template<>
void dumpValue<RValueId>(
        const PackFromGlsl& pack, const FunctionDefinition& function, RValueId valueId,
        bool parenthesize, std::ostringstream& out) {
    if (valueId.id == 0) {
        out << "INVALID_RVALUE";
        return;
    }
    ASSERT_PRECONDITION(pack.rValues.find(valueId) != pack.rValues.end(),
            "Missing RValue");
    std::visit([&](auto&& value) {
        dumpRValue(pack, function, value, parenthesize, out);
    }, pack.rValues.at(valueId));
}

template<>
void dumpValue<GlobalSymbolId>(
        const PackFromGlsl& pack, const FunctionDefinition& function, GlobalSymbolId valueId,
        bool parenthesize, std::ostringstream& out) {
    if (valueId.id == 0) {
        out << "INVALID_GLOBAL_SYMBOL";
        return;
    }
    ASSERT_PRECONDITION(pack.globalSymbols.find(valueId) != pack.globalSymbols.end(),
            "Missing global symbol");

    auto globalSymbol = pack.globalSymbols.at(valueId);
    dumpString(pack, globalSymbol.name, out);
}

template<>
void dumpValue<LocalSymbolId>(
        const PackFromGlsl& pack, const FunctionDefinition& function, LocalSymbolId valueId,
        bool parenthesize, std::ostringstream& out) {
    if (valueId.id == 0) {
        out << "INVALID_LOCAL_SYMBOL";
        return;
    }
    ASSERT_PRECONDITION(function.localSymbols.find(valueId) != function.localSymbols.end(),
            "Missing local symbol");

    auto& localSymbol = function.localSymbols.at(valueId);
    dumpString(pack, localSymbol.name, out);
}

template<>
void dumpStatement<RValueId>(
        const PackFromGlsl& pack, const FunctionDefinition& function, StatementBlockId blockId,
        const RValueId& statement, int depth,
        std::ostringstream& out) {
    indent(depth, out);
    dumpAnyValue(pack, function, statement, /*parenthesize=*/false, out);
    out << ";" << kNewline;
}

template<>
void dumpStatement<IfStatement>(
        const PackFromGlsl& pack, const FunctionDefinition& function, StatementBlockId blockId,
        const IfStatement& statement, int depth,
        std::ostringstream& out) {
    indent(depth, out);
    out << "if" << kSpace << "(";
    dumpAnyValue(pack, function, statement.condition, /*parenthesize=*/false, out);
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
        const PackFromGlsl& pack, const FunctionDefinition& function, StatementBlockId blockId,
        const SwitchStatement& statement, int depth,
        std::ostringstream& out) {
    indent(depth, out);
    out << "switch" << kSpace << "(";
    dumpAnyValue(pack, function, statement.condition, /*parenthesize=*/false, out);
    out << ")" << kSpace << "{" << kNewline;
    dumpBlock(pack, function, statement.body, depth + 1, out);
    indent(depth, out);
    out << "}" << kNewline;
}

template<>
void dumpStatement<BranchStatement>(
        const PackFromGlsl& pack, const FunctionDefinition& function, StatementBlockId blockId,
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
        dumpAnyValue(pack, function, statement.operand.value(),
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
        const PackFromGlsl& pack, const FunctionDefinition& function, StatementBlockId blockId,
        const LoopStatement& statement, int depth,
        std::ostringstream& out) {
    if (statement.testFirst) {
        if (statement.terminal) {
            indent(depth, out);
            out << "for" << kSpace << "(;" << kSpace;
            dumpAnyValue(pack, function, statement.condition,
                    /*parenthesize=*/false, out);
            out << ";" << kSpace;
            dumpAnyValue(pack, function, statement.terminal.value(),
                    /*parenthesize=*/false, out);
        } else {
            indent(depth, out);
            out << "while" << kSpace << "(";
            dumpAnyValue(pack, function, statement.condition,
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
        dumpAnyValue(pack, function, statement.condition, /*parenthesize=*/false, out);
        out << ");" << kNewline;
    }
}

void dumpAnyValue(
        const PackFromGlsl& pack, const FunctionDefinition& function, ValueId valueId,
        bool parenthesize, std::ostringstream& out) {
    std::visit([&](auto&& variantValueId){
        dumpValue(pack, function, variantValueId, parenthesize, out);
    }, valueId);
}

void dumpBlock(
        const PackFromGlsl& pack, const FunctionDefinition& function, StatementBlockId blockId,
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
        const PackFromGlsl& pack, const Symbol& symbol,
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
    if (!dumpBody && pack.functionDefinitions.find(functionId) == pack.functionDefinitions.end()) {
        // TODO: prune function prototypes with no actual definition.
        return;
    }
    ASSERT_PRECONDITION(pack.functionDefinitions.find(functionId) != pack.functionDefinitions.end(),
            "Missing function definition");
    auto& function = pack.functionDefinitions.at(functionId);
    dumpType(pack, function.returnType, out);
    out << " ";
    dumpFunctionName(pack, function.name, out);
    out << "(";
    std::unordered_set<LocalSymbolId> parameterSymbolIds;
    bool firstParameter = true;
    for (const auto& parameterId : function.parameters) {
        if (firstParameter) {
            firstParameter = false;
        } else {
            out << "," << kSpace;
        }
        parameterSymbolIds.insert(parameterId);
        const auto& parameter = function.localSymbols.at(parameterId);
        dumpSymbolDefinition(pack, parameter, out);
    }
    out << ")";
    if (dumpBody) {
        out << kSpace << "{" << kNewline;
        for (const auto& localSymbol : function.localSymbols) {
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

void toGlsl(const PackFromGlsl& pack, std::ostringstream& out) {
    const FunctionDefinition emptyFunction{};
    for (auto globalSymbolPair : pack.globalSymbolDefinitionsInOrder) {
        auto globalSymbolId = std::get<0>(globalSymbolPair);
        auto valueId = std::get<1>(globalSymbolPair);
        const auto& globalSymbol = pack.globalSymbols.at(globalSymbolId);
        dumpSymbolDefinition(pack, globalSymbol, out);
        out << kSpace << "=" << kSpace;
        dumpAnyValue(pack, emptyFunction, valueId, /*parenthesize=*/false, out);
        out << ";" << kNewline;
    }
    for (auto functionId : pack.functionPrototypes) {
        dumpFunction(pack, functionId, /*dumpBody=*/false, out);
    }
    for (auto functionId : pack.functionDefinitionsInOrder) {
        dumpFunction(pack, functionId, /*dumpBody=*/true, out);
    }
}

} // namespace astrict
