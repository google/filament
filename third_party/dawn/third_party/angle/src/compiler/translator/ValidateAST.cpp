//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/ValidateAST.h"

#include "common/utilities.h"
#include "compiler/translator/Diagnostics.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/Name.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/SpecializationConstant.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

class ValidateAST : public TIntermTraverser
{
  public:
    static bool validate(TIntermNode *root,
                         TDiagnostics *diagnostics,
                         const ValidateASTOptions &options);

    void visitSymbol(TIntermSymbol *node) override;
    void visitConstantUnion(TIntermConstantUnion *node) override;
    bool visitSwizzle(Visit visit, TIntermSwizzle *node) override;
    bool visitBinary(Visit visit, TIntermBinary *node) override;
    bool visitUnary(Visit visit, TIntermUnary *node) override;
    bool visitTernary(Visit visit, TIntermTernary *node) override;
    bool visitIfElse(Visit visit, TIntermIfElse *node) override;
    bool visitSwitch(Visit visit, TIntermSwitch *node) override;
    bool visitCase(Visit visit, TIntermCase *node) override;
    void visitFunctionPrototype(TIntermFunctionPrototype *node) override;
    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override;
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;
    bool visitBlock(Visit visit, TIntermBlock *node) override;
    bool visitGlobalQualifierDeclaration(Visit visit,
                                         TIntermGlobalQualifierDeclaration *node) override;
    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override;
    bool visitLoop(Visit visit, TIntermLoop *node) override;
    bool visitBranch(Visit visit, TIntermBranch *node) override;
    void visitPreprocessorDirective(TIntermPreprocessorDirective *node) override;

  private:
    ValidateAST(TIntermNode *root, TDiagnostics *diagnostics, const ValidateASTOptions &options);

    // Visit as a generic node
    void visitNode(Visit visit, TIntermNode *node);
    // Visit a structure or interface block, and recursively visit its fields of structure type.
    void visitStructOrInterfaceBlockDeclaration(const TType &type, const TSourceLoc &location);
    void visitStructUsage(const TType &type, const TSourceLoc &location);
    // Visit a unary or aggregate node and validate its built-in op against its built-in function.
    void visitBuiltInFunction(TIntermOperator *op, const TFunction *function);
    // Visit an aggregate node and validate its function call is to one that's already defined.
    void visitFunctionCall(TIntermAggregate *node);
    // Visit a binary node and validate its type against its operands.
    void validateExpressionTypeBinary(TIntermBinary *node);
    // Visit a switch node and validate its selector type is integer.
    void validateExpressionTypeSwitch(TIntermSwitch *node);
    // Visit a symbol node and validate it's declared previously.
    void visitVariableNeedingDeclaration(TIntermSymbol *node);
    // Visit a built-in symbol node and validate it's consistently used across the tree.
    void visitBuiltInVariable(TIntermSymbol *node);

    void scope(Visit visit);
    bool isVariableDeclared(const TVariable *variable);
    bool variableNeedsDeclaration(const TVariable *variable);
    const TFieldListCollection *getStructOrInterfaceBlock(const TType &type, Name *typeNameOut);

    void expectNonNullChildren(Visit visit, TIntermNode *node, size_t least_count);

    bool validateInternal();
    bool isInDeclaration() const;

    ValidateASTOptions mOptions;
    TDiagnostics *mDiagnostics;

    // For validateSingleParent:
    std::map<TIntermNode *, TIntermNode *> mParent;
    bool mSingleParentFailed = false;

    // For validateVariableReferences:
    std::vector<std::set<const TVariable *>> mDeclaredVariables;
    std::set<const TInterfaceBlock *> mNamelessInterfaceBlocks;
    std::map<ImmutableString, const TVariable *> mReferencedBuiltIns;
    bool mVariableReferencesFailed = false;

    // For validateOps:
    bool mOpsFailed = false;

    // For validateBuiltInOps:
    bool mBuiltInOpsFailed = false;

    // For validateFunctionCall:
    std::set<const TFunction *> mDeclaredFunctions;
    bool mFunctionCallFailed = false;

    // For validateNoRawFunctionCalls:
    bool mNoRawFunctionCallsFailed = false;

    // For validateNullNodes:
    bool mNullNodesFailed = false;

    // For validateQualifiers:
    bool mQualifiersFailed = false;

    // For validatePrecision:
    bool mPrecisionFailed = false;

    // For validateStructUsage:
    std::vector<std::map<Name, const TFieldListCollection *>> mStructsAndBlocksByName;
    std::set<const TFunction *> mStructUsageProcessedFunctions;
    bool mStructUsageFailed = false;

    // For validateExpressionTypes:
    bool mExpressionTypesFailed = false;

    // For validateMultiDeclarations:
    bool mMultiDeclarationsFailed = false;

    // For validateNoSwizzleOfSwizzle:
    bool mNoSwizzleOfSwizzleFailed = false;

    // For validateNoQualifiersOnConstructors:
    bool mNoQualifiersOnConstructorsFailed = false;

    // For validateNoStatementsAfterBranch:
    bool mIsBranchVisitedInBlock        = false;
    bool mNoStatementsAfterBranchFailed = false;

