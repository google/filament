//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FindPreciseNodes.cpp: Propagates |precise| to AST nodes.
//
// The high level algorithm is as follows.  For every node that "assigns" to a precise object,
// subobject (a precise struct whose field is being assigned) or superobject (a struct with a
// precise field), two things happen:
//
// - The operation is marked precise if it's an arithmetic operation
// - The right hand side of the assignment is made precise.  If only a subobject is precise, only
//   the corresponding subobject of the right hand side is made precise.
//

#include "compiler/translator/tree_util/FindPreciseNodes.h"

#include "common/hash_containers.h"
#include "common/hash_utils.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

// An access chain applied to a variable.  The |precise|-ness of a node does not change when
// indexing arrays, selecting matrix columns or swizzle vectors.  This access chain thus only
// includes block field selections.  The access chain is used to identify the part of an object
// that is or should be |precise|.  If both a.b.c and a.b are precise, only a.b is ever considered.
class AccessChain
{
  public:
    AccessChain() = default;

    bool operator==(const AccessChain &other) const { return mChain == other.mChain; }

    const TVariable *build(TIntermTyped *lvalue);

    const TVector<size_t> &getChain() const { return mChain; }

    void reduceChain(size_t newSize)
    {
        ASSERT(newSize <= mChain.size());
        mChain.resize(newSize);
    }
    void clear() { reduceChain(0); }
    void push_back(size_t index) { mChain.push_back(index); }
    void pop_front(size_t n);
    void append(const AccessChain &other)
    {
        mChain.insert(mChain.end(), other.mChain.begin(), other.mChain.end());
    }
    bool removePrefix(const AccessChain &other);

  private:
    TVector<size_t> mChain;
};

bool IsIndexOp(TOperator op)
{
    switch (op)
    {
        case EOpIndexDirect:
        case EOpIndexDirectStruct:
        case EOpIndexDirectInterfaceBlock:
        case EOpIndexIndirect:
            return true;
        default:
            return false;
    }
}

const TVariable *AccessChain::build(TIntermTyped *lvalue)
{
    if (lvalue->getAsSwizzleNode())
    {
        return build(lvalue->getAsSwizzleNode()->getOperand());
    }
    if (lvalue->getAsSymbolNode())
    {
        const TVariable *var = &lvalue->getAsSymbolNode()->variable();

        // For fields of nameless interface blocks, add the field index too.
        if (var->getType().getInterfaceBlock() != nullptr)
        {
            mChain.push_back(var->getType().getInterfaceBlockFieldIndex());
        }

        return var;
    }
    if (lvalue->getAsAggregate())
    {
        return nullptr;
    }

    TIntermBinary *binary = lvalue->getAsBinaryNode();
    ASSERT(binary);

    TOperator op = binary->getOp();
    ASSERT(IsIndexOp(op));

    const TVariable *var = build(binary->getLeft());

    if (op == EOpIndexDirectStruct || op == EOpIndexDirectInterfaceBlock)
    {
        int fieldIndex = binary->getRight()->getAsConstantUnion()->getIConst(0);
        mChain.push_back(fieldIndex);
    }

    return var;
}

void AccessChain::pop_front(size_t n)
{
    std::rotate(mChain.begin(), mChain.begin() + n, mChain.end());
    reduceChain(mChain.size() - n);
}

bool AccessChain::removePrefix(const AccessChain &other)
{
    // First, make sure the common part of the two access chains match.
    size_t commonSize = std::min(mChain.size(), other.mChain.size());

    for (size_t index = 0; index < commonSize; ++index)
    {
        if (mChain[index] != other.mChain[index])
        {
            return false;
        }
    }

    // Remove the common part from the access chain.  If other is a deeper access chain, this access
    // chain will become empty.
    pop_front(commonSize);

    return true;
}

AccessChain GetAssignmentAccessChain(TIntermOperator *node)
{
    // The assignment is either a unary or a binary node, and the lvalue is always the first child.
    AccessChain lvalueAccessChain;
    lvalueAccessChain.build(node->getChildNode(0)->getAsTyped());
    return lvalueAccessChain;
}

