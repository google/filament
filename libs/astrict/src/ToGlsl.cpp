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
#include <utils/Panic.h>

namespace astrict {

void dumpFunctionName(const PackFromGlsl& pack, const FunctionId& functionId,
        std::ostringstream& out) {
    auto name = pack.functionNames.at(functionId);
    auto indexParenthesis = name.find('(');
    out << name.substr(0, indexParenthesis);
}

void dumpValue(
        const PackFromGlsl& pack, const FunctionDefinition& function, ValueId valueId,
        std::ostringstream& out);

void dumpRValue(
        const PackFromGlsl& pack, const FunctionDefinition& function, RValueId rValueId,
        std::ostringstream& out) {
    if (rValueId.id == 0) {
        out << "INVALID_RVALUE";
        return;
    }
    ASSERT_PRECONDITION(pack.rValues.find(rValueId) != pack.rValues.end(),
            "missing RValue");
    auto& rValue = pack.rValues.at(rValueId);
    if (auto* evaluable = std::get_if<EvaluableRValue>(&rValue)) {
        if (auto* op = std::get_if<RValueOperator>(&evaluable->op)) {
            out << "(" << rValueOperatorToString(*op);
        } else if (auto* function = std::get_if<FunctionId>(&evaluable->op)) {
            dumpFunctionName(pack, *function, out);
            out << "(";
        } else {
            PANIC_PRECONDITION("Unreachable");
        }
        for (auto& arg : evaluable->args) {
            out << " ";
            dumpValue(pack, function, arg, out);
        }
        out << ")";
    } else if (auto* literal = std::get_if<LiteralRValue>(&rValue)) {
        out << "LITERAL";
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
            "missing global symbol");

    auto globalSymbol = pack.globalSymbols.at(globalSymbolId);
    out << globalSymbol.name;
}

void dumpLocalSymbol(
        const FunctionDefinition& function, LocalSymbolId localSymbolId, std::ostringstream& out) {
    if (localSymbolId.id == 0) {
        out << "INVALID_LOCAL_SYMBOL";
        return;
    }
    ASSERT_PRECONDITION(function.localSymbols.find(localSymbolId) != function.localSymbols.end(),
            "missing local symbol");

    auto& localSymbol = function.localSymbols.at(localSymbolId);
    out << localSymbol.debugName;
}

void dumpValue(
        const PackFromGlsl& pack, const FunctionDefinition& function, ValueId valueId,
        std::ostringstream& out) {
    if (auto* rValueId = std::get_if<RValueId>(&valueId)) {
        dumpRValue(pack, function, *rValueId, out);
    } else if (auto *globalSymbolId = std::get_if<GlobalSymbolId>(&valueId)) {
        dumpGlobalSymbol(pack, *globalSymbolId, out);
    } else if (auto *localSymbolId = std::get_if<LocalSymbolId>(&valueId)) {
        dumpLocalSymbol(function, *localSymbolId, out);
    } else {
        PANIC_PRECONDITION("Unreachable");
    }
}

void dumpType(const PackFromGlsl& pack, TypeId typeId, std::ostringstream& out) {
    ASSERT_PRECONDITION(pack.types.find(typeId) != pack.types.end(),
            "missing type definition");
    auto& type = pack.types.at(typeId);
    if (!type.precision.empty()) {
        out << type.precision << " ";
    }
    out << type.name;
    for (const auto& arraySize : type.arraySizes) {
        out << "[" << arraySize << "]";
    }
}

void dumpBlock(
        const PackFromGlsl& pack, const FunctionDefinition& function, StatementBlockId blockId,
        int depth, std::ostringstream& out) {
    ASSERT_PRECONDITION(pack.statementBlocks.find(blockId) != pack.statementBlocks.end(),
            "missing block definition");
    std::string indentMinusOne;
    for (int i = 0; i < depth - 1; ++i) {
        indentMinusOne += "  ";
    }
    std::string indent = indentMinusOne;
    if (depth > 0) {
        indent += "  ";
    }
    for (auto statement : pack.statementBlocks.at(blockId)) {
        if (auto* rValueId = std::get_if<RValueId>(&statement)) {
            out << indent;
            dumpRValue(pack, function, *rValueId, out);
            out << ";\n";
        } else if (auto* ifStatement = std::get_if<IfStatement>(&statement)) {
            out << indent << "if (";
            dumpValue(pack, function, ifStatement->condition, out);
            out << ") {\n";
            dumpBlock(pack, function, ifStatement->thenBlock, depth + 1, out);
            if (ifStatement->elseBlock) {
                out << indent << "} else {\n";
                dumpBlock(pack, function, ifStatement->elseBlock.value(), depth + 1, out);
            }
            out << indent << "}\n";
        } else if (auto* switchStatement = std::get_if<SwitchStatement>(&statement)) {
            out << indent << "switch (";
            dumpValue(pack, function, switchStatement->condition, out);
            out << ") {\n";
            dumpBlock(pack, function, switchStatement->body, depth + 1, out);
            out << indent << "}\n";
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
                dumpValue(pack, function, branchStatement->operand.value(), out);
            }
            switch (branchStatement->op) {
                case BranchOperator::Case:
                case BranchOperator::Default:
                    out << ":\n";
                    break;
                default:
                    out << ";\n";
                    break;
            }
        } else if (auto* loopStatement = std::get_if<LoopStatement>(&statement)) {
            if (loopStatement->testFirst) {
                if (loopStatement->terminal) {
                    out << indent << "for (; ";
                    dumpValue(pack, function, loopStatement->condition, out);
                    out << "; ";
                    dumpRValue(pack, function, loopStatement->terminal.value(), out);
                } else {
                    out << indent << "while (";
                    dumpValue(pack, function, loopStatement->condition, out);
                }
                out << ") {\n";
                dumpBlock(pack, function, loopStatement->body, depth + 1, out);
                out << indent << "}\n";
            } else {
                out << indent << "do {\n";
                dumpBlock(pack, function, loopStatement->body, depth + 1, out);
                out << indent << "} while (";
                dumpValue(pack, function, loopStatement->condition, out);
                out << ");\n";
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
            "missing function definition");
    auto& function = pack.functionDefinitions.at(functionId);
    dumpType(pack, function.returnType, out);
    out << " ";
    dumpFunctionName(pack, function.name, out);
    out << "(";
    bool firstParameter = true;
    for (const auto& parameter : function.parameters) {
        if (firstParameter) {
            firstParameter = false;
        } else {
            out << ", ";
        }
        dumpType(pack, parameter.type, out);
        out << " ";
        dumpLocalSymbol(function, parameter.name, out);
    }
    out << ")";
    if (dumpBody) {
        out << " {\n";
        dumpBlock(pack, function, function.body, 1, out);
        out << "}\n";
    } else {
        out << ";\n";
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
