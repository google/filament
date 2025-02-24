//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EmulateAdvancedBlendEquations.cpp: Emulate advanced blend equations by implicitly reading back
// from the color attachment (as an input attachment) and apply the equation function based on a
// uniform.
//

#include "compiler/translator/tree_ops/spirv/EmulateAdvancedBlendEquations.h"

#include <map>

#include "GLSLANG/ShaderVars.h"
#include "common/PackedEnums.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/DriverUniform.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/RunAtTheEndOfShader.h"

namespace sh
{
namespace
{

// All helper functions that may be generated.
class Builder
{
  public:
    Builder(TCompiler *compiler,
            TSymbolTable *symbolTable,
            const AdvancedBlendEquations &advancedBlendEquations,
            const DriverUniform *driverUniforms,
            InputAttachmentMap *inputAttachmentMap)
        : mCompiler(compiler),
          mSymbolTable(symbolTable),
          mDriverUniforms(driverUniforms),
          mInputAttachmentMap(inputAttachmentMap),
          mAdvancedBlendEquations(advancedBlendEquations)
    {}

    bool build(TIntermBlock *root);

  private:
    void findColorOutput(TIntermBlock *root);
    void createSubpassInputVar(TIntermBlock *root);
    void generateHslHelperFunctions();
    void generateBlendFunctions();
    void insertGeneratedFunctions(TIntermBlock *root);
    TIntermTyped *divideFloatNode(TIntermTyped *dividend, TIntermTyped *divisor);
    TIntermSymbol *premultiplyAlpha(TIntermBlock *blendBlock, TIntermTyped *var, const char *name);
    void generatePreamble(TIntermBlock *blendBlock);
    void generateEquationSwitch(TIntermBlock *blendBlock);

    TCompiler *mCompiler;
    TSymbolTable *mSymbolTable;
    const DriverUniform *mDriverUniforms;
    InputAttachmentMap *mInputAttachmentMap;
    const AdvancedBlendEquations &mAdvancedBlendEquations;

    // The color input and output.  Output is the blend source, and input is the destination.
    const TVariable *mSubpassInputVar = nullptr;
    const TVariable *mOutputVar       = nullptr;

    // The value of output, premultiplied by alpha
    TIntermSymbol *mSrc = nullptr;
    // The value of input, premultiplied by alpha
    TIntermSymbol *mDst = nullptr;

    // p0, p1 and p2 coefficients
    TIntermSymbol *mP0 = nullptr;
    TIntermSymbol *mP1 = nullptr;
    TIntermSymbol *mP2 = nullptr;

    // Functions implementing an advanced blend equation:
    angle::PackedEnumMap<gl::BlendEquationType, TIntermFunctionDefinition *> mBlendFuncs = {};