template <typename Traverser>
void TraverseIndexNodesOnly(TIntermNode *node, Traverser *traverser)
{
    if (node->getAsSwizzleNode())
    {
        node = node->getAsSwizzleNode()->getOperand();
    }

    if (node->getAsSymbolNode() || node->getAsAggregate())
    {
        return;
    }

    TIntermBinary *binary = node->getAsBinaryNode();
    ASSERT(binary);

    TOperator op = binary->getOp();
    ASSERT(IsIndexOp(op));

    if (op == EOpIndexIndirect)
    {
        binary->getRight()->traverse(traverser);
    }

    TraverseIndexNodesOnly(binary->getLeft(), traverser);
}

// An object, which could be a sub-object of a variable.
struct ObjectAndAccessChain
{
    const TVariable *variable;
    AccessChain accessChain;
};

bool operator==(const ObjectAndAccessChain &a, const ObjectAndAccessChain &b)
{
    return a.variable == b.variable && a.accessChain == b.accessChain;
}

struct ObjectAndAccessChainHash
{
    size_t operator()(const ObjectAndAccessChain &object) const
    {
        size_t result = angle::ComputeGenericHash(&object.variable, sizeof(object.variable));
        if (!object.accessChain.getChain().empty())
        {
            result =
                result ^ angle::ComputeGenericHash(object.accessChain.getChain().data(),
                                                   object.accessChain.getChain().size() *
                                                       sizeof(object.accessChain.getChain()[0]));
        }
        return result;
    }
};

// A map from variables to AST nodes that modify them (i.e. nodes where IsAssignment(op)).
using VariableToAssignmentNodeMap = angle::HashMap<const TVariable *, TVector<TIntermOperator *>>;
// A set of |return| nodes from functions with a |precise| return value.
using PreciseReturnNodes = angle::HashSet<TIntermBranch *>;
// A set of precise objects that need processing, or have been processed.
using PreciseObjectSet = angle::HashSet<ObjectAndAccessChain, ObjectAndAccessChainHash>;

struct ASTInfo
{
    // Generic information about the tree:
    VariableToAssignmentNodeMap variableAssignmentNodeMap;
    // Information pertaining to |precise| expressions:
    PreciseReturnNodes preciseReturnNodes;
    PreciseObjectSet preciseObjectsToProcess;
    PreciseObjectSet preciseObjectsVisited;
};

int GetObjectPreciseSubChainLength(const ObjectAndAccessChain &object)
{
    const TType &type = object.variable->getType();

    if (type.isPrecise())
    {
        return 0;
    }

    const TFieldListCollection *block = type.getInterfaceBlock();
    if (block == nullptr)
    {
        block = type.getStruct();
    }
    const TVector<size_t> &accessChain = object.accessChain.getChain();

    for (size_t length = 0; length < accessChain.size(); ++length)
    {
        ASSERT(block != nullptr);

        const TField *field = block->fields()[accessChain[length]];
        if (field->type()->isPrecise())
        {
            return static_cast<int>(length + 1);
        }

        block = field->type()->getStruct();
    }

    return -1;
}

void AddPreciseObject(ASTInfo *info, const ObjectAndAccessChain &object)
{
    if (info->preciseObjectsVisited.count(object) > 0)
    {
        return;
    }

    info->preciseObjectsToProcess.insert(object);
    info->preciseObjectsVisited.insert(object);
}

void AddPreciseSubObjects(ASTInfo *info, const ObjectAndAccessChain &object);

void AddObjectIfPrecise(ASTInfo *info, const ObjectAndAccessChain &object)
{
    // See if the access chain is already precise, and if so add the minimum access chain that is
    // precise.
    int preciseSubChainLength = GetObjectPreciseSubChainLength(object);
    if (preciseSubChainLength == -1)
    {
        // If the access chain is not precise, see if there are any fields of it that are precise,
        // and add those individually.
        AddPreciseSubObjects(info, object);
        return;
    }

    ObjectAndAccessChain preciseObject = object;
    preciseObject.accessChain.reduceChain(preciseSubChainLength);

    AddPreciseObject(info, preciseObject);
}

