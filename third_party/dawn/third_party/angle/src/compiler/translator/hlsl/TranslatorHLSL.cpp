//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/hlsl/TranslatorHLSL.h"

#include "compiler/translator/hlsl/OutputHLSL.h"
#include "compiler/translator/tree_ops/RemoveDynamicIndexing.h"
#include "compiler/translator/tree_ops/RewriteTexelFetchOffset.h"
#include "compiler/translator/tree_ops/SimplifyLoopConditions.h"
#include "compiler/translator/tree_ops/SplitSequenceOperator.h"
#include "compiler/translator/tree_ops/hlsl/AddDefaultReturnStatements.h"
#include "compiler/translator/tree_ops/hlsl/AggregateAssignArraysInSSBOs.h"
#include "compiler/translator/tree_ops/hlsl/AggregateAssignStructsInSSBOs.h"
#include "compiler/translator/tree_ops/hlsl/ArrayReturnValueToOutParameter.h"
#include "compiler/translator/tree_ops/hlsl/BreakVariableAliasingInInnerLoops.h"
#include "compiler/translator/tree_ops/hlsl/ExpandIntegerPowExpressions.h"
#include "compiler/translator/tree_ops/hlsl/RecordUniformBlocksWithLargeArrayMember.h"
#include "compiler/translator/tree_ops/hlsl/RewriteAtomicFunctionExpressions.h"
#include "compiler/translator/tree_ops/hlsl/RewriteElseBlocks.h"
#include "compiler/translator/tree_ops/hlsl/RewriteExpressionsWithShaderStorageBlock.h"
#include "compiler/translator/tree_ops/hlsl/RewriteUnaryMinusOperatorInt.h"
#include "compiler/translator/tree_ops/hlsl/SeparateArrayConstructorStatements.h"
#include "compiler/translator/tree_ops/hlsl/SeparateArrayInitialization.h"
#include "compiler/translator/tree_ops/hlsl/SeparateExpressionsReturningArrays.h"
#include "compiler/translator/tree_ops/hlsl/UnfoldShortCircuitToIf.h"
#include "compiler/translator/tree_ops/hlsl/WrapSwitchStatementsInBlocks.h"
#include "compiler/translator/tree_util/IntermNodePatternMatcher.h"