    // HSL helpers:
    TIntermFunctionDefinition *mMinv3     = nullptr;
    TIntermFunctionDefinition *mMaxv3     = nullptr;
    TIntermFunctionDefinition *mLumv3     = nullptr;
    TIntermFunctionDefinition *mSatv3     = nullptr;
    TIntermFunctionDefinition *mClipColor = nullptr;
    TIntermFunctionDefinition *mSetLum    = nullptr;
    TIntermFunctionDefinition *mSetLumSat = nullptr;
};

bool Builder::build(TIntermBlock *root)
{
    // Find the output variable for which advanced blend is specified.  Note that advanced blend can
    // only be used when rendering is done to a single color attachment.
    findColorOutput(root);
    if (mSubpassInputVar == nullptr)
    {
        createSubpassInputVar(root);
    }

    // If any HSL blend equation is used, generate a few utility functions used in Table X.2 in the
    // spec.
    if (mAdvancedBlendEquations.anyHsl())
    {
        generateHslHelperFunctions();
    }

    // Generate a function for each enabled blend equation.  This is |f| in the spec.
    generateBlendFunctions();

    // Insert the generated functions to root.
    insertGeneratedFunctions(root);

    // Prepare for blend by:
    //
    // - Premultiplying src and dst color by alpha
    // - Calculating p0, p1 and p2
    //
    // Note that the color coefficients (X,Y,Z) are always (1,1,1) in the KHR extension (they were
    // not in the NV extension), so they are implicitly dropped.
    TIntermBlock *blendBlock = new TIntermBlock;
    generatePreamble(blendBlock);

    // Generate the |switch| that calls the right function based on a driver uniform.
    generateEquationSwitch(blendBlock);

    // Place the entire blend block under an if (equation != 0)
    TIntermTyped *equationUniform = mDriverUniforms->getAdvancedBlendEquation();
    TIntermTyped *notZero = new TIntermBinary(EOpNotEqual, equationUniform, CreateUIntNode(0));

    TIntermIfElse *blend = new TIntermIfElse(notZero, blendBlock, nullptr);
    return RunAtTheEndOfShader(mCompiler, root, blend, mSymbolTable);
}

void Builder::findColorOutput(TIntermBlock *root)
{
    for (TIntermNode *node : *root->getSequence())
    {
        TIntermDeclaration *asDecl = node->getAsDeclarationNode();
        if (asDecl == nullptr)
        {
            continue;
        }

        // SeparateDeclarations should have already been run.
        ASSERT(asDecl->getSequence()->size() == 1u);

        TIntermSymbol *symbol = asDecl->getSequence()->front()->getAsSymbolNode();
        if (symbol == nullptr)
        {
            continue;
        }

        const TType &type = symbol->getType();
        if (type.getQualifier() == EvqFragmentOut || type.getQualifier() == EvqFragmentInOut)
        {
            // There can only be one output with advanced blend per spec.
            // If there are multiple outputs, take the one one with location 0.
            if (mOutputVar == nullptr || mOutputVar->getType().getLayoutQualifier().location > 0)
            {
                mOutputVar = &symbol->variable();
            }
        }

        if (IsSubpassInputType(type.getBasicType()) &&
            symbol->getName() != "ANGLEDepthInputAttachment" &&
            symbol->getName() != "ANGLEStencilInputAttachment")
        {
            // There can only be one output with advanced blend, so there can only be a maximum of
            // one subpass input already defined (by framebuffer fetch emulation).
            ASSERT(mSubpassInputVar == nullptr);
            mSubpassInputVar = &symbol->variable();
        }
    }

    // This transformation is only ever called when advanced blend is specified.
    ASSERT(mOutputVar != nullptr);
}

TIntermSymbol *MakeVariable(TSymbolTable *symbolTable, const char *name, const TType *type)
{
    const TVariable *var =
        new TVariable(symbolTable, ImmutableString(name), type, SymbolType::AngleInternal);
    return new TIntermSymbol(var);
}

void Builder::createSubpassInputVar(TIntermBlock *root)
{
    const TPrecision precision = mOutputVar->getType().getPrecision();

    // The input attachment index used for this color attachment would be identical to its location
    // (or implicitly 0 if not specified).
    const unsigned int inputAttachmentIndex =
        std::max(0, mOutputVar->getType().getLayoutQualifier().location);

    // Note that blending can only happen on float/fixed-point output.
    ASSERT(mOutputVar->getType().getBasicType() == EbtFloat);

    // Create the subpass input uniform.
    TType *inputAttachmentType = new TType(EbtSubpassInput, precision, EvqUniform, 1);
    TLayoutQualifier inputAttachmentQualifier     = inputAttachmentType->getLayoutQualifier();
    inputAttachmentQualifier.inputAttachmentIndex = inputAttachmentIndex;
    inputAttachmentType->setLayoutQualifier(inputAttachmentQualifier);

    const char *kSubpassInputName = "ANGLEFragmentInput";
    TIntermSymbol *subpassInputSymbol =
        MakeVariable(mSymbolTable, kSubpassInputName, inputAttachmentType);
    mSubpassInputVar = &subpassInputSymbol->variable();

    // Add its declaration to the shader.
    TIntermDeclaration *subpassInputDecl = new TIntermDeclaration;
    subpassInputDecl->appendDeclarator(subpassInputSymbol);
    root->insertStatement(0, subpassInputDecl);

    mInputAttachmentMap->color[inputAttachmentIndex] = mSubpassInputVar;
}

TIntermTyped *Float(float f)
{
    return CreateFloatNode(f, EbpMedium);
}

TFunction *MakeFunction(TSymbolTable *symbolTable,
                        const char *name,
                        const TType *returnType,
                        const TVector<const TVariable *> &args)
{
    TFunction *function = new TFunction(symbolTable, ImmutableString(name),
                                        SymbolType::AngleInternal, returnType, false);
    for (const TVariable *arg : args)
    {
        function->addParameter(arg);
    }
    return function;
}

TIntermFunctionDefinition *MakeFunctionDefinition(const TFunction *function, TIntermBlock *body)
{
    return new TIntermFunctionDefinition(new TIntermFunctionPrototype(function), body);
}

TIntermFunctionDefinition *MakeSimpleFunctionDefinition(TSymbolTable *symbolTable,
                                                        const char *name,
                                                        TIntermTyped *returnExpression,
                                                        const TVector<TIntermSymbol *> &args)
{
    TVector<const TVariable *> argsAsVar;
    for (TIntermSymbol *arg : args)
    {
        argsAsVar.push_back(&arg->variable());
    }

    TIntermBlock *body = new TIntermBlock;
    body->appendStatement(new TIntermBranch(EOpReturn, returnExpression));

    const TFunction *function =
        MakeFunction(symbolTable, name, &returnExpression->getType(), argsAsVar);
    return MakeFunctionDefinition(function, body);
}

void Builder::generateHslHelperFunctions()
{
    const TPrecision precision = mOutputVar->getType().getPrecision();

    TType *floatType     = new TType(EbtFloat, precision, EvqTemporary, 1);
    TType *vec3Type      = new TType(EbtFloat, precision, EvqTemporary, 3);
    TType *vec3ParamType = new TType(EbtFloat, precision, EvqParamIn, 3);

    // float ANGLE_minv3(vec3 c)
    // {
    //     return min(min(c.r, c.g), c.b);
    // }
    {
        TIntermSymbol *c = MakeVariable(mSymbolTable, "c", vec3ParamType);

        TIntermTyped *cR = new TIntermSwizzle(c, {0});
        TIntermTyped *cG = new TIntermSwizzle(c->deepCopy(), {1});
        TIntermTyped *cB = new TIntermSwizzle(c->deepCopy(), {2});

        // min(c.r, c.g)
        TIntermSequence cRcG = {cR, cG};
        TIntermTyped *minRG  = CreateBuiltInFunctionCallNode("min", &cRcG, *mSymbolTable, 100);

        // min(min(c.r, c.g), c.b)
        TIntermSequence minRGcB = {minRG, cB};
        TIntermTyped *minRGB = CreateBuiltInFunctionCallNode("min", &minRGcB, *mSymbolTable, 100);

        mMinv3 = MakeSimpleFunctionDefinition(mSymbolTable, "ANGLE_minv3", minRGB, {c});
    }

    // float ANGLE_maxv3(vec3 c)
    // {
    //     return max(max(c.r, c.g), c.b);
    // }
    {
        TIntermSymbol *c = MakeVariable(mSymbolTable, "c", vec3ParamType);

        TIntermTyped *cR = new TIntermSwizzle(c, {0});
        TIntermTyped *cG = new TIntermSwizzle(c->deepCopy(), {1});
        TIntermTyped *cB = new TIntermSwizzle(c->deepCopy(), {2});

        // max(c.r, c.g)
        TIntermSequence cRcG = {cR, cG};
        TIntermTyped *maxRG  = CreateBuiltInFunctionCallNode("max", &cRcG, *mSymbolTable, 100);

        // max(max(c.r, c.g), c.b)
        TIntermSequence maxRGcB = {maxRG, cB};
        TIntermTyped *maxRGB = CreateBuiltInFunctionCallNode("max", &maxRGcB, *mSymbolTable, 100);

        mMaxv3 = MakeSimpleFunctionDefinition(mSymbolTable, "ANGLE_maxv3", maxRGB, {c});
    }

    // float ANGLE_lumv3(vec3 c)
    // {
    //     return dot(c, vec3(0.30f, 0.59f, 0.11f));
    // }
    {
        TIntermSymbol *c = MakeVariable(mSymbolTable, "c", vec3ParamType);

        constexpr std::array<float, 3> kCoeff = {0.30f, 0.59f, 0.11f};
        TIntermConstantUnion *coeff           = CreateVecNode(kCoeff.data(), 3, EbpMedium);

        // dot(c, coeff)
        TIntermSequence cCoeff = {c, coeff};
        TIntermTyped *dot      = CreateBuiltInFunctionCallNode("dot", &cCoeff, *mSymbolTable, 100);

        mLumv3 = MakeSimpleFunctionDefinition(mSymbolTable, "ANGLE_lumv3", dot, {c});
    }

    // float ANGLE_satv3(vec3 c)
    // {
    //     return ANGLE_maxv3(c) - ANGLE_minv3(c);
    // }
    {
        TIntermSymbol *c = MakeVariable(mSymbolTable, "c", vec3ParamType);

        // ANGLE_maxv3(c)
        TIntermSequence cMaxArg = {c};
        TIntermTyped *maxv3 =
            TIntermAggregate::CreateFunctionCall(*mMaxv3->getFunction(), &cMaxArg);

        // ANGLE_minv3(c)
        TIntermSequence cMinArg = {c->deepCopy()};
        TIntermTyped *minv3 =
            TIntermAggregate::CreateFunctionCall(*mMinv3->getFunction(), &cMinArg);

        // max - min
        TIntermTyped *diff = new TIntermBinary(EOpSub, maxv3, minv3);

        mSatv3 = MakeSimpleFunctionDefinition(mSymbolTable, "ANGLE_satv3", diff, {c});
    }

    // vec3 ANGLE_clip_color(vec3 color)
    // {
    //     float lum = ANGLE_lumv3(color);
    //     float mincol = ANGLE_minv3(color);
    //     float maxcol = ANGLE_maxv3(color);
    //     if (mincol < 0.0f)
    //     {
    //         color = lum + ((color - lum) * lum) / (lum - mincol);
    //     }
    //     if (maxcol > 1.0f)
    //     {
    //         color = lum + ((color - lum) * (1.0f - lum)) / (maxcol - lum);
    //     }
    //     return color;
    // }
    {
        TIntermSymbol *color  = MakeVariable(mSymbolTable, "color", vec3ParamType);
        TIntermSymbol *lum    = MakeVariable(mSymbolTable, "lum", floatType);
        TIntermSymbol *mincol = MakeVariable(mSymbolTable, "mincol", floatType);
        TIntermSymbol *maxcol = MakeVariable(mSymbolTable, "maxcol", floatType);

        // ANGLE_lumv3(color)
        TIntermSequence cLumArg = {color};
        TIntermTyped *lumv3 =
            TIntermAggregate::CreateFunctionCall(*mLumv3->getFunction(), &cLumArg);

        // ANGLE_minv3(color)
        TIntermSequence cMinArg = {color->deepCopy()};
        TIntermTyped *minv3 =
            TIntermAggregate::CreateFunctionCall(*mMinv3->getFunction(), &cMinArg);

        // ANGLE_maxv3(color)
        TIntermSequence cMaxArg = {color->deepCopy()};
        TIntermTyped *maxv3 =
            TIntermAggregate::CreateFunctionCall(*mMaxv3->getFunction(), &cMaxArg);

        TIntermBlock *body = new TIntermBlock;
        body->appendStatement(CreateTempInitDeclarationNode(&lum->variable(), lumv3));
        body->appendStatement(CreateTempInitDeclarationNode(&mincol->variable(), minv3));
        body->appendStatement(CreateTempInitDeclarationNode(&maxcol->variable(), maxv3));

        // color - lum
        TIntermTyped *colorMinusLum = new TIntermBinary(EOpSub, color->deepCopy(), lum);
        // (color - lum) * lum
        TIntermTyped *colorMinusLumTimesLum =
            new TIntermBinary(EOpVectorTimesScalar, colorMinusLum, lum->deepCopy());
        // lum - mincol
        TIntermTyped *lumMinusMincol = new TIntermBinary(EOpSub, lum->deepCopy(), mincol);
        // ((color - lum) * lum) / (lum - mincol)
        TIntermTyped *negativeMincolLumOffset =
            new TIntermBinary(EOpDiv, colorMinusLumTimesLum, lumMinusMincol);
        // lum + ((color - lum) * lum) / (lum - mincol)
        TIntermTyped *negativeMincolOffset =
            new TIntermBinary(EOpAdd, lum->deepCopy(), negativeMincolLumOffset);
        // color = lum + ((color - lum) * lum) / (lum - mincol)
        TIntermBlock *if1Body = new TIntermBlock;
        if1Body->appendStatement(
            new TIntermBinary(EOpAssign, color->deepCopy(), negativeMincolOffset));

        // mincol < 0.0f
        TIntermTyped *lessZero = new TIntermBinary(EOpLessThan, mincol->deepCopy(), Float(0));
        // if (mincol < 0.0f) ...
        body->appendStatement(new TIntermIfElse(lessZero, if1Body, nullptr));

        // 1.0f - lum
        TIntermTyped *oneMinusLum = new TIntermBinary(EOpSub, Float(1.0f), lum->deepCopy());
        // (color - lum) * (1.0f - lum)
        TIntermTyped *colorMinusLumTimesOneMinusLum =
            new TIntermBinary(EOpVectorTimesScalar, colorMinusLum->deepCopy(), oneMinusLum);
        // maxcol - lum
        TIntermTyped *maxcolMinusLum = new TIntermBinary(EOpSub, maxcol, lum->deepCopy());
        // (color - lum) * (1.0f - lum) / (maxcol - lum)
        TIntermTyped *largeMaxcolLumOffset =
            new TIntermBinary(EOpDiv, colorMinusLumTimesOneMinusLum, maxcolMinusLum);
        // lum + (color - lum) * (1.0f - lum) / (maxcol - lum)
        TIntermTyped *largeMaxcolOffset =
            new TIntermBinary(EOpAdd, lum->deepCopy(), largeMaxcolLumOffset);
        // color = lum + (color - lum) * (1.0f - lum) / (maxcol - lum)
        TIntermBlock *if2Body = new TIntermBlock;
        if2Body->appendStatement(
            new TIntermBinary(EOpAssign, color->deepCopy(), largeMaxcolOffset));

        // maxcol > 1.0f
        TIntermTyped *largerOne = new TIntermBinary(EOpGreaterThan, maxcol->deepCopy(), Float(1));
        // if (maxcol > 1.0f) ...
        body->appendStatement(new TIntermIfElse(largerOne, if2Body, nullptr));

        body->appendStatement(new TIntermBranch(EOpReturn, color->deepCopy()));

        const TFunction *function =
            MakeFunction(mSymbolTable, "ANGLE_clip_color", vec3Type, {&color->variable()});
        mClipColor = MakeFunctionDefinition(function, body);
    }

    // vec3 ANGLE_set_lum(vec3 cbase, vec3 clum)
    // {
    //     float lbase = ANGLE_lumv3(cbase);
    //     float llum = ANGLE_lumv3(clum);
    //     float ldiff = llum - lbase;
    //     vec3 color = cbase + ldiff;
    //     return ANGLE_clip_color(color);
    // }
    {
        TIntermSymbol *cbase = MakeVariable(mSymbolTable, "cbase", vec3ParamType);
        TIntermSymbol *clum  = MakeVariable(mSymbolTable, "clum", vec3ParamType);

        // ANGLE_lumv3(cbase)
        TIntermSequence cbaseArg = {cbase};
        TIntermTyped *lbase =
            TIntermAggregate::CreateFunctionCall(*mLumv3->getFunction(), &cbaseArg);

        // ANGLE_lumv3(clum)
        TIntermSequence clumArg = {clum};
        TIntermTyped *llum = TIntermAggregate::CreateFunctionCall(*mLumv3->getFunction(), &clumArg);

        // llum - lbase
        TIntermTyped *ldiff = new TIntermBinary(EOpSub, llum, lbase);
        // cbase + ldiff
        TIntermTyped *color = new TIntermBinary(EOpAdd, cbase->deepCopy(), ldiff);
        // ANGLE_clip_color(color);
        TIntermSequence clipColorArg = {color};
        TIntermTyped *result =
            TIntermAggregate::CreateFunctionCall(*mClipColor->getFunction(), &clipColorArg);

        TIntermBlock *body = new TIntermBlock;
        body->appendStatement(new TIntermBranch(EOpReturn, result));

        const TFunction *function = MakeFunction(mSymbolTable, "ANGLE_set_lum", vec3Type,
                                                 {&cbase->variable(), &clum->variable()});
        mSetLum                   = MakeFunctionDefinition(function, body);
    }

    // vec3 ANGLE_set_lum_sat(vec3 cbase, vec3 csat, vec3 clum)
    // {
    //     float minbase = ANGLE_minv3(cbase);
    //     float sbase = ANGLE_satv3(cbase);
    //     float ssat = ANGLE_satv3(csat);
    //     vec3 color;
    //     if (sbase > 0.0f)
    //     {
    //         color = (cbase - minbase) * ssat / sbase;
    //     }
    //     else
    //     {
    //         color = vec3(0.0f);
    //     }
    //     return ANGLE_set_lum(color, clum);
    // }
    {
        TIntermSymbol *cbase   = MakeVariable(mSymbolTable, "cbase", vec3ParamType);
        TIntermSymbol *csat    = MakeVariable(mSymbolTable, "csat", vec3ParamType);
        TIntermSymbol *clum    = MakeVariable(mSymbolTable, "clum", vec3ParamType);
        TIntermSymbol *minbase = MakeVariable(mSymbolTable, "minbase", floatType);
        TIntermSymbol *sbase   = MakeVariable(mSymbolTable, "sbase", floatType);
        TIntermSymbol *ssat    = MakeVariable(mSymbolTable, "ssat", floatType);

        // ANGLE_minv3(cbase)
        TIntermSequence cMinArg = {cbase};
        TIntermTyped *minv3 =
            TIntermAggregate::CreateFunctionCall(*mMinv3->getFunction(), &cMinArg);

        // ANGLE_satv3(cbase)
        TIntermSequence cSatArg = {cbase->deepCopy()};
        TIntermTyped *baseSatv3 =
            TIntermAggregate::CreateFunctionCall(*mSatv3->getFunction(), &cSatArg);

        // ANGLE_satv3(csat)
        TIntermSequence sSatArg = {csat};
        TIntermTyped *satSatv3 =
            TIntermAggregate::CreateFunctionCall(*mSatv3->getFunction(), &sSatArg);

        TIntermBlock *body = new TIntermBlock;
        body->appendStatement(CreateTempInitDeclarationNode(&minbase->variable(), minv3));
        body->appendStatement(CreateTempInitDeclarationNode(&sbase->variable(), baseSatv3));
        body->appendStatement(CreateTempInitDeclarationNode(&ssat->variable(), satSatv3));

        // cbase - minbase
        TIntermTyped *cbaseMinusMinbase = new TIntermBinary(EOpSub, cbase->deepCopy(), minbase);
        // (cbase - minbase) * ssat
        TIntermTyped *cbaseMinusMinbaseTimesSsat =
            new TIntermBinary(EOpVectorTimesScalar, cbaseMinusMinbase, ssat);
        // (cbase - minbase) * ssat / sbase
        TIntermTyped *colorSbaseGreaterZero =
            new TIntermBinary(EOpDiv, cbaseMinusMinbaseTimesSsat, sbase);

        // sbase > 0.0f
        TIntermTyped *greaterZero = new TIntermBinary(EOpGreaterThan, sbase->deepCopy(), Float(0));

        // sbase > 0.0f ? (cbase - minbase) * ssat / sbase : vec3(0.0)
        TIntermTyped *color =
            new TIntermTernary(greaterZero, colorSbaseGreaterZero, CreateZeroNode(*vec3Type));

        // ANGLE_set_lum(color);
        TIntermSequence setLumArg = {color, clum};
        TIntermTyped *result =
            TIntermAggregate::CreateFunctionCall(*mSetLum->getFunction(), &setLumArg);

        body->appendStatement(new TIntermBranch(EOpReturn, result));

        const TFunction *function =
            MakeFunction(mSymbolTable, "ANGLE_set_lum_sat", vec3Type,
                         {&cbase->variable(), &csat->variable(), &clum->variable()});
        mSetLumSat = MakeFunctionDefinition(function, body);
    }
}

void Builder::generateBlendFunctions()
{
    const TPrecision precision = mOutputVar->getType().getPrecision();

    TType *floatParamType = new TType(EbtFloat, precision, EvqParamIn, 1);
    TType *vec3ParamType  = new TType(EbtFloat, precision, EvqParamIn, 3);

    gl::BlendEquationBitSet enabledBlendEquations(mAdvancedBlendEquations.bits());
    for (gl::BlendEquationType equation : enabledBlendEquations)
    {
        switch (equation)
        {
            case gl::BlendEquationType::Multiply:
                // float ANGLE_blend_multiply(float src, float dst)
                // {
                //     return src * dst;
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // src * dst
                    TIntermTyped *result = new TIntermBinary(EOpMul, src, dst);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_multiply", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::Screen:
                // float ANGLE_blend_screen(float src, float dst)
                // {
                //     return src + dst - src * dst;
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // src + dst
                    TIntermTyped *sum = new TIntermBinary(EOpAdd, src, dst);
                    // src * dst
                    TIntermTyped *mul = new TIntermBinary(EOpMul, src->deepCopy(), dst->deepCopy());
                    // src + dst - src * dst
                    TIntermTyped *result = new TIntermBinary(EOpSub, sum, mul);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_screen", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::Overlay:
            case gl::BlendEquationType::Hardlight:
                // float ANGLE_blend_overlay(float src, float dst)
                // {
                //     if (dst <= 0.5f)
                //     {
                //         return (2.0f * src * dst);
                //     }
                //     else
                //     {
                //         return (1.0f - 2.0f * (1.0f - src) * (1.0f - dst));
                //     }
                //
                //     // Equivalently generated as:
                //     // return dst <= 0.5f ? 2.*src*dst : 2.*(src+dst) - 2.*src*dst - 1.;
                // }
                //
                // float ANGLE_blend_hardlight(float src, float dst)
                // {
                //     // Same as overlay, with the |if| checking |src| instead of |dst|.
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // src + dst
                    TIntermTyped *sum = new TIntermBinary(EOpAdd, src, dst);
                    // 2 * (src + dst)
                    TIntermTyped *sum2 = new TIntermBinary(EOpMul, sum, Float(2));
                    // src * dst
                    TIntermTyped *mul = new TIntermBinary(EOpMul, src->deepCopy(), dst->deepCopy());
                    // 2 * src * dst
                    TIntermTyped *mul2 = new TIntermBinary(EOpMul, mul, Float(2));
                    // 2 * (src + dst) - 2 * src * dst
                    TIntermTyped *sum2MinusMul2 = new TIntermBinary(EOpSub, sum2, mul2);
                    // 2 * (src + dst) - 2 * src * dst - 1
                    TIntermTyped *sum2MinusMul2Minus1 =
                        new TIntermBinary(EOpSub, sum2MinusMul2, Float(1));

                    // dst[src] <= 0.5
                    TIntermSymbol *conditionSymbol =
                        equation == gl::BlendEquationType::Overlay ? dst : src;
                    TIntermTyped *lessHalf = new TIntermBinary(
                        EOpLessThanEqual, conditionSymbol->deepCopy(), Float(0.5));
                    // dst[src] <= 0.5f ? ...
                    TIntermTyped *result =
                        new TIntermTernary(lessHalf, mul2->deepCopy(), sum2MinusMul2Minus1);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable,
                        equation == gl::BlendEquationType::Overlay ? "ANGLE_blend_overlay"
                                                                   : "ANGLE_blend_hardlight",
                        result, {src, dst});
                }
                break;
            case gl::BlendEquationType::Darken:
                // float ANGLE_blend_darken(float src, float dst)
                // {
                //     return min(src, dst);
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // src * dst
                    TIntermSequence minArgs = {src, dst};
                    TIntermTyped *result =
                        CreateBuiltInFunctionCallNode("min", &minArgs, *mSymbolTable, 100);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_darken", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::Lighten:
                // float ANGLE_blend_lighten(float src, float dst)
                // {
                //     return max(src, dst);
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // src * dst
                    TIntermSequence maxArgs = {src, dst};
                    TIntermTyped *result =
                        CreateBuiltInFunctionCallNode("max", &maxArgs, *mSymbolTable, 100);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_lighten", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::Colordodge:
                // float ANGLE_blend_dodge(float src, float dst)
                // {
                //     if (dst <= 0.0f)
                //     {
                //         return 0.0;
                //     }
                //     else if (src >= 1.0f)   // dst > 0.0
                //     {
                //         return 1.0;
                //     }
                //     else                    // dst > 0.0 && src < 1.0
                //     {
                //         return min(1.0, dst / (1.0 - src));
                //     }
                //
                //     // Equivalently generated as:
                //     // return dst <= 0. ? 0. : src >= 1. ? 1. : min(1., dst / (1. - src));
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // 1. - src
                    TIntermTyped *oneMinusSrc = new TIntermBinary(EOpSub, Float(1), src);
                    // dst / (1. - src)
                    TIntermTyped *dstDivOneMinusSrc = new TIntermBinary(EOpDiv, dst, oneMinusSrc);
                    // min(1., dst / (1. - src))
                    TIntermSequence minArgs = {Float(1), dstDivOneMinusSrc};
                    TIntermTyped *result =
                        CreateBuiltInFunctionCallNode("min", &minArgs, *mSymbolTable, 100);

                    // src >= 1
                    TIntermTyped *greaterOne =
                        new TIntermBinary(EOpGreaterThanEqual, src->deepCopy(), Float(1));
                    // src >= 1. ? ...
                    result = new TIntermTernary(greaterOne, Float(1), result);

                    // dst <= 0
                    TIntermTyped *lessZero =
                        new TIntermBinary(EOpLessThanEqual, dst->deepCopy(), Float(0));
                    // dst <= 0. ? ...
                    result = new TIntermTernary(lessZero, Float(0), result);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_dodge", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::Colorburn:
                // float ANGLE_blend_burn(float src, float dst)
                // {
                //     if (dst >= 1.0f)
                //     {
                //         return 1.0;
                //     }
                //     else if (src <= 0.0f)   // dst < 1.0
                //     {
                //         return 0.0;
                //     }
                //     else                    // dst < 1.0 && src > 0.0
                //     {
                //         return 1.0f - min(1.0f, (1.0f - dst) / src);
                //     }
                //
                //     // Equivalently generated as:
                //     // return dst >= 1. ? 1. : src <= 0. ? 0. : 1. - min(1., (1. - dst) / src);
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // 1. - dst
                    TIntermTyped *oneMinusDst = new TIntermBinary(EOpSub, Float(1), dst);
                    // (1. - dst) / src
                    TIntermTyped *oneMinusDstDivSrc = new TIntermBinary(EOpDiv, oneMinusDst, src);
                    // min(1., (1. - dst) / src)
                    TIntermSequence minArgs = {Float(1), oneMinusDstDivSrc};
                    TIntermTyped *result =
                        CreateBuiltInFunctionCallNode("min", &minArgs, *mSymbolTable, 100);
                    // 1. - min(1., (1. - dst) / src)
                    result = new TIntermBinary(EOpSub, Float(1), result);

                    // src <= 0
                    TIntermTyped *lessZero =
                        new TIntermBinary(EOpLessThanEqual, src->deepCopy(), Float(0));
                    // src <= 0. ? ...
                    result = new TIntermTernary(lessZero, Float(0), result);

                    // dst >= 1
                    TIntermTyped *greaterOne =
                        new TIntermBinary(EOpGreaterThanEqual, dst->deepCopy(), Float(1));
                    // dst >= 1. ? ...
                    result = new TIntermTernary(greaterOne, Float(1), result);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_burn", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::Softlight:
                // float ANGLE_blend_softlight(float src, float dst)
                // {
                //     if (src <= 0.5f)
                //     {
                //         return (dst - (1.0f - 2.0f * src) * dst * (1.0f - dst));
                //     }
                //     else if (dst <= 0.25f)  // src > 0.5
                //     {
                //         return (dst + (2.0f * src - 1.0f) * dst * ((16.0f * dst - 12.0f) * dst
                //         + 3.0f));
                //     }
                //     else                    // src > 0.5 && dst > 0.25
                //     {
                //         return (dst + (2.0f * src - 1.0f) * (sqrt(dst) - dst));
                //     }
                //
                //     // Equivalently generated as:
                //     // return dst + (2. * src - 1.) * (
                //     //            src <= 0.5  ? dst * (1. - dst) :
                //     //            dst <= 0.25 ? dst * ((16. * dst - 12.) * dst + 3.) :
                //     //                          sqrt(dst) - dst)
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // 2. * src
                    TIntermTyped *src2 = new TIntermBinary(EOpMul, Float(2), src);
                    // 2. * src - 1.
                    TIntermTyped *src2Minus1 = new TIntermBinary(EOpSub, src2, Float(1));
                    // 1. - dst
                    TIntermTyped *oneMinusDst = new TIntermBinary(EOpSub, Float(1), dst);
                    // dst * (1. - dst)
                    TIntermTyped *dstTimesOneMinusDst =
                        new TIntermBinary(EOpMul, dst->deepCopy(), oneMinusDst);
                    // 16. * dst
                    TIntermTyped *dst16 = new TIntermBinary(EOpMul, Float(16), dst->deepCopy());
                    // 16. * dst - 12.
                    TIntermTyped *dst16Minus12 = new TIntermBinary(EOpSub, dst16, Float(12));
                    // (16. * dst - 12.) * dst
                    TIntermTyped *dst16Minus12TimesDst =
                        new TIntermBinary(EOpMul, dst16Minus12, dst->deepCopy());
                    // (16. * dst - 12.) * dst + 3.
                    TIntermTyped *dst16Minus12TimesDstPlus3 =
                        new TIntermBinary(EOpAdd, dst16Minus12TimesDst, Float(3));
                    // dst * ((16. * dst - 12.) * dst + 3.)
                    TIntermTyped *dstTimesDst16Minus12TimesDstPlus3 =
                        new TIntermBinary(EOpMul, dst->deepCopy(), dst16Minus12TimesDstPlus3);
                    // sqrt(dst)
                    TIntermSequence sqrtArg = {dst->deepCopy()};
                    TIntermTyped *sqrtDst =
                        CreateBuiltInFunctionCallNode("sqrt", &sqrtArg, *mSymbolTable, 100);
                    // sqrt(dst) - dst
                    TIntermTyped *sqrtDstMinusDst =
                        new TIntermBinary(EOpSub, sqrtDst, dst->deepCopy());

                    // dst <= 0.25
                    TIntermTyped *lessQuarter =
                        new TIntermBinary(EOpLessThanEqual, dst->deepCopy(), Float(0.25));
                    // dst <= 0.25 ? ...
                    TIntermTyped *result = new TIntermTernary(
                        lessQuarter, dstTimesDst16Minus12TimesDstPlus3, sqrtDstMinusDst);

                    // src <= 0.5
                    TIntermTyped *lessHalf =
                        new TIntermBinary(EOpLessThanEqual, src->deepCopy(), Float(0.5));
                    // src <= 0.5 ? ...
                    result = new TIntermTernary(lessHalf, dstTimesOneMinusDst, result);

                    // (2. * src - 1.) * ...
                    result = new TIntermBinary(EOpMul, src2Minus1, result);
                    // dst + (2. * src - 1.) * ...
                    result = new TIntermBinary(EOpAdd, dst->deepCopy(), result);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_softlight", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::Difference:
                // float ANGLE_blend_difference(float src, float dst)
                // {
                //     return abs(dst - src);
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // dst - src
                    TIntermTyped *dstMinusSrc = new TIntermBinary(EOpSub, dst, src);
                    // abs(dst - src)
                    TIntermSequence absArgs = {dstMinusSrc};
                    TIntermTyped *result =
                        CreateBuiltInFunctionCallNode("abs", &absArgs, *mSymbolTable, 100);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_difference", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::Exclusion:
                // float ANGLE_blend_exclusion(float src, float dst)
                // {
                //     return src + dst - (2.0f * src * dst);
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // src + dst
                    TIntermTyped *sum = new TIntermBinary(EOpAdd, src, dst);
                    // src * dst
                    TIntermTyped *mul = new TIntermBinary(EOpMul, src->deepCopy(), dst->deepCopy());
                    // 2 * src * dst
                    TIntermTyped *mul2 = new TIntermBinary(EOpMul, mul, Float(2));
                    // src + dst - 2 * src * dst
                    TIntermTyped *result = new TIntermBinary(EOpSub, sum, mul2);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_exclusion", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::HslHue:
                // vec3 ANGLE_blend_hsl_hue(vec3 src, vec3 dst)
                // {
                //     return ANGLE_set_lum_sat(src, dst, dst);
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", vec3ParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", vec3ParamType);

