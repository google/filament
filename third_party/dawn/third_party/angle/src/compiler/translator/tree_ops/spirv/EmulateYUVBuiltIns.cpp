//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EmulateYUVBuiltIns: Adds functions that emulate yuv_2_rgb and rgb_2_yuv built-ins.
//

#include "compiler/translator/tree_ops/spirv/EmulateYUVBuiltIns.h"

#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{
namespace
{
// A traverser that replaces the yuv built-ins with a function call that emulates it.
class EmulateYUVBuiltInsTraverser : public TIntermTraverser
{
  public:
    EmulateYUVBuiltInsTraverser(TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, false, symbolTable)
    {}

    bool visitAggregate(Visit visit, TIntermAggregate *node) override;

    bool update(TCompiler *compiler, TIntermBlock *root);

  private:
    const TFunction *getYUV2RGBFunc(TPrecision precision);
    const TFunction *getRGB2YUVFunc(TPrecision precision);
    const TFunction *getYUVFunc(TPrecision precision,
                                const char *name,
                                TIntermTyped *itu601Matrix,
                                TIntermTyped *itu601WideMatrix,
                                TIntermTyped *itu709Matrix,
                                TIntermFunctionDefinition **funcDefOut);

    TIntermTyped *replaceYUVFuncCall(TIntermTyped *node);

    // One emulation function for each sampler precision
    std::array<TIntermFunctionDefinition *, EbpLast> mYUV2RGBFuncDefs = {};
    std::array<TIntermFunctionDefinition *, EbpLast> mRGB2YUVFuncDefs = {};
};

bool EmulateYUVBuiltInsTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    TIntermTyped *replacement = replaceYUVFuncCall(node);

    if (replacement != nullptr)
    {
        queueReplacement(replacement, OriginalNode::IS_DROPPED);
        return false;
    }

    return true;
}

TIntermTyped *EmulateYUVBuiltInsTraverser::replaceYUVFuncCall(TIntermTyped *node)
{
    TIntermAggregate *asAggregate = node->getAsAggregate();
    if (asAggregate == nullptr)
    {
        return nullptr;
    }

    TOperator op = asAggregate->getOp();
    if (op != EOpYuv_2_rgb && op != EOpRgb_2_yuv)
    {
        return nullptr;
    }

    ASSERT(asAggregate->getChildCount() == 2);

    TIntermTyped *param0 = asAggregate->getChildNode(0)->getAsTyped();
    TPrecision precision = param0->getPrecision();
    if (precision == EbpUndefined)
    {
        precision = EbpMedium;
    }

    const TFunction *emulatedFunction =
        op == EOpYuv_2_rgb ? getYUV2RGBFunc(precision) : getRGB2YUVFunc(precision);

    // The first parameter of the built-ins (|color|) may itself contain a built-in call.  With
    // TIntermTraverser, if the direct children also needs to be replaced that needs to be done
    // while constructing this node as replacement doesn't work.
    TIntermTyped *param0Replacement = replaceYUVFuncCall(param0);

    if (param0Replacement == nullptr)
    {
        // If param0 is not directly a YUV built-in call, visit it recursively so YIV built-in call
        // sub expressions are replaced.
        param0->traverse(this);
        param0Replacement = param0;
    }

    // Create the function call
    TIntermSequence args = {
        param0Replacement,
        asAggregate->getChildNode(1),
    };
    return TIntermAggregate::CreateFunctionCall(*emulatedFunction, &args);
}

TIntermTyped *MakeMatrix(const std::array<float, 12> &elements)
{
    TIntermSequence matrix;
    for (float element : elements)
    {
        matrix.push_back(CreateFloatNode(element, EbpMedium));
    }

    const TType *matType = StaticType::GetBasic<EbtFloat, EbpMedium, 4, 3>();
    return TIntermAggregate::CreateConstructor(*matType, &matrix);
}

