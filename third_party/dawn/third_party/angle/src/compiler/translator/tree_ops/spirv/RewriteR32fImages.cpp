//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RewriteR32fImages: Change images qualified with r32f to use r32ui instead.
//

#include "compiler/translator/tree_ops/spirv/RewriteR32fImages.h"

#include "compiler/translator/Compiler.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"

namespace sh
{
namespace
{
bool IsR32fImage(const TType &type)
{
    return type.getQualifier() == EvqUniform && type.isImage() &&
           type.getLayoutQualifier().imageInternalFormat == EiifR32F;
}

using ImageMap = angle::HashMap<const TVariable *, const TVariable *>;

TIntermTyped *RewriteBuiltinFunctionCall(TCompiler *compiler,
                                         TSymbolTable *symbolTable,
                                         TIntermAggregate *node,
                                         const ImageMap &imageMap);

// Given an expression, this traverser calculates a new expression where builtin function calls to
// r32f images are replaced with ones to the mapped r32ui image.  In particular, this is run on the
// right node of EOpIndexIndirect binary nodes, so that the expression in the index gets a chance to
// go through this transformation.
class RewriteExpressionTraverser final : public TIntermTraverser
{
  public:
    explicit RewriteExpressionTraverser(TCompiler *compiler,
                                        TSymbolTable *symbolTable,
                                        const ImageMap &imageMap)
        : TIntermTraverser(true, false, false, symbolTable),
          mCompiler(compiler),
          mImageMap(imageMap)
    {}

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        TIntermTyped *rewritten =
            RewriteBuiltinFunctionCall(mCompiler, mSymbolTable, node, mImageMap);
        if (rewritten == nullptr)
        {
            return true;
        }

        queueReplacement(rewritten, OriginalNode::IS_DROPPED);

        // Don't iterate as the expression is rewritten.
        return false;
    }

  private:
    TCompiler *mCompiler;

