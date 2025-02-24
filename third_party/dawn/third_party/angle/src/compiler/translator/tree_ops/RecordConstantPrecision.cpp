//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// During parsing, all constant expressions are folded to constant union nodes. The expressions that
// have been folded may have had precision qualifiers, which should affect the precision of the
// consuming operation. If the folded constant union nodes are written to output as such they won't
// have any precision qualifiers, and their effect on the precision of the consuming operation is
// lost.
//
// RecordConstantPrecision is an AST traverser that inspects the precision qualifiers of constants
// and hoists the constants outside the containing expression as precision qualified named variables
// in case that is required for correct precision propagation.
//

#include "compiler/translator/tree_ops/RecordConstantPrecision.h"

#include "compiler/translator/InfoSink.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

class RecordConstantPrecisionTraverser : public TIntermTraverser
{
  public:
    RecordConstantPrecisionTraverser(TSymbolTable *symbolTable);

    void visitConstantUnion(TIntermConstantUnion *node) override;

  protected:
    bool operandAffectsParentOperationPrecision(TIntermTyped *operand);
};

RecordConstantPrecisionTraverser::RecordConstantPrecisionTraverser(TSymbolTable *symbolTable)
    : TIntermTraverser(true, false, true, symbolTable)
{}

bool RecordConstantPrecisionTraverser::operandAffectsParentOperationPrecision(TIntermTyped *operand)
{
    if (getParentNode()->getAsCaseNode() || getParentNode()->getAsBlock())
    {
        return false;
    }

    if (operand->getBasicType() == EbtBool || operand->getBasicType() == EbtStruct)
    {
        return false;
    }

    const TIntermBinary *parentAsBinary = getParentNode()->getAsBinaryNode();
    if (parentAsBinary != nullptr)
    {
        // If the constant is assigned or is used to initialize a variable, or if it's an index,
        // its precision has no effect.
        switch (parentAsBinary->getOp())
        {
            case EOpInitialize:
            case EOpAssign:
            case EOpIndexDirect:
            case EOpIndexDirectStruct:
            case EOpIndexDirectInterfaceBlock:
            case EOpIndexIndirect:
                return false;
            default:
                return true;
        }
    }

    TIntermAggregate *parentAsAggregate = getParentNode()->getAsAggregate();
    if (parentAsAggregate != nullptr)
    {
        // The precision of an aggregate is derived from children only in the following conditions:
        //
        // - Built-in math operations
        // - Constructors
        //
        return parentAsAggregate->isConstructor() ||
               BuiltInGroup::IsMath(parentAsAggregate->getOp());
    }

    return true;
}

void RecordConstantPrecisionTraverser::visitConstantUnion(TIntermConstantUnion *node)
{
    // If the constant has lowp or undefined precision, it can't increase the precision of consuming
    // operations.
    if (node->getPrecision() < EbpMedium)
        return;

    // It's possible the node has no effect on the precision of the consuming expression, depending
    // on the consuming expression, and the precision of the other parameters of the expression.
    if (!operandAffectsParentOperationPrecision(node))
        return;

    // Make the constant a precision-qualified named variable to make sure it affects the precision
    // of the consuming expression.
    TIntermDeclaration *variableDeclaration = nullptr;
    TVariable *variable = DeclareTempVariable(mSymbolTable, node, EvqConst, &variableDeclaration);
    insertStatementInParentBlock(variableDeclaration);
    queueReplacement(CreateTempSymbolNode(variable), OriginalNode::IS_DROPPED);
}

}  // namespace

bool RecordConstantPrecision(TCompiler *compiler, TIntermNode *root, TSymbolTable *symbolTable)
{
    RecordConstantPrecisionTraverser traverser(symbolTable);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

}  // namespace sh