                    TIntermSequence args = {src, dst, dst->deepCopy()};
                    TIntermTyped *result =
                        TIntermAggregate::CreateFunctionCall(*mSetLumSat->getFunction(), &args);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_hsl_hue", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::HslSaturation:
                // vec3 ANGLE_blend_hsl_saturation(vec3 src, vec3 dst)
                // {
                //     return ANGLE_set_lum_sat(dst, src, dst);
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", vec3ParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", vec3ParamType);

                    TIntermSequence args = {dst, src, dst->deepCopy()};
                    TIntermTyped *result =
                        TIntermAggregate::CreateFunctionCall(*mSetLumSat->getFunction(), &args);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_hsl_saturation", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::HslColor:
                // vec3 ANGLE_blend_hsl_color(vec3 src, vec3 dst)
                // {
                //     return ANGLE_set_lum(src, dst);
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", vec3ParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", vec3ParamType);

                    TIntermSequence args = {src, dst};
                    TIntermTyped *result =
                        TIntermAggregate::CreateFunctionCall(*mSetLum->getFunction(), &args);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_hsl_color", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::HslLuminosity:
                // vec3 ANGLE_blend_hsl_luminosity(vec3 src, vec3 dst)
                // {
                //     return ANGLE_set_lum(dst, src);
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", vec3ParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", vec3ParamType);

                    TIntermSequence args = {dst, src};
                    TIntermTyped *result =
                        TIntermAggregate::CreateFunctionCall(*mSetLum->getFunction(), &args);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_hsl_luminosity", result, {src, dst});
                }
                break;
            default:
                // Only advanced blend equations are possible.
                UNREACHABLE();
        }
    }
}

