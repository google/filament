//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/msl/TranslatorMSL.h"

#include "angle_gl.h"
#include "common/utilities.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/Name.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/msl/AstHelpers.h"
#include "compiler/translator/msl/DriverUniformMetal.h"
#include "compiler/translator/msl/EmitMetal.h"
#include "compiler/translator/msl/RewritePipelines.h"
#include "compiler/translator/msl/SymbolEnv.h"
#include "compiler/translator/msl/ToposortStructs.h"
#include "compiler/translator/msl/UtilsMSL.h"
#include "compiler/translator/tree_ops/InitializeVariables.h"
#include "compiler/translator/tree_ops/MonomorphizeUnsupportedFunctions.h"
#include "compiler/translator/tree_ops/PreTransformTextureCubeGradDerivatives.h"
#include "compiler/translator/tree_ops/RemoveAtomicCounterBuiltins.h"
#include "compiler/translator/tree_ops/RemoveInactiveInterfaceVariables.h"
#include "compiler/translator/tree_ops/RewriteArrayOfArrayOfOpaqueUniforms.h"
#include "compiler/translator/tree_ops/RewriteAtomicCounters.h"
#include "compiler/translator/tree_ops/RewriteDfdy.h"
#include "compiler/translator/tree_ops/RewriteStructSamplers.h"
#include "compiler/translator/tree_ops/SeparateStructFromUniformDeclarations.h"
#include "compiler/translator/tree_ops/msl/AddExplicitTypeCasts.h"
#include "compiler/translator/tree_ops/msl/ConvertUnsupportedConstructorsToFunctionCalls.h"
#include "compiler/translator/tree_ops/msl/FixTypeConstructors.h"
#include "compiler/translator/tree_ops/msl/HoistConstants.h"
#include "compiler/translator/tree_ops/msl/IntroduceVertexIndexID.h"
#include "compiler/translator/tree_ops/msl/ReduceInterfaceBlocks.h"
#include "compiler/translator/tree_ops/msl/RewriteCaseDeclarations.h"
#include "compiler/translator/tree_ops/msl/RewriteInterpolants.h"
#include "compiler/translator/tree_ops/msl/RewriteOutArgs.h"
#include "compiler/translator/tree_ops/msl/RewriteUnaddressableReferences.h"
#include "compiler/translator/tree_ops/msl/SeparateCompoundExpressions.h"
#include "compiler/translator/tree_ops/msl/WrapMain.h"
#include "compiler/translator/tree_util/BuiltIn.h"
#include "compiler/translator/tree_util/DriverUniform.h"
#include "compiler/translator/tree_util/FindFunction.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/FindSymbolNode.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/ReplaceClipCullDistanceVariable.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"
#include "compiler/translator/tree_util/RunAtTheBeginningOfShader.h"
#include "compiler/translator/tree_util/RunAtTheEndOfShader.h"
#include "compiler/translator/tree_util/SpecializationConstant.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

constexpr Name kFlippedPointCoordName("flippedPointCoord", SymbolType::AngleInternal);
constexpr Name kFlippedFragCoordName("flippedFragCoord", SymbolType::AngleInternal);

class DeclareStructTypesTraverser : public TIntermTraverser
{
  public:
    explicit DeclareStructTypesTraverser(TOutputMSL *outputMSL)
        : TIntermTraverser(true, false, false), mOutputMSL(outputMSL)
    {}

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        ASSERT(visit == PreVisit);
        if (!mInGlobalScope)
        {
            return false;
        }

        const TIntermSequence &sequence = *(node->getSequence());
        TIntermTyped *declarator        = sequence.front()->getAsTyped();
        const TType &type               = declarator->getType();

        if (type.isStructSpecifier())
        {
            const TStructure *structure = type.getStruct();

            // Embedded structs should be parsed away by now.
            ASSERT(structure->symbolType() != SymbolType::Empty);
            // outputMSL->writeStructType(structure);

            TIntermSymbol *symbolNode = declarator->getAsSymbolNode();
            if (symbolNode && symbolNode->variable().symbolType() == SymbolType::Empty)
            {
                // Remove the struct specifier declaration from the tree so it isn't parsed again.
                TIntermSequence emptyReplacement;
                mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), node,
                                                std::move(emptyReplacement));
            }
        }
        // TODO: REMOVE, used to remove 'unsued' warning
        mOutputMSL = nullptr;

        return false;
    }

  private:
    TOutputMSL *mOutputMSL;
};

class DeclareDefaultUniformsTraverser : public TIntermTraverser
{
  public:
    DeclareDefaultUniformsTraverser(TInfoSinkBase *sink,
                                    ShHashFunction64 hashFunction,
                                    NameMap *nameMap)
        : TIntermTraverser(true, true, true),
          mSink(sink),
          mHashFunction(hashFunction),
          mNameMap(nameMap),
          mInDefaultUniform(false)
    {}

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        const TIntermSequence &sequence = *(node->getSequence());

        // TODO(jmadill): Compound declarations.
        ASSERT(sequence.size() == 1);

        TIntermTyped *variable = sequence.front()->getAsTyped();
        const TType &type      = variable->getType();
        bool isUniform         = type.getQualifier() == EvqUniform && !type.isInterfaceBlock() &&
                         !IsOpaqueType(type.getBasicType());

        if (visit == PreVisit)
        {
            if (isUniform)
            {
                (*mSink) << "    " << GetTypeName(type, mHashFunction, mNameMap) << " ";
                mInDefaultUniform = true;
            }
        }
        else if (visit == InVisit)
        {
            mInDefaultUniform = isUniform;
        }
        else if (visit == PostVisit)
        {
            if (isUniform)
            {
                (*mSink) << ";\n";

                // Remove the uniform declaration from the tree so it isn't parsed again.
                TIntermSequence emptyReplacement;
                mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), node,
                                                std::move(emptyReplacement));
            }

            mInDefaultUniform = false;
        }
        return true;
    }

    void visitSymbol(TIntermSymbol *symbol) override
    {
        if (mInDefaultUniform)
        {
            const ImmutableString &name = symbol->variable().name();
            ASSERT(!gl::IsBuiltInName(name.data()));
            (*mSink) << HashName(&symbol->variable(), mHashFunction, mNameMap)
                     << ArrayString(symbol->getType());
        }
    }

  private:
    TInfoSinkBase *mSink;
    ShHashFunction64 mHashFunction;
    NameMap *mNameMap;
    bool mInDefaultUniform;
};

// Declares a new variable to replace gl_DepthRange, its values are fed from a driver uniform.
[[nodiscard]] bool ReplaceGLDepthRangeWithDriverUniform(TCompiler *compiler,
                                                        TIntermBlock *root,
                                                        const DriverUniformMetal *driverUniforms,
                                                        TSymbolTable *symbolTable)
{
    // Create a symbol reference to "gl_DepthRange"
    const TVariable *depthRangeVar = static_cast<const TVariable *>(
        symbolTable->findBuiltIn(ImmutableString("gl_DepthRange"), 0));

    // ANGLEUniforms.depthRange
    TIntermTyped *angleEmulatedDepthRangeRef = driverUniforms->getDepthRange();

    // Use this variable instead of gl_DepthRange everywhere.
    return ReplaceVariableWithTyped(compiler, root, depthRangeVar, angleEmulatedDepthRangeRef);
}

TIntermSequence *GetMainSequence(TIntermBlock *root)
{
    TIntermFunctionDefinition *main = FindMain(root);
    return main->getBody()->getSequence();
}