void AddPreciseSubObjects(ASTInfo *info, const ObjectAndAccessChain &object)
{
    const TFieldListCollection *block = object.variable->getType().getInterfaceBlock();
    if (block == nullptr)
    {
        block = object.variable->getType().getStruct();
    }
    const TVector<size_t> &accessChain = object.accessChain.getChain();

    for (size_t length = 0; length < accessChain.size(); ++length)
    {
        block = block->fields()[accessChain[length]]->type()->getStruct();
    }

    if (block == nullptr)
    {
        return;
    }

    for (size_t fieldIndex = 0; fieldIndex < block->fields().size(); ++fieldIndex)
    {
        ObjectAndAccessChain subObject = object;
        subObject.accessChain.push_back(fieldIndex);

        // If the field is precise, add it as a precise subobject.  Otherwise recurse.
        if (block->fields()[fieldIndex]->type()->isPrecise())
        {
            AddPreciseObject(info, subObject);
        }
        else
        {
            AddPreciseSubObjects(info, subObject);
        }
    }
}

bool IsArithmeticOp(TOperator op)
{
    switch (op)
    {
        case EOpNegative:

        case EOpPostIncrement:
        case EOpPostDecrement:
        case EOpPreIncrement:
        case EOpPreDecrement:

        case EOpAdd:
        case EOpSub:
        case EOpMul:
        case EOpDiv:
        case EOpIMod:

        case EOpVectorTimesScalar:
        case EOpVectorTimesMatrix:
        case EOpMatrixTimesVector:
        case EOpMatrixTimesScalar:
        case EOpMatrixTimesMatrix:

        case EOpAddAssign:
        case EOpSubAssign:

        case EOpMulAssign:
        case EOpVectorTimesMatrixAssign:
        case EOpVectorTimesScalarAssign:
        case EOpMatrixTimesScalarAssign:
        case EOpMatrixTimesMatrixAssign:

        case EOpDivAssign:
        case EOpIModAssign:

        case EOpDot:
            return true;
        default:
            return false;
    }
}

// A traverser that gathers the following information, used to kick off processing:
//
// - For each variable, the AST nodes that modify it.
// - The set of |precise| return AST node.
// - The set of |precise| access chains assigned to.
//
class InfoGatherTraverser : public TIntermTraverser
{
  public:
    InfoGatherTraverser(ASTInfo *info) : TIntermTraverser(true, false, false), mInfo(info) {}

    bool visitUnary(Visit visit, TIntermUnary *node) override
    {
        // If the node is an assignment (i.e. ++ and --), store the relevant information.
        if (!IsAssignment(node->getOp()))
        {
            return true;
        }

        visitLvalue(node, node->getOperand());
        return false;
    }

    bool visitBinary(Visit visit, TIntermBinary *node) override
    {
        if (IsAssignment(node->getOp()))
        {
            visitLvalue(node, node->getLeft());

            node->getRight()->traverse(this);

            return false;
        }

        return true;
    }

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        const TIntermSequence &sequence = *(node->getSequence());
        TIntermSymbol *symbol           = sequence.front()->getAsSymbolNode();
        TIntermBinary *initNode         = sequence.front()->getAsBinaryNode();
        TIntermTyped *initExpression    = nullptr;

        if (symbol == nullptr)
        {
            ASSERT(initNode->getOp() == EOpInitialize);

            symbol         = initNode->getLeft()->getAsSymbolNode();
            initExpression = initNode->getRight();
        }

        ASSERT(symbol);
        ObjectAndAccessChain object = {&symbol->variable(), {}};
        AddObjectIfPrecise(mInfo, object);

        if (initExpression)
        {
            mInfo->variableAssignmentNodeMap[object.variable].push_back(initNode);

            // Visit the init expression, which may itself have assignments.
            initExpression->traverse(this);
        }

        return false;
    }

    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override
    {
        mCurrentFunction = node->getFunction();

        for (size_t paramIndex = 0; paramIndex < mCurrentFunction->getParamCount(); ++paramIndex)
        {
            ObjectAndAccessChain param = {mCurrentFunction->getParam(paramIndex), {}};
            AddObjectIfPrecise(mInfo, param);
        }

        return true;
    }

    bool visitBranch(Visit visit, TIntermBranch *node) override
    {
        if (node->getFlowOp() == EOpReturn && node->getChildCount() == 1 &&
            mCurrentFunction->getReturnType().isPrecise())
        {
            mInfo->preciseReturnNodes.insert(node);
        }

        return true;
    }

    bool visitGlobalQualifierDeclaration(Visit visit,
                                         TIntermGlobalQualifierDeclaration *node) override
    {
        if (node->isPrecise())
        {
            ObjectAndAccessChain preciseObject = {&node->getSymbol()->variable(), {}};
            AddPreciseObject(mInfo, preciseObject);
        }

        return false;
    }

  private:
    void visitLvalue(TIntermOperator *assignmentNode, TIntermTyped *lvalueNode)
    {
        AccessChain lvalueChain;
        const TVariable *lvalueBase = lvalueChain.build(lvalueNode);
        if (lvalueBase != nullptr)
        {
            mInfo->variableAssignmentNodeMap[lvalueBase].push_back(assignmentNode);

            ObjectAndAccessChain lvalue = {lvalueBase, lvalueChain};
            AddObjectIfPrecise(mInfo, lvalue);
        }

        TraverseIndexNodesOnly(lvalueNode, this);
    }

    ASTInfo *mInfo                    = nullptr;
    const TFunction *mCurrentFunction = nullptr;
};

