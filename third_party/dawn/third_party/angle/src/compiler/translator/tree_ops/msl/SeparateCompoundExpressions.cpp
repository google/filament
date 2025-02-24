//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <unordered_map>

#include "common/system_utils.h"
#include "compiler/translator/IntermRebuild.h"
#include "compiler/translator/tree_ops/msl/SeparateCompoundExpressions.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/util.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

namespace
{

bool IsIndex(TOperator op)
{
    switch (op)
    {
        case TOperator::EOpIndexDirect:
        case TOperator::EOpIndexDirectInterfaceBlock:
        case TOperator::EOpIndexDirectStruct:
        case TOperator::EOpIndexIndirect:
            return true;
        default:
            return false;
    }
}

bool IsIndex(TIntermTyped &expr)
{
    if (auto *binary = expr.getAsBinaryNode())
    {
        return IsIndex(binary->getOp());
    }
    return expr.getAsSwizzleNode();
}

bool IsCompoundAssignment(TOperator op)
{
    switch (op)
    {
        case EOpAddAssign:
        case EOpSubAssign:
        case EOpMulAssign:
        case EOpVectorTimesMatrixAssign:
        case EOpVectorTimesScalarAssign:
        case EOpMatrixTimesScalarAssign:
        case EOpMatrixTimesMatrixAssign:
        case EOpDivAssign:
        case EOpIModAssign:
        case EOpBitShiftLeftAssign:
        case EOpBitShiftRightAssign:
        case EOpBitwiseAndAssign:
        case EOpBitwiseXorAssign:
        case EOpBitwiseOrAssign:
            return true;
        default:
            return false;
    }
}

bool ViewBinaryChain(TOperator op, TIntermTyped &node, std::vector<TIntermTyped *> &out)
{
    TIntermBinary *binary = node.getAsBinaryNode();
    if (!binary || binary->getOp() != op)
    {
        return false;
    }

    TIntermTyped *left  = binary->getLeft();
    TIntermTyped *right = binary->getRight();

    if (!ViewBinaryChain(op, *left, out))
    {
        out.push_back(left);
    }

    if (!ViewBinaryChain(op, *right, out))
    {
        out.push_back(right);
    }

    return true;
}

std::vector<TIntermTyped *> ViewBinaryChain(TIntermBinary &node)
{
    std::vector<TIntermTyped *> chain;
    ViewBinaryChain(node.getOp(), node, chain);
    ASSERT(chain.size() >= 2);
    return chain;
}

class PrePass : public TIntermRebuild
{
  public:
    PrePass(TCompiler &compiler) : TIntermRebuild(compiler, true, true) {}

  private:
    // Change chains of
    //      x OP y OP z
    // to
    //      x OP (y OP z)
    // regardless of original parenthesization.
    TIntermTyped &reassociateRight(TIntermBinary &node)
    {
        const TOperator op                = node.getOp();
        std::vector<TIntermTyped *> chain = ViewBinaryChain(node);

        TIntermTyped *result = chain.back();
        chain.pop_back();
        ASSERT(result);

        const auto begin = chain.rbegin();
        const auto end   = chain.rend();

        for (auto iter = begin; iter != end; ++iter)
        {
            TIntermTyped *part = *iter;
            ASSERT(part);
            TIntermNode *temp = rebuild(*part).single();
            ASSERT(temp);
            part = temp->getAsTyped();
            ASSERT(part);
            result = new TIntermBinary(op, part, result);
        }
        return *result;
    }

  private:
    PreResult visitBinaryPre(TIntermBinary &node) override
    {
        const TOperator op = node.getOp();
        if (op == TOperator::EOpLogicalAnd || op == TOperator::EOpLogicalOr)
        {
            return {reassociateRight(node), VisitBits::Neither};
        }
        return node;
    }
};

class Separator : public TIntermRebuild
{
    IdGen &mIdGen;
    std::vector<std::vector<TIntermNode *>> mStmtsStack;
    std::vector<std::unordered_map<const TVariable *, TIntermDeclaration *>> mBindingMapStack;
    std::unordered_map<TIntermTyped *, TIntermTyped *> mExprMap;
    std::unordered_set<TIntermDeclaration *> mMaskedDecls;

  public:
    Separator(TCompiler &compiler, SymbolEnv &symbolEnv, IdGen &idGen)
        : TIntermRebuild(compiler, true, true), mIdGen(idGen)
    {}

    ~Separator() override
    {
        ASSERT(mStmtsStack.empty());
        ASSERT(mExprMap.empty());
        ASSERT(mBindingMapStack.empty());
    }

  private:
    std::vector<TIntermNode *> &getCurrStmts()
    {
        ASSERT(!mStmtsStack.empty());
        return mStmtsStack.back();
    }

