//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// The ValidateClipCullDistance function:
// * gathers clip/cull distance usages
// * checks if the sum of array sizes for gl_ClipDistance and
//   gl_CullDistance exceeds gl_MaxCombinedClipAndCullDistances
// * checks if length() operator is used correctly
// * adds an explicit clip/cull distance declaration
//

#include "ValidateClipCullDistance.h"

#include "compiler/translator/Diagnostics.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

void error(const TIntermSymbol &symbol, const char *reason, TDiagnostics *diagnostics)
{
    diagnostics->error(symbol.getLine(), reason, symbol.getName().data());
}

class ValidateClipCullDistanceTraverser : public TIntermTraverser
{
  public:
    ValidateClipCullDistanceTraverser();
    void validate(TDiagnostics *diagnostics,
                  const unsigned int maxCombinedClipAndCullDistances,
                  uint8_t *clipDistanceSizeOut,
                  uint8_t *cullDistanceSizeOut,
                  bool *clipDistanceRedeclaredOut,
                  bool *cullDistanceRedeclaredOut,
                  bool *clipDistanceUsedOut);

  private:
    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override;
    bool visitBinary(Visit visit, TIntermBinary *node) override;

    uint8_t mClipDistanceSize;
    uint8_t mCullDistanceSize;

    int8_t mMaxClipDistanceIndex;
    int8_t mMaxCullDistanceIndex;

    bool mHasNonConstClipDistanceIndex;
    bool mHasNonConstCullDistanceIndex;

    const TIntermSymbol *mClipDistance;
    const TIntermSymbol *mCullDistance;
};

ValidateClipCullDistanceTraverser::ValidateClipCullDistanceTraverser()
    : TIntermTraverser(true, false, false),
      mClipDistanceSize(0),
      mCullDistanceSize(0),
      mMaxClipDistanceIndex(-1),
      mMaxCullDistanceIndex(-1),
      mHasNonConstClipDistanceIndex(false),
      mHasNonConstCullDistanceIndex(false),
      mClipDistance(nullptr),
      mCullDistance(nullptr)
{}

bool ValidateClipCullDistanceTraverser::visitDeclaration(Visit visit, TIntermDeclaration *node)
{
    const TIntermSequence &sequence = *(node->getSequence());

    if (sequence.size() != 1)
    {
        return true;
    }

    const TIntermSymbol *symbol = sequence.front()->getAsSymbolNode();
    if (symbol == nullptr)
    {
        return true;
    }

    if (symbol->getName() == "gl_ClipDistance")
    {
        mClipDistanceSize = static_cast<uint8_t>(symbol->getOutermostArraySize());
        mClipDistance     = symbol;
    }
    else if (symbol->getName() == "gl_CullDistance")
    {
        mCullDistanceSize = static_cast<uint8_t>(symbol->getOutermostArraySize());
        mCullDistance     = symbol;
    }

    return true;
}

bool ValidateClipCullDistanceTraverser::visitBinary(Visit visit, TIntermBinary *node)
{
    TOperator op = node->getOp();
    if (op != EOpIndexDirect && op != EOpIndexIndirect)
    {
        return true;
    }

    TIntermSymbol *left = node->getLeft()->getAsSymbolNode();
    if (!left)
    {
        return true;
    }

    ImmutableString varName(left->getName());
    if (varName != "gl_ClipDistance" && varName != "gl_CullDistance")
    {
        return true;
    }

    const TConstantUnion *constIdx = node->getRight()->getConstantValue();
    if (constIdx)
    {
        int idx = 0;
        switch (constIdx->getType())
        {
            case EbtInt:
                idx = constIdx->getIConst();
                break;
            case EbtUInt:
                idx = constIdx->getUConst();
                break;
            default:
                UNREACHABLE();
                break;
        }

        if (varName == "gl_ClipDistance")
        {
            if (idx > mMaxClipDistanceIndex)
            {
                mMaxClipDistanceIndex = static_cast<int8_t>(idx);
                if (!mClipDistance)
                {
                    mClipDistance = left;
                }
            }
        }
        else
        {
            ASSERT(varName == "gl_CullDistance");
            if (idx > mMaxCullDistanceIndex)
            {
                mMaxCullDistanceIndex = static_cast<int8_t>(idx);
                if (!mCullDistance)
                {
                    mCullDistance = left;
                }
            }
        }
    }
    else
    {
        if (varName == "gl_ClipDistance")
        {
            mHasNonConstClipDistanceIndex = true;
            if (!mClipDistance)
            {
                mClipDistance = left;
            }
        }
        else
        {
            ASSERT(varName == "gl_CullDistance");
            mHasNonConstCullDistanceIndex = true;
            if (!mCullDistance)
            {
                mCullDistance = left;
            }
        }
    }

    return true;
}

