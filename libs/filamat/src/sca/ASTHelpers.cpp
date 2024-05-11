/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "ASTHelpers.h"

#include "GLSLTools.h"

#include <intermediate.h>
#include <localintermediate.h>

#include <utils/Log.h>

using namespace glslang;

namespace ASTHelpers {

// Traverse the AST to find the definition of a function based on its name/signature.
// e.g: prepareMaterial(struct-MaterialInputs-vf4-vf41;
class FunctionDefinitionFinder : public TIntermTraverser {
public:
    explicit FunctionDefinitionFinder(std::string_view functionName, bool useFQN = true)
            : mFunctionName(functionName), mUseFQN(useFQN) {
    }

    bool visitAggregate(TVisit, TIntermAggregate* node) override {
        if (node->getOp() == EOpFunction) {
            bool match;
            if (mUseFQN) {
                match = node->getName() == mFunctionName;
            } else {
                std::string_view const prospectFunctionName = getFunctionName(node->getName());
                std::string_view const cleanedFunctionName = getFunctionName(mFunctionName);
                match = prospectFunctionName == cleanedFunctionName;
            }
            if (match) {
                mFunctionDefinitionNode = node;
                return false;
            }
        }
        return true;
    }

    TIntermAggregate* getFunctionDefinitionNode() const noexcept {
        return mFunctionDefinitionNode;
    }
private:
    const std::string_view mFunctionName;
    bool mUseFQN;
    TIntermAggregate* mFunctionDefinitionNode = nullptr;
};


// Traverse the AST to find out if a function is called in the function node or in any of its
// child function call nodes.
class FunctionCallFinder : public TIntermTraverser {
public:
    FunctionCallFinder(std::string_view functionName, TIntermNode& root) :
            mFunctionName(functionName), mRoot(root) {}

    bool functionWasCalled() const noexcept {
        return mFunctionFound;
    }