// Replaces a builtin variable with a version that is rotated and corrects the X and Y coordinates.
[[nodiscard]] bool FlipBuiltinVariable(TCompiler *compiler,
                                       TIntermBlock *root,
                                       TIntermSequence *insertSequence,
                                       TIntermTyped *flipXY,
                                       TSymbolTable *symbolTable,
                                       const TVariable *builtin,
                                       const Name &flippedVariableName,
                                       TIntermTyped *pivot)
{
    // Create a symbol reference to 'builtin'.
    TIntermSymbol *builtinRef = new TIntermSymbol(builtin);

    // Create a swizzle to "builtin.xy"
    TVector<int> swizzleOffsetXY = {0, 1};
    TIntermSwizzle *builtinXY    = new TIntermSwizzle(builtinRef, swizzleOffsetXY);

    // Create a symbol reference to our new variable that will hold the modified builtin.
    const TType *type =
        StaticType::GetForVec<EbtFloat, EbpHigh>(EvqGlobal, builtin->getType().getNominalSize());
    TVariable *replacementVar =
        new TVariable(symbolTable, flippedVariableName.rawName(), type, SymbolType::AngleInternal);
    DeclareGlobalVariable(root, replacementVar);
    TIntermSymbol *flippedBuiltinRef = new TIntermSymbol(replacementVar);

    // Use this new variable instead of 'builtin' everywhere.
    if (!ReplaceVariable(compiler, root, builtin, replacementVar))
    {
        return false;
    }

    // Create the expression "(builtin.xy - pivot) * flipXY + pivot
    TIntermBinary *removePivot = new TIntermBinary(EOpSub, builtinXY, pivot);
    TIntermBinary *inverseXY   = new TIntermBinary(EOpMul, removePivot, flipXY);
    TIntermBinary *plusPivot   = new TIntermBinary(EOpAdd, inverseXY, pivot->deepCopy());

    // Create the corrected variable and copy the value of the original builtin.
    TIntermSequence sequence;
    sequence.push_back(builtinRef->deepCopy());
    TIntermAggregate *aggregate =
        TIntermAggregate::CreateConstructor(builtin->getType(), &sequence);
    TIntermBinary *assignment = new TIntermBinary(EOpAssign, flippedBuiltinRef, aggregate);

    // Create an assignment to the replaced variable's .xy.
    TIntermSwizzle *correctedXY =
        new TIntermSwizzle(flippedBuiltinRef->deepCopy(), swizzleOffsetXY);
    TIntermBinary *assignToY = new TIntermBinary(EOpAssign, correctedXY, plusPivot);

    // Add this assigment at the beginning of the main function
    insertSequence->insert(insertSequence->begin(), assignToY);
    insertSequence->insert(insertSequence->begin(), assignment);

    return compiler->validateAST(root);
}

[[nodiscard]] bool InsertFragCoordCorrection(TCompiler *compiler,
                                             const ShCompileOptions &compileOptions,
                                             TIntermBlock *root,
                                             TIntermSequence *insertSequence,
                                             TSymbolTable *symbolTable,
                                             const DriverUniformMetal *driverUniforms)
{
    TIntermTyped *flipXY = driverUniforms->getFlipXY(symbolTable, DriverUniformFlip::Fragment);
    TIntermTyped *pivot  = driverUniforms->getHalfRenderArea();

    const TVariable *fragCoord = static_cast<const TVariable *>(
        symbolTable->findBuiltIn(ImmutableString("gl_FragCoord"), compiler->getShaderVersion()));
    return FlipBuiltinVariable(compiler, root, insertSequence, flipXY, symbolTable, fragCoord,
                               kFlippedFragCoordName, pivot);
}

void DeclareRightBeforeMain(TIntermBlock &root, const TVariable &var)
{
    root.insertChildNodes(FindMainIndex(&root), {new TIntermDeclaration{&var}});
}

void AddFragColorDeclaration(TIntermBlock &root, TSymbolTable &symbolTable, const TVariable &var)
{
    root.insertChildNodes(FindMainIndex(&root), TIntermSequence{new TIntermDeclaration{&var}});
}

void AddBuiltInDeclaration(TIntermBlock &root, TSymbolTable &symbolTable, const TVariable &builtIn)
{
    // Check if the variable has been already declared.
    const TIntermSymbol *builtInSymbol = new TIntermSymbol(&builtIn);
    const TIntermSymbol *foundSymbol   = FindSymbolNode(&root, builtIn.name());
    if (foundSymbol && foundSymbol->uniqueId() != builtInSymbol->uniqueId())
    {
        return;
    }
    root.insertChildNodes(FindMainIndex(&root), TIntermSequence{new TIntermDeclaration{&builtIn}});
}

void AddFragDepthEXTDeclaration(TCompiler &compiler, TIntermBlock &root, TSymbolTable &symbolTable)
{
    const TIntermSymbol *glFragDepthExt = FindSymbolNode(&root, ImmutableString("gl_FragDepthEXT"));
    ASSERT(glFragDepthExt);
    // Replace gl_FragData with our globally defined fragdata.
    if (!ReplaceVariable(&compiler, &root, &(glFragDepthExt->variable()),
                         BuiltInVariable::gl_FragDepth()))
    {
        return;
    }
    AddBuiltInDeclaration(root, symbolTable, *BuiltInVariable::gl_FragDepth());
}

[[nodiscard]] bool AddNumSamplesDeclaration(TCompiler &compiler,
                                            TIntermBlock &root,
                                            TSymbolTable &symbolTable)
{
    const TVariable *glNumSamples = BuiltInVariable::gl_NumSamples();
    DeclareRightBeforeMain(root, *glNumSamples);

    // gl_NumSamples = metal::get_num_samples();
    TIntermBinary *assignment = new TIntermBinary(
        TOperator::EOpAssign, new TIntermSymbol(glNumSamples),
        CreateBuiltInFunctionCallNode("numSamples", {}, symbolTable, kESSLInternalBackendBuiltIns));
    return RunAtTheBeginningOfShader(&compiler, &root, assignment);
}

[[nodiscard]] bool AddSamplePositionDeclaration(TCompiler &compiler,
                                                TIntermBlock &root,
                                                TSymbolTable &symbolTable,
                                                const DriverUniformMetal *driverUniforms)
{
    const TVariable *glSamplePosition = BuiltInVariable::gl_SamplePosition();
    DeclareRightBeforeMain(root, *glSamplePosition);

    // When rendering to a default FBO, gl_SamplePosition should
    // be Y-flipped to match the actual sample location
    // gl_SamplePosition = metal::get_sample_position(uint(gl_SampleID));
    // gl_SamplePosition -= 0.5;
    // gl_SamplePosition *= flipXY;
    // gl_SamplePosition += 0.5;
    TIntermBlock *block = new TIntermBlock;
    block->appendStatement(new TIntermBinary(
        TOperator::EOpAssign, new TIntermSymbol(glSamplePosition),
        CreateBuiltInFunctionCallNode("samplePosition",
                                      {TIntermAggregate::CreateConstructor(
                                          *StaticType::GetBasic<EbtUInt, EbpHigh>(),
                                          {new TIntermSymbol(BuiltInVariable::gl_SampleID())})},
                                      symbolTable, kESSLInternalBackendBuiltIns)));
    block->appendStatement(new TIntermBinary(TOperator::EOpSubAssign,
                                             new TIntermSymbol(glSamplePosition),
                                             CreateFloatNode(0.5f, EbpHigh)));
    block->appendStatement(
        new TIntermBinary(EOpMulAssign, new TIntermSymbol(glSamplePosition),
                          driverUniforms->getFlipXY(&symbolTable, DriverUniformFlip::Fragment)));
    block->appendStatement(new TIntermBinary(TOperator::EOpAddAssign,
                                             new TIntermSymbol(glSamplePosition),
                                             CreateFloatNode(0.5f, EbpHigh)));
    return RunAtTheBeginningOfShader(&compiler, &root, block);
}