void Builder::insertGeneratedFunctions(TIntermBlock *root)
{
    // Insert all generated functions in root.  Since they are all inserted at index 0, HSL helpers
    // are inserted last, and in opposite order.
    for (TIntermFunctionDefinition *blendFunc : mBlendFuncs)
    {
        if (blendFunc != nullptr)
        {
            root->insertStatement(0, blendFunc);
        }
    }
    if (mMinv3 != nullptr)
    {
        root->insertStatement(0, mSetLumSat);
        root->insertStatement(0, mSetLum);
        root->insertStatement(0, mClipColor);
        root->insertStatement(0, mSatv3);
        root->insertStatement(0, mLumv3);
        root->insertStatement(0, mMaxv3);
        root->insertStatement(0, mMinv3);
    }
}

// On some platforms 1.0f is not returned even when the dividend and divisor have the same value.
// In such cases emit 1.0f when the dividend and divisor are equal, else return the divide node
TIntermTyped *Builder::divideFloatNode(TIntermTyped *dividend, TIntermTyped *divisor)
{
    TIntermBinary *cond = new TIntermBinary(EOpEqual, dividend->deepCopy(), divisor->deepCopy());
    TIntermBinary *divideExpr =
        new TIntermBinary(EOpDiv, dividend->deepCopy(), divisor->deepCopy());
    return new TIntermTernary(cond, CreateFloatNode(1.0f, EbpHigh), divideExpr->deepCopy());
}

