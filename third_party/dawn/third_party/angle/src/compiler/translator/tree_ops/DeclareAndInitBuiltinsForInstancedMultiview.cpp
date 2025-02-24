//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Applies the necessary AST transformations to support multiview rendering through instancing.
// Check the header file For more information.
//

#include "compiler/translator/tree_ops/DeclareAndInitBuiltinsForInstancedMultiview.h"

#include "compiler/translator/Compiler.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_ops/InitializeVariables.h"
#include "compiler/translator/tree_util/BuiltIn.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

constexpr const ImmutableString kViewIDVariableName("ViewID_OVR");
constexpr const ImmutableString kInstanceIDVariableName("InstanceID");
constexpr const ImmutableString kMultiviewBaseViewLayerIndexVariableName(
    "multiviewBaseViewLayerIndex");

// Adds the InstanceID and ViewID_OVR initializers to the end of the initializers' sequence.
void InitializeViewIDAndInstanceID(const TVariable *viewID,
                                   const TVariable *instanceID,
                                   unsigned numberOfViews,
                                   const TSymbolTable &symbolTable,
                                   TIntermSequence *initializers)
{
    // Create an unsigned numberOfViews node.
    TConstantUnion *numberOfViewsUnsignedConstant = new TConstantUnion();
    numberOfViewsUnsignedConstant->setUConst(numberOfViews);
    TIntermConstantUnion *numberOfViewsUint =
        new TIntermConstantUnion(numberOfViewsUnsignedConstant, TType(EbtUInt, EbpLow, EvqConst));

    // Create a uint(gl_InstanceID) node.
    TIntermSequence glInstanceIDSymbolCastArguments;
    glInstanceIDSymbolCastArguments.push_back(new TIntermSymbol(BuiltInVariable::gl_InstanceID()));
    TIntermAggregate *glInstanceIDAsUint = TIntermAggregate::CreateConstructor(
        TType(EbtUInt, EbpHigh, EvqTemporary), &glInstanceIDSymbolCastArguments);

    // Create a uint(gl_InstanceID) / numberOfViews node.
    TIntermBinary *normalizedInstanceID =
        new TIntermBinary(EOpDiv, glInstanceIDAsUint, numberOfViewsUint);

    // Create an int(uint(gl_InstanceID) / numberOfViews) node.
    TIntermSequence normalizedInstanceIDCastArguments;
    normalizedInstanceIDCastArguments.push_back(normalizedInstanceID);
    TIntermAggregate *normalizedInstanceIDAsInt = TIntermAggregate::CreateConstructor(
        TType(EbtInt, EbpHigh, EvqTemporary), &normalizedInstanceIDCastArguments);

    // Create an InstanceID = int(uint(gl_InstanceID) / numberOfViews) node.
    TIntermBinary *instanceIDInitializer =
        new TIntermBinary(EOpAssign, new TIntermSymbol(instanceID), normalizedInstanceIDAsInt);
    initializers->push_back(instanceIDInitializer);

    // Create a uint(gl_InstanceID) % numberOfViews node.
    TIntermBinary *normalizedViewID =
        new TIntermBinary(EOpIMod, glInstanceIDAsUint->deepCopy(), numberOfViewsUint->deepCopy());

    // Create a ViewID_OVR = uint(gl_InstanceID) % numberOfViews node.
    TIntermBinary *viewIDInitializer =
        new TIntermBinary(EOpAssign, new TIntermSymbol(viewID), normalizedViewID);
    initializers->push_back(viewIDInitializer);
}