void ValidateClipCullDistanceTraverser::validate(TDiagnostics *diagnostics,
                                                 const unsigned int maxCombinedClipAndCullDistances,
                                                 uint8_t *clipDistanceSizeOut,
                                                 uint8_t *cullDistanceSizeOut,
                                                 bool *clipDistanceRedeclaredOut,
                                                 bool *cullDistanceRedeclaredOut,
                                                 bool *clipDistanceUsedOut)
{
    ASSERT(diagnostics);

    if (mClipDistanceSize == 0 && mHasNonConstClipDistanceIndex)
    {
        error(*mClipDistance,
              "The array must be sized by the shader either redeclaring it with a size or "
              "indexing it only with constant integral expressions",
              diagnostics);
    }

    if (mCullDistanceSize == 0 && mHasNonConstCullDistanceIndex)
    {
        error(*mCullDistance,
              "The array must be sized by the shader either redeclaring it with a size or "
              "indexing it only with constant integral expressions",
              diagnostics);
    }

    unsigned int enabledClipDistances =
        (mClipDistanceSize > 0 ? mClipDistanceSize
                               : (mClipDistance ? mMaxClipDistanceIndex + 1 : 0));
    unsigned int enabledCullDistances =
        (mCullDistanceSize > 0 ? mCullDistanceSize
                               : (mCullDistance ? mMaxCullDistanceIndex + 1 : 0));
    unsigned int combinedClipAndCullDistances =
        (enabledClipDistances > 0 && enabledCullDistances > 0
             ? enabledClipDistances + enabledCullDistances
             : 0);

    // When cull distances are not supported, i.e., when GL_ANGLE_clip_cull_distance is
    // exposed but GL_EXT_clip_cull_distance is not exposed, the combined limit is 0.
    if (enabledCullDistances > 0 && maxCombinedClipAndCullDistances == 0)
    {
        error(*mCullDistance, "Cull distance functionality is not available", diagnostics);
    }

    if (combinedClipAndCullDistances > maxCombinedClipAndCullDistances)
    {
        const TIntermSymbol *greaterSymbol =
            (enabledClipDistances >= enabledCullDistances ? mClipDistance : mCullDistance);

        std::stringstream strstr = sh::InitializeStream<std::stringstream>();
        strstr << "The sum of 'gl_ClipDistance' and 'gl_CullDistance' size is greater than "
                  "gl_MaxCombinedClipAndCullDistances ("
               << combinedClipAndCullDistances << " > " << maxCombinedClipAndCullDistances << ")";
        error(*greaterSymbol, strstr.str().c_str(), diagnostics);
    }

    // Update the compiler state
    *clipDistanceSizeOut = mClipDistanceSize ? mClipDistanceSize : (mMaxClipDistanceIndex + 1);
    *cullDistanceSizeOut = mCullDistanceSize ? mCullDistanceSize : (mMaxCullDistanceIndex + 1);
    *clipDistanceRedeclaredOut = mClipDistanceSize != 0;
    *cullDistanceRedeclaredOut = mCullDistanceSize != 0;
    *clipDistanceUsedOut       = (mMaxClipDistanceIndex != -1) || mHasNonConstClipDistanceIndex;
}

class ValidateClipCullDistanceLengthTraverser : public TIntermTraverser
{
  public:
    ValidateClipCullDistanceLengthTraverser(TDiagnostics *diagnostics,
                                            uint8_t clipDistanceSized,
                                            uint8_t cullDistanceSized);

