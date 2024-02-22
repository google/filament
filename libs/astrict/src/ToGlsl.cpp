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

void dumpFunctionName(const PackFromGlsl& pack, const FunctionId& functionId,
        std::ostringstream& out) {
    auto name = pack.functionNames.at(functionId);
    auto indexParenthesis = name.find('(');
    out << name.substr(0, indexParenthesis);
}

void dumpType(const PackFromGlsl& pack, TypeId typeId, std::ostringstream& out) {
    ASSERT_PRECONDITION(pack.types.find(typeId) != pack.types.end(),
            "Missing type definition");
    auto& type = pack.types.at(typeId);
    if (!type.precision.empty()) {
        out << type.precision << " ";
    }
    out << type.name;
    for (const auto& arraySize : type.arraySizes) {
        out << "[" << arraySize << "]";
    }
}

void dumpValue(
        const PackFromGlsl& pack, const FunctionDefinition& function, ValueId valueId,
        bool parenthesize, std::ostringstream& out);

void dumpBinaryRValueOperator(
        const PackFromGlsl& pack, const FunctionDefinition& function,
        RValueOperator op, const std::vector<ValueId>& args,
        const char* opString,
        std::ostringstream& out) {
    ASSERT_PRECONDITION(args.size() == 2,
            "%s must be a binary operator", rValueOperatorToString(op));
    dumpValue(pack, function, args[0], /*parenthesize=*/true, out);
    out << kSpace << opString << kSpace;
    dumpValue(pack, function, args[1], /*parenthesize=*/true, out);
}

void dumpRValueOperator(
        const PackFromGlsl& pack, const FunctionDefinition& function,
        RValueOperator op, const std::vector<ValueId>& args,
        bool parenthesize, std::ostringstream& out) {
    const char* lParen = parenthesize ? "(" : "";
    const char* rParen = parenthesize ? ")" : "";
    switch (op) {
        // Unary
        case RValueOperator::Negative:
            ASSERT_PRECONDITION(args.size() == 1,
                    "Negative must be a unary operator");
            out << "-" << lParen;
            dumpValue(pack, function, args[0], /*parenthesize=*/true, out);
            out << rParen;
            break;
        case RValueOperator::LogicalNot:
            ASSERT_PRECONDITION(args.size() == 1,
                    "LogicalNot must be a unary operator");
            out << "!" << lParen;
            dumpValue(pack, function, args[0], /*parenthesize=*/true, out);
            out << rParen;
            break;
        case RValueOperator::BitwiseNot:
            ASSERT_PRECONDITION(args.size() == 1,
                    "BitwiseNot must be a unary operator");
            out << "~" << lParen;
            dumpValue(pack, function, args[0], /*parenthesize=*/true, out);
            out << rParen;
            break;
        case RValueOperator::PostIncrement:
            ASSERT_PRECONDITION(args.size() == 1,
                    "PostIncrement must be a unary operator");
            dumpValue(pack, function, args[0], /*parenthesize=*/true, out);
            out << "++";
            break;
        case RValueOperator::PostDecrement:
            ASSERT_PRECONDITION(args.size() == 1,
                    "PostDecrement must be a unary operator");
            dumpValue(pack, function, args[0], /*parenthesize=*/true, out);
            out << "--";
            break;
        case RValueOperator::PreIncrement:
            ASSERT_PRECONDITION(args.size() == 1,
                    "PreIncrement must be a unary operator");
            out << "++";
            dumpValue(pack, function, args[0], /*parenthesize=*/true, out);
            break;
        case RValueOperator::PreDecrement:
            ASSERT_PRECONDITION(args.size() == 1,
                    "PreDecrement must be a unary operator");
            out << "--";
            dumpValue(pack, function, args[0], /*parenthesize=*/true, out);
            break;
        case RValueOperator::ArrayLength:
            ASSERT_PRECONDITION(args.size() == 1,
                    "ArrayLength must be a unary operator");
            dumpValue(pack, function, args[0], /*parenthesize=*/true, out);
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
        case RValueOperator::Comma:
            out << lParen;
            dumpBinaryRValueOperator(pack, function, op, args, ",", out);
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
            dumpValue(pack, function, args[0], /*parenthesize=*/true, out);
            out << "[";
            dumpValue(pack, function, args[1], /*parenthesize=*/true, out);
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
            dumpValue(pack, function, args[0], /*parenthesize=*/true, out);
            out << ")" << kSpace << "?" << kSpace << "(";
            dumpValue(pack, function, args[1], /*parenthesize=*/true, out);
            out << ")" << kSpace << ":" << kSpace << "(";
            dumpValue(pack, function, args[2], /*parenthesize=*/true, out);
            out << ")" << rParen;
            break;

        // Misc
        case RValueOperator::ConstructStruct:
        default:
            out << "(" << rValueOperatorToString(op);
            for (auto& arg : args) {
                out << kSpace;
                dumpValue(pack, function, arg, /*parenthesize=*/true, out);
            }
            out << ")";
            break;
    }
}