    bool mVariableNamingFailed = false;
};

bool IsSameType(const TType &a, const TType &b)
{
    return a.getBasicType() == b.getBasicType() && a.getNominalSize() == b.getNominalSize() &&
           a.getSecondarySize() == b.getSecondarySize() && a.getArraySizes() == b.getArraySizes() &&
           a.getStruct() == b.getStruct() &&
           (!a.isInterfaceBlock() || a.getInterfaceBlock() == b.getInterfaceBlock());
}

bool IsUnaryOp(TOperator op)
{
    switch (op)
    {
        case EOpNegative:
        case EOpPositive:
        case EOpLogicalNot:
        case EOpBitwiseNot:
        case EOpPostIncrement:
        case EOpPostDecrement:
        case EOpPreIncrement:
        case EOpPreDecrement:
        case EOpArrayLength:
            return true;
        default:
            return false;
    }
}

bool IsBinaryOp(TOperator op)
{
    switch (op)
    {
        case EOpAdd:
        case EOpSub:
        case EOpMul:
        case EOpDiv:
        case EOpIMod:
        case EOpEqual:
        case EOpNotEqual:
        case EOpLessThan:
        case EOpGreaterThan:
        case EOpLessThanEqual:
        case EOpGreaterThanEqual:
        case EOpComma:
        case EOpVectorTimesScalar:
        case EOpVectorTimesMatrix:
        case EOpMatrixTimesVector:
        case EOpMatrixTimesScalar:
        case EOpMatrixTimesMatrix:
        case EOpLogicalOr:
        case EOpLogicalXor:
        case EOpLogicalAnd:
        case EOpBitShiftLeft:
        case EOpBitShiftRight:
        case EOpBitwiseAnd:
        case EOpBitwiseXor:
        case EOpBitwiseOr:
        case EOpIndexDirect:
        case EOpIndexIndirect:
        case EOpIndexDirectStruct:
        case EOpIndexDirectInterfaceBlock:
        case EOpAssign:
        case EOpInitialize:
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

bool IsBranchOp(TOperator op)
{
    switch (op)
    {
        case EOpKill:
        case EOpReturn:
        case EOpBreak:
        case EOpContinue:
            return true;
        default:
            return false;
    }
}

bool ValidateAST::validate(TIntermNode *root,
                           TDiagnostics *diagnostics,
                           const ValidateASTOptions &options)
{
    ValidateAST validate(root, diagnostics, options);
    root->traverse(&validate);
    return validate.validateInternal();
}

ValidateAST::ValidateAST(TIntermNode *root,
                         TDiagnostics *diagnostics,
                         const ValidateASTOptions &options)
    : TIntermTraverser(true, false, true, nullptr), mOptions(options), mDiagnostics(diagnostics)
{
    bool isTreeRoot = root->getAsBlock() && root->getAsBlock()->isTreeRoot();

    // Some validations are not applicable unless run on the entire tree.
    if (!isTreeRoot)
    {
        mOptions.validateVariableReferences = false;
        mOptions.validateFunctionCall       = false;
        mOptions.validateStructUsage        = false;
    }

    if (mOptions.validateSingleParent)
    {
        mParent[root] = nullptr;
    }
}

void ValidateAST::visitNode(Visit visit, TIntermNode *node)
{
    if (visit == PreVisit && mOptions.validateSingleParent)
    {
        size_t childCount = node->getChildCount();
        for (size_t i = 0; i < childCount; ++i)
        {
            TIntermNode *child = node->getChildNode(i);
            if (mParent.find(child) != mParent.end())
            {
                // If child is visited twice but through the same parent, the problem is in one of
                // the ancestors.
                if (mParent[child] != node)
                {
                    mDiagnostics->error(node->getLine(), "Found child with two parents",
                                        "<validateSingleParent>");
                    mSingleParentFailed = true;
                }
            }

            mParent[child] = node;
        }
    }

    if (visit == PreVisit && mOptions.validateNoStatementsAfterBranch)
    {
        // If a branch has already been visited in this block, there should be no statements that
        // follow.  Only expected node visit should be PostVisit of the block.
        if (mIsBranchVisitedInBlock)
        {
            mDiagnostics->error(node->getLine(), "Found dead code after branch",
                                "<validateNoStatementsAfterBranch>");
            mNoStatementsAfterBranchFailed = true;
        }
    }
}

void ValidateAST::visitStructOrInterfaceBlockDeclaration(const TType &type,
                                                         const TSourceLoc &location)
{
    if (type.getStruct() == nullptr && type.getInterfaceBlock() == nullptr)
    {
        return;
    }

    // Make sure the structure or interface block is not doubly defined.
    Name typeName;
    const TFieldListCollection *namedStructOrBlock = getStructOrInterfaceBlock(type, &typeName);

    // Recurse the fields of the structure or interface block and check members of structure type.
    // This is done before visiting the struct itself, because if the fields refer to a struct with
    // the same name, they would be referencing the struct declared in an outer scope.
    {
        // Note that structOrBlock was previously only set for named structures, so make sure
        // nameless structs are also recursed.
        const TFieldListCollection *structOrBlock = namedStructOrBlock;
        if (structOrBlock == nullptr)
        {
            structOrBlock = type.getStruct();
        }
        ASSERT(structOrBlock != nullptr);

        for (const TField *field : structOrBlock->fields())
        {
            visitStructUsage(*field->type(), field->line());
        }
    }

    if (namedStructOrBlock)
    {
        ASSERT(!typeName.empty());
        // Structures are not allowed to be doubly defined
        if (type.getStruct() == nullptr)
        {
            // Allow interfaces to be doubly-defined.
            ImmutableString rawName = typeName.rawName();

            if (IsShaderIn(type.getQualifier()))
            {
                rawName = BuildConcatenatedImmutableString(rawName, "<input>");
            }
            else if (IsShaderOut(type.getQualifier()))
            {
                rawName = BuildConcatenatedImmutableString(rawName, "<output>");
            }
            else if (IsStorageBuffer(type.getQualifier()))
            {
                rawName = BuildConcatenatedImmutableString(rawName, "<buffer>");
            }
            else if (type.getQualifier() == EvqUniform)
            {
                rawName = BuildConcatenatedImmutableString(rawName, "<uniform>");
            }
            typeName = Name(rawName, typeName.symbolType());
        }

        if (mStructsAndBlocksByName.back().find(typeName) != mStructsAndBlocksByName.back().end())
        {
            mDiagnostics->error(location,
                                "Found redeclaration of struct or interface block with the same "
                                "name in the same scope <validateStructUsage>",
                                typeName.rawName().data());
            mStructUsageFailed = true;
        }
        else
        {
            // First encounter.
            mStructsAndBlocksByName.back()[typeName] = namedStructOrBlock;
        }
    }
}

void ValidateAST::visitStructUsage(const TType &type, const TSourceLoc &location)
{
    if (type.getStruct() == nullptr)
    {
        return;
    }

    // Make sure the structure being referenced has the same pointer as the closest (in scope)
    // definition.
    const TStructure *structure     = type.getStruct();
    const Name typeName(*structure);

    bool foundDeclaration = false;
    for (size_t scopeIndex = mStructsAndBlocksByName.size(); scopeIndex > 0; --scopeIndex)
    {
        const std::map<Name, const TFieldListCollection *> &scopeDecls =
            mStructsAndBlocksByName[scopeIndex - 1];

        auto iter = scopeDecls.find(typeName);
        if (iter != scopeDecls.end())
        {
            foundDeclaration = true;

            if (iter->second != structure)
            {
                mDiagnostics->error(location,
                                    "Found reference to struct or interface block with doubly "
                                    "created type <validateStructUsage>",
                                    typeName.rawName().data());
                mStructUsageFailed = true;
            }

            break;
        }
    }

    if (!foundDeclaration)
    {
        mDiagnostics->error(location,
                            "Found reference to struct or interface block with no declaration "
                            "<validateStructUsage>",
                            typeName.rawName().data());
        mStructUsageFailed = true;
    }
}

void ValidateAST::visitBuiltInFunction(TIntermOperator *node, const TFunction *function)
{
    const TOperator op = node->getOp();
    if (!BuiltInGroup::IsBuiltIn(op))
    {
        return;
    }

    ImmutableString opValue = BuildConcatenatedImmutableString("op: ", op);
    if (function == nullptr)
    {
        mDiagnostics->error(node->getLine(),
                            "Found node calling built-in without a reference to the built-in "
                            "function <validateBuiltInOps>",
                            opValue.data());
        mVariableReferencesFailed = true;
    }
    else if (function->getBuiltInOp() != op)
    {
        mDiagnostics->error(node->getLine(),
                            "Found node calling built-in with a reference to a different function "
                            "<validateBuiltInOps>",
                            opValue.data());
        mVariableReferencesFailed = true;
    }
}

void ValidateAST::visitFunctionCall(TIntermAggregate *node)
{
    if (node->getOp() != EOpCallFunctionInAST)
    {
        return;
    }

    const TFunction *function = node->getFunction();

    if (function == nullptr)
    {
        mDiagnostics->error(node->getLine(),
                            "Found node calling function without a reference to it",
                            "<validateFunctionCall>");
        mFunctionCallFailed = true;
    }
    else if (mDeclaredFunctions.find(function) == mDeclaredFunctions.end())
    {
        mDiagnostics->error(node->getLine(),
                            "Found node calling previously undeclared function "
                            "<validateFunctionCall>",
                            function->name().data());
        mFunctionCallFailed = true;
    }
}

void ValidateAST::validateExpressionTypeBinary(TIntermBinary *node)
{
    switch (node->getOp())
    {
        case EOpIndexDirect:
        case EOpIndexIndirect:
        {
            TType expectedType(node->getLeft()->getType());
            if (!expectedType.isArray())
            {
                // TODO: Validate matrix column selection and vector component selection.
                // http://anglebug.com/42261441
                break;
            }

            expectedType.toArrayElementType();

            if (!IsSameType(node->getType(), expectedType))
            {
                const TSymbol *symbol = expectedType.getStruct();
                if (symbol == nullptr)
                {
                    symbol = expectedType.getInterfaceBlock();
                }
                const char *name = nullptr;
                if (symbol)
                {
                    name = symbol->name().data();
                }
                else if (expectedType.isScalar())
                {
                    name = "<scalar array>";
                }
                else if (expectedType.isVector())
                {
                    name = "<vector array>";
                }
                else
                {
                    ASSERT(expectedType.isMatrix());
                    name = "<matrix array>";
                }

                mDiagnostics->error(
                    node->getLine(),
                    "Found index node with type that is inconsistent with the array being indexed "
                    "<validateExpressionTypes>",
                    name);
                mExpressionTypesFailed = true;
            }
        }
        break;
        default:
            // TODO: Validate other expressions. http://anglebug.com/42261441
            break;
    }

    switch (node->getOp())
    {
        case EOpIndexDirect:
        case EOpIndexDirectStruct:
        case EOpIndexDirectInterfaceBlock:
            if (node->getRight()->getAsConstantUnion() == nullptr)
            {
                mDiagnostics->error(node->getLine(),
                                    "Found direct index node with a non-constant index",
                                    "<validateExpressionTypes>");
                mExpressionTypesFailed = true;
            }
            break;
        default:
            break;
    }
}

void ValidateAST::validateExpressionTypeSwitch(TIntermSwitch *node)
{
    const TType &selectorType = node->getInit()->getType();

    if (selectorType.getBasicType() != EbtYuvCscStandardEXT &&
        selectorType.getBasicType() != EbtInt && selectorType.getBasicType() != EbtUInt)
    {
        mDiagnostics->error(node->getLine(), "Found switch selector expression that is not integer",
                            "<validateExpressionTypes>");
        mExpressionTypesFailed = true;
    }
    else if (!selectorType.isScalar())
    {
        mDiagnostics->error(node->getLine(), "Found switch selector expression that is not scalar",
                            "<validateExpressionTypes>");
        mExpressionTypesFailed = true;
    }
}

void ValidateAST::visitVariableNeedingDeclaration(TIntermSymbol *node)
{
    const TVariable *variable = &node->variable();
    const TType &type         = node->getType();

    // If it's a reference to a field of a nameless interface block, match it by index and name.
    if (type.getInterfaceBlock() && !type.isInterfaceBlock())
    {
        const TInterfaceBlock *interfaceBlock = type.getInterfaceBlock();
        const TFieldList &fieldList           = interfaceBlock->fields();
        const size_t fieldIndex               = type.getInterfaceBlockFieldIndex();

        if (mNamelessInterfaceBlocks.count(interfaceBlock) == 0)
        {
            mDiagnostics->error(node->getLine(),
                                "Found reference to undeclared or inconsistenly transformed "
                                "nameless interface block <validateVariableReferences>",
                                node->getName().data());
            mVariableReferencesFailed = true;
        }
        else if (fieldIndex >= fieldList.size() || node->getName() != fieldList[fieldIndex]->name())
        {
            mDiagnostics->error(node->getLine(),
                                "Found reference to inconsistenly transformed nameless "
                                "interface block field <validateVariableReferences>",
                                node->getName().data());
            mVariableReferencesFailed = true;
        }
        return;
    }

    const bool isStructDeclaration =
        type.isStructSpecifier() && variable->symbolType() == SymbolType::Empty;

    if (!isStructDeclaration && !isVariableDeclared(variable))
    {
        mDiagnostics->error(node->getLine(),
                            "Found reference to undeclared or inconsistently transformed "
                            "variable <validateVariableReferences>",
                            node->getName().data());
        mVariableReferencesFailed = true;
    }
}

void ValidateAST::visitBuiltInVariable(TIntermSymbol *node)
{
    const TVariable *variable = &node->variable();
    ImmutableString name      = variable->name();

    if (mOptions.validateVariableReferences)
    {
        auto iter = mReferencedBuiltIns.find(name);
        if (iter == mReferencedBuiltIns.end())
        {
            mReferencedBuiltIns[name] = variable;
            return;
        }

        if (variable != iter->second)
        {
            mDiagnostics->error(
                node->getLine(),
                "Found inconsistent references to built-in variable <validateVariableReferences>",
                name.data());
            mVariableReferencesFailed = true;
        }
    }

    if (mOptions.validateQualifiers)
    {
        TQualifier qualifier = variable->getType().getQualifier();

        if ((name == "gl_ClipDistance" && qualifier != EvqClipDistance) ||
            (name == "gl_CullDistance" && qualifier != EvqCullDistance) ||
            (name == "gl_FragDepth" && qualifier != EvqFragDepth) ||
            (name == "gl_LastFragData" && qualifier != EvqLastFragData) ||
            (name == "gl_LastFragColorARM" && qualifier != EvqLastFragColor) ||
            (name == "gl_LastFragDepthARM" && qualifier != EvqLastFragDepth) ||
            (name == "gl_LastFragStencilARM" && qualifier != EvqLastFragStencil))
        {
            mDiagnostics->error(
                node->getLine(),
                "Incorrect qualifier applied to redeclared built-in <validateQualifiers>",
                name.data());
            mQualifiersFailed = true;
        }
    }
}

void ValidateAST::scope(Visit visit)
{
    if (mOptions.validateVariableReferences)
    {
        if (visit == PreVisit)
        {
            mDeclaredVariables.push_back({});
        }
        else if (visit == PostVisit)
        {
            mDeclaredVariables.pop_back();
        }
    }

    if (mOptions.validateStructUsage)
    {
        if (visit == PreVisit)
        {
            mStructsAndBlocksByName.push_back({});
        }
        else if (visit == PostVisit)
        {
            mStructsAndBlocksByName.pop_back();
        }
    }
}

bool ValidateAST::isVariableDeclared(const TVariable *variable)
{
    ASSERT(mOptions.validateVariableReferences);

    for (const std::set<const TVariable *> &scopeVariables : mDeclaredVariables)
    {
        if (scopeVariables.count(variable) > 0)
        {
            return true;
        }
    }

    return false;
}

bool ValidateAST::variableNeedsDeclaration(const TVariable *variable)
{
    // Don't expect declaration for built-in variables.
    if (gl::IsBuiltInName(variable->name().data()))
    {
        return false;
    }

    // Additionally, don't expect declaration for Vulkan specialization constants if not enabled.
    // The declaration of these variables is deferred.
    if (variable->getType().getQualifier() == EvqSpecConst)
    {
        return mOptions.validateSpecConstReferences;
    }

    return true;
}

const TFieldListCollection *ValidateAST::getStructOrInterfaceBlock(const TType &type,
                                                                   Name *typeNameOut)
{
    const TStructure *structure           = type.getStruct();
    const TInterfaceBlock *interfaceBlock = type.getInterfaceBlock();

    ASSERT(structure != nullptr || interfaceBlock != nullptr);

    // Make sure the structure or interface block is not doubly defined.
    const TFieldListCollection *structOrBlock = nullptr;
    if (structure != nullptr && structure->symbolType() != SymbolType::Empty)
    {
        structOrBlock = structure;
        *typeNameOut  = Name(*structure);
    }
    else if (interfaceBlock != nullptr)
    {
        structOrBlock = interfaceBlock;
        *typeNameOut  = Name(*interfaceBlock);
    }

    return structOrBlock;
}

void ValidateAST::expectNonNullChildren(Visit visit, TIntermNode *node, size_t least_count)
{
    if (visit == PreVisit && mOptions.validateNullNodes)
    {
        size_t childCount = node->getChildCount();
        if (childCount < least_count)
        {
            mDiagnostics->error(node->getLine(), "Too few children", "<validateNullNodes>");
            mNullNodesFailed = true;
        }

        for (size_t i = 0; i < childCount; ++i)
        {
            if (node->getChildNode(i) == nullptr)
            {
                mDiagnostics->error(node->getLine(), "Found nullptr child", "<validateNullNodes>");
                mNullNodesFailed = true;
            }
        }
    }
}

void ValidateAST::visitSymbol(TIntermSymbol *node)
{
    visitNode(PreVisit, node);

    const TVariable *variable = &node->variable();

    if (mOptions.validateVariableReferences)
    {
        if (variableNeedsDeclaration(variable))
        {
            visitVariableNeedingDeclaration(node);
        }
    }
    if (variable->symbolType() == SymbolType::Empty)
    {
        if (!isInDeclaration())
        {
            mDiagnostics->error(node->getLine(), "Found symbol with empty name", "");
            mVariableNamingFailed = true;
        }
    }
    const bool isBuiltIn = gl::IsBuiltInName(variable->name().data());
    if (isBuiltIn)
    {
        visitBuiltInVariable(node);
    }

    if (mOptions.validatePrecision)
    {
        if (!isBuiltIn && IsPrecisionApplicableToType(node->getBasicType()) &&
            node->getType().getPrecision() == EbpUndefined)
        {
            // Note that some built-ins don't have a precision.
            mDiagnostics->error(node->getLine(),
                                "Found symbol with undefined precision <validatePrecision>",
                                variable->name().data());
            mPrecisionFailed = true;
        }
    }
}

void ValidateAST::visitConstantUnion(TIntermConstantUnion *node)
{
    visitNode(PreVisit, node);
}

bool ValidateAST::visitSwizzle(Visit visit, TIntermSwizzle *node)
{
    visitNode(visit, node);

    if (mOptions.validateNoSwizzleOfSwizzle)
    {
        if (node->getOperand()->getAsSwizzleNode() != nullptr)
        {
            mDiagnostics->error(node->getLine(), "Found swizzle applied to swizzle",
                                "<validateNoSwizzleOfSwizzle>");
            mNoSwizzleOfSwizzleFailed = true;
        }
    }

    return true;
}

bool ValidateAST::visitBinary(Visit visit, TIntermBinary *node)
{
    visitNode(visit, node);

    if (visit == PreVisit && mOptions.validateOps)
    {
        const bool hasParent = getParentNode() != nullptr;
        const bool isInDeclaration =
            hasParent && getParentNode()->getAsDeclarationNode() != nullptr;
        const TOperator op = node->getOp();
        if (!BuiltInGroup::IsBuiltIn(op) && !IsBinaryOp(op))
        {
            mDiagnostics->error(node->getLine(),
                                "Found binary node with non-binary op <validateOps>",
                                GetOperatorString(op));
            mOpsFailed = true;
        }
        else if (op == EOpInitialize && hasParent && !isInDeclaration)
        {
            mDiagnostics->error(node->getLine(),
                                "Found EOpInitialize node outside declaration <validateOps>",
                                GetOperatorString(op));
            mOpsFailed = true;
        }
        else if (op == EOpAssign && hasParent && isInDeclaration)
        {
            mDiagnostics->error(node->getLine(),
                                "Found EOpAssign node inside declaration <validateOps>",
                                GetOperatorString(op));
            mOpsFailed = true;
        }
    }
    if (mOptions.validateExpressionTypes && visit == PreVisit)
    {
        validateExpressionTypeBinary(node);
    }

    return true;
}

bool ValidateAST::visitUnary(Visit visit, TIntermUnary *node)
{
    visitNode(visit, node);

    if (visit == PreVisit && mOptions.validateOps)
    {
        const TOperator op = node->getOp();
        if (!BuiltInGroup::IsBuiltIn(op) && !IsUnaryOp(op))
        {
            mDiagnostics->error(node->getLine(), "Found unary node with non-unary op <validateOps>",
                                GetOperatorString(op));
            mOpsFailed = true;
        }
    }
    if (visit == PreVisit && mOptions.validateBuiltInOps)
    {
        visitBuiltInFunction(node, node->getFunction());
    }

    return true;
}

bool ValidateAST::visitTernary(Visit visit, TIntermTernary *node)
{
    visitNode(visit, node);
    return true;
}

bool ValidateAST::visitIfElse(Visit visit, TIntermIfElse *node)
{
    visitNode(visit, node);
    return true;
}

bool ValidateAST::visitSwitch(Visit visit, TIntermSwitch *node)
{
    visitNode(visit, node);

    if (mOptions.validateExpressionTypes && visit == PreVisit)
    {
        validateExpressionTypeSwitch(node);
    }

    return true;
}

bool ValidateAST::visitCase(Visit visit, TIntermCase *node)
{
    // Case is allowed to come after a branch, and for dead-code-elimination purposes acts as if a
    // new block is started.
    mIsBranchVisitedInBlock = false;

    visitNode(visit, node);

    return true;
}

void ValidateAST::visitFunctionPrototype(TIntermFunctionPrototype *node)
{
    visitNode(PreVisit, node);

    if (mOptions.validateFunctionCall)
    {
        const TFunction *function = node->getFunction();
        mDeclaredFunctions.insert(function);
    }

    const TFunction *function = node->getFunction();
    const TType &returnType   = function->getReturnType();
    if (mOptions.validatePrecision && IsPrecisionApplicableToType(returnType.getBasicType()) &&
        returnType.getPrecision() == EbpUndefined)
    {
        mDiagnostics->error(
            node->getLine(),
            "Found function with undefined precision on return value <validatePrecision>",
            function->name().data());
        mPrecisionFailed = true;
    }

    if (mOptions.validateStructUsage)
    {
        bool needsProcessing =
            mStructUsageProcessedFunctions.find(function) == mStructUsageProcessedFunctions.end();
        if (needsProcessing && returnType.isStructSpecifier())
        {
            visitStructOrInterfaceBlockDeclaration(returnType, node->getLine());
            mStructUsageProcessedFunctions.insert(function);
        }
        else
        {
            visitStructUsage(returnType, node->getLine());
        }
    }

    for (size_t paramIndex = 0; paramIndex < function->getParamCount(); ++paramIndex)
    {
        const TVariable *param = function->getParam(paramIndex);
        const TType &paramType = param->getType();

        if (mOptions.validateStructUsage)
        {
            visitStructUsage(paramType, node->getLine());
        }

        if (mOptions.validateQualifiers)
        {
            TQualifier qualifier = paramType.getQualifier();
            if (qualifier != EvqParamIn && qualifier != EvqParamOut && qualifier != EvqParamInOut &&
                qualifier != EvqParamConst)
            {
                mDiagnostics->error(node->getLine(),
                                    "Found function prototype with an invalid qualifier "
                                    "<validateQualifiers>",
                                    param->name().data());
                mQualifiersFailed = true;
            }

            if (IsOpaqueType(paramType.getBasicType()) && qualifier != EvqParamIn)
            {
                mDiagnostics->error(
                    node->getLine(),
                    "Found function prototype with an invalid qualifier on opaque parameter "
                    "<validateQualifiers>",
                    param->name().data());
                mQualifiersFailed = true;
            }
        }

        if (mOptions.validatePrecision && IsPrecisionApplicableToType(paramType.getBasicType()) &&
            paramType.getPrecision() == EbpUndefined)
        {
            mDiagnostics->error(
                node->getLine(),
                "Found function parameter with undefined precision <validatePrecision>",
                param->name().data());
            mPrecisionFailed = true;
        }
    }
}

bool ValidateAST::visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node)
{
    visitNode(visit, node);

    if (mOptions.validateVariableReferences && visit == PreVisit)
    {
        const TFunction *function = node->getFunction();

        size_t paramCount = function->getParamCount();
        for (size_t paramIndex = 0; paramIndex < paramCount; ++paramIndex)
        {
            const TVariable *variable = function->getParam(paramIndex);

            if (isVariableDeclared(variable))
            {
                mDiagnostics->error(node->getLine(),
                                    "Found two declarations of the same function argument "
                                    "<validateVariableReferences>",
                                    variable->name().data());
                mVariableReferencesFailed = true;
                break;
            }

            mDeclaredVariables.back().insert(variable);
        }
    }

    return true;
}

bool ValidateAST::visitAggregate(Visit visit, TIntermAggregate *node)
{
    visitNode(visit, node);
    expectNonNullChildren(visit, node, 0);

    if (visit == PreVisit && mOptions.validateBuiltInOps)
    {
        visitBuiltInFunction(node, node->getFunction());
    }

    if (visit == PreVisit && mOptions.validateFunctionCall)
    {
        visitFunctionCall(node);
    }

    if (visit == PreVisit && mOptions.validateNoRawFunctionCalls)
    {
        if (node->getOp() == EOpCallInternalRawFunction)
        {
            mDiagnostics->error(node->getLine(),
                                "Found node calling a raw function (deprecated) "
                                "<validateNoRawFunctionCalls>",
                                node->getFunction()->name().data());
            mNoRawFunctionCallsFailed = true;
        }
    }

    if (visit == PreVisit && mOptions.validateNoQualifiersOnConstructors)
    {
        if (node->getOp() == EOpConstruct)
        {
            if (node->getType().isInvariant())
            {
                mDiagnostics->error(node->getLine(), "Found constructor node with invariant type",
                                    "<validateNoQualifiersOnConstructors>");
                mNoQualifiersOnConstructorsFailed = true;
            }
            if (node->getType().isPrecise())
            {
                mDiagnostics->error(node->getLine(), "Found constructor node with precise type",
                                    "<validateNoQualifiersOnConstructors>");
                mNoQualifiersOnConstructorsFailed = true;
            }
            if (node->getType().isInterpolant())
            {
                mDiagnostics->error(node->getLine(), "Found constructor node with interpolant type",
                                    "<validateNoQualifiersOnConstructors>");
                mNoQualifiersOnConstructorsFailed = true;
            }
            if (!node->getType().getMemoryQualifier().isEmpty())
            {
                mDiagnostics->error(node->getLine(),
                                    "Found constructor node whose type has a memory qualifier",
                                    "<validateNoQualifiersOnConstructors>");
                mNoQualifiersOnConstructorsFailed = true;
            }
            if (node->getType().getInterfaceBlock() != nullptr)
            {
                mDiagnostics->error(
                    node->getLine(),
                    "Found constructor node whose type references an interface block",
                    "<validateNoQualifiersOnConstructors>");
                mNoQualifiersOnConstructorsFailed = true;
            }
            if (!node->getType().getLayoutQualifier().isEmpty())
            {
                mDiagnostics->error(node->getLine(),
                                    "Found constructor node whose type has a layout qualifier",
                                    "<validateNoQualifiersOnConstructors>");
                mNoQualifiersOnConstructorsFailed = true;
            }
        }
    }

    return true;
}

bool ValidateAST::visitBlock(Visit visit, TIntermBlock *node)
{
    visitNode(visit, node);
    scope(visit);
    expectNonNullChildren(visit, node, 0);

    if (visit == PostVisit)
    {
        // If the parent is a block and mIsBranchVisitedInBlock is set, this is a nested block
        // without any condition (like if, loop or switch), so the rest of the parent block is also
        // dead code.  Otherwise the parent block can contain code after this.
        if (getParentNode() == nullptr || getParentNode()->getAsBlock() == nullptr)
        {
            mIsBranchVisitedInBlock = false;
        }
    }

    return true;
}

bool ValidateAST::visitGlobalQualifierDeclaration(Visit visit,
                                                  TIntermGlobalQualifierDeclaration *node)
{
    visitNode(visit, node);

    const TVariable *variable = &node->getSymbol()->variable();

    if (mOptions.validateVariableReferences && variableNeedsDeclaration(variable))
    {
        if (!isVariableDeclared(variable))
        {
            mDiagnostics->error(node->getLine(),
                                "Found reference to undeclared or inconsistently transformed "
                                "variable <validateVariableReferences>",
                                variable->name().data());
            mVariableReferencesFailed = true;
        }
    }
    return true;
}

bool ValidateAST::visitDeclaration(Visit visit, TIntermDeclaration *node)
{
    visitNode(visit, node);
    expectNonNullChildren(visit, node, 0);

    const TIntermSequence &sequence = *(node->getSequence());

    if (mOptions.validateMultiDeclarations && sequence.size() > 1)
    {
        TIntermSymbol *symbol = sequence[1]->getAsSymbolNode();
        if (symbol == nullptr)
        {
            TIntermBinary *init = sequence[1]->getAsBinaryNode();
            ASSERT(init && init->getOp() == EOpInitialize);
            symbol = init->getLeft()->getAsSymbolNode();
        }
        ASSERT(symbol);

        mDiagnostics->error(node->getLine(),
                            "Found multiple declarations where SeparateDeclarations should have "
                            "separated them <validateMultiDeclarations>",
                            symbol->variable().name().data());
        mMultiDeclarationsFailed = true;
    }

    if (visit == PreVisit)
    {
        bool validateStructUsage = mOptions.validateStructUsage;

        for (TIntermNode *instance : sequence)
        {
            TIntermSymbol *symbol = instance->getAsSymbolNode();
            if (symbol == nullptr)
            {
                TIntermBinary *init = instance->getAsBinaryNode();
                ASSERT(init && init->getOp() == EOpInitialize);
                symbol = init->getLeft()->getAsSymbolNode();
            }
            ASSERT(symbol);

            const TVariable *variable = &symbol->variable();
            const TType &type         = variable->getType();

            if (mOptions.validateVariableReferences)
            {
                if (isVariableDeclared(variable))
                {
                    mDiagnostics->error(
                        node->getLine(),
                        "Found two declarations of the same variable <validateVariableReferences>",
                        variable->name().data());
                    mVariableReferencesFailed = true;
                    break;
                }

                mDeclaredVariables.back().insert(variable);

                const TInterfaceBlock *interfaceBlock = variable->getType().getInterfaceBlock();

                if (variable->symbolType() == SymbolType::Empty && interfaceBlock != nullptr)
                {
                    // Nameless interface blocks can only be declared at the top level.  Their
                    // fields are matched by field index, and then verified to match by name.
                    // Conflict in names should have already generated a compile error.
                    ASSERT(mDeclaredVariables.size() == 1);
                    ASSERT(mNamelessInterfaceBlocks.count(interfaceBlock) == 0);

                    mNamelessInterfaceBlocks.insert(interfaceBlock);
                }
            }

            if (validateStructUsage)
            {
                // Only declare and/or validate the struct once.
                validateStructUsage = false;

                if (type.isStructSpecifier() || type.isInterfaceBlock())
                {
                    visitStructOrInterfaceBlockDeclaration(type, node->getLine());
                }
                else
                {
                    visitStructUsage(type, node->getLine());
                }
            }

            if (gl::IsBuiltInName(variable->name().data()))
            {
                visitBuiltInVariable(symbol);
            }

            if (mOptions.validatePrecision && (type.isStructSpecifier() || type.isInterfaceBlock()))
            {
                const TFieldListCollection *structOrBlock = type.getStruct();
                if (structOrBlock == nullptr)
                {
                    structOrBlock = type.getInterfaceBlock();
                }

                for (const TField *field : structOrBlock->fields())
                {
                    const TType *fieldType = field->type();
                    if (IsPrecisionApplicableToType(fieldType->getBasicType()) &&
                        fieldType->getPrecision() == EbpUndefined)
                    {
                        mDiagnostics->error(
                            node->getLine(),
                            "Found block field with undefined precision <validatePrecision>",
                            field->name().data());
                        mPrecisionFailed = true;
                    }
                }
            }
        }
    }

    return true;
}

bool ValidateAST::visitLoop(Visit visit, TIntermLoop *node)
{
    visitNode(visit, node);
    return true;
}

bool ValidateAST::visitBranch(Visit visit, TIntermBranch *node)
{
    visitNode(visit, node);

    if (visit == PreVisit && mOptions.validateOps)
    {
        const TOperator op = node->getFlowOp();
        if (!IsBranchOp(op))
        {
            mDiagnostics->error(node->getLine(),
                                "Found branch node with non-branch op <validateOps>",
                                GetOperatorString(op));
            mOpsFailed = true;
        }
    }
    if (visit == PostVisit)
    {
        mIsBranchVisitedInBlock = true;
    }

    return true;
}

void ValidateAST::visitPreprocessorDirective(TIntermPreprocessorDirective *node)
{
    visitNode(PreVisit, node);
}

bool ValidateAST::validateInternal()
{
    return !mSingleParentFailed && !mVariableReferencesFailed && !mOpsFailed &&
           !mBuiltInOpsFailed && !mFunctionCallFailed && !mNoRawFunctionCallsFailed &&
           !mNullNodesFailed && !mQualifiersFailed && !mPrecisionFailed && !mStructUsageFailed &&
           !mExpressionTypesFailed && !mMultiDeclarationsFailed && !mNoSwizzleOfSwizzleFailed &&
           !mNoQualifiersOnConstructorsFailed && !mNoStatementsAfterBranchFailed &&
           !mVariableNamingFailed;
}

bool ValidateAST::isInDeclaration() const
{
    auto *parent = getParentNode();
    return parent != nullptr && parent->getAsDeclarationNode() != nullptr;
}

}  // anonymous namespace

bool ValidateAST(TIntermNode *root, TDiagnostics *diagnostics, const ValidateASTOptions &options)
{
    // ValidateAST is called after transformations, so if |validateNoMoreTransformations| is set,
    // it's immediately an error.
    if (options.validateNoMoreTransformations)
    {
        diagnostics->error(kNoSourceLoc, "Unexpected transformation after AST post-processing",
                           "<validateNoMoreTransformations>");
        return false;
    }

    return ValidateAST::validate(root, diagnostics, options);
}

}  // namespace sh
