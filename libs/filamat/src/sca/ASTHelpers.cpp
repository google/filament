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

namespace ASTUtils {

// Traverse the AST to find the definition of a function based on its name/signature.
// e.g: prepareMaterial(struct-MaterialInputs-vf4-vf41;
class FunctionDefinitionFinder : public TIntermTraverser {
public:
    explicit FunctionDefinitionFinder(const std::string& functionName, bool useFQN = true)
            : mFunctionName (functionName), mUseFQN(useFQN) {
    }

    bool visitAggregate(TVisit, TIntermAggregate* node) override {
        if (node->getOp() == EOpFunction) {
            bool match;
            if (mUseFQN) {
                match = std::string(node->getName().c_str()) == mFunctionName.c_str();
            } else {
                std::string prospectFunctionName = getFunctionName(node->getName().c_str());
                std::string cleanedFunctionName = getFunctionName(mFunctionName);
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
    const std::string& mFunctionName;
    bool mUseFQN;
    TIntermAggregate* mFunctionDefinitionNode = nullptr;
};


// Traverse the AST to find out if a function is called in the function node or in any of its
// child function call nodes.
class FunctionCallFinder : public TIntermTraverser {
public:
    FunctionCallFinder(const std::string& functionName, TIntermNode& root) :
            mFunctionName(functionName), mRoot(root) {}

    bool functionWasCalled() const noexcept {
        return mFunctionFound;
    }

    bool visitAggregate(TVisit, TIntermAggregate* node) override {
        if (node->getOp() != EOpFunctionCall) {
            return true;
        }
        std::string functionCalledName = node->getName().c_str();
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
    const std::string& mFunctionName;
    TIntermNode& mRoot;
    bool mFunctionFound = false;
};

// For debugging and printing out an AST portion. Mostly incomplete but complete enough for our need
// TODO: Add more switch cases as needed.
const char* op2Str(TOperator op) {
    switch (op) {
        case EOpAssign : return "EOpAssign";
        case EOpAddAssign: return "EOpAddAssign";
        case EOpSubAssign: return "EOpSubAssign";
        case EOpMulAssign: return "EOpMulAssign";
        case EOpDivAssign: return "EOpDivAssign";
        case EOpVectorSwizzle: return "EOpVectorSwizzle";
        case EOpIndexDirectStruct :return "EOpIndexDirectStruct";
        case EOpFunction:return "EOpFunction";
        case EOpFunctionCall:return "EOpFunctionCall";
        case EOpParameters:return "EOpParameters";
        default: return "???";
    }
}

static std::string getIndexDirectStructString(const TIntermBinary& node) {
    const TTypeList& structNode = *(node.getLeft()->getType().getStruct());
    TIntermConstantUnion* index =   node.getRight() ->getAsConstantUnion();
    return structNode[index->getConstArray()[0].getIConst()].type->getFieldName().c_str();
}

// Meant to explore the Lvalue in an assignment. Depth traverse the left child of an assignment
// binary node to find out the symbol and all access applied on it.
static const TIntermTyped* findLValueBase(const TIntermTyped* node, Symbol& symbol)
{
    do {
        // Make sure we have a binary node
        const TIntermBinary* binary = node->getAsBinaryNode();
        if (binary == nullptr) {
            return node;
        }

        // Check Operator
        TOperator op = binary->getOp();
        if (op != EOpIndexDirect && op != EOpIndexIndirect && op != EOpIndexDirectStruct && op !=
                EOpVectorSwizzle && op != EOpMatrixSwizzle) {
            return nullptr;
        }
        Access access;
        if (op == EOpIndexDirectStruct) {
            access.string = getIndexDirectStructString(*binary);
            access.type = Access::DirectIndexForStruct;
        } else {
            access.string = op2Str(op) ;
            access.type = Access::Swizzling;
        }
        symbol.add(access);
        node = node->getAsBinaryNode()->getLeft();
    } while (true);
}


class SymbolsTracer : public TIntermTraverser {
public:
    explicit SymbolsTracer(std::deque<Symbol>& events) : mEvents(events) {
    }

    // Function call site.
    bool visitAggregate(TVisit, TIntermAggregate* node) override {
        if (node->getOp() != EOpFunctionCall) {
            return true;
        }

        // Find function name.
        std::string functionName = node->getName().c_str();

        // Iterate on function parameters.
        for (size_t parameterIdx = 0; parameterIdx < node->getSequence().size(); parameterIdx++) {
            TIntermNode* parameter = node->getSequence().at(parameterIdx);
            // Parameter is not a pure symbol. It is indexed or swizzled.
            if (parameter->getAsBinaryNode()) {
                Symbol symbol;
                std::vector<Symbol> events;
                const TIntermTyped* n = findLValueBase(parameter->getAsBinaryNode(), symbol);
                if (n != nullptr && n->getAsSymbolNode() != nullptr) {
                    const TString& symbolTString = n->getAsSymbolNode()->getName();
                    symbol.setName(symbolTString.c_str());
                    events.push_back(symbol);
                }

                for (Symbol symbol: events) {
                    Access fCall = {Access::FunctionCall, functionName, parameterIdx};
                    symbol.add(fCall);
                    mEvents.push_back(symbol);
                }

            }
            // Parameter is a pure symbol.
            if (parameter->getAsSymbolNode()) {
                Symbol s(parameter->getAsSymbolNode()->getName().c_str());
                Access fCall = {Access::FunctionCall, functionName, parameterIdx};
                s.add(fCall);
                mEvents.push_back(s);
            }
        }

        return true;
    }

    // Assign operations
    bool visitBinary(TVisit, TIntermBinary* node) override {
        TOperator op = node->getOp();
        Symbol symbol;
        if (op == EOpAssign || op == EOpAddAssign || op == EOpDivAssign || op == EOpSubAssign
                || op == EOpMulAssign ) {
            const TIntermTyped* n = findLValueBase(node->getLeft(), symbol);
            if (n != nullptr && n->getAsSymbolNode() != nullptr) {
                const TString& symbolTString = n->getAsSymbolNode()->getName();
                symbol.setName(symbolTString.c_str());
                mEvents.push_back(symbol);
                return false; // Don't visit subtree since we just traced it with findLValueBase()
            }
        }
        return true;
    }

private:
    std::deque<Symbol>& mEvents;
};

std::string getFunctionName(const std::string& functionSignature) noexcept {
  auto indexParenthesis = functionSignature.find("(");
  return functionSignature.substr(0, indexParenthesis);
}

class NodeToString: public TIntermTraverser {
public:

    void pad() {
       for (int i = 0; i < depth; ++i) {
           utils::slog.e << "    ";
       }
    }

    bool visitBinary(TVisit, TIntermBinary* node) override {
        pad();
        utils::slog.e << "Binary " << op2Str(node->getOp());
        utils::slog.e << utils::io::endl;
        return true;
    }

    bool visitUnary(TVisit, TIntermUnary* node) override {
        pad();
        utils::slog.e << "Unary" << op2Str(node->getOp());
        utils::slog.e << utils::io::endl;
        return true;
    }

    bool visitAggregate(TVisit, TIntermAggregate* node) override {
        pad();
        utils::slog.e << "Aggregate" << op2Str(node->getOp());
        utils::slog.e << utils::io::endl;
        return true;
    }

    bool visitSelection(TVisit, TIntermSelection* node) override {
        pad();
        utils::slog.e << "Selection";
        utils::slog.e << utils::io::endl;
        return true;
    }

    void visitConstantUnion(TIntermConstantUnion* node) override {
        pad();
        utils::slog.e << "ConstantUnion";
        utils::slog.e << utils::io::endl;
    }

    void visitSymbol(TIntermSymbol* node) override {
        pad();
        utils::slog.e << "Symbol " << node->getAsSymbolNode()->getName().c_str();
        utils::slog.e << utils::io::endl;
    }

    bool visitLoop(TVisit, TIntermLoop* node) override {
        pad();
        utils::slog.e << "Loop";
        utils::slog.e << utils::io::endl;
        return true;
    }

    bool visitBranch(TVisit, TIntermBranch* node) override {
        pad();
        utils::slog.e << "Branch";
        utils::slog.e << utils::io::endl;
        return true;
    }

    bool visitSwitch(TVisit, TIntermSwitch* node) override {
        utils::slog.e << "Binary ";
        utils::slog.e << utils::io::endl;
        return true;
    }
};

glslang::TIntermAggregate* getFunctionBySignature(const std::string& functionSignature,
        TIntermNode& rootNode)
        noexcept {
    FunctionDefinitionFinder functionDefinitionFinder(functionSignature);
    rootNode.traverse(&functionDefinitionFinder);
    return functionDefinitionFinder.getFunctionDefinitionNode();
}

glslang::TIntermAggregate* getFunctionByNameOnly(const std::string& functionName,
        TIntermNode& rootNode) noexcept {
    FunctionDefinitionFinder functionDefinitionFinder(functionName, false);
    rootNode.traverse(&functionDefinitionFinder);
    return functionDefinitionFinder.getFunctionDefinitionNode();
}

bool isFunctionCalled(const std::string& functionName, TIntermNode& functionNode,
        TIntermNode& rootNode) noexcept {
    FunctionCallFinder traverser(functionName, rootNode);
    functionNode.traverse(&traverser);
    return traverser.functionWasCalled();
}

void traceSymbols(TIntermNode& functionNode, std::deque<Symbol>& events) {
    SymbolsTracer variableTracer(events);
    functionNode.traverse(&variableTracer);
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

void getFunctionParameters(TIntermAggregate* func, std::vector<FunctionParameter>& output) noexcept {
    if (func == nullptr) {
        return;
    }

    // Does it have a list of params
    // The second aggregate is the list of instructions, but the function may be empty
    if (func->getSequence().size() < 1) {
        return;
    }

    // A function aggregate has a sequence of two aggregate children:
    // Index 0 is a list of params (IntermSymbol).
    for(TIntermNode* parameterNode : func->getSequence().at(0)->getAsAggregate()->getSequence()) {
        TIntermSymbol* parameter = parameterNode->getAsSymbolNode();
        FunctionParameter p = {
                parameter->getName().c_str(),
                parameter->getType().getCompleteString().c_str(),
                glslangQualifier2FunctionParameter(parameter->getType().getQualifier().storage)
        };
        output.push_back(p);
    }
}

template <typename F>
class TraverserAdapter: public TIntermTraverser {
    F closure;
public:
    explicit TraverserAdapter(F closure)
        : TIntermTraverser(true, false, false, false),
          closure(closure) {
    }
    bool visitAggregate(TVisit visit, TIntermAggregate* node) override {
        return closure(visit, node);
    }
};

void textureLodBias(TIntermediate* intermediate, TIntermNode* root,
        const char* lodBiasSymbolName) {
    // we need to run this only from the user's main entry point

    // First, find the "lodBias" symbol
    TIntermSymbol* pIntermSymbolLodBias = nullptr;
    TraverserAdapter findLodBiasSymbol(
            [&, done = false](TVisit visit, TIntermAggregate* node) mutable {
        if (node->getOp() == glslang::EOpSequence) {
            return !done;
        }
        if (node->getOp() == glslang::EOpLinkerObjects) {
            for (TIntermNode* item : node->getSequence()) {
                TIntermSymbol* symbol = item->getAsSymbolNode();
                if (symbol && symbol->getBasicType() == TBasicType::EbtFloat) {
                    if (symbol->getName() == lodBiasSymbolName) {
                        pIntermSymbolLodBias = symbol;
                        done = true;
                        break;
                    }
                }
            }
        }
        return false;
    });
    root->traverse(&findLodBiasSymbol);

    if (!pIntermSymbolLodBias) {
        // something went wrong
        utils::slog.e << "lod bias ignored because \"" << lodBiasSymbolName << "\" was not found!" << utils::io::endl;
        return;
    }

    // add lod bias to texture calls
    TraverserAdapter addLodBiasToTextureCalls(
            [&](TVisit visit, TIntermAggregate* node) {
        // skip everything that's not a texture() call
        if (node->getOp() != glslang::EOpTexture) {
            return true;
        }

        TIntermSequence& sequence = node->getSequence();

        // first check that we have the correct sampler
        TIntermTyped* pTyped = sequence[0]->getAsTyped();
        if (!pTyped) {
            return false;
        }

        TSampler const& sampler = pTyped->getType().getSampler();
        if (sampler.isArrayed() && sampler.isShadow()) {
            // sampler2DArrayShadow is not supported
            return false;
        }

        // Then add the lod bias to the texture() call
        if (sequence.size() == 2) {
            // we only have 2 parameters, add the 3rd one
            TIntermSymbol* symbol = intermediate->addSymbol(*pIntermSymbolLodBias);
            sequence.push_back(symbol);
        } else if (sequence.size() == 3) {
            // load bias is already specified
            TIntermSymbol* symbol = intermediate->addSymbol(*pIntermSymbolLodBias);
            TIntermTyped* pAdd = intermediate->addBinaryMath(TOperator::EOpAdd,
                    sequence[2]->getAsTyped(), symbol,
                    node->getLoc());
            sequence[2] = pAdd;
        }

        return false;
    });
    root->traverse(&addLodBiasToTextureCalls);
}

} // namespace ASTHelpers