    std::unordered_map<const TVariable *, TIntermDeclaration *> &getCurrBindingMap()
    {
        ASSERT(!mBindingMapStack.empty());
        return mBindingMapStack.back();
    }

    void pushStmt(TIntermNode &node) { getCurrStmts().push_back(&node); }

    bool isTerminalExpr(TIntermNode &node)
    {
        NodeType nodeType = getNodeType(node);
        switch (nodeType)
        {
            case NodeType::Symbol:
            case NodeType::ConstantUnion:
                return true;
            default:
                return false;
        }
    }

    TIntermTyped *pullMappedExpr(TIntermTyped *node, bool allowBacktrack)
    {
        TIntermTyped *expr;

        {
            auto iter = mExprMap.find(node);
            if (iter == mExprMap.end())
            {
                return node;
            }
            ASSERT(node);
            expr = iter->second;
            ASSERT(expr);
            mExprMap.erase(iter);
        }

        if (allowBacktrack)
        {
            auto &bindingMap = getCurrBindingMap();
            while (TIntermSymbol *symbol = expr->getAsSymbolNode())
            {
                const TVariable &var = symbol->variable();
                auto iter            = bindingMap.find(&var);
                if (iter == bindingMap.end())
                {
                    return expr;
                }
                ASSERT(var.symbolType() == SymbolType::AngleInternal);
                TIntermDeclaration *decl = iter->second;
                ASSERT(decl);
                expr = ViewDeclaration(*decl).initExpr;
                ASSERT(expr);
                bindingMap.erase(iter);
                mMaskedDecls.insert(decl);
            }
        }

        return expr;
    }

    bool isStandaloneExpr(TIntermTyped &expr)
    {
        if (getParentNode()->getAsBlock())
        {
            return true;
        }
        // https://bugs.webkit.org/show_bug.cgi?id=227723: Fix for sequence operator.
        if ((expr.getType().getBasicType() == TBasicType::EbtVoid))
        {
            return true;
        }
        return false;
    }

    void pushBinding(TIntermTyped &oldExpr, TIntermTyped &newExpr)
    {
        if (isStandaloneExpr(newExpr))
        {
            pushStmt(newExpr);
            return;
        }
        if (IsIndex(newExpr))
        {
            mExprMap[&oldExpr] = &newExpr;
            return;
        }
        auto &bindingMap = getCurrBindingMap();
        auto *var        = CreateTempVariable(&mSymbolTable, &newExpr.getType(), EvqTemporary);
        auto *decl = new TIntermDeclaration(var, &newExpr);
        pushStmt(*decl);
        mExprMap[&oldExpr] = new TIntermSymbol(var);
        bindingMap[var]    = decl;
    }

    void pushStacks()
    {
        mStmtsStack.emplace_back();
        mBindingMapStack.emplace_back();
    }

    void popStacks()
    {
        ASSERT(!mBindingMapStack.empty());
        ASSERT(!mStmtsStack.empty());
        ASSERT(mStmtsStack.back().empty());
        mBindingMapStack.pop_back();
        mStmtsStack.pop_back();
    }

    void pushStmtsIntoBlock(TIntermBlock &block, std::vector<TIntermNode *> &stmts)
    {
        TIntermSequence &seq = *block.getSequence();
        for (TIntermNode *stmt : stmts)
        {
            if (TIntermDeclaration *decl = stmt->getAsDeclarationNode())
            {
                auto iter = mMaskedDecls.find(decl);
                if (iter != mMaskedDecls.end())
                {
                    mMaskedDecls.erase(iter);
                    continue;
                }
            }
            seq.push_back(stmt);
        }
    }

    TIntermBlock &buildBlockWithTailAssign(const TVariable &var, TIntermTyped &newExpr)
    {
        std::vector<TIntermNode *> stmts = std::move(getCurrStmts());
        popStacks();

        auto &block = *new TIntermBlock();
        auto &seq   = *block.getSequence();
        seq.reserve(1 + stmts.size());
        pushStmtsIntoBlock(block, stmts);
        seq.push_back(new TIntermBinary(TOperator::EOpAssign, new TIntermSymbol(&var), &newExpr));

        return block;
    }

  private:
    PreResult visitBlockPre(TIntermBlock &node) override
    {
        pushStacks();
        return node;
    }

    PostResult visitBlockPost(TIntermBlock &node) override
    {
        std::vector<TIntermNode *> stmts = std::move(getCurrStmts());
        popStacks();

        TIntermSequence &seq = *node.getSequence();
        seq.clear();
        seq.reserve(stmts.size());
        pushStmtsIntoBlock(node, stmts);

        TIntermNode *parent = getParentNode();
        if (parent && parent->getAsBlock())
        {
            pushStmt(node);
        }

        return node;
    }

