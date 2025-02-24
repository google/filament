//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Scalarize vector and matrix constructor args, so that vectors built from components don't have
// matrix arguments, and matrices built from components don't have vector arguments. This avoids
// driver bugs around vector and matrix constructors.
//

#include "compiler/translator/tree_ops/glsl/ScalarizeVecAndMatConstructorArgs.h"

#include "angle_gl.h"
#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{
const TType *GetHelperType(const TType &type, TQualifier qualifier)
{
    // If the type does not have a precision, it means that non of the parameters of the constructor
    // have precision (for example because they are constants, or bool), and there is any precision
    // propagation happening from nearby operands.  In that case, assign a highp precision to them;
    // the driver will inline and eliminate the call anyway, and the precision does not affect
    // anything.
    constexpr TPrecision kDefaultPrecision = EbpHigh;

    TType *newType = new TType(type.getBasicType(), type.getNominalSize(), type.getSecondarySize());
    if (type.getBasicType() != EbtBool)
    {
        newType->setPrecision(type.getPrecision() != EbpUndefined ? type.getPrecision()
                                                                  : kDefaultPrecision);
    }
    newType->setQualifier(qualifier);

    return newType;
}

// Traverser that converts a vector or matrix constructor to one that only uses scalars.  To support
// all the various places such a constructor could be found, a helper function is created for each
// such constructor.  The helper function takes the constructor arguments and creates the object.
//
// Constructors that are transformed are:
//
// - vecN(scalar): translates to vecN(scalar, ..., scalar)
// - vecN(vec1, vec2, ...): translates to vecN(vec1.x, vec1.y, vec2.x, ...)
// - vecN(matrix): translates to vecN(matrix[0][0], matrix[0][1], ...)
// - matNxM(scalar): translates to matNxM(scalar, 0, ..., 0
//                                        0, scalar, ..., 0
//                                        ...
//                                        0, 0, ..., scalar)
// - matNxM(vec1, vec2, ...): translates to matNxM(vec1.x, vec1.y, vec2.x, ...)
// - matNxM(matrixAxB): translates to matNxM(matrix[0][0], matrix[0][1], ..., 0
//                                           matrix[1][0], matrix[1][1], ..., 0
//                                           ...
//                                           0,            0,            ..., 1)
//
class ScalarizeTraverser : public TIntermTraverser
{
  public:
    ScalarizeTraverser(TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, false, symbolTable)
    {}

    bool update(TCompiler *compiler, TIntermBlock *root);

  protected:
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;

  private:
    bool shouldScalarize(TIntermTyped *node);

    // Create a helper function that takes the same arguments as the constructor it replaces.
    const TFunction *createHelper(TIntermAggregate *node);
    TIntermTyped *createHelperCall(TIntermAggregate *node, const TFunction *helper);
    void addHelperDefinition(const TFunction *helper, TIntermBlock *body);

    // If given a constructor, convert it to a function call.  Recursively processes constructor
    // arguments.  Otherwise, recursively visit the node.
    TIntermTyped *createConstructor(TIntermTyped *node);

    void extractComponents(const TFunction *helper,
                           size_t componentCount,
                           TIntermSequence *componentsOut);

    void createConstructorVectorFromScalar(TIntermAggregate *node,
                                           const TFunction *helper,
                                           TIntermSequence *constructorArgsOut);
    void createConstructorVectorFromMultiple(TIntermAggregate *node,
                                             const TFunction *helper,
                                             TIntermSequence *constructorArgsOut);
    void createConstructorMatrixFromScalar(TIntermAggregate *node,
                                           const TFunction *helper,
                                           TIntermSequence *constructorArgsOut);
    void createConstructorMatrixFromVectors(TIntermAggregate *node,
                                            const TFunction *helper,
                                            TIntermSequence *constructorArgsOut);
    void createConstructorMatrixFromMatrix(TIntermAggregate *node,
                                           const TFunction *helper,
                                           TIntermSequence *constructorArgsOut);

    TIntermSequence mFunctionsToAdd;
};

bool ScalarizeTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    if (!shouldScalarize(node))
    {
        return true;
    }

    TIntermTyped *replacement = createConstructor(node);
    if (replacement != node)
    {
        queueReplacement(replacement, OriginalNode::IS_DROPPED);
    }
    // createConstructor already visits children
    return false;
}