// A traverser that, given an access chain, traverses an expression and marks parts of it |precise|.
// For example, in the expression |Struct1(a, Struct2(b, c), d)|:
//
// - Given access chain [1], both |b| and |c| are marked precise.
// - Given access chain [1, 0], only |b| is marked precise.
//
// When access chain is empty, arithmetic nodes are marked |precise| and any access chains found in
// their children is recursively added for processing.
//
// The access chain given to the traverser is derived from the left hand side of an assignment,
// while the traverser is run on the right hand side.
class PropagatePreciseTraverser : public TIntermTraverser
{
  public:
    PropagatePreciseTraverser(ASTInfo *info) : TIntermTraverser(true, false, false), mInfo(info) {}

    void propagatePrecise(TIntermNode *expression, const AccessChain &accessChain)
    {
        mCurrentAccessChain = accessChain;
        expression->traverse(this);
    }

    bool visitUnary(Visit visit, TIntermUnary *node) override
    {
        // Unary operations cannot be applied to structures.
        ASSERT(mCurrentAccessChain.getChain().empty());

        // Mark arithmetic nodes as |precise|.
        if (IsArithmeticOp(node->getOp()))
        {
            node->setIsPrecise();
        }

        // Mark the operand itself |precise| too.
        return true;
    }

    bool visitBinary(Visit visit, TIntermBinary *node) override
    {
        if (IsIndexOp(node->getOp()))
        {
            // Append the remaining access chain with that of the node, and mark that as |precise|.
            // For example, if we are evaluating an expression and expecting to mark the access
            // chain [1, 3] as |precise|, and the node itself has access chain [0, 2] applied to
            // variable V, then what ends up being |precise| is V with access chain [0, 2, 1, 3].
            AccessChain nodeAccessChain;
            const TVariable *baseVariable = nodeAccessChain.build(node);
            if (baseVariable != nullptr)
            {
                nodeAccessChain.append(mCurrentAccessChain);

                ObjectAndAccessChain preciseObject = {baseVariable, nodeAccessChain};
                AddPreciseObject(mInfo, preciseObject);
            }

            // Visit index nodes, each of which should be considered |precise| in its entirety.
            mCurrentAccessChain.clear();
            TraverseIndexNodesOnly(node, this);

            return false;
        }

        if (node->getOp() == EOpComma)
        {
            // For expr1,expr2, consider only expr2 as that's the one whose calculation is relevant.
            node->getRight()->traverse(this);
            return false;
        }

        // Mark arithmetic nodes as |precise|.
        if (IsArithmeticOp(node->getOp()))
        {
            node->setIsPrecise();
        }

        if (IsAssignment(node->getOp()) || node->getOp() == EOpInitialize)
        {
            // If the node itself is a[...] op= expr, consider only expr as |precise|, as that's the
            // one whose calculation is significant.
            node->getRight()->traverse(this);

            // The indices used on the left hand side are also significant in their entirety.
            mCurrentAccessChain.clear();
            TraverseIndexNodesOnly(node->getLeft(), this);

            return false;
        }

        // Binary operations cannot be applied to structures.
        ASSERT(mCurrentAccessChain.getChain().empty());

        // Mark the operands themselves |precise| too.
        return true;
    }

