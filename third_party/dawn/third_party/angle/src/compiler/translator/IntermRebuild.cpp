//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <algorithm>

#include "compiler/translator/Compiler.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/AsNode.h"
#include "compiler/translator/IntermRebuild.h"

#define GUARD2(cond, failVal) \
    do                        \
    {                         \
        if (!(cond))          \
        {                     \
            return failVal;   \
        }                     \
    } while (false)

#define GUARD(cond) GUARD2(cond, nullptr)

namespace sh
{

template <typename T, typename U>
ANGLE_INLINE bool AllBits(T haystack, U needle)
{
    return (haystack & needle) == needle;
}

template <typename T, typename U>
ANGLE_INLINE bool AnyBits(T haystack, U needle)
{
    return (haystack & needle) != 0;
}

////////////////////////////////////////////////////////////////////////////////

TIntermRebuild::BaseResult::BaseResult(BaseResult &other)
    : mAction(other.mAction),
      mVisit(other.mVisit),
      mSingle(other.mSingle),
      mMulti(std::move(other.mMulti))
{}

TIntermRebuild::BaseResult::BaseResult(TIntermNode &node, VisitBits visit)
    : mAction(Action::ReplaceSingle), mVisit(visit), mSingle(&node)
{}

TIntermRebuild::BaseResult::BaseResult(TIntermNode *node, VisitBits visit)
    : mAction(node ? Action::ReplaceSingle : Action::Drop),
      mVisit(node ? visit : VisitBits::Neither),
      mSingle(node)
{}

TIntermRebuild::BaseResult::BaseResult(std::nullptr_t)
    : mAction(Action::Drop), mVisit(VisitBits::Neither), mSingle(nullptr)
{}

TIntermRebuild::BaseResult::BaseResult(Fail)
    : mAction(Action::Fail), mVisit(VisitBits::Neither), mSingle(nullptr)
{}

TIntermRebuild::BaseResult::BaseResult(std::vector<TIntermNode *> &&nodes)
    : mAction(Action::ReplaceMulti),
      mVisit(VisitBits::Neither),
      mSingle(nullptr),
      mMulti(std::move(nodes))
{}

void TIntermRebuild::BaseResult::moveAssignImpl(BaseResult &other)
{
    mAction = other.mAction;
    mVisit  = other.mVisit;
    mSingle = other.mSingle;
    mMulti  = std::move(other.mMulti);
}

TIntermRebuild::BaseResult TIntermRebuild::BaseResult::Multi(std::vector<TIntermNode *> &&nodes)
{
    auto it = std::remove(nodes.begin(), nodes.end(), nullptr);
    nodes.erase(it, nodes.end());
    return std::move(nodes);
}

bool TIntermRebuild::BaseResult::isFail() const
{
    return mAction == Action::Fail;
}

bool TIntermRebuild::BaseResult::isDrop() const
{
    return mAction == Action::Drop;
}

TIntermNode *TIntermRebuild::BaseResult::single() const
{
    return mSingle;
}

const std::vector<TIntermNode *> *TIntermRebuild::BaseResult::multi() const
{
    if (mAction == Action::ReplaceMulti)
    {
        return &mMulti;
    }
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////

using PreResult = TIntermRebuild::PreResult;

PreResult::PreResult(TIntermNode &node, VisitBits visit) : BaseResult(node, visit) {}
PreResult::PreResult(TIntermNode *node, VisitBits visit) : BaseResult(node, visit) {}
PreResult::PreResult(std::nullptr_t) : BaseResult(nullptr) {}
PreResult::PreResult(Fail) : BaseResult(Fail()) {}

PreResult::PreResult(BaseResult &&other) : BaseResult(other) {}
PreResult::PreResult(PreResult &&other) : BaseResult(other) {}

void PreResult::operator=(PreResult &&other)
{
    moveAssignImpl(other);
}

////////////////////////////////////////////////////////////////////////////////

using PostResult = TIntermRebuild::PostResult;

PostResult::PostResult(TIntermNode &node) : BaseResult(node, VisitBits::Neither) {}
PostResult::PostResult(TIntermNode *node) : BaseResult(node, VisitBits::Neither) {}
PostResult::PostResult(std::nullptr_t) : BaseResult(nullptr) {}
PostResult::PostResult(Fail) : BaseResult(Fail()) {}

PostResult::PostResult(PostResult &&other) : BaseResult(other) {}
PostResult::PostResult(BaseResult &&other) : BaseResult(other) {}

void PostResult::operator=(PostResult &&other)
{
    moveAssignImpl(other);
}

////////////////////////////////////////////////////////////////////////////////

TIntermRebuild::TIntermRebuild(TCompiler &compiler, bool preVisit, bool postVisit)
    : mCompiler(compiler),
      mSymbolTable(compiler.getSymbolTable()),
      mPreVisit(preVisit),
      mPostVisit(postVisit)
{
    ASSERT(preVisit || postVisit);
}

TIntermRebuild::~TIntermRebuild()
{
    ASSERT(!mNodeStack.value);
    ASSERT(!mNodeStack.tail);
}

const TFunction *TIntermRebuild::getParentFunction() const
{
    return mParentFunc;
}

TIntermNode *TIntermRebuild::getParentNode(size_t offset) const
{
    ASSERT(mNodeStack.tail);
    auto parent = *mNodeStack.tail;
    while (offset > 0)
    {
        --offset;
        ASSERT(parent.tail);
        parent = *parent.tail;
    }
    return parent.value;
}

bool TIntermRebuild::rebuildRoot(TIntermBlock &root)
{
    if (!rebuildInPlace(root))
    {
        return false;
    }
    return mCompiler.validateAST(&root);
}

bool TIntermRebuild::rebuildInPlace(TIntermAggregate &node)
{
    return rebuildInPlaceImpl(node);
}

bool TIntermRebuild::rebuildInPlace(TIntermBlock &node)
{
    return rebuildInPlaceImpl(node);
}

bool TIntermRebuild::rebuildInPlace(TIntermDeclaration &node)
{
    return rebuildInPlaceImpl(node);
}

template <typename Node>
bool TIntermRebuild::rebuildInPlaceImpl(Node &node)
{
    auto *newNode = traverseAnyAs<Node>(node);
    if (!newNode)
    {
        return false;
    }

    if (newNode != &node)
    {
        *node.getSequence() = std::move(*newNode->getSequence());
    }

    return true;
}

PostResult TIntermRebuild::rebuild(TIntermNode &node)
{
    return traverseAny(node);
}

////////////////////////////////////////////////////////////////////////////////

template <typename Node>
Node *TIntermRebuild::traverseAnyAs(TIntermNode &node)
{
    PostResult result(traverseAny(node));
    if (result.mAction == Action::Fail || !result.mSingle)
    {
        return nullptr;
    }
    return asNode<Node>(result.mSingle);
}

template <typename Node>
bool TIntermRebuild::traverseAnyAs(TIntermNode &node, Node *&out)
{
    PostResult result(traverseAny(node));
    if (result.mAction == Action::Fail || result.mAction == Action::ReplaceMulti)
    {
        return false;
    }
    if (!result.mSingle)
    {
        return true;
    }
    out = asNode<Node>(result.mSingle);
    return out != nullptr;
}

bool TIntermRebuild::traverseAggregateBaseChildren(TIntermAggregateBase &node)
{
    auto *const children = node.getSequence();
    ASSERT(children);
    TIntermSequence newChildren;

    for (TIntermNode *child : *children)
    {
        ASSERT(child);
        PostResult result(traverseAny(*child));

        switch (result.mAction)
        {
            case Action::ReplaceSingle:
                newChildren.push_back(result.mSingle);
                break;

            case Action::ReplaceMulti:
                for (TIntermNode *newNode : result.mMulti)
                {
                    if (newNode)
                    {
                        newChildren.push_back(newNode);
                    }
                }
                break;

            case Action::Drop:
                break;

            case Action::Fail:
                return false;

            default:
                ASSERT(false);
                return false;
        }
    }

    *children = std::move(newChildren);

    return true;
}

////////////////////////////////////////////////////////////////////////////////

struct TIntermRebuild::NodeStackGuard
{
    ConsList<TIntermNode *> oldNodeStack;
    ConsList<TIntermNode *> &nodeStack;
    NodeStackGuard(ConsList<TIntermNode *> &nodeStack, TIntermNode *node)
        : oldNodeStack(nodeStack), nodeStack(nodeStack)
    {
        nodeStack = {node, &oldNodeStack};
    }
    ~NodeStackGuard() { nodeStack = oldNodeStack; }
};

PostResult TIntermRebuild::traverseAny(TIntermNode &originalNode)
{
    PreResult preResult = traversePre(originalNode);
    if (!preResult.mSingle)
    {
        ASSERT(preResult.mVisit == VisitBits::Neither);
        return std::move(preResult);
    }

    TIntermNode *currNode       = preResult.mSingle;
    const VisitBits visit       = preResult.mVisit;
    const NodeType currNodeType = getNodeType(*currNode);

    currNode = traverseChildren(currNodeType, originalNode, *currNode, visit);
    if (!currNode)
    {
        return Fail();
    }

    return traversePost(currNodeType, originalNode, *currNode, visit);
}

PreResult TIntermRebuild::traversePre(TIntermNode &originalNode)
{
    if (!mPreVisit)
    {
        return {originalNode, VisitBits::Both};
    }

    NodeStackGuard guard(mNodeStack, &originalNode);

    const NodeType originalNodeType = getNodeType(originalNode);

    switch (originalNodeType)
    {
        case NodeType::Unknown:
            ASSERT(false);
            return Fail();
        case NodeType::Symbol:
            return visitSymbolPre(*originalNode.getAsSymbolNode());
        case NodeType::ConstantUnion:
            return visitConstantUnionPre(*originalNode.getAsConstantUnion());
        case NodeType::FunctionPrototype:
            return visitFunctionPrototypePre(*originalNode.getAsFunctionPrototypeNode());
        case NodeType::PreprocessorDirective:
            return visitPreprocessorDirectivePre(*originalNode.getAsPreprocessorDirective());
        case NodeType::Unary:
            return visitUnaryPre(*originalNode.getAsUnaryNode());
        case NodeType::Binary:
            return visitBinaryPre(*originalNode.getAsBinaryNode());
        case NodeType::Ternary:
            return visitTernaryPre(*originalNode.getAsTernaryNode());
        case NodeType::Swizzle:
            return visitSwizzlePre(*originalNode.getAsSwizzleNode());
        case NodeType::IfElse:
            return visitIfElsePre(*originalNode.getAsIfElseNode());
        case NodeType::Switch:
            return visitSwitchPre(*originalNode.getAsSwitchNode());
        case NodeType::Case:
            return visitCasePre(*originalNode.getAsCaseNode());
        case NodeType::FunctionDefinition:
            return visitFunctionDefinitionPre(*originalNode.getAsFunctionDefinition());
        case NodeType::Aggregate:
            return visitAggregatePre(*originalNode.getAsAggregate());
        case NodeType::Block:
            return visitBlockPre(*originalNode.getAsBlock());
        case NodeType::GlobalQualifierDeclaration:
            return visitGlobalQualifierDeclarationPre(
                *originalNode.getAsGlobalQualifierDeclarationNode());
        case NodeType::Declaration:
            return visitDeclarationPre(*originalNode.getAsDeclarationNode());
        case NodeType::Loop:
            return visitLoopPre(*originalNode.getAsLoopNode());
        case NodeType::Branch:
            return visitBranchPre(*originalNode.getAsBranchNode());
        default:
            ASSERT(false);
            return Fail();
    }
}

TIntermNode *TIntermRebuild::traverseChildren(NodeType currNodeType,
                                              const TIntermNode &originalNode,
                                              TIntermNode &currNode,
                                              VisitBits visit)
{
    if (!AnyBits(visit, VisitBits::Children))
    {
        return &currNode;
    }

    if (AnyBits(visit, VisitBits::ChildrenRequiresSame) && &originalNode != &currNode)
    {
        return &currNode;
    }

    NodeStackGuard guard(mNodeStack, &currNode);

    switch (currNodeType)
    {
        case NodeType::Unknown:
            ASSERT(false);
            return nullptr;
        case NodeType::Symbol:
            return &currNode;
        case NodeType::ConstantUnion:
            return &currNode;
        case NodeType::FunctionPrototype:
            return &currNode;
        case NodeType::PreprocessorDirective:
            return &currNode;
        case NodeType::Unary:
            return traverseUnaryChildren(*currNode.getAsUnaryNode());
        case NodeType::Binary:
            return traverseBinaryChildren(*currNode.getAsBinaryNode());
        case NodeType::Ternary:
            return traverseTernaryChildren(*currNode.getAsTernaryNode());
        case NodeType::Swizzle:
            return traverseSwizzleChildren(*currNode.getAsSwizzleNode());
        case NodeType::IfElse:
            return traverseIfElseChildren(*currNode.getAsIfElseNode());
        case NodeType::Switch:
            return traverseSwitchChildren(*currNode.getAsSwitchNode());
        case NodeType::Case:
            return traverseCaseChildren(*currNode.getAsCaseNode());
        case NodeType::FunctionDefinition:
            return traverseFunctionDefinitionChildren(*currNode.getAsFunctionDefinition());
        case NodeType::Aggregate:
            return traverseAggregateChildren(*currNode.getAsAggregate());
        case NodeType::Block:
            return traverseBlockChildren(*currNode.getAsBlock());
        case NodeType::GlobalQualifierDeclaration:
            return traverseGlobalQualifierDeclarationChildren(
                *currNode.getAsGlobalQualifierDeclarationNode());
        case NodeType::Declaration:
            return traverseDeclarationChildren(*currNode.getAsDeclarationNode());
        case NodeType::Loop:
            return traverseLoopChildren(*currNode.getAsLoopNode());
        case NodeType::Branch:
            return traverseBranchChildren(*currNode.getAsBranchNode());
        default:
            ASSERT(false);
            return nullptr;
    }
}

PostResult TIntermRebuild::traversePost(NodeType currNodeType,
                                        const TIntermNode &originalNode,
                                        TIntermNode &currNode,
                                        VisitBits visit)
{
    if (!mPostVisit)
    {
        return currNode;
    }

    if (!AnyBits(visit, VisitBits::Post))
    {
        return currNode;
    }

    if (AnyBits(visit, VisitBits::PostRequiresSame) && &originalNode != &currNode)
    {
        return currNode;
    }

    NodeStackGuard guard(mNodeStack, &currNode);

    switch (currNodeType)
    {
        case NodeType::Unknown:
            ASSERT(false);
            return Fail();
        case NodeType::Symbol:
            return visitSymbolPost(*currNode.getAsSymbolNode());
        case NodeType::ConstantUnion:
            return visitConstantUnionPost(*currNode.getAsConstantUnion());
        case NodeType::FunctionPrototype:
            return visitFunctionPrototypePost(*currNode.getAsFunctionPrototypeNode());
        case NodeType::PreprocessorDirective:
            return visitPreprocessorDirectivePost(*currNode.getAsPreprocessorDirective());
        case NodeType::Unary:
            return visitUnaryPost(*currNode.getAsUnaryNode());
        case NodeType::Binary:
            return visitBinaryPost(*currNode.getAsBinaryNode());
        case NodeType::Ternary:
            return visitTernaryPost(*currNode.getAsTernaryNode());
        case NodeType::Swizzle:
            return visitSwizzlePost(*currNode.getAsSwizzleNode());
        case NodeType::IfElse:
            return visitIfElsePost(*currNode.getAsIfElseNode());
        case NodeType::Switch:
            return visitSwitchPost(*currNode.getAsSwitchNode());
        case NodeType::Case:
            return visitCasePost(*currNode.getAsCaseNode());
        case NodeType::FunctionDefinition:
            return visitFunctionDefinitionPost(*currNode.getAsFunctionDefinition());
        case NodeType::Aggregate:
            return visitAggregatePost(*currNode.getAsAggregate());
        case NodeType::Block:
            return visitBlockPost(*currNode.getAsBlock());
        case NodeType::GlobalQualifierDeclaration:
            return visitGlobalQualifierDeclarationPost(
                *currNode.getAsGlobalQualifierDeclarationNode());
        case NodeType::Declaration:
            return visitDeclarationPost(*currNode.getAsDeclarationNode());
        case NodeType::Loop:
            return visitLoopPost(*currNode.getAsLoopNode());
        case NodeType::Branch:
            return visitBranchPost(*currNode.getAsBranchNode());
        default:
            ASSERT(false);
            return Fail();
    }
}

////////////////////////////////////////////////////////////////////////////////

TIntermNode *TIntermRebuild::traverseAggregateChildren(TIntermAggregate &node)
{
    if (traverseAggregateBaseChildren(node))
    {
        return &node;
    }
    return nullptr;
}

TIntermNode *TIntermRebuild::traverseBlockChildren(TIntermBlock &node)
{
    if (traverseAggregateBaseChildren(node))
    {
        return &node;
    }
    return nullptr;
}

TIntermNode *TIntermRebuild::traverseDeclarationChildren(TIntermDeclaration &node)
{
    if (traverseAggregateBaseChildren(node))
    {
        return &node;
    }
    return nullptr;
}

TIntermNode *TIntermRebuild::traverseSwizzleChildren(TIntermSwizzle &node)
{
    auto *const operand = node.getOperand();
    ASSERT(operand);

    auto *newOperand = traverseAnyAs<TIntermTyped>(*operand);
    GUARD(newOperand);

    if (newOperand != operand)
    {
        return new TIntermSwizzle(newOperand, node.getSwizzleOffsets());
    }

    return &node;
}

TIntermNode *TIntermRebuild::traverseBinaryChildren(TIntermBinary &node)
{
    auto *const left = node.getLeft();
    ASSERT(left);
    auto *const right = node.getRight();
    ASSERT(right);

    auto *const newLeft = traverseAnyAs<TIntermTyped>(*left);
    GUARD(newLeft);
    auto *const newRight = traverseAnyAs<TIntermTyped>(*right);
    GUARD(newRight);

    if (newLeft != left || newRight != right)
    {
        TOperator op = node.getOp();
        switch (op)
        {
            case TOperator::EOpIndexDirectStruct:
            {
                if (newLeft->getType().getInterfaceBlock())
                {
                    op = TOperator::EOpIndexDirectInterfaceBlock;
                }
            }
            break;

            case TOperator::EOpIndexDirectInterfaceBlock:
            {
                if (newLeft->getType().getStruct())
                {
                    op = TOperator::EOpIndexDirectStruct;
                }
            }
            break;

            case TOperator::EOpComma:
                return TIntermBinary::CreateComma(newLeft, newRight, mCompiler.getShaderVersion());

            default:
                break;
        }

        return new TIntermBinary(op, newLeft, newRight);
    }

    return &node;
}

TIntermNode *TIntermRebuild::traverseUnaryChildren(TIntermUnary &node)
{
    auto *const operand = node.getOperand();
    ASSERT(operand);

    auto *const newOperand = traverseAnyAs<TIntermTyped>(*operand);
    GUARD(newOperand);

    if (newOperand != operand)
    {
        return new TIntermUnary(node.getOp(), newOperand, node.getFunction());
    }

    return &node;
}

TIntermNode *TIntermRebuild::traverseTernaryChildren(TIntermTernary &node)
{
    auto *const cond = node.getCondition();
    ASSERT(cond);
    auto *const true_ = node.getTrueExpression();
    ASSERT(true_);
    auto *const false_ = node.getFalseExpression();
    ASSERT(false_);

    auto *const newCond = traverseAnyAs<TIntermTyped>(*cond);
    GUARD(newCond);
    auto *const newTrue = traverseAnyAs<TIntermTyped>(*true_);
    GUARD(newTrue);
    auto *const newFalse = traverseAnyAs<TIntermTyped>(*false_);
    GUARD(newFalse);

    if (newCond != cond || newTrue != true_ || newFalse != false_)
    {
        return new TIntermTernary(newCond, newTrue, newFalse);
    }

    return &node;
}

TIntermNode *TIntermRebuild::traverseIfElseChildren(TIntermIfElse &node)
{
    auto *const cond = node.getCondition();
    ASSERT(cond);
    auto *const true_  = node.getTrueBlock();
    auto *const false_ = node.getFalseBlock();

    auto *const newCond = traverseAnyAs<TIntermTyped>(*cond);
    GUARD(newCond);
    TIntermBlock *newTrue = nullptr;
    if (true_)
    {
        GUARD(traverseAnyAs(*true_, newTrue));
    }
    TIntermBlock *newFalse = nullptr;
    if (false_)
    {
        GUARD(traverseAnyAs(*false_, newFalse));
    }

    if (newCond != cond || newTrue != true_ || newFalse != false_)
    {
        return new TIntermIfElse(newCond, newTrue, newFalse);
    }

    return &node;
}

TIntermNode *TIntermRebuild::traverseSwitchChildren(TIntermSwitch &node)
{
    auto *const init = node.getInit();
    ASSERT(init);
    auto *const stmts = node.getStatementList();
    ASSERT(stmts);

    auto *const newInit = traverseAnyAs<TIntermTyped>(*init);
    GUARD(newInit);
    auto *const newStmts = traverseAnyAs<TIntermBlock>(*stmts);
    GUARD(newStmts);

    if (newInit != init || newStmts != stmts)
    {
        return new TIntermSwitch(newInit, newStmts);
    }

    return &node;
}

TIntermNode *TIntermRebuild::traverseCaseChildren(TIntermCase &node)
{
    auto *const cond = node.getCondition();

    TIntermTyped *newCond = nullptr;
    if (cond)
    {
        GUARD(traverseAnyAs(*cond, newCond));
    }

    if (newCond != cond)
    {
        return new TIntermCase(newCond);
    }

    return &node;
}

TIntermNode *TIntermRebuild::traverseFunctionDefinitionChildren(TIntermFunctionDefinition &node)
{
    GUARD(!mParentFunc);  // Function definitions cannot be nested.
    mParentFunc = node.getFunction();
    struct OnExit
    {
        const TFunction *&parentFunc;
        OnExit(const TFunction *&parentFunc) : parentFunc(parentFunc) {}
        ~OnExit() { parentFunc = nullptr; }
    } onExit(mParentFunc);

    auto *const proto = node.getFunctionPrototype();
    ASSERT(proto);
    auto *const body = node.getBody();
    ASSERT(body);

    auto *const newProto = traverseAnyAs<TIntermFunctionPrototype>(*proto);
    GUARD(newProto);
    auto *const newBody = traverseAnyAs<TIntermBlock>(*body);
    GUARD(newBody);

    if (newProto != proto || newBody != body)
    {
        return new TIntermFunctionDefinition(newProto, newBody);
    }

    return &node;
}

TIntermNode *TIntermRebuild::traverseGlobalQualifierDeclarationChildren(
    TIntermGlobalQualifierDeclaration &node)
{
    auto *const symbol = node.getSymbol();
    ASSERT(symbol);

    auto *const newSymbol = traverseAnyAs<TIntermSymbol>(*symbol);
    GUARD(newSymbol);

    if (newSymbol != symbol)
    {
        return new TIntermGlobalQualifierDeclaration(newSymbol, node.isPrecise(), node.getLine());
    }

    return &node;
}

TIntermNode *TIntermRebuild::traverseLoopChildren(TIntermLoop &node)
{
    const TLoopType loopType = node.getType();

    auto *const init = node.getInit();
    auto *const cond = node.getCondition();
    auto *const expr = node.getExpression();
    auto *const body = node.getBody();

#if defined(ANGLE_ENABLE_ASSERTS)
    switch (loopType)
    {
        case TLoopType::ELoopFor:
            break;
        case TLoopType::ELoopWhile:
        case TLoopType::ELoopDoWhile:
            ASSERT(cond);
            ASSERT(!init && !expr);
            break;
        default:
            ASSERT(false);
            break;
    }
#endif

    auto *const newBody = traverseAnyAs<TIntermBlock>(*body);
    GUARD(newBody);
    TIntermNode *newInit = nullptr;
    if (init)
    {
        GUARD(traverseAnyAs(*init, newInit));
    }
    TIntermTyped *newCond = nullptr;
    if (cond)
    {
        GUARD(traverseAnyAs(*cond, newCond));
    }
    TIntermTyped *newExpr = nullptr;
    if (expr)
    {
        GUARD(traverseAnyAs(*expr, newExpr));
    }

    if (newInit != init || newCond != cond || newExpr != expr || newBody != body)
    {
        switch (loopType)
        {
            case TLoopType::ELoopFor:
                break;
            case TLoopType::ELoopWhile:
            case TLoopType::ELoopDoWhile:
                GUARD(newCond);
                GUARD(!newInit && !newExpr);
                break;
            default:
                ASSERT(false);
                break;
        }
        return new TIntermLoop(loopType, newInit, newCond, newExpr, newBody);
    }

    return &node;
}

TIntermNode *TIntermRebuild::traverseBranchChildren(TIntermBranch &node)
{
    auto *const expr = node.getExpression();

    TIntermTyped *newExpr = nullptr;
    if (expr)
    {
        GUARD(traverseAnyAs<TIntermTyped>(*expr, newExpr));
    }

    if (newExpr != expr)
    {
        return new TIntermBranch(node.getFlowOp(), newExpr);
    }

    return &node;
}

////////////////////////////////////////////////////////////////////////////////

PreResult TIntermRebuild::visitSymbolPre(TIntermSymbol &node)
{
    return {node, VisitBits::Both};
}

PreResult TIntermRebuild::visitConstantUnionPre(TIntermConstantUnion &node)
{
    return {node, VisitBits::Both};
}

PreResult TIntermRebuild::visitFunctionPrototypePre(TIntermFunctionPrototype &node)
{
    return {node, VisitBits::Both};
}

PreResult TIntermRebuild::visitPreprocessorDirectivePre(TIntermPreprocessorDirective &node)
{
    return {node, VisitBits::Both};
}

PreResult TIntermRebuild::visitUnaryPre(TIntermUnary &node)
{
    return {node, VisitBits::Both};
}

PreResult TIntermRebuild::visitBinaryPre(TIntermBinary &node)
{
    return {node, VisitBits::Both};
}

PreResult TIntermRebuild::visitTernaryPre(TIntermTernary &node)
{
    return {node, VisitBits::Both};
}

PreResult TIntermRebuild::visitSwizzlePre(TIntermSwizzle &node)
{
    return {node, VisitBits::Both};
}

PreResult TIntermRebuild::visitIfElsePre(TIntermIfElse &node)
{
    return {node, VisitBits::Both};
}

PreResult TIntermRebuild::visitSwitchPre(TIntermSwitch &node)
{
    return {node, VisitBits::Both};
}

PreResult TIntermRebuild::visitCasePre(TIntermCase &node)
{
    return {node, VisitBits::Both};
}

PreResult TIntermRebuild::visitLoopPre(TIntermLoop &node)
{
    return {node, VisitBits::Both};
}

PreResult TIntermRebuild::visitBranchPre(TIntermBranch &node)
{
    return {node, VisitBits::Both};
}

PreResult TIntermRebuild::visitDeclarationPre(TIntermDeclaration &node)
{
    return {node, VisitBits::Both};
}

PreResult TIntermRebuild::visitBlockPre(TIntermBlock &node)
{
    return {node, VisitBits::Both};
}

PreResult TIntermRebuild::visitAggregatePre(TIntermAggregate &node)
{
    return {node, VisitBits::Both};
}

PreResult TIntermRebuild::visitFunctionDefinitionPre(TIntermFunctionDefinition &node)
{
    return {node, VisitBits::Both};
}

PreResult TIntermRebuild::visitGlobalQualifierDeclarationPre(
    TIntermGlobalQualifierDeclaration &node)
{
    return {node, VisitBits::Both};
}

////////////////////////////////////////////////////////////////////////////////

PostResult TIntermRebuild::visitSymbolPost(TIntermSymbol &node)
{
    return node;
}

PostResult TIntermRebuild::visitConstantUnionPost(TIntermConstantUnion &node)
{
    return node;
}

PostResult TIntermRebuild::visitFunctionPrototypePost(TIntermFunctionPrototype &node)
{
    return node;
}

PostResult TIntermRebuild::visitPreprocessorDirectivePost(TIntermPreprocessorDirective &node)
{
    return node;
}

PostResult TIntermRebuild::visitUnaryPost(TIntermUnary &node)
{
    return node;
}

PostResult TIntermRebuild::visitBinaryPost(TIntermBinary &node)
{
    return node;
}

PostResult TIntermRebuild::visitTernaryPost(TIntermTernary &node)
{
    return node;
}

PostResult TIntermRebuild::visitSwizzlePost(TIntermSwizzle &node)
{
    return node;
}

PostResult TIntermRebuild::visitIfElsePost(TIntermIfElse &node)
{
    return node;
}

PostResult TIntermRebuild::visitSwitchPost(TIntermSwitch &node)
{
    return node;
}

PostResult TIntermRebuild::visitCasePost(TIntermCase &node)
{
    return node;
}

PostResult TIntermRebuild::visitLoopPost(TIntermLoop &node)
{
    return node;
}

PostResult TIntermRebuild::visitBranchPost(TIntermBranch &node)
{
    return node;
}

PostResult TIntermRebuild::visitDeclarationPost(TIntermDeclaration &node)
{
    return node;
}

PostResult TIntermRebuild::visitBlockPost(TIntermBlock &node)
{
    return node;
}

PostResult TIntermRebuild::visitAggregatePost(TIntermAggregate &node)
{
    return node;
}

PostResult TIntermRebuild::visitFunctionDefinitionPost(TIntermFunctionDefinition &node)
{
    return node;
}

PostResult TIntermRebuild::visitGlobalQualifierDeclarationPost(
    TIntermGlobalQualifierDeclaration &node)
{
    return node;
}

}  // namespace sh