bool ScalarizeTraverser::shouldScalarize(TIntermTyped *typed)
{
    TIntermAggregate *node = typed->getAsAggregate();
    if (node == nullptr || node->getOp() != EOpConstruct)
    {
        return false;
    }

    const TType &type                = node->getType();
    const TIntermSequence &arguments = *node->getSequence();
    const TType &arg0Type            = arguments[0]->getAsTyped()->getType();

    const bool isSingleVectorCast = arguments.size() == 1 && type.isVector() &&
                                    arg0Type.isVector() &&
                                    type.getNominalSize() == arg0Type.getNominalSize();
    const bool isSingleMatrixCast = arguments.size() == 1 && type.isMatrix() &&
                                    arg0Type.isMatrix() && type.getCols() == arg0Type.getCols() &&
                                    type.getRows() == arg0Type.getRows();

    // Skip non-vector non-matrix constructors, as well as trivial constructors.
    if (type.isArray() || type.getStruct() != nullptr || type.isScalar() || isSingleVectorCast ||
        isSingleMatrixCast)
    {
        return false;
    }

    return true;
}

const TFunction *ScalarizeTraverser::createHelper(TIntermAggregate *node)
{
    TFunction *helper =
        new TFunction(mSymbolTable, kEmptyImmutableString, SymbolType::AngleInternal,
                      GetHelperType(node->getType(), EvqTemporary), true);

    const TIntermSequence &arguments = *node->getSequence();
    for (TIntermNode *arg : arguments)
    {
        const TType *argType = GetHelperType(arg->getAsTyped()->getType(), EvqParamIn);

        TVariable *argVar =
            new TVariable(mSymbolTable, kEmptyImmutableString, argType, SymbolType::AngleInternal);
        helper->addParameter(argVar);
    }

    return helper;
}

TIntermTyped *ScalarizeTraverser::createHelperCall(TIntermAggregate *node, const TFunction *helper)
{
    TIntermSequence callArgs;

    const TIntermSequence &arguments = *node->getSequence();
    for (TIntermNode *arg : arguments)
    {
        // Note: createConstructor makes sure the arg is visited even if not constructor.
        callArgs.push_back(createConstructor(arg->getAsTyped()));
    }

    return TIntermAggregate::CreateFunctionCall(*helper, &callArgs);
}

void ScalarizeTraverser::addHelperDefinition(const TFunction *helper, TIntermBlock *body)
{
    mFunctionsToAdd.push_back(
        new TIntermFunctionDefinition(new TIntermFunctionPrototype(helper), body));
}

TIntermTyped *ScalarizeTraverser::createConstructor(TIntermTyped *typed)
{
    if (!shouldScalarize(typed))
    {
        typed->traverse(this);
        return typed;
    }

    TIntermAggregate *node           = typed->getAsAggregate();
    const TType &type                = node->getType();
    const TIntermSequence &arguments = *node->getSequence();
    const TType &arg0Type            = arguments[0]->getAsTyped()->getType();

    const TFunction *helper = createHelper(node);
    TIntermSequence constructorArgs;

    if (type.isVector())
    {
        if (arguments.size() == 1 && arg0Type.isScalar())
        {
            createConstructorVectorFromScalar(node, helper, &constructorArgs);
        }
        createConstructorVectorFromMultiple(node, helper, &constructorArgs);
    }
    else
    {
        ASSERT(type.isMatrix());

        if (arg0Type.isScalar() && arguments.size() == 1)
        {
            createConstructorMatrixFromScalar(node, helper, &constructorArgs);
        }
        if (arg0Type.isMatrix())
        {
            createConstructorMatrixFromMatrix(node, helper, &constructorArgs);
        }
        createConstructorMatrixFromVectors(node, helper, &constructorArgs);
    }

    TIntermBlock *body = new TIntermBlock;
    body->appendStatement(
        new TIntermBranch(EOpReturn, TIntermAggregate::CreateConstructor(type, &constructorArgs)));
    addHelperDefinition(helper, body);

    return createHelperCall(node, helper);
}

// Extract enough scalar arguments from the arguments of helper to produce enough arguments for the
// constructor call (given in componentCount).
void ScalarizeTraverser::extractComponents(const TFunction *helper,
                                           size_t componentCount,
                                           TIntermSequence *componentsOut)
{
    for (size_t argumentIndex = 0;
         argumentIndex < helper->getParamCount() && componentsOut->size() < componentCount;
         ++argumentIndex)
    {
        TIntermTyped *argument    = new TIntermSymbol(helper->getParam(argumentIndex));
        const TType &argumentType = argument->getType();

        if (argumentType.isScalar())
        {
            // For scalar parameters, there's nothing to do
            componentsOut->push_back(argument);
            continue;
        }
        if (argumentType.isVector())
        {
            // For vector parameters, take components out of the vector one by one.
            for (uint8_t componentIndex = 0; componentIndex < argumentType.getNominalSize() &&
                                             componentsOut->size() < componentCount;
                 ++componentIndex)
            {
                componentsOut->push_back(
                    new TIntermSwizzle(argument->deepCopy(), {componentIndex}));
            }
            continue;
        }

        ASSERT(argumentType.isMatrix());

        // For matrix parameters, take components out of the matrix one by one in column-major
        // order.
        for (uint8_t columnIndex = 0;
             columnIndex < argumentType.getCols() && componentsOut->size() < componentCount;
             ++columnIndex)
        {
            TIntermTyped *col = new TIntermBinary(EOpIndexDirect, argument->deepCopy(),
                                                  CreateIndexNode(columnIndex));

            for (uint8_t componentIndex = 0;
                 componentIndex < argumentType.getRows() && componentsOut->size() < componentCount;
                 ++componentIndex)
            {
                componentsOut->push_back(new TIntermSwizzle(col->deepCopy(), {componentIndex}));
            }
        }
    }
}