// Write int(ViewID_OVR) to gl_Layer. The assignment is
// added to the end of the initializers' sequence.
void SelectViewIndexInVertexShader(const TVariable *viewID,
                                   const TVariable *multiviewBaseViewLayerIndex,
                                   TIntermSequence *initializers,
                                   const TSymbolTable &symbolTable)
{
    // Create an int(ViewID_OVR) node.
    TIntermSequence viewIDSymbolCastArguments;
    viewIDSymbolCastArguments.push_back(new TIntermSymbol(viewID));
    TIntermAggregate *viewIDAsInt = TIntermAggregate::CreateConstructor(
        TType(EbtInt, EbpHigh, EvqTemporary), &viewIDSymbolCastArguments);

    // Create a gl_Layer node.
    TIntermSymbol *layerSymbol = new TIntermSymbol(BuiltInVariable::gl_LayerVS());

    // Create an int(ViewID_OVR) + multiviewBaseViewLayerIndex node
    TIntermBinary *sumOfViewIDAndBaseViewIndex = new TIntermBinary(
        EOpAdd, viewIDAsInt->deepCopy(), new TIntermSymbol(multiviewBaseViewLayerIndex));

    initializers->push_back(new TIntermBinary(EOpAssign, layerSymbol, sumOfViewIDAndBaseViewIndex));
}

}  // namespace

bool DeclareAndInitBuiltinsForInstancedMultiview(TCompiler *compiler,
                                                 TIntermBlock *root,
                                                 unsigned numberOfViews,
                                                 GLenum shaderType,
                                                 const ShCompileOptions &compileOptions,
                                                 ShShaderOutput shaderOutput,
                                                 TSymbolTable *symbolTable)
{
    ASSERT(shaderType == GL_VERTEX_SHADER || shaderType == GL_FRAGMENT_SHADER);

    const TVariable *viewID =
        new TVariable(symbolTable, kViewIDVariableName, new TType(EbtUInt, EbpHigh, EvqViewIDOVR),
                      SymbolType::AngleInternal);

    DeclareGlobalVariable(root, viewID);
    if (!ReplaceVariable(compiler, root, BuiltInVariable::gl_ViewID_OVR(), viewID))
    {
        return false;
    }
    if (shaderType == GL_VERTEX_SHADER)
    {
        // Replacing gl_InstanceID with InstanceID should happen before adding the initializers of
        // InstanceID and ViewID.
        const TType *instanceIDVariableType = StaticType::Get<EbtInt, EbpHigh, EvqGlobal, 1, 1>();
        const TVariable *instanceID =
            new TVariable(symbolTable, kInstanceIDVariableName, instanceIDVariableType,
                          SymbolType::AngleInternal);
        DeclareGlobalVariable(root, instanceID);
        if (!ReplaceVariable(compiler, root, BuiltInVariable::gl_InstanceID(), instanceID))
        {
            return false;
        }

        TIntermSequence initializers;
        InitializeViewIDAndInstanceID(viewID, instanceID, numberOfViews, *symbolTable,
                                      &initializers);

        // The AST transformation which adds the expression to select the layer index should
        // be done only for the GLSL and ESSL output.
        const bool selectView = compileOptions.selectViewInNvGLSLVertexShader;
        // Assert that if the view is selected in the vertex shader, then the output is
        // either GLSL or ESSL.
        ASSERT(!selectView || IsOutputGLSL(shaderOutput) || IsOutputESSL(shaderOutput));
        if (selectView)
        {
            // Add a uniform to pass the base view index.
            const TType *baseLayerIndexVariableType =
                StaticType::Get<EbtInt, EbpHigh, EvqUniform, 1, 1>();
            const TVariable *multiviewBaseViewLayerIndex =
                new TVariable(symbolTable, kMultiviewBaseViewLayerIndexVariableName,
                              baseLayerIndexVariableType, SymbolType::AngleInternal);
            DeclareGlobalVariable(root, multiviewBaseViewLayerIndex);

            // Setting a value to gl_Layer should happen after ViewID_OVR's initialization.
            SelectViewIndexInVertexShader(viewID, multiviewBaseViewLayerIndex, &initializers,
                                          *symbolTable);
        }

        // Insert initializers at the beginning of main().
        TIntermBlock *initializersBlock = new TIntermBlock();
        initializersBlock->getSequence()->swap(initializers);
        TIntermBlock *mainBody = FindMainBody(root);
        mainBody->getSequence()->insert(mainBody->getSequence()->begin(), initializersBlock);
    }

    return compiler->validateAST(root);
}

}  // namespace sh