    const ImageMap &mImageMap;
};

// Rewrite the index of an EOpIndexIndirect expression as well as any arguments to the builtin
// function call.
TIntermTyped *RewriteExpression(TCompiler *compiler,
                                TSymbolTable *symbolTable,
                                TIntermTyped *expression,
                                const ImageMap &imageMap)
{
    // Create a fake block to insert the node in.  The root itself may need changing.
    TIntermBlock block;
    block.appendStatement(expression);

    RewriteExpressionTraverser traverser(compiler, symbolTable, imageMap);
    block.traverse(&traverser);

    bool valid = traverser.updateTree(compiler, &block);
    ASSERT(valid);

    TIntermTyped *rewritten = block.getChildNode(0)->getAsTyped();

    return rewritten;
}

// Given a builtin function call such as the following:
//
//     imageLoad(expression, ...);
//
// expression is in the form of:
//
// - image uniform
// - image uniform array indexed with EOpIndexDirect or EOpIndexIndirect.  Note that
//   RewriteArrayOfArrayOfOpaqueUniforms has already ensured that the image array is
//   single-dimension.
//
// The latter case (with EOpIndexIndirect) is not valid GLSL (up to GL_EXT_gpu_shader5), but if it
// were, the index itself could have contained an image builtin function call, so is recursively
// processed (in case supported in future).  Additionally, the other builtin function arguments may
// need processing too.
//
// This function creates a similar expression where the image uniforms (of type r32f) are replaced
// with those of r32ui type.
//
TIntermTyped *RewriteBuiltinFunctionCall(TCompiler *compiler,
                                         TSymbolTable *symbolTable,
                                         TIntermAggregate *node,
                                         const ImageMap &imageMap)
{
    if (!BuiltInGroup::IsBuiltIn(node->getOp()))
    {
        // AST functions don't require modification as r32f image function parameters are removed by
        // MonomorphizeUnsupportedFunctions.
        return nullptr;
    }

    // If it's an |image*| function, replace the function with an equivalent that uses an r32ui
    // image.
    if (!node->getFunction()->isImageFunction())
    {
        return nullptr;
    }

    TIntermSequence *arguments = node->getSequence();

    TIntermTyped *imageExpression = (*arguments)[0]->getAsTyped();
    ASSERT(imageExpression);

    // Find the image uniform that's being indexed, if indexed.
    TIntermBinary *asBinary     = imageExpression->getAsBinaryNode();
    TIntermSymbol *imageUniform = imageExpression->getAsSymbolNode();

    if (asBinary)
    {
        ASSERT(asBinary->getOp() == EOpIndexDirect || asBinary->getOp() == EOpIndexIndirect);
        imageUniform = asBinary->getLeft()->getAsSymbolNode();
    }

    ASSERT(imageUniform);
    if (!IsR32fImage(imageUniform->getType()))
    {
        return nullptr;
    }

    ASSERT(imageMap.find(&imageUniform->variable()) != imageMap.end());
    const TVariable *replacementImage = imageMap.at(&imageUniform->variable());

    // Build the expression again, with the image uniform replaced.  If index is dynamic,
    // recursively process it.
    TIntermTyped *replacementExpression = new TIntermSymbol(replacementImage);

    // Index it, if indexed.
    if (asBinary != nullptr)
    {
        TIntermTyped *index = asBinary->getRight();

        switch (asBinary->getOp())
        {
            case EOpIndexDirect:
                break;
            case EOpIndexIndirect:
            {
                // Run RewriteExpressionTraverser on the index node.  This case is currently
                // impossible with known extensions.
                UNREACHABLE();
                index = RewriteExpression(compiler, symbolTable, index, imageMap);
                break;
            }
            default:
                UNREACHABLE();
                break;
        }

        replacementExpression = new TIntermBinary(asBinary->getOp(), replacementExpression, index);
    }

    TIntermSequence substituteArguments;
    substituteArguments.push_back(replacementExpression);

    for (size_t argIndex = 1; argIndex < arguments->size(); ++argIndex)
    {
        TIntermTyped *arg = (*arguments)[argIndex]->getAsTyped();

        // Run RewriteExpressionTraverser on the argument.  It may itself be an expression with an
        // r32f image that needs to be rewritten.
        arg = RewriteExpression(compiler, symbolTable, arg, imageMap);
        substituteArguments.push_back(arg);
    }

    const ImmutableString &functionName = node->getFunction()->name();
    bool isImageAtomicExchange          = functionName == "imageAtomicExchange";
    bool isImageLoad                    = false;

    if (functionName == "imageStore" || isImageAtomicExchange)
    {
        // The last parameter is float data, which should be changed to floatBitsToUint(data).
        TIntermTyped *data = substituteArguments.back()->getAsTyped();
        substituteArguments.back() =
            CreateBuiltInUnaryFunctionCallNode("floatBitsToUint", data, *symbolTable, 300);
    }
    else if (functionName == "imageLoad")
    {
        isImageLoad = true;
    }
    else
    {
        // imageSize does not have any other arguments.
        ASSERT(functionName == "imageSize");
        ASSERT(arguments->size() == 1);
    }

    TIntermTyped *replacementCall =
        CreateBuiltInFunctionCallNode(functionName.data(), &substituteArguments, *symbolTable, 310);

    // If imageLoad or imageAtomicExchange, the result is now uint, which should be converted with
    // uintBitsToFloat.  With imageLoad, the alpha channel should always read 1.0 regardless.
    if (isImageLoad || isImageAtomicExchange)
    {
        if (isImageLoad)
        {
            // imageLoad().rgb
            replacementCall = new TIntermSwizzle(replacementCall, {0, 1, 2});
        }

        // uintBitsToFloat(imageLoad().rgb), or uintBitsToFloat(imageAtomicExchange())
        replacementCall = CreateBuiltInUnaryFunctionCallNode("uintBitsToFloat", replacementCall,
                                                             *symbolTable, 300);

        if (isImageLoad)
        {
            // vec4(uintBitsToFloat(imageLoad().rgb), 1.0)
            const TType &vec4Type           = *StaticType::GetBasic<EbtFloat, EbpHigh, 4>();
            TIntermSequence constructorArgs = {replacementCall, CreateFloatNode(1.0f, EbpMedium)};
            replacementCall = TIntermAggregate::CreateConstructor(vec4Type, &constructorArgs);
        }
    }

    return replacementCall;
}

// Traverser that:
//
// 1. Converts the layout(r32f, ...) ... image* name; declarations to use the r32ui format
// 2. Converts |imageLoad| and |imageStore| functions to use |uintBitsToFloat| and |floatBitsToUint|
//    respectively.
// 3. Converts |imageAtomicExchange| to use |floatBitsToUint| and |uintBitsToFloat|.
class RewriteR32fImagesTraverser : public TIntermTraverser
{
  public:
    RewriteR32fImagesTraverser(TCompiler *compiler, TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, false, symbolTable), mCompiler(compiler)
    {}

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        if (visit != PreVisit)
        {
            return true;
        }

