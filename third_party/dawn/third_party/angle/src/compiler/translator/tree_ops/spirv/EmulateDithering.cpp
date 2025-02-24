//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EmulateDithering: Adds dithering code to fragment shader outputs based on a specialization
// constant control value.
//

#include "compiler/translator/tree_ops/spirv/EmulateDithering.h"

#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/DriverUniform.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/RunAtTheEndOfShader.h"
#include "compiler/translator/tree_util/SpecializationConstant.h"

namespace sh
{
namespace
{
using FragmentOutputVariableList = TVector<const TVariable *>;

void GatherFragmentOutputs(TIntermBlock *root,
                           FragmentOutputVariableList *fragmentOutputVariablesOut)
{
    TIntermSequence &sequence = *root->getSequence();

    for (TIntermNode *node : sequence)
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
        if (type.getQualifier() == EvqFragmentOut)
        {
            fragmentOutputVariablesOut->push_back(&symbol->variable());
        }
    }
}

TIntermTyped *CreateDitherValue(const TType &type, TIntermSequence *ditherValueElements)
{
    uint8_t channelCount = type.getNominalSize();
    if (channelCount == 1)
    {
        return ditherValueElements->at(0)->getAsTyped();
    }

    if (ditherValueElements->size() > channelCount)
    {
        ditherValueElements->resize(channelCount);
    }
    return TIntermAggregate::CreateConstructor(type, ditherValueElements);
}

void EmitFragmentOutputDither(TCompiler *compiler,
                              const ShCompileOptions &compileOptions,
                              TSymbolTable *symbolTable,
                              TIntermBlock *ditherBlock,
                              TIntermTyped *ditherControl,
                              TIntermTyped *ditherParam,
                              TIntermTyped *fragmentOutput,
                              uint32_t location)
{
    bool roundOutputAfterDithering = compileOptions.roundOutputAfterDithering;

    // dither >> 2*location
    TIntermBinary *ditherControlShifted = new TIntermBinary(
        EOpBitShiftRight, ditherControl->deepCopy(), CreateUIntNode(location * 2));

    // (dither >> 2*location) & 3
    TIntermBinary *thisDitherControlValue =
        new TIntermBinary(EOpBitwiseAnd, ditherControlShifted, CreateUIntNode(3));

    // const uint dither_i = (dither >> 2*location) & 3
    TIntermSymbol *thisDitherControl = new TIntermSymbol(
        CreateTempVariable(symbolTable, StaticType::GetBasic<EbtUInt, EbpHigh>()));
    TIntermDeclaration *thisDitherControlDecl =
        CreateTempInitDeclarationNode(&thisDitherControl->variable(), thisDitherControlValue);
    ditherBlock->appendStatement(thisDitherControlDecl);

    // The comments below assume the output is vec4, but the code handles float, vec2 and vec3
    // outputs.
    const TType &type          = fragmentOutput->getType();
    const uint8_t channelCount = std::min<uint8_t>(type.getNominalSize(), 3u);
    TType *outputType          = new TType(EbtFloat, EbpMedium, EvqTemporary, channelCount);

    // vec3 ditherValue = vec3(0)
    TIntermSymbol *ditherValue = new TIntermSymbol(CreateTempVariable(symbolTable, outputType));
    TIntermDeclaration *ditherValueDecl =
        CreateTempInitDeclarationNode(&ditherValue->variable(), CreateZeroNode(*outputType));
    ditherBlock->appendStatement(ditherValueDecl);

    // If a workaround is enabled, the bit-range of each channel is also tracked, so a round() can
    // be applied.
    TIntermSymbol *roundMultiplier = nullptr;
    if (roundOutputAfterDithering)
    {
        roundMultiplier = new TIntermSymbol(
            CreateTempVariable(symbolTable, StaticType::GetBasic<EbtFloat, EbpMedium, 3>()));

        constexpr std::array<float, 3> kDefaultMultiplier = {255, 255, 255};
        TIntermConstantUnion *defaultMultiplier =
            CreateVecNode(kDefaultMultiplier.data(), 3, EbpMedium);

        TIntermDeclaration *roundMultiplierDecl =
            CreateTempInitDeclarationNode(&roundMultiplier->variable(), defaultMultiplier);
        ditherBlock->appendStatement(roundMultiplierDecl);
    }

    TIntermBlock *switchBody = new TIntermBlock;

    // case kDitherControlDither4444:
    //     ditherValue = vec3(ditherParam * 2)
    {
        TIntermSequence ditherValueElements = {
            new TIntermBinary(EOpMul, ditherParam->deepCopy(), CreateFloatNode(2.0f, EbpMedium)),
        };
        TIntermTyped *value = CreateDitherValue(*outputType, &ditherValueElements);

        TIntermTyped *setDitherValue = new TIntermBinary(EOpAssign, ditherValue->deepCopy(), value);

        switchBody->appendStatement(new TIntermCase(CreateUIntNode(vk::kDitherControlDither4444)));
        switchBody->appendStatement(setDitherValue);

        if (roundOutputAfterDithering)
        {
            constexpr std::array<float, 3> kMultiplier = {15, 15, 15};
            TIntermConstantUnion *roundMultiplierValue =
                CreateVecNode(kMultiplier.data(), 3, EbpMedium);
            switchBody->appendStatement(
                new TIntermBinary(EOpAssign, roundMultiplier->deepCopy(), roundMultiplierValue));
        }

        switchBody->appendStatement(new TIntermBranch(EOpBreak, nullptr));
    }

    // case kDitherControlDither5551:
    //     ditherValue = vec3(ditherParam)
    {
        TIntermSequence ditherValueElements = {
            ditherParam->deepCopy(),
        };
        TIntermTyped *value = CreateDitherValue(*outputType, &ditherValueElements);

        TIntermTyped *setDitherValue = new TIntermBinary(EOpAssign, ditherValue->deepCopy(), value);

        switchBody->appendStatement(new TIntermCase(CreateUIntNode(vk::kDitherControlDither5551)));
        switchBody->appendStatement(setDitherValue);

        if (roundOutputAfterDithering)
        {
            constexpr std::array<float, 3> kMultiplier = {31, 31, 31};
            TIntermConstantUnion *roundMultiplierValue =
                CreateVecNode(kMultiplier.data(), 3, EbpMedium);
            switchBody->appendStatement(
                new TIntermBinary(EOpAssign, roundMultiplier->deepCopy(), roundMultiplierValue));
        }

        switchBody->appendStatement(new TIntermBranch(EOpBreak, nullptr));
    }

    // case kDitherControlDither565:
    //     ditherValue = vec3(ditherParam, ditherParam / 2, ditherParam)
    {
        TIntermSequence ditherValueElements = {
            ditherParam->deepCopy(),
            new TIntermBinary(EOpMul, ditherParam->deepCopy(), CreateFloatNode(0.5f, EbpMedium)),
            ditherParam->deepCopy(),
        };
        TIntermTyped *value = CreateDitherValue(*outputType, &ditherValueElements);

        TIntermTyped *setDitherValue = new TIntermBinary(EOpAssign, ditherValue->deepCopy(), value);

        switchBody->appendStatement(new TIntermCase(CreateUIntNode(vk::kDitherControlDither565)));
        switchBody->appendStatement(setDitherValue);

        if (roundOutputAfterDithering)
        {
            constexpr std::array<float, 3> kMultiplier = {31, 63, 31};
            TIntermConstantUnion *roundMultiplierValue =
                CreateVecNode(kMultiplier.data(), 3, EbpMedium);
            switchBody->appendStatement(
                new TIntermBinary(EOpAssign, roundMultiplier->deepCopy(), roundMultiplierValue));
        }

        switchBody->appendStatement(new TIntermBranch(EOpBreak, nullptr));
    }

    // switch (dither_i)
    // {
    //     ...
    // }
    TIntermSwitch *formatSwitch = new TIntermSwitch(thisDitherControl, switchBody);
    ditherBlock->appendStatement(formatSwitch);

    // fragmentOutput.rgb += ditherValue
    if (type.getNominalSize() > 3)
    {
        fragmentOutput = new TIntermSwizzle(fragmentOutput, {0, 1, 2});
    }
    ditherBlock->appendStatement(new TIntermBinary(EOpAddAssign, fragmentOutput, ditherValue));

    // round() the output if workaround is enabled
    if (roundOutputAfterDithering)
    {
        TVector<int> swizzle = {0, 1, 2};
        swizzle.resize(fragmentOutput->getNominalSize());
        TIntermTyped *multiplier = new TIntermSwizzle(roundMultiplier, swizzle);

        // fragmentOutput.rgb = round(fragmentOutput.rgb * roundMultiplier) / roundMultiplier
        TIntermTyped *scaledUp = new TIntermBinary(EOpMul, fragmentOutput->deepCopy(), multiplier);
        TIntermTyped *rounded =
            CreateBuiltInUnaryFunctionCallNode("round", scaledUp, *symbolTable, 300);
        TIntermTyped *scaledDown = new TIntermBinary(EOpDiv, rounded, multiplier->deepCopy());
        ditherBlock->appendStatement(
            new TIntermBinary(EOpAssign, fragmentOutput->deepCopy(), scaledDown));
    }
}

void EmitFragmentVariableDither(TCompiler *compiler,
                                const ShCompileOptions &compileOptions,
                                TSymbolTable *symbolTable,
                                TIntermBlock *ditherBlock,
                                TIntermTyped *ditherControl,
                                TIntermTyped *ditherParam,
                                const TVariable &fragmentVariable)
{
    const TType &type = fragmentVariable.getType();
    if (type.getBasicType() != EbtFloat)
    {
        return;
    }

    const TLayoutQualifier &layoutQualifier = type.getLayoutQualifier();

    const uint32_t location = layoutQualifier.locationsSpecified ? layoutQualifier.location : 0;

    // Fragment outputs cannot be an array of array.
    ASSERT(!type.isArrayOfArrays());

    // Emit one block of dithering output per element of array (if array).
    TIntermSymbol *fragmentOutputSymbol = new TIntermSymbol(&fragmentVariable);
    if (!type.isArray())
    {
        EmitFragmentOutputDither(compiler, compileOptions, symbolTable, ditherBlock, ditherControl,
                                 ditherParam, fragmentOutputSymbol, location);
        return;
    }

    for (uint32_t index = 0; index < type.getOutermostArraySize(); ++index)
    {
        TIntermBinary *element = new TIntermBinary(EOpIndexDirect, fragmentOutputSymbol->deepCopy(),
                                                   CreateIndexNode(index));
        EmitFragmentOutputDither(compiler, compileOptions, symbolTable, ditherBlock, ditherControl,
                                 ditherParam, element, location + static_cast<uint32_t>(index));
    }
}

TIntermNode *EmitDitheringBlock(TCompiler *compiler,
                                const ShCompileOptions &compileOptions,
                                TSymbolTable *symbolTable,
                                SpecConst *specConst,
                                const DriverUniform *driverUniforms,
                                const FragmentOutputVariableList &fragmentOutputVariables)
{
    // Add dithering code.  A specialization constant is taken (dither control) in the following
    // form:
    //
    //     0000000000000000dfdfdfdfdfdfdfdf
    //
    // Where every pair of bits df[i] means for attachment i:
    //
    //     00: no dithering
    //     01: dither for the R4G4B4A4 format
    //     10: dither for the R5G5B5A1 format
    //     11: dither for the R5G6B5 format
    //
    // Only the above formats are dithered to avoid paying a high cost on formats that usually don't
    // need dithering.  Applications that require dithering often perform the dithering themselves.
    // Additionally, dithering is not applied to alpha as it creates artifacts when blending.
    //
    // The generated code is as such:
    //
    //     if (dither != 0)
    //     {
    //         const mediump float bayer[4] = { balanced 2x2 bayer divided by 32 };
    //         const mediump float b = bayer[(uint(gl_FragCoord.x) & 1) << 1 |
    //                                       (uint(gl_FragCoord.y) & 1)];
    //
    //         // for each attachment i
    //         uint ditheri = dither >> (2 * i) & 0x3;
    //         vec3 bi = vec3(0);
    //         switch (ditheri)
    //         {
    //         case kDitherControlDither4444:
    //             bi = vec3(b * 2)
    //             break;
    //         case kDitherControlDither5551:
    //             bi = vec3(b)
    //             break;
    //         case kDitherControlDither565:
    //             bi = vec3(b, b / 2, b)
    //             break;
    //         }
    //         colori.rgb += bi;
    //     }

    TIntermTyped *ditherControl = specConst->getDither();
    if (ditherControl == nullptr)
    {
        ditherControl = driverUniforms->getDither();
    }

    // if (dither != 0)
    TIntermTyped *ifAnyDitherCondition =
        new TIntermBinary(EOpNotEqual, ditherControl, CreateUIntNode(0));

    TIntermBlock *ditherBlock = new TIntermBlock;

    // The dithering (Bayer) matrix.  A 2x2 matrix is used which has acceptable results with minimal
    // impact on performance.  The 2x2 Bayer matrix is defined as:
    //
    //                [ 0  2 ]
    //     B = 0.25 * |      |
    //                [ 3  1 ]
    //
    // Using this matrix adds energy to the output however, and so it is balanced by subtracting the
    // elements by the average value:
    //
    //                         [ -1.5   0.5 ]
    //     B_balanced = 0.25 * |            |
    //                         [  1.5  -0.5 ]
    //
    // For each pixel, one of the four values in this matrix is selected (indexed by
    // gl_FragCoord.xy % 2), is scaled by the precision of the attachment format (per channel, if
    // different) and is added to the color output.  For example, if the value `b` is selected for a
    // pixel, and the attachment has the RGB565 format, then the following value is added to the
    // color output:
    //
    //      vec3(b/32, b/64, b/32)
    //
    // For RGBA5551, that would be:
    //
    //      vec3(b/32, b/32, b/32)
    //
    // And for RGBA4444, that would be:
    //
    //      vec3(b/16, b/16, b/16)
    //
    // Given the relative popularity of RGB565, and that b/32 is most often used in the above, the
    // Bayer matrix constant used here is pre-scaled by 1/32, avoiding further scaling in most
    // cases.
    TType *bayerType = new TType(*StaticType::GetBasic<EbtFloat, EbpMedium>());
    bayerType->setQualifier(EvqConst);
    bayerType->makeArray(4);

    TIntermSequence bayerElements = {
        CreateFloatNode(-1.5f * 0.25f / 32.0f, EbpMedium),
        CreateFloatNode(0.5f * 0.25f / 32.0f, EbpMedium),
        CreateFloatNode(1.5f * 0.25f / 32.0f, EbpMedium),
        CreateFloatNode(-0.5f * 0.25f / 32.0f, EbpMedium),
    };
    TIntermAggregate *bayerValue = TIntermAggregate::CreateConstructor(*bayerType, &bayerElements);

    // const float bayer[4] = { balanced 2x2 bayer divided by 32 }
    TIntermSymbol *bayer          = new TIntermSymbol(CreateTempVariable(symbolTable, bayerType));
    TIntermDeclaration *bayerDecl = CreateTempInitDeclarationNode(&bayer->variable(), bayerValue);
    ditherBlock->appendStatement(bayerDecl);

    // Take the coordinates of the pixel and determine which element of the bayer matrix should be
    // used:
    //
    //     (uint(gl_FragCoord.x) & 1) << 1 | (uint(gl_FragCoord.y) & 1)
    const TVariable *fragCoord = static_cast<const TVariable *>(
        symbolTable->findBuiltIn(ImmutableString("gl_FragCoord"), compiler->getShaderVersion()));

    TIntermTyped *fragCoordX          = new TIntermSwizzle(new TIntermSymbol(fragCoord), {0});
    TIntermSequence fragCoordXIntArgs = {
        fragCoordX,
    };
    TIntermTyped *fragCoordXInt = TIntermAggregate::CreateConstructor(
        *StaticType::GetBasic<EbtUInt, EbpMedium>(), &fragCoordXIntArgs);
    TIntermTyped *fragCoordXBit0 =
        new TIntermBinary(EOpBitwiseAnd, fragCoordXInt, CreateUIntNode(1));
    TIntermTyped *fragCoordXBit0Shifted =
        new TIntermBinary(EOpBitShiftLeft, fragCoordXBit0, CreateUIntNode(1));

    TIntermTyped *fragCoordY          = new TIntermSwizzle(new TIntermSymbol(fragCoord), {1});
    TIntermSequence fragCoordYIntArgs = {
        fragCoordY,
    };
    TIntermTyped *fragCoordYInt = TIntermAggregate::CreateConstructor(
        *StaticType::GetBasic<EbtUInt, EbpMedium>(), &fragCoordYIntArgs);
    TIntermTyped *fragCoordYBit0 =
        new TIntermBinary(EOpBitwiseAnd, fragCoordYInt, CreateUIntNode(1));

    TIntermTyped *bayerIndex =
        new TIntermBinary(EOpBitwiseOr, fragCoordXBit0Shifted, fragCoordYBit0);

    // const mediump float b = bayer[(uint(gl_FragCoord.x) & 1) << 1 |
    //                               (uint(gl_FragCoord.y) & 1)];
    TIntermSymbol *ditherParam = new TIntermSymbol(
        CreateTempVariable(symbolTable, StaticType::GetBasic<EbtFloat, EbpMedium>()));
    TIntermDeclaration *ditherParamDecl = CreateTempInitDeclarationNode(
        &ditherParam->variable(),
        new TIntermBinary(EOpIndexIndirect, bayer->deepCopy(), bayerIndex));
    ditherBlock->appendStatement(ditherParamDecl);

    // Dither blocks for each fragment output
    for (const TVariable *fragmentVariable : fragmentOutputVariables)
    {
        EmitFragmentVariableDither(compiler, compileOptions, symbolTable, ditherBlock,
                                   ditherControl, ditherParam, *fragmentVariable);
    }

    return new TIntermIfElse(ifAnyDitherCondition, ditherBlock, nullptr);
}
}  // anonymous namespace

bool EmulateDithering(TCompiler *compiler,
                      const ShCompileOptions &compileOptions,
                      TIntermBlock *root,
                      TSymbolTable *symbolTable,
                      SpecConst *specConst,
                      const DriverUniform *driverUniforms)
{
    FragmentOutputVariableList fragmentOutputVariables;
    GatherFragmentOutputs(root, &fragmentOutputVariables);

    TIntermNode *ditherCode = EmitDitheringBlock(compiler, compileOptions, symbolTable, specConst,
                                                 driverUniforms, fragmentOutputVariables);

    return RunAtTheEndOfShader(compiler, root, ditherCode, symbolTable);
}
}  // namespace sh