    PreResult visitDeclarationPre(TIntermDeclaration &node) override
    {
        Declaration decl = ViewDeclaration(node);
        if (!decl.initExpr || isTerminalExpr(*decl.initExpr))
        {
            pushStmt(node);
            return {node, VisitBits::Neither};
        }
        return node;
    }

    PostResult visitDeclarationPost(TIntermDeclaration &node) override
    {
        Declaration decl = ViewDeclaration(node);
        ASSERT(decl.symbol.variable().symbolType() != SymbolType::Empty);
        ASSERT(!decl.symbol.variable().getType().isStructSpecifier());

        TIntermTyped *newInitExpr = pullMappedExpr(decl.initExpr, true);
        if (decl.initExpr == newInitExpr)
        {
            pushStmt(node);
        }
        else
        {
            auto &newNode = *new TIntermDeclaration();
            newNode.appendDeclarator(
                new TIntermBinary(TOperator::EOpInitialize, &decl.symbol, newInitExpr));
            pushStmt(newNode);
        }
        return node;
    }

    PostResult visitUnaryPost(TIntermUnary &node) override
    {
        TIntermTyped *expr    = node.getOperand();
        TIntermTyped *newExpr = pullMappedExpr(expr, false);
        if (expr == newExpr)
        {
            pushBinding(node, node);
        }
        else
        {
            pushBinding(node, *new TIntermUnary(node.getOp(), newExpr, node.getFunction()));
        }
        return node;
    }

    PreResult visitBinaryPre(TIntermBinary &node) override
    {
        const TOperator op = node.getOp();
        if (op == TOperator::EOpLogicalAnd || op == TOperator::EOpLogicalOr)
        {
            TIntermTyped *left  = node.getLeft();
            TIntermTyped *right = node.getRight();

            PostResult leftResult = rebuild(*left);
            ASSERT(leftResult.single());

            pushStacks();
            PostResult rightResult = rebuild(*right);
            ASSERT(rightResult.single());

            return {node, VisitBits::Post};
        }

        return node;
    }

    PostResult visitBinaryPost(TIntermBinary &node) override
    {
        const TOperator op = node.getOp();
        if (op == TOperator::EOpInitialize && getParentNode()->getAsDeclarationNode())
        {
            // Special case is handled by visitDeclarationPost
            return node;
        }

        TIntermTyped *left  = node.getLeft();
        TIntermTyped *right = node.getRight();

        if (op == TOperator::EOpLogicalAnd || op == TOperator::EOpLogicalOr)
        {
            const Name name = mIdGen.createNewName();
            auto *var = new TVariable(&mSymbolTable, name.rawName(), new TType(TBasicType::EbtBool),
                                      name.symbolType());

            TIntermTyped *newRight   = pullMappedExpr(right, true);
            TIntermBlock *rightBlock = &buildBlockWithTailAssign(*var, *newRight);
            TIntermTyped *newLeft    = pullMappedExpr(left, true);

            TIntermTyped *cond = new TIntermSymbol(var);
            if (op == TOperator::EOpLogicalOr)
            {
                cond = new TIntermUnary(TOperator::EOpLogicalNot, cond, nullptr);
            }

            pushStmt(*new TIntermDeclaration(var, newLeft));
            pushStmt(*new TIntermIfElse(cond, rightBlock, nullptr));
            if (!isStandaloneExpr(node))
            {
                mExprMap[&node] = new TIntermSymbol(var);
            }

            return node;
        }

        const bool isAssign         = IsAssignment(op);
        const bool isCompoundAssign = IsCompoundAssignment(op);
        TIntermTyped *newLeft       = pullMappedExpr(left, false);
        TIntermTyped *newRight      = pullMappedExpr(right, isAssign && !isCompoundAssign);
        if (op == TOperator::EOpComma)
        {
            pushBinding(node, *newRight);
            return node;
        }
        else
        {
            TIntermBinary *newNode;
            if (left == newLeft && right == newRight)
            {
                newNode = &node;
            }
            else
            {
                newNode = new TIntermBinary(op, newLeft, newRight);
            }
            pushBinding(node, *newNode);
            return node;
        }
    }

    PreResult visitTernaryPre(TIntermTernary &node) override
    {
        PostResult condResult = rebuild(*node.getCondition());
        ASSERT(condResult.single());

        pushStacks();
        PostResult thenResult = rebuild(*node.getTrueExpression());
        ASSERT(thenResult.single());

        pushStacks();
        PostResult elseResult = rebuild(*node.getFalseExpression());
        ASSERT(elseResult.single());

        return {node, VisitBits::Post};
    }