void ScalarizeTraverser::createConstructorVectorFromScalar(TIntermAggregate *node,
                                                           const TFunction *helper,
                                                           TIntermSequence *constructorArgsOut)
{
    ASSERT(helper->getParamCount() == 1);
    TIntermTyped *scalar = new TIntermSymbol(helper->getParam(0));
    const TType &type    = node->getType();

    // Replicate the single scalar argument as many times as necessary.
    for (size_t index = 0; index < type.getNominalSize(); ++index)
    {
        constructorArgsOut->push_back(scalar->deepCopy());
    }
}

void ScalarizeTraverser::createConstructorVectorFromMultiple(TIntermAggregate *node,
                                                             const TFunction *helper,
                                                             TIntermSequence *constructorArgsOut)
{
    extractComponents(helper, node->getType().getNominalSize(), constructorArgsOut);
}

void ScalarizeTraverser::createConstructorMatrixFromScalar(TIntermAggregate *node,
                                                           const TFunction *helper,
                                                           TIntermSequence *constructorArgsOut)
{
    ASSERT(helper->getParamCount() == 1);
    TIntermTyped *scalar = new TIntermSymbol(helper->getParam(0));
    const TType &type    = node->getType();

    // Create the scalar over the diagonal.  Every other element is 0.
    for (uint8_t columnIndex = 0; columnIndex < type.getCols(); ++columnIndex)
    {
        for (uint8_t rowIndex = 0; rowIndex < type.getRows(); ++rowIndex)
        {
            if (columnIndex == rowIndex)
            {
                constructorArgsOut->push_back(scalar->deepCopy());
            }
            else
            {
                ASSERT(type.getBasicType() == EbtFloat);
                constructorArgsOut->push_back(CreateFloatNode(0, type.getPrecision()));
            }
        }
    }
}

void ScalarizeTraverser::createConstructorMatrixFromVectors(TIntermAggregate *node,
                                                            const TFunction *helper,
                                                            TIntermSequence *constructorArgsOut)
{
    const TType &type = node->getType();
    extractComponents(helper, type.getCols() * type.getRows(), constructorArgsOut);
}

void ScalarizeTraverser::createConstructorMatrixFromMatrix(TIntermAggregate *node,
                                                           const TFunction *helper,
                                                           TIntermSequence *constructorArgsOut)
{
    ASSERT(helper->getParamCount() == 1);
    TIntermTyped *matrix = new TIntermSymbol(helper->getParam(0));
    const TType &type    = node->getType();

    // The result is the identity matrix with the size of the result, superimposed by the input
    for (uint8_t columnIndex = 0; columnIndex < type.getCols(); ++columnIndex)
    {
        for (uint8_t rowIndex = 0; rowIndex < type.getRows(); ++rowIndex)
        {
            if (columnIndex < matrix->getType().getCols() && rowIndex < matrix->getType().getRows())
            {
                TIntermTyped *col = new TIntermBinary(EOpIndexDirect, matrix->deepCopy(),
                                                      CreateIndexNode(columnIndex));
                constructorArgsOut->push_back(
                    new TIntermSwizzle(col, {static_cast<int>(rowIndex)}));
            }
            else
            {
                ASSERT(type.getBasicType() == EbtFloat);
                constructorArgsOut->push_back(
                    CreateFloatNode(columnIndex == rowIndex ? 1.0f : 0.0f, type.getPrecision()));
            }
        }
    }
}

bool ScalarizeTraverser::update(TCompiler *compiler, TIntermBlock *root)
{
    // Insert any added function definitions at the tope of the block
    root->insertChildNodes(0, mFunctionsToAdd);

    // Apply updates and validate
    return updateTree(compiler, root);
}
}  // namespace

bool ScalarizeVecAndMatConstructorArgs(TCompiler *compiler,
                                       TIntermBlock *root,
                                       TSymbolTable *symbolTable)
{
    ScalarizeTraverser scalarizer(symbolTable);
    root->traverse(&scalarizer);
    return scalarizer.update(compiler, root);
}
}  // namespace sh