[[nodiscard]] bool AddSampleMaskInDeclaration(TCompiler &compiler,
                                              TIntermBlock &root,
                                              TSymbolTable &symbolTable,
                                              const DriverUniformMetal *driverUniforms,
                                              bool perSampleShading)
{
    // in highp int gl_SampleMaskIn[1]
    const TVariable *glSampleMaskIn = static_cast<const TVariable *>(
        symbolTable.findBuiltIn(ImmutableString("gl_SampleMaskIn"), compiler.getShaderVersion()));
    DeclareRightBeforeMain(root, *glSampleMaskIn);

    // Reference to gl_SampleMaskIn[0]
    TIntermBinary *glSampleMaskIn0 =
        new TIntermBinary(EOpIndexDirect, new TIntermSymbol(glSampleMaskIn), CreateIndexNode(0));

    // When per-sample shading is active due to the use of a fragment input qualified
    // by sample or due to the use of the gl_SampleID or gl_SamplePosition variables,
    // only the bit for the current sample is set in gl_SampleMaskIn.
    TIntermBlock *block = new TIntermBlock;
    if (perSampleShading)
    {
        // gl_SampleMaskIn[0] = 1 << gl_SampleID;
        block->appendStatement(new TIntermBinary(
            EOpAssign, glSampleMaskIn0,
            new TIntermBinary(EOpBitShiftLeft, CreateUIntNode(1),
                              new TIntermSymbol(BuiltInVariable::gl_SampleID()))));
    }
    else
    {
        // uint32_t ANGLE_metal_SampleMaskIn [[sample_mask]]
        TVariable *angleSampleMaskIn = new TVariable(
            &symbolTable, ImmutableString("metal_SampleMaskIn"),
            new TType(EbtUInt, EbpHigh, EvqSampleMaskIn, 1), SymbolType::AngleInternal);
        DeclareRightBeforeMain(root, *angleSampleMaskIn);

        // gl_SampleMaskIn[0] = ANGLE_metal_SampleMaskIn;
        block->appendStatement(
            new TIntermBinary(EOpAssign, glSampleMaskIn0, new TIntermSymbol(angleSampleMaskIn)));
    }

    // Bits in the sample mask corresponding to covered samples
    // that will be unset due to SAMPLE_COVERAGE or SAMPLE_MASK
    // will not be set (section 4.1.3).
    // if (ANGLEMultisampledRendering)
    // {
    //      gl_SampleMaskIn[0] &= ANGLE_angleUniforms.coverageMask;
    // }
    TIntermBlock *coverageBlock = new TIntermBlock;
    coverageBlock->appendStatement(new TIntermBinary(
        EOpBitwiseAndAssign, glSampleMaskIn0->deepCopy(), driverUniforms->getCoverageMaskField()));

    TVariable *sampleMaskEnabledVar = new TVariable(
        &symbolTable, sh::ImmutableString(mtl::kMultisampledRenderingConstName),
        StaticType::Get<EbtBool, EbpUndefined, EvqSpecConst, 1, 1>(), SymbolType::AngleInternal);
    block->appendStatement(
        new TIntermIfElse(new TIntermSymbol(sampleMaskEnabledVar), coverageBlock, nullptr));

    return RunAtTheBeginningOfShader(&compiler, &root, block);
}