    void visitSymbol(TIntermSymbol *symbol) override
    {
        // Mark the symbol together with the current access chain as |precise|.
        ObjectAndAccessChain preciseObject = {&symbol->variable(), mCurrentAccessChain};
        AddPreciseObject(mInfo, preciseObject);
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        // If this is a struct constructor and the access chain is not empty, only apply |precise|
        // to the field selected by the access chain.
        const TType &type = node->getType();
        const bool isStructConstructor =
            node->getOp() == EOpConstruct && type.getStruct() != nullptr && !type.isArray();

        if (!mCurrentAccessChain.getChain().empty() && isStructConstructor)
        {
            size_t selectedFieldIndex = mCurrentAccessChain.getChain().front();
            mCurrentAccessChain.pop_front(1);

            ASSERT(selectedFieldIndex < node->getChildCount());

            // Visit only said field.
            node->getChildNode(selectedFieldIndex)->traverse(this);
            return false;
        }

        // If this is an array constructor, each element is equally |precise| with the same access
        // chain.  Otherwise there cannot be any access chain for constructors.
        if (node->getOp() == EOpConstruct)
        {
            ASSERT(type.isArray() || mCurrentAccessChain.getChain().empty());
            return true;
        }

        // Otherwise this is a function call.  The access chain is irrelevant and every (non-out)
        // parameter of the function call should be considered |precise|.
        mCurrentAccessChain.clear();

        const TFunction *function = node->getFunction();
        ASSERT(function);

        for (size_t paramIndex = 0; paramIndex < function->getParamCount(); ++paramIndex)
        {
            if (function->getParam(paramIndex)->getType().getQualifier() != EvqParamOut)
            {
                node->getChildNode(paramIndex)->traverse(this);
            }
        }

        // Mark arithmetic nodes as |precise|.
        if (IsArithmeticOp(node->getOp()))
        {
            node->setIsPrecise();
        }

        return false;
    }

  private:
    ASTInfo *mInfo = nullptr;
    AccessChain mCurrentAccessChain;
};
}  // anonymous namespace

void FindPreciseNodes(TCompiler *compiler, TIntermBlock *root)
{
    ASTInfo info;

    InfoGatherTraverser infoGather(&info);
    root->traverse(&infoGather);

    PropagatePreciseTraverser propagator(&info);

    // First, get return expressions out of the way by propagating |precise|.
    for (TIntermBranch *returnNode : info.preciseReturnNodes)
    {
        ASSERT(returnNode->getChildCount() == 1);
        propagator.propagatePrecise(returnNode->getChildNode(0), {});
    }

    // Now take |precise| access chains one by one, and propagate their |precise|-ness to the right
    // hand side of all assignments in which they are on the left hand side, as well as the
    // arithmetic expression that assigns to them.

    while (!info.preciseObjectsToProcess.empty())
    {
        // Get one |precise| object to process.
        auto first                           = info.preciseObjectsToProcess.begin();
        const ObjectAndAccessChain toProcess = *first;
        info.preciseObjectsToProcess.erase(first);

        // Propagate |precise| to every node where it's assigned to.
        const TVector<TIntermOperator *> &assignmentNodes =
            info.variableAssignmentNodeMap[toProcess.variable];
        for (TIntermOperator *assignmentNode : assignmentNodes)
        {
            AccessChain assignmentAccessChain = GetAssignmentAccessChain(assignmentNode);

            // There are two possibilities:
            //
            // - The assignment is to a bigger access chain than that which is being processed, in
            //   which case the entire right hand side is marked |precise|,
            // - The assignment is to a smaller access chain, in which case only the subobject of
            //   the right hand side that corresponds to the remaining part of the access chain must
            //   be marked |precise|.
            //
            // For example, if processing |a.b.c| as a |precise| access chain:
            //
            // - If the assignment is to |a.b.c.d|, then the entire right hand side must be
            //   |precise|.
            // - If the assignment is to |a.b|, only the |.c| part of the right hand side expression
            //   must be |precise|.
            // - If the assignment is to |a.e|, there is nothing to do.
            //
            AccessChain remainingAccessChain = toProcess.accessChain;
            if (!remainingAccessChain.removePrefix(assignmentAccessChain))
            {
                continue;
            }

            propagator.propagatePrecise(assignmentNode, remainingAccessChain);
        }
    }

    // The AST nodes now contain information gathered by this post-processing step, and so the tree
    // must no longer be transformed.
    compiler->enableValidateNoMoreTransformations();
}

}  // namespace sh