        const TIntermSequence &sequence = *(node->getSequence());

        TIntermTyped *declVariable = sequence.front()->getAsTyped();
        const TType &type          = declVariable->getType();

        if (!IsR32fImage(type))
        {
            return true;
        }

        TIntermSymbol *oldSymbol = declVariable->getAsSymbolNode();
        ASSERT(oldSymbol != nullptr);

        const TVariable &oldVariable = oldSymbol->variable();

        TType *newType                      = new TType(type);
        TLayoutQualifier layoutQualifier    = type.getLayoutQualifier();
        layoutQualifier.imageInternalFormat = EiifR32UI;
        newType->setLayoutQualifier(layoutQualifier);

        switch (type.getBasicType())
        {
            case EbtImage2D:
                newType->setBasicType(EbtUImage2D);
                break;
            case EbtImage3D:
                newType->setBasicType(EbtUImage3D);
                break;
            case EbtImage2DArray:
                newType->setBasicType(EbtUImage2DArray);
                break;
            case EbtImageCube:
                newType->setBasicType(EbtUImageCube);
                break;
            case EbtImage2DMS:
                newType->setBasicType(EbtUImage2DMS);
                break;
            case EbtImage2DMSArray:
                newType->setBasicType(EbtUImage2DMSArray);
                break;
            case EbtImageCubeArray:
                newType->setBasicType(EbtUImageCubeArray);
                break;
            case EbtImageRect:
                newType->setBasicType(EbtUImageRect);
                break;
            case EbtImageBuffer:
                newType->setBasicType(EbtUImageBuffer);
                break;
            default:
                UNREACHABLE();
        }

        TVariable *newVariable =
            new TVariable(oldVariable.uniqueId(), oldVariable.name(), oldVariable.symbolType(),
                          oldVariable.extensions(), newType);

        mImageMap[&oldVariable] = newVariable;

        TIntermDeclaration *newDecl = new TIntermDeclaration();
        newDecl->appendDeclarator(new TIntermSymbol(newVariable));

        queueReplacement(newDecl, OriginalNode::IS_DROPPED);

        return false;
    }

    // Same implementation as in RewriteExpressionTraverser.  That traverser cannot replace root.
    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        TIntermTyped *rewritten =
            RewriteBuiltinFunctionCall(mCompiler, mSymbolTable, node, mImageMap);
        if (rewritten == nullptr)
        {
            return true;
        }

        queueReplacement(rewritten, OriginalNode::IS_DROPPED);

        return false;
    }

    void visitSymbol(TIntermSymbol *symbol) override
    {
        // Cannot encounter the image symbol directly.  It can only be used with built-in functions,
        // and therefore it's handled by visitAggregate.
        ASSERT(!IsR32fImage(symbol->getType()));
    }

  private:
    TCompiler *mCompiler;

    // Map from r32f image to r32ui image
    ImageMap mImageMap;
};

}  // anonymous namespace

bool RewriteR32fImages(TCompiler *compiler, TIntermBlock *root, TSymbolTable *symbolTable)
{
    RewriteR32fImagesTraverser traverser(compiler, symbolTable);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}
}  // namespace sh