void dumpRValueFunctionCall(
        const PackFromGlsl& pack, const FunctionDefinition& function,
        FunctionId functionId, const std::vector<ValueId>& args,
        std::ostringstream& out) {
    dumpFunctionName(pack, functionId, out);
    out << "(";
    bool firstArg = true;
    for (auto& arg : args) {
        if (firstArg) {
            firstArg = false;
        } else {
            out << "," << kSpace;
        }
        dumpValue(pack, function, arg, /*parenthesize=*/false, out);
    }
    out << ")";
}

void dumpRValueLiteral(const LiteralRValue& literal, std::ostringstream& out) {
    if (auto* valueAsBool = std::get_if<bool>(&literal.value)) {
        out << *valueAsBool;
    } else if (auto* valueAsInt = std::get_if<int>(&literal.value)) {
        out << *valueAsInt;
    } else if (auto* valueAsDouble = std::get_if<double>(&literal.value)) {
        out << *valueAsDouble;
    } else if (auto* valueAsUnsignedInt = std::get_if<unsigned int>(&literal.value)) {
        out << *valueAsUnsignedInt;
    } else {
        PANIC_PRECONDITION("Unreachable");
    }
}

void dumpRValue(
        const PackFromGlsl& pack, const FunctionDefinition& function, RValueId rValueId,
        bool parenthesize, std::ostringstream& out) {
    if (rValueId.id == 0) {
        out << "INVALID_RVALUE";
        return;
    }
    ASSERT_PRECONDITION(pack.rValues.find(rValueId) != pack.rValues.end(),
            "Missing RValue");
    auto& rValue = pack.rValues.at(rValueId);
    if (auto* evaluable = std::get_if<EvaluableRValue>(&rValue)) {
        if (auto* op = std::get_if<RValueOperator>(&evaluable->op)) {
            dumpRValueOperator(pack, function, *op, evaluable->args, parenthesize, out);
        } else if (auto* functionId = std::get_if<FunctionId>(&evaluable->op)) {
            dumpRValueFunctionCall(pack, function, *functionId, evaluable->args, out);
        } else {
            PANIC_PRECONDITION("Unreachable");
        }
    } else if (auto* literal = std::get_if<LiteralRValue>(&rValue)) {
        dumpRValueLiteral(*literal, out);
    } else {
        PANIC_PRECONDITION("Unreachable");
    }
}

void dumpGlobalSymbol(
        const PackFromGlsl& pack, GlobalSymbolId globalSymbolId, std::ostringstream& out) {
    if (globalSymbolId.id == 0) {
        out << "INVALID_GLOBAL_SYMBOL";
        return;
    }
    ASSERT_PRECONDITION(pack.globalSymbols.find(globalSymbolId) != pack.globalSymbols.end(),
            "Missing global symbol");

    auto globalSymbol = pack.globalSymbols.at(globalSymbolId);
    out << globalSymbol.name;
}

void dumpLocalSymbol(
        const PackFromGlsl& pack, const FunctionDefinition& function, LocalSymbolId localSymbolId,
        bool dumpType, std::ostringstream& out) {
    if (localSymbolId.id == 0) {
        out << "INVALID_LOCAL_SYMBOL";
        return;
    }
    ASSERT_PRECONDITION(function.localSymbols.find(localSymbolId) != function.localSymbols.end(),
            "Missing local symbol");

    auto& localSymbol = function.localSymbols.at(localSymbolId);
    if (dumpType) {
        astrict::dumpType(pack, localSymbol.type, out);
        out << " ";
    }
    out << localSymbol.name;
}

void dumpValue(
        const PackFromGlsl& pack, const FunctionDefinition& function, ValueId valueId,
        bool parenthesize, std::ostringstream& out) {
    if (auto* rValueId = std::get_if<RValueId>(&valueId)) {
        dumpRValue(pack, function, *rValueId, parenthesize, out);
    } else if (auto *globalSymbolId = std::get_if<GlobalSymbolId>(&valueId)) {
        dumpGlobalSymbol(pack, *globalSymbolId, out);
    } else if (auto *localSymbolId = std::get_if<LocalSymbolId>(&valueId)) {
        dumpLocalSymbol(pack, function, *localSymbolId, /*dumpType=*/false, out);
    } else {
        PANIC_PRECONDITION("Unreachable");
    }
}