[[nodiscard]] bool AddSampleMaskDeclaration(TCompiler &compiler,
                                            TIntermBlock &root,
                                            TSymbolTable &symbolTable,
                                            const DriverUniformMetal *driverUniforms,
                                            bool includeEmulateAlphaToCoverage,
                                            bool usesSampleMask)
{
    // uint32_t ANGLE_metal_SampleMask [[sample_mask]]
    TVariable *angleSampleMask =
        new TVariable(&symbolTable, ImmutableString("metal_SampleMask"),
                      new TType(EbtUInt, EbpHigh, EvqSampleMask, 1), SymbolType::AngleInternal);
    DeclareRightBeforeMain(root, *angleSampleMask);

    // Write all-enabled sample mask even for single-sampled rendering
    // when the shader uses derivatives to workaround a driver bug.
    if (compiler.usesDerivatives())
    {
        TIntermBlock *helperAssignBlock = new TIntermBlock;
        helperAssignBlock->appendStatement(new TIntermBinary(
            EOpAssign, new TIntermSymbol(angleSampleMask), CreateUIntNode(0xFFFFFFFFu)));

        TVariable *writeHelperSampleMaskVar =
            new TVariable(&symbolTable, sh::ImmutableString(mtl::kWriteHelperSampleMaskConstName),
                          StaticType::Get<EbtBool, EbpUndefined, EvqSpecConst, 1, 1>(),
                          SymbolType::AngleInternal);

        if (!RunAtTheBeginningOfShader(
                &compiler, &root,
                new TIntermIfElse(new TIntermSymbol(writeHelperSampleMaskVar), helperAssignBlock,
                                  nullptr)))
        {
            return false;
        }
    }

    // ANGLE_metal_SampleMask = ANGLE_angleUniforms.coverageMask;
    TIntermBlock *block = new TIntermBlock;
    block->appendStatement(new TIntermBinary(EOpAssign, new TIntermSymbol(angleSampleMask),
                                             driverUniforms->getCoverageMaskField()));
    if (usesSampleMask)
    {
        // out highp int gl_SampleMask[1];
        const TVariable *glSampleMask = static_cast<const TVariable *>(
            symbolTable.findBuiltIn(ImmutableString("gl_SampleMask"), compiler.getShaderVersion()));
        DeclareRightBeforeMain(root, *glSampleMask);

        // ANGLE_metal_SampleMask &= gl_SampleMask[0];
        TIntermBinary *glSampleMask0 =
            new TIntermBinary(EOpIndexDirect, new TIntermSymbol(glSampleMask), CreateIndexNode(0));
        block->appendStatement(new TIntermBinary(
            EOpBitwiseAndAssign, new TIntermSymbol(angleSampleMask), glSampleMask0));
    }

    if (includeEmulateAlphaToCoverage)
    {
        // Some Metal drivers ignore alpha-to-coverage state when a fragment
        // shader writes to [[sample_mask]]. Moreover, Metal pipeline state
        // does not support setting a global coverage mask, which would be used
        // for emulating GL_SAMPLE_COVERAGE, so [[sample_mask]] is used instead.
        // To support alpha-to-coverage regardless of the [[sample_mask]] usage,
        // the former is always emulated on such drivers.
        TIntermBlock *alphaBlock = new TIntermBlock;

        // To reduce image artifacts due to regular coverage sample locations,
        // alpha value thresholds that toggle individual samples are slightly
        // different within 2x2 pixel blocks. Consider MSAAx4, for example.
        // Instead of always enabling samples on evenly distributed alpha
        // values like {51, 102, 153, 204} these thresholds may vary as follows
        //
        //    Sample 0       Sample 1       Sample 2       Sample 3
        //   ----- -----    ----- -----    ----- -----    ----- -----
        //  |  7.5| 39.5|  | 71.5|103.5|  |135.5|167.5|  |199.5|231.5|
        //  |----- -----|  |----- -----|  |----- -----|  |----- -----|
        //  | 55.5| 23.5|  |119.5| 87.5|  |183.5|151.5|  |247.5|215.5|
        //   ----- -----    ----- -----    ----- -----    ----- -----
        // These threshold values may be expressed as
        //    7.5 + P * 16 + 64 * sampleID
        // where P is
        //    ((x << 1) - (y & 1)) & 3
        // and constant values depend on the number of samples used.
        TVariable *p = CreateTempVariable(&symbolTable, StaticType::GetBasic<EbtInt, EbpHigh>());
        TVariable *y = CreateTempVariable(&symbolTable, StaticType::GetBasic<EbtInt, EbpHigh>());
        alphaBlock->appendStatement(CreateTempInitDeclarationNode(
            p, new TIntermSwizzle(new TIntermSymbol(BuiltInVariable::gl_FragCoord()), {0})));
        alphaBlock->appendStatement(CreateTempInitDeclarationNode(
            y, new TIntermSwizzle(new TIntermSymbol(BuiltInVariable::gl_FragCoord()), {1})));
        alphaBlock->appendStatement(
            new TIntermBinary(EOpBitShiftLeftAssign, new TIntermSymbol(p), CreateIndexNode(1)));
        alphaBlock->appendStatement(
            new TIntermBinary(EOpBitwiseAndAssign, new TIntermSymbol(y), CreateIndexNode(1)));
        alphaBlock->appendStatement(
            new TIntermBinary(EOpSubAssign, new TIntermSymbol(p), new TIntermSymbol(y)));
        alphaBlock->appendStatement(
            new TIntermBinary(EOpBitwiseAndAssign, new TIntermSymbol(p), CreateIndexNode(3)));

        // This internal variable, defined in-text in the function constants section,
        // will point to the alpha channel of the color zero output. Due to potential
        // EXT_blend_func_extended usage, the exact variable may be unknown until the
        // program is linked.
        TVariable *alpha0 =
            new TVariable(&symbolTable, sh::ImmutableString("ALPHA0"),
                          StaticType::Get<EbtFloat, EbpUndefined, EvqSpecConst, 1, 1>(),
                          SymbolType::AngleInternal);

        // Use metal::saturate to clamp the alpha value to [0.0, 1.0] and scale it
        // to [0.0, 510.0] since further operations expect an integer alpha value.
        TVariable *alphaScaled =
            CreateTempVariable(&symbolTable, StaticType::GetBasic<EbtFloat, EbpHigh>());
        alphaBlock->appendStatement(CreateTempInitDeclarationNode(
            alphaScaled, CreateBuiltInFunctionCallNode("saturate", {new TIntermSymbol(alpha0)},
                                                       symbolTable, kESSLInternalBackendBuiltIns)));
        alphaBlock->appendStatement(new TIntermBinary(EOpMulAssign, new TIntermSymbol(alphaScaled),
                                                      CreateFloatNode(510.0, EbpUndefined)));
        // int alphaMask = int(alphaScaled);
        TVariable *alphaMask =
            CreateTempVariable(&symbolTable, StaticType::GetBasic<EbtInt, EbpHigh>());
        alphaBlock->appendStatement(CreateTempInitDeclarationNode(
            alphaMask, TIntermAggregate::CreateConstructor(*StaticType::GetBasic<EbtInt, EbpHigh>(),
                                                           {new TIntermSymbol(alphaScaled)})));

        // Next operations depend on the number of samples in the curent render target.
        TIntermBlock *switchBlock = new TIntermBlock();

        auto computeNumberOfSamples = [&](int step, int bias, int scale) {
            switchBlock->appendStatement(new TIntermBinary(
                EOpBitShiftLeftAssign, new TIntermSymbol(p), CreateIndexNode(step)));
            switchBlock->appendStatement(new TIntermBinary(
                EOpAddAssign, new TIntermSymbol(alphaMask), CreateIndexNode(bias)));
            switchBlock->appendStatement(new TIntermBinary(
                EOpSubAssign, new TIntermSymbol(alphaMask), new TIntermSymbol(p)));
            switchBlock->appendStatement(new TIntermBinary(
                EOpBitShiftRightAssign, new TIntermSymbol(alphaMask), CreateIndexNode(scale)));
        };

        // MSAAx2
        switchBlock->appendStatement(new TIntermCase(CreateIndexNode(2)));

        // Canonical threshold values are
        //     15.5 + P * 32 + 128 * sampleID
        // With alpha values scaled to [0, 510], the number of covered samples is
        //     (alphaScaled + 256 - (31 + P * 64)) / 256
        // which could be simplified to
        //     (alphaScaled + 225 - (P << 6)) >> 8
        computeNumberOfSamples(6, 225, 8);

        // In a case of only two samples, the coverage mask is
        //     mask = (num_covered_samples * 3) >> 1
        switchBlock->appendStatement(
            new TIntermBinary(EOpMulAssign, new TIntermSymbol(alphaMask), CreateIndexNode(3)));
        switchBlock->appendStatement(new TIntermBinary(
            EOpBitShiftRightAssign, new TIntermSymbol(alphaMask), CreateIndexNode(1)));

        switchBlock->appendStatement(new TIntermBranch(EOpBreak, nullptr));

        // MSAAx4
        switchBlock->appendStatement(new TIntermCase(CreateIndexNode(4)));

        // Canonical threshold values are
        //     7.5 + P * 16 + 64 * sampleID
        // With alpha values scaled to [0, 510], the number of covered samples is
        //     (alphaScaled + 128 - (15 + P * 32)) / 128
        // which could be simplified to
        //     (alphaScaled + 113 - (P << 5)) >> 7
        computeNumberOfSamples(5, 113, 7);

        // When two out of four samples should be covered, prioritize
        // those that are located in the opposite corners of a pixel.
        // 0: 0000, 1: 0001, 2: 1001, 3: 1011, 4: 1111
        //     mask = (0xFB910 >> (num_covered_samples * 4)) & 0xF
        // The final AND may be omitted because the rasterizer output
        // is limited to four samples.
        switchBlock->appendStatement(new TIntermBinary(
            EOpBitShiftLeftAssign, new TIntermSymbol(alphaMask), CreateIndexNode(2)));
        switchBlock->appendStatement(
            new TIntermBinary(EOpAssign, new TIntermSymbol(alphaMask),
                              new TIntermBinary(EOpBitShiftRight, CreateIndexNode(0xFB910),
                                                new TIntermSymbol(alphaMask))));

        switchBlock->appendStatement(new TIntermBranch(EOpBreak, nullptr));

        // MSAAx8
        switchBlock->appendStatement(new TIntermCase(CreateIndexNode(8)));

        // Canonical threshold values are
        //     3.5 + P * 8 + 32 * sampleID
        // With alpha values scaled to [0, 510], the number of covered samples is
        //     (alphaScaled + 64 - (7 + P * 16)) / 64
        // which could be simplified to
        //     (alphaScaled + 57 - (P << 4)) >> 6
        computeNumberOfSamples(4, 57, 6);

        // When eight samples are used, they could be enabled one by one
        //     mask = ~(0xFFFFFFFF << num_covered_samples)
        switchBlock->appendStatement(
            new TIntermBinary(EOpAssign, new TIntermSymbol(alphaMask),
                              new TIntermBinary(EOpBitShiftLeft, CreateUIntNode(0xFFFFFFFFu),
                                                new TIntermSymbol(alphaMask))));
        switchBlock->appendStatement(new TIntermBinary(
            EOpAssign, new TIntermSymbol(alphaMask),
            new TIntermUnary(EOpBitwiseNot, new TIntermSymbol(alphaMask), nullptr)));

        switchBlock->appendStatement(new TIntermBranch(EOpBreak, nullptr));

        alphaBlock->getSequence()->push_back(
            new TIntermSwitch(CreateBuiltInFunctionCallNode("numSamples", {}, symbolTable,
                                                            kESSLInternalBackendBuiltIns),
                              switchBlock));

        alphaBlock->appendStatement(new TIntermBinary(
            EOpBitwiseAndAssign, new TIntermSymbol(angleSampleMask), new TIntermSymbol(alphaMask)));

        TIntermBlock *emulateAlphaToCoverageEnabledBlock = new TIntermBlock;
        emulateAlphaToCoverageEnabledBlock->appendStatement(
            new TIntermIfElse(driverUniforms->getAlphaToCoverage(), alphaBlock, nullptr));

        TVariable *emulateAlphaToCoverageVar =
            new TVariable(&symbolTable, sh::ImmutableString(mtl::kEmulateAlphaToCoverageConstName),
                          StaticType::Get<EbtBool, EbpUndefined, EvqSpecConst, 1, 1>(),
                          SymbolType::AngleInternal);
        TIntermIfElse *useAlphaToCoverage =
            new TIntermIfElse(new TIntermSymbol(emulateAlphaToCoverageVar),
                              emulateAlphaToCoverageEnabledBlock, nullptr);

        block->appendStatement(useAlphaToCoverage);
    }

    // Sample mask assignment is guarded by ANGLEMultisampledRendering specialization constant
    TVariable *multisampledRenderingVar = new TVariable(
        &symbolTable, sh::ImmutableString(mtl::kMultisampledRenderingConstName),
        StaticType::Get<EbtBool, EbpUndefined, EvqSpecConst, 1, 1>(), SymbolType::AngleInternal);
    return RunAtTheEndOfShader(
        &compiler, &root,
        new TIntermIfElse(new TIntermSymbol(multisampledRenderingVar), block, nullptr),
        &symbolTable);
}