const TFunction *EmulateYUVBuiltInsTraverser::getYUV2RGBFunc(TPrecision precision)
{
    const char *name = "ANGLE_yuv_2_rgb";
    switch (precision)
    {
        case EbpLow:
            name = "ANGLE_yuv_2_rgb_lowp";
            break;
        case EbpMedium:
            name = "ANGLE_yuv_2_rgb_mediump";
            break;
        case EbpHigh:
            name = "ANGLE_yuv_2_rgb_highp";
            break;
        default:
            UNREACHABLE();
    }

    // Matrix is combination of the "pure" colorspace conversion matrix for each standard,
    // the appropriate range expansion (in the case of narrow range) and shifting down of chroma
    // components to be centered on zero. These arrays are interpreted as mat4x3
    //
    // Pure conversion used for itu601:
    //   1.0, 1.0, 1.0, 0.0, -0.3441, 1.7720, 1.4020, -0.7141, 0.0
    // Pure conversion used for itu709:
    //   1.0, 1.0, 1.0, 0.0, -0.1873, 1.8556, 1.5748, -0.4681, 0.0
    //
    // For narrow range, Y is rescaled from [16/255, 235/255] to [0,1]
    // and Cb/Cr are rescaled from [16/255, 240/255] to [0,1] and shifted by -128/255
    // to center on zero. For wide range, only the Cb/Cr shifting by -128/255 is performed.

    constexpr std::array<float, 12> itu601Matrix = {1.164384,  1.164384,  1.164384, 0.0,
                                                    -0.391721, 2.017232,  1.596027, -0.812926,
                                                    0.0,       -0.874202, 0.531626, -1.085631};

    constexpr std::array<float, 12> itu601WideMatrix = {1.000000,  1.000000,  1.000000, 0.000000,
                                                        -0.344100, 1.772000,  1.402000, -0.714100,
                                                        0.000000,  -0.703749, 0.531175, -0.889475};

    constexpr std::array<float, 12> itu709Matrix = {1.164384,  1.164384,  1.164384, 0.000000,
                                                    -0.213221, 2.112402,  1.792741, -0.532882,
                                                    0.000000,  -0.972945, 0.301455, -1.133402};

    return getYUVFunc(precision, name, MakeMatrix(itu601Matrix), MakeMatrix(itu601WideMatrix),
                      MakeMatrix(itu709Matrix), &mYUV2RGBFuncDefs[precision]);
}

const TFunction *EmulateYUVBuiltInsTraverser::getRGB2YUVFunc(TPrecision precision)
{
    const char *name = "ANGLE_rgb_2_yuv";
    switch (precision)
    {
        case EbpLow:
            name = "ANGLE_rgb_2_yuv_lowp";
            break;
        case EbpMedium:
            name = "ANGLE_rgb_2_yuv_mediump";
            break;
        case EbpHigh:
            name = "ANGLE_rgb_2_yuv_highp";
            break;
        default:
            UNREACHABLE();
    }

    // Inverse of yuv_2_rgb transforms above
    const std::array<float, 12> itu601Matrix = {0.256782,  -0.148219, 0.439220, 0.504143,
                                                -0.291001, -0.367798, 0.097898, 0.439220,
                                                -0.071422, 0.062745,  0.501961, 0.501961};

    const std::array<float, 12> itu601WideMatrix = {0.298993,  -0.168732, 0.500005, 0.587016,
                                                    -0.331273, -0.418699, 0.113991, 0.500005,
                                                    -0.081306, 0.000000,  0.501961, 0.501961};

    const std::array<float, 12> itu709Matrix = {0.182580,  -0.100641, 0.439219, 0.614243,
                                                -0.338579, -0.398950, 0.062000, 0.439219,
                                                -0.040269, 0.062745,  0.501961, 0.501961};

    return getYUVFunc(precision, name, MakeMatrix(itu601Matrix), MakeMatrix(itu601WideMatrix),
                      MakeMatrix(itu709Matrix), &mRGB2YUVFuncDefs[precision]);
}