TIntermSymbol *Builder::premultiplyAlpha(TIntermBlock *blendBlock,
                                         TIntermTyped *var,
                                         const char *name)
{
    const TPrecision precision = mOutputVar->getType().getPrecision();
    TType *vec3Type            = new TType(EbtFloat, precision, EvqTemporary, 3);

    // symbol = vec3(0)
    // If alpha != 0, divide by alpha.  Note that due to precision issues, component == alpha is
    // handled especially.  This precision issue affects multiple vendors, and most drivers seem to
    // be carrying a similar workaround to pass the CTS test.
    TIntermTyped *alpha            = new TIntermSwizzle(var, {3});
    TIntermSymbol *symbol          = MakeVariable(mSymbolTable, name, vec3Type);
    TIntermTyped *alphaNotZero     = new TIntermBinary(EOpNotEqual, alpha, Float(0));
    TIntermBlock *rgbDivAlphaBlock = new TIntermBlock;

    constexpr int kColorChannels = 3;
    // For each component:
    // symbol.x = (var.x == var.w) ? 1.0 : var.x / var.w
    for (int index = 0; index < kColorChannels; index++)
    {
        TIntermTyped *divideNode        = divideFloatNode(new TIntermSwizzle(var, {index}), alpha);
        TIntermBinary *assignDivideNode = new TIntermBinary(
            EOpAssign, new TIntermSwizzle(symbol->deepCopy(), {index}), divideNode);
        rgbDivAlphaBlock->appendStatement(assignDivideNode);
    }

    TIntermIfElse *ifBlock = new TIntermIfElse(alphaNotZero, rgbDivAlphaBlock, nullptr);
    blendBlock->appendStatement(
        CreateTempInitDeclarationNode(&symbol->variable(), CreateZeroNode(*vec3Type)));
    blendBlock->appendStatement(ifBlock);

    return symbol;
}