void dumpBlock(
        const PackFromGlsl& pack, const FunctionDefinition& function, StatementBlockId blockId,
        int depth, std::ostringstream& out) {
    ASSERT_PRECONDITION(pack.statementBlocks.find(blockId) != pack.statementBlocks.end(),
            "Missing block definition");
    std::string indentMinusOne;
    for (int i = 0; i < depth - 1; ++i) {
        indentMinusOne += kIndentAmount;
    }
    std::string indent = indentMinusOne;
    if (depth > 0) {
        indent += kIndentAmount;
    }
    for (auto statement : pack.statementBlocks.at(blockId)) {
        if (auto* rValueId = std::get_if<RValueId>(&statement)) {
            out << indent;
            dumpRValue(pack, function, *rValueId, /*parenthesize=*/false, out);
            out << ";" << kNewline;
        } else if (auto* ifStatement = std::get_if<IfStatement>(&statement)) {
            out << indent << "if" << kSpace << "(";
            dumpValue(pack, function, ifStatement->condition, /*parenthesize=*/false, out);
            out << ")" << kSpace << "{" << kNewline;
            dumpBlock(pack, function, ifStatement->thenBlock, depth + 1, out);
            if (ifStatement->elseBlock) {
                out << indent << "}" << kSpace << "else" << kSpace << "{" << kNewline;
                dumpBlock(pack, function, ifStatement->elseBlock.value(), depth + 1, out);
            }
            out << indent << "}" << kNewline;
        } else if (auto* switchStatement = std::get_if<SwitchStatement>(&statement)) {
            out << indent << "switch" << kSpace << "(";
            dumpValue(pack, function, switchStatement->condition, /*parenthesize=*/false, out);
            out << ")" << kSpace << "{" << kNewline;
            dumpBlock(pack, function, switchStatement->body, depth + 1, out);
            out << indent << "}" << kNewline;
        } else if (auto* branchStatement = std::get_if<BranchStatement>(&statement)) {
            switch (branchStatement->op) {
                case BranchOperator::Discard:
                    out << indent << "discard";
                    break;
                case BranchOperator::TerminateInvocation:
                    out << indent << "terminateInvocation";
                    break;
                case BranchOperator::Demote:
                    out << indent << "demote";
                    break;
                case BranchOperator::TerminateRayEXT:
                    out << indent << "terminateRayEXT";
                    break;
                case BranchOperator::IgnoreIntersectionEXT:
                    out << indent << "terminateIntersectionEXT";
                    break;
                case BranchOperator::Return:
                    out << indent << "return";
                    break;
                case BranchOperator::Break:
                    out << indent << "break";
                    break;
                case BranchOperator::Continue:
                    out << indent << "continue";
                    break;
                case BranchOperator::Case:
                    out << indentMinusOne << "case";
                    break;
                case BranchOperator::Default:
                    out << indentMinusOne << "default";
                    break;
            }
            if (branchStatement->operand) {
                out << " ";
                dumpValue(pack, function, branchStatement->operand.value(),
                        /*parenthesize=*/false, out);
            }
            switch (branchStatement->op) {
                case BranchOperator::Case:
                case BranchOperator::Default:
                    out << ":" << kNewline;
                    break;
                default:
                    out << ";" << kNewline;
                    break;
            }
        } else if (auto* loopStatement = std::get_if<LoopStatement>(&statement)) {
            if (loopStatement->testFirst) {
                if (loopStatement->terminal) {
                    out << indent << "for" << kSpace << "(;" << kSpace;
                    dumpValue(pack, function, loopStatement->condition,
                            /*parenthesize=*/false, out);
                    out << ";" << kSpace;
                    dumpRValue(pack, function, loopStatement->terminal.value(),
                            /*parenthesize=*/false, out);
                } else {
                    out << indent << "while" << kSpace << "(";
                    dumpValue(pack, function, loopStatement->condition,
                            /*parenthesize=*/false, out);
                }
                out << ")" << kSpace << "{" << kNewline;
                dumpBlock(pack, function, loopStatement->body, depth + 1, out);
                out << indent << "}" << kNewline;
            } else {
                out << indent << "do" << kSpace << "{" << kNewline;
                dumpBlock(pack, function, loopStatement->body, depth + 1, out);
                out << indent << "}" << kSpace << "while" << kSpace << "(";
                dumpValue(pack, function, loopStatement->condition, /*parenthesize=*/false, out);
                out << ");" << kNewline;
            }
        } else {
            PANIC_PRECONDITION("Unreachable");
        }
    }
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
    for (const auto& parameter : function.parameters) {
        if (firstParameter) {
            firstParameter = false;
        } else {
            out << "," << kSpace;
        }
        parameterSymbolIds.insert(parameter.name);
        dumpLocalSymbol(pack, function, parameter.name, /*dumpType=*/true, out);
    }
    out << ")";
    if (dumpBody) {
        out << kSpace << "{" << kNewline;
        for (const auto& localSymbol : function.localSymbols) {
            if (parameterSymbolIds.find(localSymbol.first) == parameterSymbolIds.end()) {
                out << kIndentAmount;
                dumpType(pack, localSymbol.second.type, out);
                out << kSpace << localSymbol.second.name << ";" << kNewline;
            }
        }
        dumpBlock(pack, function, function.body, 1, out);
        out << "}" << kNewline;
    } else {
        out << ";" << kNewline;
    }
}

void toGlsl(const PackFromGlsl& pack, std::ostringstream& out) {
    for (auto functionId : pack.functionPrototypes) {
        dumpFunction(pack, functionId, /*dumpBody=*/false, out);
    }
    for (auto functionId : pack.functionDefinitionOrder) {
        dumpFunction(pack, functionId, /*dumpBody=*/true, out);
    }
}

} // namespace astrict