const TFunction *EmulateYUVBuiltInsTraverser::getYUVFunc(TPrecision precision,
                                                         const char *name,
                                                         TIntermTyped *itu601Matrix,
                                                         TIntermTyped *itu601WideMatrix,
                                                         TIntermTyped *itu709Matrix,
                                                         TIntermFunctionDefinition **funcDefOut)
{
    if (*funcDefOut != nullptr)
    {
        return (*funcDefOut)->getFunction();
    }

    // The function prototype is vec3 name(vec3 color, yuvCscStandardEXT conv_standard)
    TType *vec3Type = new TType(*StaticType::GetBasic<EbtFloat, EbpMedium, 3>());
    vec3Type->setPrecision(precision);
    const TType *yuvCscType = StaticType::GetBasic<EbtYuvCscStandardEXT, EbpUndefined>();

    TType *colorType = new TType(*vec3Type);
    TType *convType  = new TType(*yuvCscType);
    colorType->setQualifier(EvqParamIn);
    convType->setQualifier(EvqParamIn);

    TVariable *colorParam =
        new TVariable(mSymbolTable, ImmutableString("color"), colorType, SymbolType::AngleInternal);
    TVariable *convParam = new TVariable(mSymbolTable, ImmutableString("conv_standard"), convType,
                                         SymbolType::AngleInternal);

    TFunction *function = new TFunction(mSymbolTable, ImmutableString(name),
                                        SymbolType::AngleInternal, vec3Type, true);
    function->addParameter(colorParam);
    function->addParameter(convParam);

    TType *vec4Type = new TType(*StaticType::GetBasic<EbtFloat, EbpMedium, 4>());
    vec4Type->setPrecision(precision);

    TIntermSequence components;
    components.push_back(new TIntermSymbol(colorParam));
    components.push_back(CreateFloatNode(1.0f, EbpMedium));
    // vec4(color, 1)
    TIntermTyped *extendedColor = TIntermAggregate::CreateConstructor(*vec4Type, &components);

    // The function body is as such:
    //
    //     switch (conv_standard)
    //     {
    //       case itu_601:
    //         return itu601Matrix * color;
    //       case itu_601_full_range:
    //         return itu601WideMatrix * color;
    //       case itu_709:
    //         return itu709Matrix * color;
    //     }
    //
    //     // error
    //     return vec3(0.0);

    // Matrix * color
    TIntermTyped *itu601Mul = new TIntermBinary(EOpMatrixTimesVector, itu601Matrix, extendedColor);
    TIntermTyped *itu601FullRangeMul =
        new TIntermBinary(EOpMatrixTimesVector, itu601WideMatrix, extendedColor->deepCopy());
    TIntermTyped *itu709Mul =
        new TIntermBinary(EOpMatrixTimesVector, itu709Matrix, extendedColor->deepCopy());

    // return Matrix * color
    TIntermBranch *returnItu601Mul          = new TIntermBranch(EOpReturn, itu601Mul);
    TIntermBranch *returnItu601FullRangeMul = new TIntermBranch(EOpReturn, itu601FullRangeMul);
    TIntermBranch *returnItu709Mul          = new TIntermBranch(EOpReturn, itu709Mul);

    // itu_* constants
    TConstantUnion *ituConstants = new TConstantUnion[3];
    ituConstants[0].setYuvCscStandardEXTConst(EycsItu601);
    ituConstants[1].setYuvCscStandardEXTConst(EycsItu601FullRange);
    ituConstants[2].setYuvCscStandardEXTConst(EycsItu709);

    TIntermConstantUnion *itu601          = new TIntermConstantUnion(&ituConstants[0], *yuvCscType);
    TIntermConstantUnion *itu601FullRange = new TIntermConstantUnion(&ituConstants[1], *yuvCscType);
    TIntermConstantUnion *itu709          = new TIntermConstantUnion(&ituConstants[2], *yuvCscType);

    // case ...: return ...
    TIntermBlock *switchBody = new TIntermBlock;

    switchBody->appendStatement(new TIntermCase(itu601));
    switchBody->appendStatement(returnItu601Mul);
    switchBody->appendStatement(new TIntermCase(itu601FullRange));
    switchBody->appendStatement(returnItu601FullRangeMul);
    switchBody->appendStatement(new TIntermCase(itu709));
    switchBody->appendStatement(returnItu709Mul);

    // switch (conv_standard) ...
    TIntermSwitch *switchStatement = new TIntermSwitch(new TIntermSymbol(convParam), switchBody);

    TIntermBlock *body = new TIntermBlock;

    body->appendStatement(switchStatement);
    body->appendStatement(new TIntermBranch(EOpReturn, CreateZeroNode(*vec3Type)));

    *funcDefOut = new TIntermFunctionDefinition(new TIntermFunctionPrototype(function), body);

    return function;
}

bool EmulateYUVBuiltInsTraverser::update(TCompiler *compiler, TIntermBlock *root)
{
    // Insert any added function definitions before the first function.
    const size_t firstFunctionIndex = FindFirstFunctionDefinitionIndex(root);
    TIntermSequence funcDefs;

    for (TIntermFunctionDefinition *funcDef : mYUV2RGBFuncDefs)
    {
        if (funcDef != nullptr)
        {
            funcDefs.push_back(funcDef);
        }
    }

    for (TIntermFunctionDefinition *funcDef : mRGB2YUVFuncDefs)
    {
        if (funcDef != nullptr)
        {
            funcDefs.push_back(funcDef);
        }
    }

    root->insertChildNodes(firstFunctionIndex, funcDefs);

    return updateTree(compiler, root);
}
}  // anonymous namespace

bool EmulateYUVBuiltIns(TCompiler *compiler, TIntermBlock *root, TSymbolTable *symbolTable)
{
    EmulateYUVBuiltInsTraverser traverser(symbolTable);
    root->traverse(&traverser);
    return traverser.update(compiler, root);
}
}  // namespace sh
