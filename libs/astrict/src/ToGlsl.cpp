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

void dumpValue(const PackFromGlsl& pack, ValueId valueId, std::ostringstream &out);

void dumpRValue(const PackFromGlsl& pack, RValueId rValueId, std::ostringstream &out) {
    if (rValueId.id == 0) {
        out << "INVALID_RVALUE";
        return;
    }
    auto rValue = pack.rValues.getById(rValueId);
    if (auto* evaluable = std::get_if<EvaluableRValue>(&rValue)) {
        if (auto* op = std::get_if<RValueOperator>(&evaluable->op)) {
            out << "(" << rValueOperatorToString(*op);
        } else if (auto* function = std::get_if<FunctionId>(&evaluable->op)) {
            auto name = pack.functionNames.getById(*function);
            out << name << "(";
        } else {
            PANIC_PRECONDITION("Unreachable");
        }
        for (auto& arg : evaluable->args) {
            out << " ";
            dumpValue(pack, arg, out);
        }
        out << ")";
    } else if (auto* literal = std::get_if<LiteralRValue>(&rValue)) {
        out << "LITERAL";
    } else {
        PANIC_PRECONDITION("Unreachable");
    }
}

void dumpLValue(const PackFromGlsl& pack, LValueId lValueId, std::ostringstream &out) {
    if (lValueId.id == 0) {
        out << "INVALID_LVALUE";
        return;
    }
    auto lValue = pack.lValues.getById(lValueId);
    out << lValue.name;
}

void dumpValue(const PackFromGlsl& pack, ValueId valueId, std::ostringstream &out) {
    if (auto* rValueId = std::get_if<RValueId>(&valueId)) {
        dumpRValue(pack, *rValueId, out);
    } else if (auto *lValueId = std::get_if<LValueId>(&valueId)) {
        dumpLValue(pack, *lValueId, out);
    } else {
        PANIC_PRECONDITION("Unreachable");
    }
}

void dumpType(const PackFromGlsl& pack, TypeId typeId, std::ostringstream &out) {
    auto type = pack.types.getById(typeId);
    if (!type.precision.empty()) {
        out << type.precision << " ";
    }
    out << type.name;
    for (const auto& arraySize : type.arraySizes) {
        out << "[" << arraySize << "]";
    }
}

void dumpBlock(const PackFromGlsl& pack, StatementBlockId blockId, int depth, std::ostringstream &out) {
    std::string indentMinusOne;
    for (int i = 0; i < depth - 1; ++i) {
        indentMinusOne += "  ";
    }
    std::string indent = indentMinusOne;
    if (depth > 0) {
        indent += "  ";
    }
    for (auto statement : pack.statementBlocks.getById(blockId)) {
        if (auto* rValueId = std::get_if<RValueId>(&statement)) {
            out << indent;
            dumpRValue(pack, *rValueId, out);
            out << ";\n";
        } else if (auto* ifStatement = std::get_if<IfStatement>(&statement)) {
            out << indent << "if (";
            dumpValue(pack, ifStatement->condition, out);
            out << ") {\n";
            dumpBlock(pack, ifStatement->thenBlock, depth + 1, out);
            if (ifStatement->elseBlock) {
                out << indent << "} else {\n";
                dumpBlock(pack, ifStatement->elseBlock.value(), depth + 1, out);
            }
            out << indent << "}\n";
        } else if (auto* switchStatement = std::get_if<SwitchStatement>(&statement)) {
            out << indent << "switch (";
            dumpValue(pack, switchStatement->condition, out);
            out << ") {\n";
            dumpBlock(pack, switchStatement->body, depth + 1, out);
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
                dumpValue(pack, branchStatement->operand.value(), out);
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
                    dumpValue(pack, loopStatement->condition, out);
                    out << "; ";
                    dumpRValue(pack, loopStatement->terminal.value(), out);
                } else {
                    out << indent << "while (";
                    dumpValue(pack, loopStatement->condition, out);
                }
                out << ") {\n";
                dumpBlock(pack, loopStatement->body, depth + 1, out);
                out << indent << "}\n";
            } else {
                out << indent << "do {\n";
                dumpBlock(pack, loopStatement->body, depth + 1, out);
                out << indent << "} while (";
                dumpValue(pack, loopStatement->condition, out);
                out << ");\n";
            }
        } else {
            PANIC_PRECONDITION("Unreachable");
        }
    }
}

void toGlsl(const PackFromGlsl& pack, std::ostringstream &out) {
    for (const auto& functionDefinition : pack.functionDefinitions) {
        auto name = pack.functionNames.getById(functionDefinition.name);
        dumpType(pack, functionDefinition.returnType, out);
        out << " " << name << "(";
        bool firstParameter = true;
        for (const auto& parameter : functionDefinition.parameters) {
            if (firstParameter) {
                firstParameter = false;
            } else {
                out << ", ";
            }
            dumpType(pack, parameter.type, out);
            out << " ";
            dumpLValue(pack, parameter.name, out);
        }
        out << ")";
        if (functionDefinition.body) {
            out << " {\n";
            dumpBlock(pack, functionDefinition.body.value(), 1, out);
            out << "}\n";
        } else {
            out << ";\n";
        }
    }
}

} // namespace astrict