namespace sh
{

TranslatorHLSL::TranslatorHLSL(sh::GLenum type, ShShaderSpec spec, ShShaderOutput output)
    : TCompiler(type, spec, output)
{}

bool TranslatorHLSL::translate(TIntermBlock *root,
                               const ShCompileOptions &compileOptions,
                               PerformanceDiagnostics *perfDiagnostics)
{
    // A few transformations leave the tree in an inconsistent state.  For example, when unfolding
    // the short-circuit in the following function:
    //
    //     mediump float f(float a) { return a > 0 ? 0.0 : 1.0; }
    //
    // a temp variable is created to hold the result of the expression.  Currently the precision of
    // the return value of the function is not propagated to its return expressions.  Additionally,
    // an expression such as
    //
    //     cond ? gl_NumWorkGroups.x : gl_NumWorkGroups.y
    //
    // does not have a precision as the built-in does not specify a precision.
    //
    // Precision is not applicable to HLSL so fixing these issues are deferred.
    mValidateASTOptions.validatePrecision = false;

    const ShBuiltInResources &resources = getResources();
    int numRenderTargets                = resources.EXT_draw_buffers ? resources.MaxDrawBuffers : 1;
    int maxDualSourceDrawBuffers =
        resources.EXT_blend_func_extended ? resources.MaxDualSourceDrawBuffers : 0;

    if (!sh::AddDefaultReturnStatements(this, root))
    {
        return false;
    }

    // Note that SimplifyLoopConditions needs to be run before any other AST transformations that
    // may need to generate new statements from loop conditions or loop expressions.
    // Note that SeparateDeclarations has already been run in TCompiler::compileTreeImpl().
    if (!SimplifyLoopConditions(
            this, root,
            IntermNodePatternMatcher::kExpressionReturningArray |
                IntermNodePatternMatcher::kUnfoldedShortCircuitExpression |
                IntermNodePatternMatcher::kDynamicIndexingOfVectorOrMatrixInLValue,
            &getSymbolTable()))
    {
        return false;
    }

    if (!SplitSequenceOperator(
            this, root,
            IntermNodePatternMatcher::kExpressionReturningArray |
                IntermNodePatternMatcher::kUnfoldedShortCircuitExpression |
                IntermNodePatternMatcher::kDynamicIndexingOfVectorOrMatrixInLValue,
            &getSymbolTable()))
    {
        return false;
    }

    // Note that SeparateDeclarations needs to be run before UnfoldShortCircuitToIf.
    if (!UnfoldShortCircuitToIf(this, root, &getSymbolTable()))
    {
        return false;
    }

    if (!SeparateArrayConstructorStatements(this, root))
    {
        return false;
    }

    if (getShaderVersion() >= 310)
    {
        // Do element-by-element assignments of arrays in SSBOs. This allows the D3D backend to use
        // RWByteAddressBuffer.Load() and .Store(), which only operate on values up to 16 bytes in
        // size. Note that this must be done before SeparateExpressionsReturningArrays.
        if (!sh::AggregateAssignArraysInSSBOs(this, root, &getSymbolTable()))
        {
            return false;
        }
        // Do field-by-field assignment of structs in SSBOs. This allows the D3D backend to use
        // RWByteAddressBuffer.Load() and .Store(), which only operate on values up to 16 bytes in
        // size.
        if (!sh::AggregateAssignStructsInSSBOs(this, root, &getSymbolTable()))
        {
            return false;
        }
    }

    if (!SeparateExpressionsReturningArrays(this, root, &getSymbolTable()))
    {
        return false;
    }

    // Note that SeparateDeclarations needs to be run before SeparateArrayInitialization.
    if (!SeparateArrayInitialization(this, root))
    {
        return false;
    }

    // HLSL doesn't support arrays as return values, we'll need to make functions that have an array
    // as a return value to use an out parameter to transfer the array data instead.
    if (!ArrayReturnValueToOutParameter(this, root, &getSymbolTable()))
    {
        return false;
    }

    if (!shouldRunLoopAndIndexingValidation(compileOptions))
    {
        // HLSL doesn't support dynamic indexing of vectors and matrices.
        if (!RemoveDynamicIndexingOfNonSSBOVectorOrMatrix(this, root, &getSymbolTable(),
                                                          perfDiagnostics))
        {
            return false;
        }
    }

    // Work around D3D9 bug that would manifest in vertex shaders with selection blocks which
    // use a vertex attribute as a condition, and some related computation in the else block.
    if (getOutputType() == SH_HLSL_3_0_OUTPUT && getShaderType() == GL_VERTEX_SHADER)
    {
        if (!sh::RewriteElseBlocks(this, root, &getSymbolTable()))
        {
            return false;
        }
    }

    // Work around an HLSL compiler frontend aliasing optimization bug.
    // TODO(cwallez) The date is 2016-08-25, Microsoft said the bug would be fixed
    // in the next release of d3dcompiler.dll, it would be nice to detect the DLL
    // version and only apply the workaround if it is too old.
    if (!sh::BreakVariableAliasingInInnerLoops(this, root))
    {
        return false;
    }

    // WrapSwitchStatementsInBlocks should be called after any AST transformations that might
    // introduce variable declarations inside the main scope of any switch statement. It cannot
    // result in no-op cases at the end of switch statements, because unreferenced variables
    // have already been pruned.
    if (!WrapSwitchStatementsInBlocks(this, root))
    {
        return false;
    }

    if (compileOptions.expandSelectHLSLIntegerPowExpressions)
    {
        if (!sh::ExpandIntegerPowExpressions(this, root, &getSymbolTable()))
        {
            return false;
        }
    }

    if (compileOptions.rewriteTexelFetchOffsetToTexelFetch)
    {
        if (!sh::RewriteTexelFetchOffset(this, root, getSymbolTable(), getShaderVersion()))
        {
            return false;
        }
    }

    if (compileOptions.rewriteIntegerUnaryMinusOperator && getShaderType() == GL_VERTEX_SHADER)
    {
        if (!sh::RewriteUnaryMinusOperatorInt(this, root))
        {
            return false;
        }
    }

    if (getShaderVersion() >= 310)
    {
        // Due to ssbo also can be used as the argument of atomic memory functions, we should put
        // RewriteExpressionsWithShaderStorageBlock before RewriteAtomicFunctionExpressions.
        if (!sh::RewriteExpressionsWithShaderStorageBlock(this, root, &getSymbolTable()))
        {
            return false;
        }
        if (!sh::RewriteAtomicFunctionExpressions(this, root, &getSymbolTable(),
                                                  getShaderVersion()))
        {
            return false;
        }
    }

    mUniformBlockOptimizedMap.clear();
    mSlowCompilingUniformBlockSet.clear();
    // In order to get the exact maximum of slots are available for shader resources, which would
    // been bound with StructuredBuffer, we only translate uniform block with a large array member
    // into StructuredBuffer when shader version is 300.
    if (getShaderVersion() == 300 && compileOptions.allowTranslateUniformBlockToStructuredBuffer)
    {
        if (!sh::RecordUniformBlocksWithLargeArrayMember(root, mUniformBlockOptimizedMap,
                                                         mSlowCompilingUniformBlockSet))
        {
            return false;
        }
    }

    sh::OutputHLSL outputHLSL(
        getShaderType(), getShaderSpec(), getShaderVersion(), getExtensionBehavior(),
        getSourcePath(), getOutputType(), numRenderTargets, maxDualSourceDrawBuffers, getUniforms(),
        compileOptions, getComputeShaderLocalSize(), &getSymbolTable(), perfDiagnostics,
        mUniformBlockOptimizedMap, mShaderStorageBlocks, getClipDistanceArraySize(),
        getCullDistanceArraySize(), isEarlyFragmentTestsSpecified());

    outputHLSL.output(root, getInfoSink().obj);

    mShaderStorageBlockRegisterMap      = outputHLSL.getShaderStorageBlockRegisterMap();
    mUniformBlockRegisterMap            = outputHLSL.getUniformBlockRegisterMap();
    mUniformBlockUseStructuredBufferMap = outputHLSL.getUniformBlockUseStructuredBufferMap();
    mUniformRegisterMap                 = outputHLSL.getUniformRegisterMap();
    mReadonlyImage2DRegisterIndex       = outputHLSL.getReadonlyImage2DRegisterIndex();
    mImage2DRegisterIndex               = outputHLSL.getImage2DRegisterIndex();
    mUsedImage2DFunctionNames           = outputHLSL.getUsedImage2DFunctionNames();

    return true;
}

bool TranslatorHLSL::shouldFlattenPragmaStdglInvariantAll()
{
    // Not necessary when translating to HLSL.
    return false;
}

bool TranslatorHLSL::hasShaderStorageBlock(const std::string &uniformBlockName) const
{
    return (mShaderStorageBlockRegisterMap.count(uniformBlockName) > 0);
}

unsigned int TranslatorHLSL::getShaderStorageBlockRegister(
    const std::string &shaderStorageBlockName) const
{
    ASSERT(hasShaderStorageBlock(shaderStorageBlockName));
    return mShaderStorageBlockRegisterMap.find(shaderStorageBlockName)->second;
}

bool TranslatorHLSL::hasUniformBlock(const std::string &uniformBlockName) const
{
    return (mUniformBlockRegisterMap.count(uniformBlockName) > 0);
}

unsigned int TranslatorHLSL::getUniformBlockRegister(const std::string &uniformBlockName) const
{
    ASSERT(hasUniformBlock(uniformBlockName));
    return mUniformBlockRegisterMap.find(uniformBlockName)->second;
}

const std::map<std::string, unsigned int> *TranslatorHLSL::getUniformRegisterMap() const
{
    return &mUniformRegisterMap;
}

const std::set<std::string> *TranslatorHLSL::getSlowCompilingUniformBlockSet() const
{
    return &mSlowCompilingUniformBlockSet;
}

unsigned int TranslatorHLSL::getReadonlyImage2DRegisterIndex() const
{
    return mReadonlyImage2DRegisterIndex;
}

unsigned int TranslatorHLSL::getImage2DRegisterIndex() const
{
    return mImage2DRegisterIndex;
}

const std::set<std::string> *TranslatorHLSL::getUsedImage2DFunctionNames() const
{
    return &mUsedImage2DFunctionNames;
}

bool TranslatorHLSL::shouldUniformBlockUseStructuredBuffer(
    const std::string &uniformBlockName) const
{
    auto uniformBlockIter = mUniformBlockUseStructuredBufferMap.find(uniformBlockName);
    return uniformBlockIter != mUniformBlockUseStructuredBufferMap.end() &&
           uniformBlockIter->second;
}

}  // namespace sh