    PostResult visitTernaryPost(TIntermTernary &node) override
    {
        TIntermTyped *cond  = node.getCondition();
        TIntermTyped *then  = node.getTrueExpression();
        TIntermTyped *else_ = node.getFalseExpression();

        auto *var               = CreateTempVariable(&mSymbolTable, &node.getType(), EvqTemporary);
        TIntermTyped *newElse   = pullMappedExpr(else_, false);
        TIntermBlock *elseBlock = &buildBlockWithTailAssign(*var, *newElse);
        TIntermTyped *newThen   = pullMappedExpr(then, true);
        TIntermBlock *thenBlock = &buildBlockWithTailAssign(*var, *newThen);
        TIntermTyped *newCond   = pullMappedExpr(cond, true);

        pushStmt(*new TIntermDeclaration{var});
        pushStmt(*new TIntermIfElse(newCond, thenBlock, elseBlock));
        if (!isStandaloneExpr(node))
        {
            mExprMap[&node] = new TIntermSymbol(var);
        }

        return node;
    }

    PostResult visitSwizzlePost(TIntermSwizzle &node) override
    {
        TIntermTyped *expr    = node.getOperand();
        TIntermTyped *newExpr = pullMappedExpr(expr, false);
        if (expr == newExpr)
        {
            pushBinding(node, node);
        }
        else
        {
            pushBinding(node, *new TIntermSwizzle(newExpr, node.getSwizzleOffsets()));
        }
        return node;
    }

    PostResult visitAggregatePost(TIntermAggregate &node) override
    {
        TIntermSequence &args = *node.getSequence();
        for (TIntermNode *&arg : args)
        {
            TIntermTyped *targ = arg->getAsTyped();
            ASSERT(targ);
            arg = pullMappedExpr(targ, false);
        }
        pushBinding(node, node);
        return node;
    }

    PostResult visitPreprocessorDirectivePost(TIntermPreprocessorDirective &node) override
    {
        pushStmt(node);
        return node;
    }

    PostResult visitFunctionPrototypePost(TIntermFunctionPrototype &node) override
    {
        if (!getParentFunction())
        {
            pushStmt(node);
        }
        return node;
    }

    PreResult visitCasePre(TIntermCase &node) override
    {
        if (TIntermTyped *cond = node.getCondition())
        {
            ASSERT(isTerminalExpr(*cond));
        }
        pushStmt(node);
        return {node, VisitBits::Neither};
    }

    PostResult visitSwitchPost(TIntermSwitch &node) override
    {
        TIntermTyped *init    = node.getInit();
        TIntermTyped *newInit = pullMappedExpr(init, false);
        if (init == newInit)
        {
            pushStmt(node);
        }
        else
        {
            pushStmt(*new TIntermSwitch(newInit, node.getStatementList()));
        }

        return node;
    }

    PostResult visitFunctionDefinitionPost(TIntermFunctionDefinition &node) override
    {
        pushStmt(node);
        return node;
    }

    PostResult visitIfElsePost(TIntermIfElse &node) override
    {
        TIntermTyped *cond    = node.getCondition();
        TIntermTyped *newCond = pullMappedExpr(cond, false);
        if (cond == newCond)
        {
            pushStmt(node);
        }
        else
        {
            pushStmt(*new TIntermIfElse(newCond, node.getTrueBlock(), node.getFalseBlock()));
        }
        return node;
    }

    PostResult visitBranchPost(TIntermBranch &node) override
    {
        TIntermTyped *expr    = node.getExpression();
        TIntermTyped *newExpr = pullMappedExpr(expr, false);
        if (expr == newExpr)
        {
            pushStmt(node);
        }
        else
        {
            pushStmt(*new TIntermBranch(node.getFlowOp(), newExpr));
        }
        return node;
    }

    PreResult visitLoopPre(TIntermLoop &node) override
    {
        if (!rebuildInPlace(*node.getBody()))
        {
            UNREACHABLE();
        }
        pushStmt(node);
        return {node, VisitBits::Neither};
    }

    PostResult visitConstantUnionPost(TIntermConstantUnion &node) override
    {
        const TType &type = node.getType();
        if (!type.isScalar() && !type.isVector() && !type.isMatrix())
        {
            pushBinding(node, node);
        }
        return node;
    }

    PostResult visitGlobalQualifierDeclarationPost(TIntermGlobalQualifierDeclaration &node) override
    {
        // With the removal of RewriteGlobalQualifierDecls, we may encounter globals while
        // seperating compound expressions.
        pushStmt(node);
        return node;
    }
};

}  // anonymous namespace

////////////////////////////////////////////////////////////////////////////////

bool sh::SeparateCompoundExpressions(TCompiler &compiler,
                                     SymbolEnv &symbolEnv,
                                     IdGen &idGen,
                                     TIntermBlock &root)
{
    if (angle::GetBoolEnvironmentVar("GMT_DISABLE_SEPARATE_COMPOUND_EXPRESSIONS"))
    {
        return true;
    }

    if (!PrePass(compiler).rebuildRoot(root))
    {
        return false;
    }

    if (!Separator(compiler, symbolEnv, idGen).rebuildRoot(root))
    {
        return false;
    }

    return true;
}