[[nodiscard]] bool AddFragDataDeclaration(TCompiler &compiler,
                                          TIntermBlock &root,
                                          bool usesSecondary,
                                          bool secondary)
{
    TSymbolTable &symbolTable = compiler.getSymbolTable();
    const int maxDrawBuffers  = usesSecondary ? compiler.getResources().MaxDualSourceDrawBuffers
                                              : compiler.getResources().MaxDrawBuffers;
    TType *gl_FragDataType =
        new TType(EbtFloat, EbpMedium, secondary ? EvqSecondaryFragDataEXT : EvqFragData, 4, 1);
    std::vector<const TVariable *> glFragDataSlots;
    TIntermSequence declareGLFragdataSequence;

    // Create gl_FragData_i or gl_SecondaryFragDataEXT_i
    const char *fragData             = "gl_FragData";
    const char *secondaryFragDataEXT = "gl_SecondaryFragDataEXT";
    const char *name                 = secondary ? secondaryFragDataEXT : fragData;
    for (int i = 0; i < maxDrawBuffers; i++)
    {
        ImmutableString varName = BuildConcatenatedImmutableString(name, '_', i);
        const TVariable *glFragData =
            new TVariable(&symbolTable, varName, gl_FragDataType, SymbolType::AngleInternal,
                          TExtension::UNDEFINED);
        glFragDataSlots.push_back(glFragData);
        declareGLFragdataSequence.push_back(new TIntermDeclaration{glFragData});
    }
    root.insertChildNodes(FindMainIndex(&root), declareGLFragdataSequence);

    // Create an internal gl_FragData array type, compatible with indexing syntax.
    TType *gl_FragDataTypeArray = new TType(EbtFloat, EbpMedium, EvqGlobal, 4, 1);
    gl_FragDataTypeArray->makeArray(maxDrawBuffers);
    const TVariable *glFragDataGlobal = new TVariable(&symbolTable, ImmutableString(name),
                                                      gl_FragDataTypeArray, SymbolType::BuiltIn);

    DeclareGlobalVariable(&root, glFragDataGlobal);
    const TIntermSymbol *originalGLFragData = FindSymbolNode(&root, ImmutableString(name));
    ASSERT(originalGLFragData);

    // Replace gl_FragData[] or gl_SecondaryFragDataEXT[] with our globally defined variable
    if (!ReplaceVariable(&compiler, &root, &(originalGLFragData->variable()), glFragDataGlobal))
    {
        return false;
    }

    // Assign each array attribute to an output
    TIntermBlock *insertSequence = new TIntermBlock();
    for (int i = 0; i < maxDrawBuffers; i++)
    {
        TIntermTyped *glFragDataSlot         = new TIntermSymbol(glFragDataSlots[i]);
        TIntermTyped *glFragDataGlobalSymbol = new TIntermSymbol(glFragDataGlobal);
        auto &access                         = AccessIndex(*glFragDataGlobalSymbol, i);
        TIntermBinary *assignment =
            new TIntermBinary(TOperator::EOpAssign, glFragDataSlot, &access);
        insertSequence->appendStatement(assignment);
    }
    return RunAtTheEndOfShader(&compiler, &root, insertSequence, &symbolTable);
}

[[nodiscard]] bool AppendVertexShaderTransformFeedbackOutputToMain(TCompiler &compiler,
                                                                   SymbolEnv &mSymbolEnv,
                                                                   TIntermBlock &root)
{
    TSymbolTable &symbolTable = compiler.getSymbolTable();

    // Append the assignment as a statement at the end of the shader.
    return RunAtTheEndOfShader(&compiler, &root,
                               &(mSymbolEnv.callFunctionOverload(Name("@@XFB-OUT@@"), *new TType(),
                                                                 *new TIntermSequence())),
                               &symbolTable);
}

// Unlike Vulkan having auto viewport flipping extension, in Metal we have to flip gl_Position.y
// manually.
// This operation performs flipping the gl_Position.y using this expression:
// gl_Position.y = gl_Position.y * negViewportScaleY
[[nodiscard]] bool AppendVertexShaderPositionYCorrectionToMain(TCompiler *compiler,
                                                               TIntermBlock *root,
                                                               TSymbolTable *symbolTable,
                                                               TIntermTyped *negFlipY)
{
    // Create a symbol reference to "gl_Position"
    const TVariable *position  = BuiltInVariable::gl_Position();
    TIntermSymbol *positionRef = new TIntermSymbol(position);

    // Create a swizzle to "gl_Position.y"
    TVector<int> swizzleOffsetY;
    swizzleOffsetY.push_back(1);
    TIntermSwizzle *positionY = new TIntermSwizzle(positionRef, swizzleOffsetY);

    // Create the expression "gl_Position.y * negFlipY"
    TIntermBinary *inverseY = new TIntermBinary(EOpMul, positionY->deepCopy(), negFlipY);

    // Create the assignment "gl_Position.y = gl_Position.y * negViewportScaleY
    TIntermTyped *positionYLHS = positionY->deepCopy();
    TIntermBinary *assignment  = new TIntermBinary(TOperator::EOpAssign, positionYLHS, inverseY);

    // Append the assignment as a statement at the end of the shader.
    return RunAtTheEndOfShader(compiler, root, assignment, symbolTable);
}

[[nodiscard]] bool EmulateClipDistanceVaryings(TCompiler *compiler,
                                               TIntermBlock *root,
                                               TSymbolTable *symbolTable,
                                               const GLenum shaderType)
{
    ASSERT(shaderType == GL_VERTEX_SHADER || shaderType == GL_FRAGMENT_SHADER);

    const TIntermSymbol *symbolNode = FindSymbolNode(root, ImmutableString("gl_ClipDistance"));
    ASSERT(symbolNode != nullptr);
    const TVariable *clipDistanceVar = &symbolNode->variable();

    const bool fragment = shaderType == GL_FRAGMENT_SHADER;
    if (fragment)
    {
        TType *globalType = new TType(EbtFloat, EbpHigh, EvqGlobal, 1, 1);
        globalType->toArrayBaseType();
        globalType->makeArray(compiler->getClipDistanceArraySize());

        const TVariable *globalVar = new TVariable(symbolTable, ImmutableString("ClipDistance"),
                                                   globalType, SymbolType::AngleInternal);

        if (!ReplaceVariable(compiler, root, clipDistanceVar, globalVar))
        {
            return false;
        }
        clipDistanceVar = globalVar;
    }

    TIntermBlock *assignBlock = new TIntermBlock();
    size_t index              = FindMainIndex(root);
    TType *type = new TType(EbtFloat, EbpHigh, fragment ? EvqFragmentIn : EvqVertexOut, 1, 1);
    for (int i = 0; i < compiler->getClipDistanceArraySize(); i++)
    {
        TVariable *varyingVar =
            new TVariable(symbolTable, BuildConcatenatedImmutableString("ClipDistance_", i), type,
                          SymbolType::AngleInternal);
        TIntermDeclaration *varyingDecl = new TIntermDeclaration();
        varyingDecl->appendDeclarator(new TIntermSymbol(varyingVar));
        root->insertStatement(index++, varyingDecl);
        TIntermSymbol *varyingSym = new TIntermSymbol(varyingVar);
        TIntermTyped *arrayAccess = new TIntermBinary(
            EOpIndexDirect, new TIntermSymbol(clipDistanceVar), CreateIndexNode(i));
        assignBlock->appendStatement(new TIntermBinary(
            EOpAssign, fragment ? arrayAccess : varyingSym, fragment ? varyingSym : arrayAccess));
    }

    return fragment ? RunAtTheBeginningOfShader(compiler, root, assignBlock)
                    : RunAtTheEndOfShader(compiler, root, assignBlock, symbolTable);
}
}  // namespace

namespace mtl
{
TranslatorMetalReflection *getTranslatorMetalReflection(const TCompiler *compiler)
{
    return ((TranslatorMSL *)compiler)->getTranslatorMetalReflection();
}
}  // namespace mtl
TranslatorMSL::TranslatorMSL(sh::GLenum type, ShShaderSpec spec, ShShaderOutput output)
    : TCompiler(type, spec, output)
{}