  private:
    bool visitUnary(Visit visit, TIntermUnary *node) override;

    TDiagnostics *mDiagnostics;
    const bool mClipDistanceSized;
    const bool mCullDistanceSized;
};

ValidateClipCullDistanceLengthTraverser::ValidateClipCullDistanceLengthTraverser(
    TDiagnostics *diagnostics,
    uint8_t clipDistanceSize,
    uint8_t cullDistanceSize)
    : TIntermTraverser(true, false, false),
      mDiagnostics(diagnostics),
      mClipDistanceSized(clipDistanceSize > 0),
      mCullDistanceSized(cullDistanceSize > 0)
{}

bool ValidateClipCullDistanceLengthTraverser::visitUnary(Visit visit, TIntermUnary *node)
{
    if (node->getOp() == EOpArrayLength)
    {
        TIntermTyped *operand = node->getOperand();
        if ((operand->getQualifier() == EvqClipDistance && !mClipDistanceSized) ||
            (operand->getQualifier() == EvqCullDistance && !mCullDistanceSized))
        {
            error(*operand->getAsSymbolNode(),
                  "The length() method cannot be called on an array that is not "
                  "runtime sized and also has not yet been explicitly sized",
                  mDiagnostics);
        }
    }
    return true;
}

bool ReplaceAndDeclareVariable(TCompiler *compiler,
                               TIntermBlock *root,
                               const ImmutableString &name,
                               unsigned int size)
{
    const TVariable *var = static_cast<const TVariable *>(
        compiler->getSymbolTable().findBuiltIn(name, compiler->getShaderVersion()));
    ASSERT(var != nullptr);

    if (size != var->getType().getOutermostArraySize())
    {
        TType *resizedType = new TType(var->getType());
        resizedType->setArraySize(0, size);
        TVariable *resizedVar =
            new TVariable(&compiler->getSymbolTable(), name, resizedType, SymbolType::BuiltIn);
        if (!ReplaceVariable(compiler, root, var, resizedVar))
        {
            return false;
        }
        var = resizedVar;
    }

    TIntermDeclaration *globalDecl = new TIntermDeclaration();
    globalDecl->appendDeclarator(new TIntermSymbol(var));
    root->insertStatement(0, globalDecl);

    return true;
}

}  // anonymous namespace

bool ValidateClipCullDistance(TCompiler *compiler,
                              TIntermBlock *root,
                              TDiagnostics *diagnostics,
                              const unsigned int maxCombinedClipAndCullDistances,
                              uint8_t *clipDistanceSizeOut,
                              uint8_t *cullDistanceSizeOut,
                              bool *clipDistanceUsedOut)
{
    ValidateClipCullDistanceTraverser varyingValidator;
    root->traverse(&varyingValidator);
    int numErrorsBefore = diagnostics->numErrors();
    bool clipDistanceRedeclared;
    bool cullDistanceRedeclared;
    varyingValidator.validate(diagnostics, maxCombinedClipAndCullDistances, clipDistanceSizeOut,
                              cullDistanceSizeOut, &clipDistanceRedeclared, &cullDistanceRedeclared,
                              clipDistanceUsedOut);

    ValidateClipCullDistanceLengthTraverser lengthValidator(diagnostics, *clipDistanceSizeOut,
                                                            *cullDistanceSizeOut);
    root->traverse(&lengthValidator);
    if (diagnostics->numErrors() != numErrorsBefore)
    {
        return false;
    }

    // If the clip/cull distance variables are not explicitly redeclared in the incoming shader,
    // redeclare them to ensure that various pruning passes will not cause inconsistent AST state.
    if (*clipDistanceSizeOut > 0 && !clipDistanceRedeclared &&
        !ReplaceAndDeclareVariable(compiler, root, ImmutableString("gl_ClipDistance"),
                                   *clipDistanceSizeOut))
    {

        return false;
    }
    if (*cullDistanceSizeOut > 0 && !cullDistanceRedeclared &&
        !ReplaceAndDeclareVariable(compiler, root, ImmutableString("gl_CullDistance"),
                                   *cullDistanceSizeOut))
    {
        return false;
    }

    return true;
}

}  // namespace sh