    bool visitAggregate(TVisit, TIntermAggregate* node) override {
        if (node->getOp() != EOpFunctionCall) {
            return true;
        }
        std::string_view const functionCalledName = node->getName();
        if (functionCalledName == mFunctionName) {
            mFunctionFound = true;
        } else {
            // This function call site A is not what we were looking for but it could be in A's body
            // so we recurse inside A to check that.
            FunctionDefinitionFinder finder(functionCalledName);
            mRoot.traverse(&finder);
            TIntermNode* functionDefNode = finder.getFunctionDefinitionNode();

            // Recurse to follow call chain.
            mFunctionFound |= isFunctionCalled(mFunctionName, *functionDefNode, mRoot);
        }
        return true;
    }

private:
    const std::string_view mFunctionName;
    TIntermNode& mRoot;
    bool mFunctionFound = false;
};

// For debugging and printing out an AST portion. Mostly incomplete but complete enough for our need
// TODO: Add more switch cases as needed.
std::string to_string(TOperator op) {
    switch (op) {
        case EOpSequence:               return "EOpSequence";
        case EOpAssign:                 return "EOpAssign";
        case EOpAddAssign:              return "EOpAddAssign";
        case EOpSubAssign:              return "EOpSubAssign";
        case EOpMulAssign:              return "EOpMulAssign";
        case EOpDivAssign:              return "EOpDivAssign";
        case EOpVectorSwizzle:          return "EOpVectorSwizzle";
        case EOpIndexDirectStruct:      return "EOpIndexDirectStruct";
        case EOpFunction:               return "EOpFunction";
        case EOpFunctionCall:           return "EOpFunctionCall";
        case EOpParameters:             return "EOpParameters";
        // branch
        case EOpKill:                   return "EOpKill";
        case EOpTerminateInvocation:    return "EOpTerminateInvocation";
        case EOpDemote:                 return "EOpDemote";
        case EOpTerminateRayKHR:        return "EOpTerminateRayKHR";
        case EOpIgnoreIntersectionKHR:  return "EOpIgnoreIntersectionKHR";
        case EOpReturn:                 return "EOpReturn";
        case EOpBreak:                  return "EOpBreak";
        case EOpContinue:               return "EOpContinue";
        case EOpCase:                   return "EOpCase";
        case EOpDefault:                return "EOpDefault";
        default:
            return std::to_string((int)op);
    }
}

std::string getIndexDirectStructString(const TIntermBinary& node) {
    const TTypeList& structNode = *(node.getLeft()->getType().getStruct());
    TIntermConstantUnion* index =   node.getRight() ->getAsConstantUnion();
    return structNode[index->getConstArray()[0].getIConst()].type->getFieldName().c_str();
}


std::string_view getFunctionName(std::string_view functionSignature) noexcept {
    auto indexParenthesis = functionSignature.find('(');
    return functionSignature.substr(0, indexParenthesis);
}

glslang::TIntermAggregate* getFunctionBySignature(std::string_view functionSignature,
        TIntermNode& rootNode) noexcept {
    FunctionDefinitionFinder functionDefinitionFinder(functionSignature);
    rootNode.traverse(&functionDefinitionFinder);
    return functionDefinitionFinder.getFunctionDefinitionNode();
}

glslang::TIntermAggregate* getFunctionByNameOnly(std::string_view functionName,
        TIntermNode& rootNode) noexcept {
    FunctionDefinitionFinder functionDefinitionFinder(functionName, false);
    rootNode.traverse(&functionDefinitionFinder);
    return functionDefinitionFinder.getFunctionDefinitionNode();
}

bool isFunctionCalled(std::string_view functionName, TIntermNode& functionNode,
        TIntermNode& rootNode) noexcept {
    FunctionCallFinder traverser(functionName, rootNode);
    functionNode.traverse(&traverser);
    return traverser.functionWasCalled();
}

static FunctionParameter::Qualifier glslangQualifier2FunctionParameter(TStorageQualifier q) {
    switch (q) {
        case EvqIn: return FunctionParameter::Qualifier::IN;
        case EvqInOut: return FunctionParameter::Qualifier::INOUT;
        case EvqOut: return FunctionParameter::Qualifier::OUT;
        case EvqConstReadOnly : return FunctionParameter::Qualifier::CONST;
        default: return FunctionParameter::Qualifier::IN;
    }
}

void getFunctionParameters(TIntermAggregate* func,
        std::vector<FunctionParameter>& output) noexcept {
    if (func == nullptr) {
        return;
    }

    // Does it have a list of params
    // The second aggregate is the list of instructions, but the function may be empty
    if (func->getSequence().empty()) {
        return;
    }

    // A function aggregate has a sequence of two aggregate children:
    // Index 0 is a list of params (IntermSymbol).
    for(TIntermNode* parameterNode : func->getSequence().at(0)->getAsAggregate()->getSequence()) {
        TIntermSymbol* parameter = parameterNode->getAsSymbolNode();
        FunctionParameter const p = {
                parameter->getName().c_str(),
                parameter->getType().getCompleteString().c_str(),
                glslangQualifier2FunctionParameter(parameter->getType().getQualifier().storage)
        };
        output.push_back(p);
    }
}

void NodeToString::pad() {
    for (int i = 0; i < depth; ++i) {
        utils::slog.d << "    ";
    }
}

bool NodeToString::visitBinary(TVisit, TIntermBinary* node) {
    pad();
    utils::slog.d << "Binary " << to_string(node->getOp()) << utils::io::endl;
    return true;
}

bool NodeToString::visitUnary(TVisit, TIntermUnary* node) {
    pad();
    utils::slog.d << "Unary " << to_string(node->getOp()) << utils::io::endl;
    return true;
}

bool NodeToString::visitAggregate(TVisit, TIntermAggregate* node) {
    pad();
    utils::slog.d << "Aggregate " << to_string(node->getOp());
    utils::slog.d << " " << node->getName().c_str();
    utils::slog.d << utils::io::endl;
    return true;
}

bool NodeToString::visitSelection(TVisit, TIntermSelection*) {
    pad();
    utils::slog.d << "Selection " << utils::io::endl;
    return true;
}

void NodeToString::visitConstantUnion(TIntermConstantUnion*) {
    pad();
    utils::slog.d << "ConstantUnion " << utils::io::endl;
}

void NodeToString::visitSymbol(TIntermSymbol* node) {
    pad();
    utils::slog.d << "Symbol " << node->getAsSymbolNode()->getName().c_str() << utils::io::endl;
}

bool NodeToString::visitLoop(TVisit, TIntermLoop*) {
    pad();
    utils::slog.d << "Loop " << utils::io::endl;
    return true;
}

bool NodeToString::visitBranch(TVisit, TIntermBranch* branch) {
    pad();
    utils::slog.d << "Branch " << to_string(branch->getFlowOp()) << utils::io::endl;
    return true;
}

bool NodeToString::visitSwitch(TVisit, TIntermSwitch*) {
    utils::slog.d << "Binary " << utils::io::endl;
    return true;
}

} // namespace ASTHelpers