[[nodiscard]] bool TranslatorMSL::insertRasterizationDiscardLogic(TIntermBlock &root)
{
    // This transformation leaves the tree in an inconsistent state by using a variable that's
    // defined in text, outside of the knowledge of the AST.
    mValidateASTOptions.validateVariableReferences = false;

    TSymbolTable *symbolTable = &getSymbolTable();

    TType *boolType = new TType(EbtBool);
    boolType->setQualifier(EvqConst);
    TVariable *discardEnabledVar =
        new TVariable(symbolTable, sh::ImmutableString(sh::mtl::kRasterizerDiscardEnabledConstName),
                      boolType, SymbolType::AngleInternal);

    const TVariable *position  = BuiltInVariable::gl_Position();
    TIntermSymbol *positionRef = new TIntermSymbol(position);

    // Create vec4(-3, -3, -3, 1):
    auto vec4Type            = new TType(EbtFloat, 4);
    TIntermSequence vec4Args = {
        CreateFloatNode(-3.0f, EbpMedium),
        CreateFloatNode(-3.0f, EbpMedium),
        CreateFloatNode(-3.0f, EbpMedium),
        CreateFloatNode(1.0f, EbpMedium),
    };
    TIntermAggregate *constVarConstructor =
        TIntermAggregate::CreateConstructor(*vec4Type, &vec4Args);

    // Create the assignment "gl_Position = vec4(-3, -3, -3, 1)"
    TIntermBinary *assignment =
        new TIntermBinary(TOperator::EOpAssign, positionRef->deepCopy(), constVarConstructor);

    TIntermBlock *discardBlock = new TIntermBlock;
    discardBlock->appendStatement(assignment);

    TIntermSymbol *discardEnabled = new TIntermSymbol(discardEnabledVar);
    TIntermIfElse *ifCall         = new TIntermIfElse(discardEnabled, discardBlock, nullptr);

    return RunAtTheEndOfShader(this, &root, ifCall, symbolTable);
}

// Metal needs to inverse the depth if depthRange is is reverse order, i.e. depth near > depth far
// This is achieved by multiply the depth value with scale value stored in
// driver uniform's depthRange.reserved
bool TranslatorMSL::transformDepthBeforeCorrection(TIntermBlock *root,
                                                   const DriverUniformMetal *driverUniforms)
{
    // Create a symbol reference to "gl_Position"
    const TVariable *position  = BuiltInVariable::gl_Position();
    TIntermSymbol *positionRef = new TIntermSymbol(position);

    // Create a swizzle to "gl_Position.z"
    TVector<int> swizzleOffsetZ = {2};
    TIntermSwizzle *positionZ   = new TIntermSwizzle(positionRef, swizzleOffsetZ);

    // Create a ref to "zscale"
    TIntermTyped *viewportZScale = driverUniforms->getViewportZScale();

    // Create the expression "gl_Position.z * zscale".
    TIntermBinary *zScale = new TIntermBinary(EOpMul, positionZ->deepCopy(), viewportZScale);

    // Create the assignment "gl_Position.z = gl_Position.z * zscale"
    TIntermTyped *positionZLHS = positionZ->deepCopy();
    TIntermBinary *assignment  = new TIntermBinary(TOperator::EOpAssign, positionZLHS, zScale);

    // Append the assignment as a statement at the end of the shader.
    return RunAtTheEndOfShader(this, root, assignment, &getSymbolTable());
}

// This operation performs the viewport depth translation needed by Metal. GL uses a
// clip space z range of -1 to +1 where as Metal uses 0 to 1. The translation becomes
// this expression
//
//     z_metal = 0.5 * (w_gl + z_gl)
//
// where z_metal is the depth output of a Metal vertex shader and z_gl is the same for GL.
// This operation is skipped when GL_CLIP_DEPTH_MODE_EXT is set to GL_ZERO_TO_ONE_EXT.
bool TranslatorMSL::appendVertexShaderDepthCorrectionToMain(
    TIntermBlock *root,
    const DriverUniformMetal *driverUniforms)
{
    const TVariable *position  = BuiltInVariable::gl_Position();
    TIntermSymbol *positionRef = new TIntermSymbol(position);

    TVector<int> swizzleOffsetZ = {2};
    TIntermSwizzle *positionZ   = new TIntermSwizzle(positionRef, swizzleOffsetZ);

    TIntermConstantUnion *oneHalf = CreateFloatNode(0.5f, EbpMedium);

    TVector<int> swizzleOffsetW = {3};
    TIntermSwizzle *positionW   = new TIntermSwizzle(positionRef->deepCopy(), swizzleOffsetW);

    // Create the expression "(gl_Position.z + gl_Position.w) * 0.5".
    TIntermBinary *zPlusW = new TIntermBinary(EOpAdd, positionZ->deepCopy(), positionW->deepCopy());
    TIntermBinary *halfZPlusW = new TIntermBinary(EOpMul, zPlusW, oneHalf->deepCopy());

    // Create the assignment "gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5"
    TIntermTyped *positionZLHS = positionZ->deepCopy();
    TIntermBinary *assignment  = new TIntermBinary(TOperator::EOpAssign, positionZLHS, halfZPlusW);

    // Apply depth correction if needed
    TIntermBlock *block = new TIntermBlock;
    block->appendStatement(assignment);
    TIntermIfElse *ifCall = new TIntermIfElse(driverUniforms->getTransformDepth(), block, nullptr);

    // Append the assignment as a statement at the end of the shader.
    return RunAtTheEndOfShader(this, root, ifCall, &getSymbolTable());
}

static inline MetalShaderType metalShaderTypeFromGLSL(sh::GLenum shaderType)
{
    switch (shaderType)
    {
        case GL_VERTEX_SHADER:
            return MetalShaderType::Vertex;
        case GL_FRAGMENT_SHADER:
            return MetalShaderType::Fragment;
        case GL_COMPUTE_SHADER:
            ASSERT(0 && "compute shaders not currently supported");
            return MetalShaderType::Compute;
        default:
            ASSERT(0 && "Invalid shader type.");
            return MetalShaderType::None;
    }
}