TIntermTyped *GetFirstElementIfArray(TIntermTyped *var)
{
    TIntermTyped *element = var;
    while (element->getType().isArray())
    {
        element = new TIntermBinary(EOpIndexDirect, element, CreateIndexNode(0));
    }
    return element;
}

void Builder::generatePreamble(TIntermBlock *blendBlock)
{
    // Use subpassLoad to read from the input attachment
    const TPrecision precision      = mOutputVar->getType().getPrecision();
    TType *vec4Type                 = new TType(EbtFloat, precision, EvqTemporary, 4);
    TIntermSymbol *subpassInputData = MakeVariable(mSymbolTable, "ANGLELastFragData", vec4Type);

    // Initialize it with subpassLoad() result.
    TIntermSequence subpassArguments  = {new TIntermSymbol(mSubpassInputVar)};
    TIntermTyped *subpassLoadFuncCall = CreateBuiltInFunctionCallNode(
        "subpassLoad", &subpassArguments, *mSymbolTable, kESSLInternalBackendBuiltIns);

    blendBlock->appendStatement(
        CreateTempInitDeclarationNode(&subpassInputData->variable(), subpassLoadFuncCall));

    // Get element 0 of the output, if array.
    TIntermTyped *output = GetFirstElementIfArray(new TIntermSymbol(mOutputVar));

    // Expand output to vec4, if not already.
    uint32_t vecSize = mOutputVar->getType().getNominalSize();
    if (vecSize < 4)
    {
        TIntermSequence vec4Args = {output};
        for (uint32_t channel = vecSize; channel < 3; ++channel)
        {
            vec4Args.push_back(Float(0));
        }
        vec4Args.push_back(Float(1));
        output = TIntermAggregate::CreateConstructor(*vec4Type, &vec4Args);
    }

    // Premultiply src and dst.
    mSrc = premultiplyAlpha(blendBlock, output, "ANGLE_blend_src");
    mDst = premultiplyAlpha(blendBlock, subpassInputData, "ANGLE_blend_dst");

    // Calculate the p coefficients:
    TIntermTyped *srcAlpha = new TIntermSwizzle(output->deepCopy(), {3});
    TIntermTyped *dstAlpha = new TIntermSwizzle(subpassInputData->deepCopy(), {3});

    // As * Ad
    TIntermTyped *AsTimesAd = new TIntermBinary(EOpMul, srcAlpha, dstAlpha);
    // As * (1. - Ad)
    TIntermTyped *oneMinusAd        = new TIntermBinary(EOpSub, Float(1), dstAlpha->deepCopy());
    TIntermTyped *AsTimesOneMinusAd = new TIntermBinary(EOpMul, srcAlpha->deepCopy(), oneMinusAd);
    // Ad * (1. - As)
    TIntermTyped *oneMinusAs        = new TIntermBinary(EOpSub, Float(1), srcAlpha->deepCopy());
    TIntermTyped *AdTimesOneMinusAs = new TIntermBinary(EOpMul, dstAlpha->deepCopy(), oneMinusAs);

    mP0 = MakeVariable(mSymbolTable, "ANGLE_blend_p0", &srcAlpha->getType());
    mP1 = MakeVariable(mSymbolTable, "ANGLE_blend_p1", &srcAlpha->getType());
    mP2 = MakeVariable(mSymbolTable, "ANGLE_blend_p2", &srcAlpha->getType());

    blendBlock->appendStatement(CreateTempInitDeclarationNode(&mP0->variable(), AsTimesAd));
    blendBlock->appendStatement(CreateTempInitDeclarationNode(&mP1->variable(), AsTimesOneMinusAd));
    blendBlock->appendStatement(CreateTempInitDeclarationNode(&mP2->variable(), AdTimesOneMinusAs));
}