bool TranslatorMSL::translateImpl(TInfoSinkBase &sink,
                                  TIntermBlock *root,
                                  const ShCompileOptions &compileOptions,
                                  PerformanceDiagnostics * /*perfDiagnostics*/,
                                  SpecConst *specConst,
                                  DriverUniformMetal *driverUniforms)
{
    TSymbolTable &symbolTable = getSymbolTable();
    IdGen idGen;
    ProgramPreludeConfig ppc(metalShaderTypeFromGLSL(getShaderType()));
    ppc.usesDerivatives = usesDerivatives();

    if (!WrapMain(*this, idGen, *root))
    {
        return false;
    }

    // Remove declarations of inactive shader interface variables so glslang wrapper doesn't need to
    // replace them.  Note: this is done before extracting samplers from structs, as removing such
    // inactive samplers is not yet supported.  Note also that currently, CollectVariables marks
    // every field of an active uniform that's of struct type as active, i.e. no extracted sampler
    // is inactive.
    if (!RemoveInactiveInterfaceVariables(this, root, &getSymbolTable(), getAttributes(),
                                          getInputVaryings(), getOutputVariables(), getUniforms(),
                                          getInterfaceBlocks(), false))
    {
        return false;
    }

    // Write out default uniforms into a uniform block assigned to a specific set/binding.
    int aggregateTypesUsedForUniforms = 0;
    int atomicCounterCount            = 0;
    for (const auto &uniform : getUniforms())
    {
        if (uniform.isStruct() || uniform.isArrayOfArrays())
        {
            ++aggregateTypesUsedForUniforms;
        }

        if (uniform.active && gl::IsAtomicCounterType(uniform.type))
        {
            ++atomicCounterCount;
        }
    }

    // If there are any function calls that take array-of-array of opaque uniform parameters, or
    // other opaque uniforms that need special handling in Vulkan, such as atomic counters,
    // monomorphize the functions by removing said parameters and replacing them in the function
    // body with the call arguments.
    //
    // This has a few benefits:
    //
    // - It dramatically simplifies future transformations w.r.t to samplers in structs, array of
    //   arrays of opaque types, atomic counters etc.
    // - Avoids the need for shader*ArrayDynamicIndexing Vulkan features.
    UnsupportedFunctionArgsBitSet args{UnsupportedFunctionArgs::StructContainingSamplers,
                                       UnsupportedFunctionArgs::ArrayOfArrayOfSamplerOrImage,
                                       UnsupportedFunctionArgs::AtomicCounter,
                                       UnsupportedFunctionArgs::Image};
    if (!MonomorphizeUnsupportedFunctions(this, root, &getSymbolTable(), args))
    {
        return false;
    }

    if (aggregateTypesUsedForUniforms > 0)
    {
        int removedUniformsCount;
        if (!RewriteStructSamplers(this, root, &getSymbolTable(), &removedUniformsCount))
        {
            return false;
        }
    }

    // Replace array of array of opaque uniforms with a flattened array.  This is run after
    // MonomorphizeUnsupportedFunctions and RewriteStructSamplers so that it's not possible for an
    // array of array of opaque type to be partially subscripted and passed to a function.
    if (!RewriteArrayOfArrayOfOpaqueUniforms(this, root, &getSymbolTable()))
    {
        return false;
    }

    if (getShaderVersion() >= 300 ||
        IsExtensionEnabled(getExtensionBehavior(), TExtension::EXT_shader_texture_lod))
    {
        if (compileOptions.preTransformTextureCubeGradDerivatives)
        {
            if (!PreTransformTextureCubeGradDerivatives(this, root, &symbolTable,
                                                        getShaderVersion()))
            {
                return false;
            }
        }
    }

    if (getShaderType() == GL_COMPUTE_SHADER)
    {
        driverUniforms->addComputeDriverUniformsToShader(root, &getSymbolTable());
    }
    else
    {
        driverUniforms->addGraphicsDriverUniformsToShader(root, &getSymbolTable());
    }

    if (atomicCounterCount > 0)
    {
        const TIntermTyped *acbBufferOffsets = driverUniforms->getAcbBufferOffsets();
        if (!RewriteAtomicCounters(this, root, &symbolTable, acbBufferOffsets, nullptr))
        {
            return false;
        }
    }
    else if (getShaderVersion() >= 310)
    {
        // Vulkan doesn't support Atomic Storage as a Storage Class, but we've seen
        // cases where builtins are using it even with no active atomic counters.
        // This pass simply removes those builtins in that scenario.
        if (!RemoveAtomicCounterBuiltins(this, root))
        {
            return false;
        }
    }

    if (getShaderType() != GL_COMPUTE_SHADER)
    {
        if (!ReplaceGLDepthRangeWithDriverUniform(this, root, driverUniforms, &getSymbolTable()))
        {
            return false;
        }
    }

    {
        bool usesInstanceId = false;
        bool usesVertexId   = false;
        for (const ShaderVariable &var : mAttributes)
        {
            if (var.isBuiltIn())
            {
                if (var.name == "gl_InstanceID")
                {
                    usesInstanceId = true;
                }
                if (var.name == "gl_VertexID")
                {
                    usesVertexId = true;
                }
            }
        }

        if (usesInstanceId)
        {
            root->insertChildNodes(
                FindMainIndex(root),
                TIntermSequence{new TIntermDeclaration{BuiltInVariable::gl_InstanceID()}});
        }
        if (usesVertexId)
        {
            AddBuiltInDeclaration(*root, symbolTable, *BuiltInVariable::gl_VertexID());
        }
    }
    SymbolEnv symbolEnv(*this, *root);

    bool usesSampleMask = false;
    if (getShaderType() == GL_FRAGMENT_SHADER)
    {
        bool usesPointCoord     = false;
        bool usesFragCoord      = false;
        bool usesFrontFacing    = false;
        bool usesSampleID       = false;
        bool usesSamplePosition = false;
        bool usesSampleMaskIn   = false;
        for (const ShaderVariable &inputVarying : mInputVaryings)
        {
            if (inputVarying.isBuiltIn())
            {
                if (inputVarying.name == "gl_PointCoord")
                {
                    usesPointCoord = true;
                }
                else if (inputVarying.name == "gl_FragCoord")
                {
                    usesFragCoord = true;
                }
                else if (inputVarying.name == "gl_FrontFacing")
                {
                    usesFrontFacing = true;
                }
                else if (inputVarying.name == "gl_SampleID")
                {
                    usesSampleID = true;
                }
                else if (inputVarying.name == "gl_SamplePosition")
                {
                    usesSampleID       = true;
                    usesSamplePosition = true;
                }
                else if (inputVarying.name == "gl_SampleMaskIn")
                {
                    usesSampleMaskIn = true;
                }
            }
        }

        bool usesFragColor             = false;
        bool usesFragData              = false;
        bool usesFragDepth             = false;
        bool usesFragDepthEXT          = false;
        bool usesSecondaryFragColorEXT = false;
        bool usesSecondaryFragDataEXT  = false;
        for (const ShaderVariable &outputVarying : mOutputVariables)
        {
            if (outputVarying.isBuiltIn())
            {
                if (outputVarying.name == "gl_FragColor")
                {
                    usesFragColor = true;
                }
                else if (outputVarying.name == "gl_FragData")
                {
                    usesFragData = true;
                }
                else if (outputVarying.name == "gl_FragDepth")
                {
                    usesFragDepth = true;
                }
                else if (outputVarying.name == "gl_FragDepthEXT")
                {
                    usesFragDepthEXT = true;
                }
                else if (outputVarying.name == "gl_SecondaryFragColorEXT")
                {
                    usesSecondaryFragColorEXT = true;
                }
                else if (outputVarying.name == "gl_SecondaryFragDataEXT")
                {
                    usesSecondaryFragDataEXT = true;
                }
                else if (outputVarying.name == "gl_SampleMask")
                {
                    usesSampleMask = true;
                }
            }
        }

        // A shader may assign values to either the set of gl_FragColor and gl_SecondaryFragColorEXT
        // or the set of gl_FragData and gl_SecondaryFragDataEXT, but not both.
        ASSERT((!usesFragColor && !usesSecondaryFragColorEXT) ||
               (!usesFragData && !usesSecondaryFragDataEXT));

        if (usesFragColor)
        {
            AddFragColorDeclaration(*root, symbolTable, *BuiltInVariable::gl_FragColor());
        }
        else if (usesFragData)
        {
            if (!AddFragDataDeclaration(*this, *root, usesSecondaryFragDataEXT, false))
            {
                return false;
            }
        }

        if (usesFragDepth)
        {
            AddBuiltInDeclaration(*root, symbolTable, *BuiltInVariable::gl_FragDepth());
        }
        else if (usesFragDepthEXT)
        {
            AddFragDepthEXTDeclaration(*this, *root, symbolTable);
        }

        if (usesSecondaryFragColorEXT)
        {
            AddFragColorDeclaration(*root, symbolTable,
                                    *BuiltInVariable::gl_SecondaryFragColorEXT());
        }
        else if (usesSecondaryFragDataEXT)
        {
            if (!AddFragDataDeclaration(*this, *root, usesSecondaryFragDataEXT, true))
            {
                return false;
            }
        }

        bool usesSampleInterpolation = false;
        bool usesSampleInterpolant   = false;
        if ((getShaderVersion() >= 320 ||
             IsExtensionEnabled(getExtensionBehavior(),
                                TExtension::OES_shader_multisample_interpolation)) &&
            !RewriteInterpolants(*this, *root, symbolTable, driverUniforms,
                                 &usesSampleInterpolation, &usesSampleInterpolant))
        {
            return false;
        }

        if (usesSampleID || (usesSampleMaskIn && usesSampleInterpolation) || usesSampleInterpolant)
        {
            DeclareRightBeforeMain(*root, *BuiltInVariable::gl_SampleID());
        }

        if (usesSamplePosition)
        {
            if (!AddSamplePositionDeclaration(*this, *root, symbolTable, driverUniforms))
            {
                return false;
            }
        }

        if (usesSampleMaskIn)
        {
            if (!AddSampleMaskInDeclaration(*this, *root, symbolTable, driverUniforms,
                                            usesSampleID || usesSampleInterpolation))
            {
                return false;
            }
        }

        if (usesPointCoord)
        {
            TIntermTyped *flipNegXY =
                driverUniforms->getNegFlipXY(&getSymbolTable(), DriverUniformFlip::Fragment);
            TIntermConstantUnion *pivot = CreateFloatNode(0.5f, EbpMedium);
            if (!FlipBuiltinVariable(this, root, GetMainSequence(root), flipNegXY,
                                     &getSymbolTable(), BuiltInVariable::gl_PointCoord(),
                                     kFlippedPointCoordName, pivot))
            {
                return false;
            }
            DeclareRightBeforeMain(*root, *BuiltInVariable::gl_PointCoord());
        }

        if (usesFragCoord || compileOptions.emulateAlphaToCoverage ||
            compileOptions.metal.generateShareableShaders)
        {
            if (!InsertFragCoordCorrection(this, compileOptions, root, GetMainSequence(root),
                                           &getSymbolTable(), driverUniforms))
            {
                return false;
            }
            const TVariable *fragCoord = static_cast<const TVariable *>(
                getSymbolTable().findBuiltIn(ImmutableString("gl_FragCoord"), getShaderVersion()));
            DeclareRightBeforeMain(*root, *fragCoord);
        }

        if (!RewriteDfdy(this, root, &getSymbolTable(), getShaderVersion(), specConst,
                         driverUniforms))
        {
            return false;
        }

        if (getClipDistanceArraySize())
        {
            if (!EmulateClipDistanceVaryings(this, root, &getSymbolTable(), getShaderType()))
            {
                return false;
            }
        }

        if (usesFrontFacing)
        {
            DeclareRightBeforeMain(*root, *BuiltInVariable::gl_FrontFacing());
        }

        bool usesNumSamples = false;
        for (const ShaderVariable &uniform : mUniforms)
        {
            if (uniform.name == "gl_NumSamples")
            {
                usesNumSamples = true;
                break;
            }
        }

        if (usesNumSamples)
        {
            if (!AddNumSamplesDeclaration(*this, *root, symbolTable))
            {
                return false;
            }
        }
    }
    else if (getShaderType() == GL_VERTEX_SHADER)
    {
        DeclareRightBeforeMain(*root, *BuiltInVariable::gl_Position());

        if (FindSymbolNode(root, BuiltInVariable::gl_PointSize()->name()))
        {
            const TVariable *pointSize = static_cast<const TVariable *>(
                getSymbolTable().findBuiltIn(ImmutableString("gl_PointSize"), getShaderVersion()));
            DeclareRightBeforeMain(*root, *pointSize);
        }

        // Append a macro for transform feedback substitution prior to modifying depth.
        if (!AppendVertexShaderTransformFeedbackOutputToMain(*this, symbolEnv, *root))
        {
            return false;
        }

        if (getClipDistanceArraySize())
        {
            if (!ZeroDisabledClipDistanceAssignments(this, root, &getSymbolTable(), getShaderType(),
                                                     driverUniforms->getClipDistancesEnabled()))
            {
                return false;
            }

            if (IsExtensionEnabled(getExtensionBehavior(), TExtension::ANGLE_clip_cull_distance) &&
                !EmulateClipDistanceVaryings(this, root, &getSymbolTable(), getShaderType()))
            {
                return false;
            }
        }

        if (!transformDepthBeforeCorrection(root, driverUniforms))
        {
            return false;
        }

        if (!appendVertexShaderDepthCorrectionToMain(root, driverUniforms))
        {
            return false;
        }
    }

    if (getShaderType() == GL_VERTEX_SHADER)
    {
        TIntermTyped *flipNegY =
            driverUniforms->getFlipXY(&getSymbolTable(), DriverUniformFlip::PreFragment);
        flipNegY = (new TIntermSwizzle(flipNegY, {1}))->fold(nullptr);

        if (!AppendVertexShaderPositionYCorrectionToMain(this, root, &getSymbolTable(), flipNegY))
        {
            return false;
        }
        if (!insertRasterizationDiscardLogic(*root))
        {
            return false;
        }
    }
    else if (getShaderType() == GL_FRAGMENT_SHADER)
    {
        mValidateASTOptions.validateVariableReferences = false;
        if (!AddSampleMaskDeclaration(*this, *root, symbolTable, driverUniforms,
                                      compileOptions.emulateAlphaToCoverage ||
                                          compileOptions.metal.generateShareableShaders,
                                      usesSampleMask))
        {
            return false;
        }
    }

    if (!validateAST(root))
    {
        return false;
    }

    // This is the largest size required to pass all the tests in
    // (dEQP-GLES3.functional.shaders.large_constant_arrays)
    // This value could in principle be smaller.
    const size_t hoistThresholdSize = 256;
    if (!HoistConstants(*this, *root, idGen, hoistThresholdSize))
    {
        return false;
    }

    if (!ConvertUnsupportedConstructorsToFunctionCalls(*this, *root))
    {
        return false;
    }

    const bool needsExplicitBoolCasts = compileOptions.addExplicitBoolCasts;
    if (!AddExplicitTypeCasts(*this, *root, symbolEnv, needsExplicitBoolCasts))
    {
        return false;
    }

    if (!SeparateCompoundExpressions(*this, symbolEnv, idGen, *root))
    {
        return false;
    }

    if (!ReduceInterfaceBlocks(*this, *root, idGen))
    {
        return false;
    }

    // The RewritePipelines phase leaves the tree in an inconsistent state by inserting
    // references to structures like "ANGLE_TextureEnv<metal::texture2d<float>>" which are
    // defined in text (in ProgramPrelude), outside of the knowledge of the AST.
    mValidateASTOptions.validateStructUsage = false;
    // The RewritePipelines phase also generates incoming arguments to synthesized
    // functions that use are missing qualifiers - for example, angleUniforms isn't marked
    // as an incoming argument.
    mValidateASTOptions.validateQualifiers = false;

    PipelineStructs pipelineStructs;
    if (!RewritePipelines(*this, *root, getInputVaryings(), getOutputVaryings(), idGen,
                          *driverUniforms, symbolEnv, pipelineStructs))
    {
        return false;
    }
    if (getShaderType() == GL_VERTEX_SHADER)
    {
        // This has to happen after RewritePipelines.
        if (!IntroduceVertexAndInstanceIndex(*this, *root))
        {
            return false;
        }
    }

    if (!RewriteCaseDeclarations(*this, *root))
    {
        return false;
    }

    if (!RewriteUnaddressableReferences(*this, *root, symbolEnv))
    {
        return false;
    }

    if (!RewriteOutArgs(*this, *root, symbolEnv))
    {
        return false;
    }
    if (!FixTypeConstructors(*this, symbolEnv, *root))
    {
        return false;
    }
    if (!ToposortStructs(*this, symbolEnv, *root, ppc))
    {
        return false;
    }
    if (!EmitMetal(*this, *root, idGen, pipelineStructs, symbolEnv, ppc, compileOptions))
    {
        return false;
    }

    ASSERT(validateAST(root));

    return true;
}

bool TranslatorMSL::translate(TIntermBlock *root,
                              const ShCompileOptions &compileOptions,
                              PerformanceDiagnostics *perfDiagnostics)
{
    if (!root)
    {
        return false;
    }

    // TODO: refactor the code in TranslatorMSL to not issue raw function calls.
    // http://anglebug.com/42264589#comment3
    mValidateASTOptions.validateNoRawFunctionCalls = false;
    // A validation error is generated in this backend due to bool uniforms.
    mValidateASTOptions.validatePrecision = false;

    TInfoSinkBase &sink = getInfoSink().obj;
    SpecConst specConst(&getSymbolTable(), compileOptions, getShaderType());
    DriverUniformMetal driverUniforms(DriverUniformMode::Structure);
    if (!translateImpl(sink, root, compileOptions, perfDiagnostics, &specConst, &driverUniforms))
    {
        return false;
    }

    return true;
}
bool TranslatorMSL::shouldFlattenPragmaStdglInvariantAll()
{
    // Not neccesary for MSL transformation.
    return false;
}

}  // namespace sh