void Builder::generateEquationSwitch(TIntermBlock *blendBlock)
{
    const TPrecision precision = mOutputVar->getType().getPrecision();

    TType *vec3Type = new TType(EbtFloat, precision, EvqTemporary, 3);
    TType *vec4Type = new TType(EbtFloat, precision, EvqTemporary, 4);

    // The following code is generated:
    //
    // vec3 f;
    // swtich (equation)
    // {
    //    case A:
    //       f = ANGLE_blend_a(..);
    //       break;
    //    case B:
    //       f = ANGLE_blend_b(..);
    //       break;
    //    ...
    // }
    //
    // vec3 rgb = f * p0 + src * p1 + dst * p2
    // float a = p0 + p1 + p2
    //
    // output = vec4(rgb, a);

    TIntermSymbol *f = MakeVariable(mSymbolTable, "ANGLE_f", vec3Type);
    blendBlock->appendStatement(CreateTempDeclarationNode(&f->variable()));

    TIntermBlock *switchBody = new TIntermBlock;

    gl::BlendEquationBitSet enabledBlendEquations(mAdvancedBlendEquations.bits());
    for (gl::BlendEquationType equation : enabledBlendEquations)
    {
        switchBody->appendStatement(
            new TIntermCase(CreateUIntNode(static_cast<uint32_t>(equation))));

        // HSL equations call the blend function with all channels.  Non-HSL equations call it per
        // component.
        if (equation < gl::BlendEquationType::HslHue)
        {
            TIntermSequence constructorArgs;
            for (int channel = 0; channel < 3; ++channel)
            {
                TIntermTyped *srcChannel = new TIntermSwizzle(mSrc->deepCopy(), {channel});
                TIntermTyped *dstChannel = new TIntermSwizzle(mDst->deepCopy(), {channel});

                TIntermSequence args = {srcChannel, dstChannel};
                constructorArgs.push_back(TIntermAggregate::CreateFunctionCall(
                    *mBlendFuncs[equation]->getFunction(), &args));
            }

            TIntermTyped *constructor =
                TIntermAggregate::CreateConstructor(*vec3Type, &constructorArgs);
            switchBody->appendStatement(new TIntermBinary(EOpAssign, f->deepCopy(), constructor));
        }
        else
        {
            TIntermSequence args = {mSrc->deepCopy(), mDst->deepCopy()};
            TIntermTyped *blendCall =
                TIntermAggregate::CreateFunctionCall(*mBlendFuncs[equation]->getFunction(), &args);

            switchBody->appendStatement(new TIntermBinary(EOpAssign, f->deepCopy(), blendCall));
        }

        switchBody->appendStatement(new TIntermBranch(EOpBreak, nullptr));
    }

    // A driver uniform is used to communicate the blend equation to use.
    TIntermTyped *equationUniform = mDriverUniforms->getAdvancedBlendEquation();

    blendBlock->appendStatement(new TIntermSwitch(equationUniform, switchBody));

    // Calculate the final blend according to the following formula:
    //
    //     RGB = f(src, dst) * p0 + src * p1 + dst * p2
    //       A = p0 + p1 + p2

    // f * p0
    TIntermTyped *fTimesP0 = new TIntermBinary(EOpVectorTimesScalar, f, mP0);
    // src * p1
    TIntermTyped *srcTimesP1 = new TIntermBinary(EOpVectorTimesScalar, mSrc, mP1);
    // dst * p2
    TIntermTyped *dstTimesP2 = new TIntermBinary(EOpVectorTimesScalar, mDst, mP2);
    // f * p0 + src * p1 + dst * p2
    TIntermTyped *rgb =
        new TIntermBinary(EOpAdd, new TIntermBinary(EOpAdd, fTimesP0, srcTimesP1), dstTimesP2);

    // p0 + p1 + p2
    TIntermTyped *a = new TIntermBinary(
        EOpAdd, new TIntermBinary(EOpAdd, mP0->deepCopy(), mP1->deepCopy()), mP2->deepCopy());

    // Intialize the output with vec4(RGB, A)
    TIntermSequence rgbaArgs  = {rgb, a};
    TIntermTyped *blendResult = TIntermAggregate::CreateConstructor(*vec4Type, &rgbaArgs);

    // If the output has fewer than four channels, swizzle the results
    uint32_t vecSize = mOutputVar->getType().getNominalSize();
    if (vecSize < 4)
    {
        TVector<int> swizzle = {0, 1, 2, 3};
        swizzle.resize(vecSize);
        blendResult = new TIntermSwizzle(blendResult, swizzle);
    }

    TIntermTyped *output = GetFirstElementIfArray(new TIntermSymbol(mOutputVar));

    blendBlock->appendStatement(new TIntermBinary(EOpAssign, output, blendResult));
}
}  // anonymous namespace

bool EmulateAdvancedBlendEquations(TCompiler *compiler,
                                   TIntermBlock *root,
                                   TSymbolTable *symbolTable,
                                   const AdvancedBlendEquations &advancedBlendEquations,
                                   const DriverUniform *driverUniforms,
                                   InputAttachmentMap *inputAttachmentMapOut)
{
    Builder builder(compiler, symbolTable, advancedBlendEquations, driverUniforms,
                    inputAttachmentMapOut);
    return builder.build(root);
}  // namespace

}  // namespace sh
