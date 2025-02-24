//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TranslatorSPIRV:
//   A set of transformations that prepare the AST to be compatible with GL_KHR_vulkan_glsl followed
//   by a pass that generates SPIR-V.
//   See: https://www.khronos.org/registry/vulkan/specs/misc/GL_KHR_vulkan_glsl.txt
//

#include "compiler/translator/spirv/TranslatorSPIRV.h"

#include "angle_gl.h"
#include "common/PackedEnums.h"
#include "common/utilities.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/spirv/BuiltinsWorkaround.h"
#include "compiler/translator/spirv/OutputSPIRV.h"
#include "compiler/translator/tree_ops/DeclarePerVertexBlocks.h"
#include "compiler/translator/tree_ops/MonomorphizeUnsupportedFunctions.h"
#include "compiler/translator/tree_ops/RecordConstantPrecision.h"
#include "compiler/translator/tree_ops/RemoveAtomicCounterBuiltins.h"
#include "compiler/translator/tree_ops/RemoveInactiveInterfaceVariables.h"
#include "compiler/translator/tree_ops/RewriteArrayOfArrayOfOpaqueUniforms.h"
#include "compiler/translator/tree_ops/RewriteAtomicCounters.h"
#include "compiler/translator/tree_ops/RewriteDfdy.h"
#include "compiler/translator/tree_ops/RewriteStructSamplers.h"
#include "compiler/translator/tree_ops/SeparateStructFromUniformDeclarations.h"
#include "compiler/translator/tree_ops/spirv/ClampGLLayer.h"
#include "compiler/translator/tree_ops/spirv/EmulateAdvancedBlendEquations.h"
#include "compiler/translator/tree_ops/spirv/EmulateDithering.h"
#include "compiler/translator/tree_ops/spirv/EmulateFragColorData.h"
#include "compiler/translator/tree_ops/spirv/EmulateFramebufferFetch.h"
#include "compiler/translator/tree_ops/spirv/EmulateYUVBuiltIns.h"
#include "compiler/translator/tree_ops/spirv/FlagSamplersWithTexelFetch.h"
#include "compiler/translator/tree_ops/spirv/ReswizzleYUVOps.h"
#include "compiler/translator/tree_ops/spirv/RewriteInterpolateAtOffset.h"
#include "compiler/translator/tree_ops/spirv/RewriteR32fImages.h"
#include "compiler/translator/tree_util/BuiltIn.h"
#include "compiler/translator/tree_util/DriverUniform.h"
#include "compiler/translator/tree_util/FindFunction.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/FindSymbolNode.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/ReplaceClipCullDistanceVariable.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"
#include "compiler/translator/tree_util/RewriteSampleMaskVariable.h"
#include "compiler/translator/tree_util/RunAtTheBeginningOfShader.h"
#include "compiler/translator/tree_util/RunAtTheEndOfShader.h"
#include "compiler/translator/tree_util/SpecializationConstant.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{
constexpr ImmutableString kFlippedPointCoordName    = ImmutableString("flippedPointCoord");
constexpr ImmutableString kFlippedFragCoordName     = ImmutableString("flippedFragCoord");
constexpr ImmutableString kDefaultUniformsBlockName = ImmutableString("defaultUniforms");

bool IsDefaultUniform(const TType &type)
{
    return type.getQualifier() == EvqUniform && type.getInterfaceBlock() == nullptr &&
           !IsOpaqueType(type.getBasicType());
}

class ReplaceDefaultUniformsTraverser : public TIntermTraverser
{
  public:
    ReplaceDefaultUniformsTraverser(const VariableReplacementMap &variableMap)
        : TIntermTraverser(true, false, false), mVariableMap(variableMap)
    {}

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        const TIntermSequence &sequence = *(node->getSequence());

        TIntermTyped *variable = sequence.front()->getAsTyped();
        const TType &type      = variable->getType();

        if (IsDefaultUniform(type))
        {
            // Remove the uniform declaration.
            TIntermSequence emptyReplacement;
            mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), node,
                                            std::move(emptyReplacement));

            return false;
        }

        return true;
    }

    void visitSymbol(TIntermSymbol *symbol) override
    {
        const TVariable &variable = symbol->variable();
        const TType &type         = variable.getType();

        if (!IsDefaultUniform(type) || gl::IsBuiltInName(variable.name().data()))
        {
            return;
        }

        ASSERT(mVariableMap.count(&variable) > 0);

        queueReplacement(mVariableMap.at(&variable)->deepCopy(), OriginalNode::IS_DROPPED);
    }

  private:
    const VariableReplacementMap &mVariableMap;
};

bool DeclareDefaultUniforms(TranslatorSPIRV *compiler,
                            TIntermBlock *root,
                            TSymbolTable *symbolTable,
                            gl::ShaderType shaderType)
{
    // First, collect all default uniforms and declare a uniform block.
    TFieldList *uniformList = new TFieldList;
    TVector<const TVariable *> uniformVars;

    for (TIntermNode *node : *root->getSequence())
    {
        TIntermDeclaration *decl = node->getAsDeclarationNode();
        if (decl == nullptr)
        {
            continue;
        }

        const TIntermSequence &sequence = *(decl->getSequence());

        TIntermSymbol *symbol = sequence.front()->getAsSymbolNode();
        if (symbol == nullptr)
        {
            continue;
        }

        const TType &type = symbol->getType();
        if (IsDefaultUniform(type))
        {
            TType *fieldType = new TType(type);

            uniformList->push_back(new TField(fieldType, symbol->getName(), symbol->getLine(),
                                              symbol->variable().symbolType()));
            uniformVars.push_back(&symbol->variable());
        }
    }

    TLayoutQualifier layoutQualifier = TLayoutQualifier::Create();
    layoutQualifier.blockStorage     = EbsStd140;
    const TVariable *uniformBlock    = DeclareInterfaceBlock(
        root, symbolTable, uniformList, EvqUniform, layoutQualifier, TMemoryQualifier::Create(), 0,
        kDefaultUniformsBlockName, ImmutableString(""));

    compiler->assignSpirvId(uniformBlock->getType().getInterfaceBlock()->uniqueId(),
                            vk::spirv::kIdDefaultUniformsBlock);

    // Create a map from the uniform variables to new variables that reference the fields of the
    // block.
    VariableReplacementMap variableMap;
    for (size_t fieldIndex = 0; fieldIndex < uniformVars.size(); ++fieldIndex)
    {
        const TVariable *variable = uniformVars[fieldIndex];

        TType *replacementType = new TType(variable->getType());
        replacementType->setInterfaceBlockField(uniformBlock->getType().getInterfaceBlock(),
                                                fieldIndex);

        TVariable *replacementVariable =
            new TVariable(symbolTable, variable->name(), replacementType, variable->symbolType());

        variableMap[variable] = new TIntermSymbol(replacementVariable);
    }

    // Finally transform the AST and make sure references to the uniforms are replaced with the new
    // variables.
    ReplaceDefaultUniformsTraverser defaultTraverser(variableMap);
    root->traverse(&defaultTraverser);
    return defaultTraverser.updateTree(compiler, root);
}

// Replaces a builtin variable with a version that is rotated and corrects the X and Y coordinates.
[[nodiscard]] bool RotateAndFlipBuiltinVariable(TCompiler *compiler,
                                                TIntermBlock *root,
                                                TIntermSequence *insertSequence,
                                                TIntermTyped *swapXY,
                                                TIntermTyped *flipXY,
                                                TSymbolTable *symbolTable,
                                                const TVariable *builtin,
                                                const ImmutableString &flippedVariableName,
                                                TIntermTyped *pivot)
{
    // Create a symbol reference to 'builtin'.
    TIntermSymbol *builtinRef = new TIntermSymbol(builtin);

    // Create a symbol reference to our new variable that will hold the modified builtin.
    TType *type = new TType(builtin->getType());
    type->setQualifier(EvqGlobal);
    type->setPrimarySize(builtin->getType().getNominalSize());
    TVariable *replacementVar =
        new TVariable(symbolTable, flippedVariableName, type, SymbolType::AngleInternal);
    DeclareGlobalVariable(root, replacementVar);
    TIntermSymbol *flippedBuiltinRef = new TIntermSymbol(replacementVar);

    // Use this new variable instead of 'builtin' everywhere.
    if (!ReplaceVariable(compiler, root, builtin, replacementVar))
    {
        return false;
    }

    // Create the expression "(swapXY ? builtin.yx : builtin.xy)"
    TIntermTyped *builtinXY = new TIntermSwizzle(builtinRef, {0, 1});
    TIntermTyped *builtinYX = new TIntermSwizzle(builtinRef->deepCopy(), {1, 0});

    builtinXY = new TIntermTernary(swapXY, builtinYX, builtinXY);

    // Create the expression "(builtin.xy - pivot) * flipXY + pivot
    TIntermBinary *removePivot = new TIntermBinary(EOpSub, builtinXY, pivot);
    TIntermBinary *inverseXY   = new TIntermBinary(EOpMul, removePivot, flipXY);
    TIntermBinary *plusPivot   = new TIntermBinary(EOpAdd, inverseXY, pivot->deepCopy());

    // Create the corrected variable and copy the value of the original builtin.
    TIntermBinary *assignment =
        new TIntermBinary(EOpAssign, flippedBuiltinRef, builtinRef->deepCopy());

    // Create an assignment to the replaced variable's .xy.
    TIntermSwizzle *correctedXY = new TIntermSwizzle(flippedBuiltinRef->deepCopy(), {0, 1});
    TIntermBinary *assignToXY   = new TIntermBinary(EOpAssign, correctedXY, plusPivot);

    // Add this assigment at the beginning of the main function
    insertSequence->insert(insertSequence->begin(), assignToXY);
    insertSequence->insert(insertSequence->begin(), assignment);

    return compiler->validateAST(root);
}

TIntermSequence *GetMainSequence(TIntermBlock *root)
{
    TIntermFunctionDefinition *main = FindMain(root);
    return main->getBody()->getSequence();
}

// Declares a new variable to replace gl_DepthRange, its values are fed from a driver uniform.
[[nodiscard]] bool ReplaceGLDepthRangeWithDriverUniform(TCompiler *compiler,
                                                        TIntermBlock *root,
                                                        const DriverUniform *driverUniforms,
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

// Declares a new variable to replace gl_BoundingBoxEXT, its values are fed from a global temporary
// variable.
[[nodiscard]] bool ReplaceGLBoundingBoxWithGlobal(TCompiler *compiler,
                                                  TIntermBlock *root,
                                                  TSymbolTable *symbolTable,
                                                  int shaderVersion)
{
    // Declare the replacement bounding box variable type
    TType *emulatedBoundingBoxDeclType = new TType(EbtFloat, EbpHigh, EvqGlobal, 4);
    emulatedBoundingBoxDeclType->makeArray(2u);

    TVariable *ANGLEBoundingBoxVar = new TVariable(
        symbolTable->nextUniqueId(), ImmutableString("ANGLEBoundingBox"), SymbolType::AngleInternal,
        TExtension::EXT_primitive_bounding_box, emulatedBoundingBoxDeclType);

    DeclareGlobalVariable(root, ANGLEBoundingBoxVar);

    const TVariable *builtinBoundingBoxVar;
    bool replacementResult = true;

    // Create a symbol reference to "gl_BoundingBoxEXT"
    builtinBoundingBoxVar = static_cast<const TVariable *>(
        symbolTable->findBuiltIn(ImmutableString("gl_BoundingBoxEXT"), shaderVersion));
    if (builtinBoundingBoxVar != nullptr)
    {
        // Use the replacement variable instead of builtin gl_BoundingBoxEXT everywhere.
        replacementResult &=
            ReplaceVariable(compiler, root, builtinBoundingBoxVar, ANGLEBoundingBoxVar);
    }

    // Create a symbol reference to "gl_BoundingBoxOES"
    builtinBoundingBoxVar = static_cast<const TVariable *>(
        symbolTable->findBuiltIn(ImmutableString("gl_BoundingBoxOES"), shaderVersion));
    if (builtinBoundingBoxVar != nullptr)
    {
        // Use the replacement variable instead of builtin gl_BoundingBoxOES everywhere.
        replacementResult &=
            ReplaceVariable(compiler, root, builtinBoundingBoxVar, ANGLEBoundingBoxVar);
    }

    if (shaderVersion >= 320)
    {
        // Create a symbol reference to "gl_BoundingBox"
        builtinBoundingBoxVar = static_cast<const TVariable *>(
            symbolTable->findBuiltIn(ImmutableString("gl_BoundingBox"), shaderVersion));
        if (builtinBoundingBoxVar != nullptr)
        {
            // Use the replacement variable instead of builtin gl_BoundingBox everywhere.
            replacementResult &=
                ReplaceVariable(compiler, root, builtinBoundingBoxVar, ANGLEBoundingBoxVar);
        }
    }
    return replacementResult;
}

[[nodiscard]] bool AddXfbEmulationSupport(TranslatorSPIRV *compiler,
                                          TIntermBlock *root,
                                          TSymbolTable *symbolTable,
                                          const DriverUniform *driverUniforms)
{
    // Generate the following function and place it before main().  This function takes a "strides"
    // parameter that is determined at link time, and calculates for each transform feedback buffer
    // (of which there are a maximum of four) what the starting index is to write to the output
    // buffer.
    //
    //     ivec4 ANGLEGetXfbOffsets(ivec4 strides)
    //     {
    //         int xfbIndex = gl_VertexIndex
    //                      + gl_InstanceIndex * ANGLEUniforms.xfbVerticesPerInstance;
    //         return ANGLEUniforms.xfbBufferOffsets + xfbIndex * strides;
    //     }

    constexpr uint32_t kMaxXfbBuffers = 4;

    const TType *ivec4Type = StaticType::GetBasic<EbtInt, EbpHigh, kMaxXfbBuffers>();
    TType *stridesType     = new TType(*ivec4Type);
    stridesType->setQualifier(EvqParamConst);

    // Create the parameter variable.
    TVariable *stridesVar = new TVariable(symbolTable, ImmutableString("strides"), stridesType,
                                          SymbolType::AngleInternal);
    TIntermSymbol *stridesSymbol = new TIntermSymbol(stridesVar);

    // Create references to gl_VertexIndex, gl_InstanceIndex, ANGLEUniforms.xfbVerticesPerInstance
    // and ANGLEUniforms.xfbBufferOffsets.
    TIntermSymbol *vertexIndex           = new TIntermSymbol(BuiltInVariable::gl_VertexIndex());
    TIntermSymbol *instanceIndex         = new TIntermSymbol(BuiltInVariable::gl_InstanceIndex());
    TIntermTyped *xfbVerticesPerInstance = driverUniforms->getXfbVerticesPerInstance();
    TIntermTyped *xfbBufferOffsets       = driverUniforms->getXfbBufferOffsets();

    // gl_InstanceIndex * ANGLEUniforms.xfbVerticesPerInstance
    TIntermBinary *xfbInstanceIndex =
        new TIntermBinary(EOpMul, instanceIndex, xfbVerticesPerInstance);

    // gl_VertexIndex + |xfbInstanceIndex|
    TIntermBinary *xfbIndex = new TIntermBinary(EOpAdd, vertexIndex, xfbInstanceIndex);

    // |xfbIndex| * |strides|
    TIntermBinary *xfbStrides = new TIntermBinary(EOpVectorTimesScalar, xfbIndex, stridesSymbol);

    // ANGLEUniforms.xfbBufferOffsets + |xfbStrides|
    TIntermBinary *xfbOffsets = new TIntermBinary(EOpAdd, xfbBufferOffsets, xfbStrides);

    // Create the function body, which has a single return statement.  Note that the `xfbIndex`
    // variable declared in the comment at the beginning of this function is simply replaced in the
    // return statement for brevity.
    TIntermBlock *body = new TIntermBlock;
    body->appendStatement(new TIntermBranch(EOpReturn, xfbOffsets));

    // Declare the function
    TFunction *getOffsetsFunction =
        new TFunction(symbolTable, ImmutableString("ANGLEGetXfbOffsets"), SymbolType::AngleInternal,
                      ivec4Type, true);
    getOffsetsFunction->addParameter(stridesVar);

    compiler->assignSpirvId(getOffsetsFunction->uniqueId(),
                            vk::spirv::kIdXfbEmulationGetOffsetsFunction);

    TIntermFunctionDefinition *functionDef =
        CreateInternalFunctionDefinitionNode(*getOffsetsFunction, body);

    // Insert the function declaration before main().
    const size_t mainIndex = FindMainIndex(root);
    root->insertChildNodes(mainIndex, {functionDef});

    // Generate the following function and place it before main().  This function will be filled
    // with transform feedback capture code at link time.
    //
    //     void ANGLECaptureXfb()
    //     {
    //     }
    const TType *voidType = StaticType::GetBasic<EbtVoid, EbpUndefined>();

    // Create the function body, which is empty.
    body = new TIntermBlock;

    // Declare the function
    TFunction *xfbCaptureFunction = new TFunction(symbolTable, ImmutableString("ANGLECaptureXfb"),
                                                  SymbolType::AngleInternal, voidType, false);

    compiler->assignSpirvId(xfbCaptureFunction->uniqueId(),
                            vk::spirv::kIdXfbEmulationCaptureFunction);

    // Insert the function declaration before main().
    root->insertChildNodes(mainIndex,
                           {CreateInternalFunctionDefinitionNode(*xfbCaptureFunction, body)});

    // Create the following logic and add it at the end of main():
    //
    //     ANGLECaptureXfb();
    //

    // Create the function call
    TIntermAggregate *captureXfbCall =
        TIntermAggregate::CreateFunctionCall(*xfbCaptureFunction, {});

    // Run it at the end of the shader.
    if (!RunAtTheEndOfShader(compiler, root, captureXfbCall, symbolTable))
    {
        return false;
    }

    // Additionally, generate the following storage buffer declarations used to capture transform
    // feedback output.  Again, there's a maximum of four buffers.
    //
    //     buffer ANGLEXfbBuffer0
    //     {
    //         float xfbOut[];
    //     } ANGLEXfb0;
    //     buffer ANGLEXfbBuffer1
    //     {
    //         float xfbOut[];
    //     } ANGLEXfb1;
    //     ...

    for (uint32_t bufferIndex = 0; bufferIndex < kMaxXfbBuffers; ++bufferIndex)
    {
        TFieldList *fieldList = new TFieldList;
        TType *xfbOutType     = new TType(EbtFloat, EbpHigh, EvqGlobal);
        xfbOutType->makeArray(0);

        TField *field = new TField(xfbOutType, ImmutableString("xfbOut"), TSourceLoc(),
                                   SymbolType::AngleInternal);

        fieldList->push_back(field);

        static_assert(
            kMaxXfbBuffers < 10,
            "ImmutableStringBuilder memory size below needs to accomodate the number of buffers");

        ImmutableString blockName = BuildConcatenatedImmutableString("ANGLEXfbBuffer", bufferIndex);
        ImmutableString varName   = BuildConcatenatedImmutableString("ANGLEXfb", bufferIndex);

        TLayoutQualifier layoutQualifier = TLayoutQualifier::Create();
        layoutQualifier.blockStorage     = EbsStd430;

        const TVariable *xfbBuffer =
            DeclareInterfaceBlock(root, symbolTable, fieldList, EvqBuffer, layoutQualifier,
                                  TMemoryQualifier::Create(), 0, blockName, varName);

        static_assert(vk::spirv::kIdXfbEmulationBufferBlockOne ==
                      vk::spirv::kIdXfbEmulationBufferBlockZero + 1);
        static_assert(vk::spirv::kIdXfbEmulationBufferBlockTwo ==
                      vk::spirv::kIdXfbEmulationBufferBlockZero + 2);
        static_assert(vk::spirv::kIdXfbEmulationBufferBlockThree ==
                      vk::spirv::kIdXfbEmulationBufferBlockZero + 3);

        static_assert(vk::spirv::kIdXfbEmulationBufferVarOne ==
                      vk::spirv::kIdXfbEmulationBufferVarZero + 1);
        static_assert(vk::spirv::kIdXfbEmulationBufferVarTwo ==
                      vk::spirv::kIdXfbEmulationBufferVarZero + 2);
        static_assert(vk::spirv::kIdXfbEmulationBufferVarThree ==
                      vk::spirv::kIdXfbEmulationBufferVarZero + 3);

        compiler->assignSpirvId(xfbBuffer->getType().getInterfaceBlock()->uniqueId(),
                                vk::spirv::kIdXfbEmulationBufferBlockZero + bufferIndex);
        compiler->assignSpirvId(xfbBuffer->uniqueId(),
                                vk::spirv::kIdXfbEmulationBufferVarZero + bufferIndex);
    }

    return compiler->validateAST(root);
}

[[nodiscard]] bool AddXfbExtensionSupport(TranslatorSPIRV *compiler,
                                          TIntermBlock *root,
                                          TSymbolTable *symbolTable,
                                          const DriverUniform *driverUniforms)
{
    // Generate the following output varying declaration used to capture transform feedback output
    // from gl_Position, as it can't be captured directly due to changes that are applied to it for
    // clip-space correction and pre-rotation.
    //
    //     out vec4 ANGLEXfbPosition;

    const TType *vec4Type = nullptr;

    switch (compiler->getShaderType())
    {
        case GL_VERTEX_SHADER:
            vec4Type = StaticType::Get<EbtFloat, EbpHigh, EvqVertexOut, 4, 1>();
            break;
        case GL_TESS_EVALUATION_SHADER_EXT:
            vec4Type = StaticType::Get<EbtFloat, EbpHigh, EvqTessEvaluationOut, 4, 1>();
            break;
        case GL_GEOMETRY_SHADER_EXT:
            vec4Type = StaticType::Get<EbtFloat, EbpHigh, EvqGeometryOut, 4, 1>();
            break;
        default:
            UNREACHABLE();
    }

    TVariable *varyingVar = new TVariable(symbolTable, ImmutableString("ANGLEXfbPosition"),
                                          vec4Type, SymbolType::AngleInternal);

    compiler->assignSpirvId(varyingVar->uniqueId(), vk::spirv::kIdXfbExtensionPosition);

    TIntermDeclaration *varyingDecl = new TIntermDeclaration();
    varyingDecl->appendDeclarator(new TIntermSymbol(varyingVar));

    // Insert the varying declaration before the first function.
    const size_t firstFunctionIndex = FindFirstFunctionDefinitionIndex(root);
    root->insertChildNodes(firstFunctionIndex, {varyingDecl});

    return compiler->validateAST(root);
}

[[nodiscard]] bool AddVertexTransformationSupport(TranslatorSPIRV *compiler,
                                                  const ShCompileOptions &compileOptions,
                                                  TIntermBlock *root,
                                                  TSymbolTable *symbolTable,
                                                  SpecConst *specConst,
                                                  const DriverUniform *driverUniforms)
{
    // In GL the viewport transformation is slightly different - see the GL 2.0 spec section "2.12.1
    // Controlling the Viewport".  In Vulkan the corresponding spec section is currently "23.4.
    // Coordinate Transformations".  The following transformation needs to be done:
    //
    //     z_vk = 0.5 * (w_gl + z_gl)
    //
    // where z_vk is the depth output of a Vulkan geometry-stage shader and z_gl is the same for GL.
    //
    // Generate the following function and place it before main().  This function takes
    // gl_Position and rotates xy, and adjusts z (if necessary).
    //
    //     vec4 ANGLETransformPosition(vec4 position)
    //     {
    //         return vec4((swapXY ? position.yx : position.xy) * flipXY,
    //                     transformDepth ? (gl_Position.z + gl_Position.w) / 2 : gl_Position.z,
    //                     gl_Postion.w);
    //     }

    const TType *vec4Type = StaticType::GetBasic<EbtFloat, EbpHigh, 4>();
    TType *positionType   = new TType(*vec4Type);
    positionType->setQualifier(EvqParamConst);

    // Create the parameter variable.
    TVariable *positionVar = new TVariable(symbolTable, ImmutableString("position"), positionType,
                                           SymbolType::AngleInternal);
    TIntermSymbol *positionSymbol = new TIntermSymbol(positionVar);

    // swapXY ? position.yx : position.xy
    TIntermTyped *swapXY = specConst->getSwapXY();
    if (swapXY == nullptr)
    {
        swapXY = driverUniforms->getSwapXY();
    }

    TIntermTyped *xy        = new TIntermSwizzle(positionSymbol, {0, 1});
    TIntermTyped *swappedXY = new TIntermSwizzle(positionSymbol->deepCopy(), {1, 0});
    TIntermTyped *rotatedXY = new TIntermTernary(swapXY, swappedXY, xy);

    // (swapXY ? position.yx : position.xy) * flipXY
    TIntermTyped *flipXY = driverUniforms->getFlipXY(symbolTable, DriverUniformFlip::PreFragment);
    TIntermTyped *rotatedFlippedXY = new TIntermBinary(EOpMul, rotatedXY, flipXY);

    // (gl_Position.z + gl_Position.w) / 2
    TIntermTyped *z = new TIntermSwizzle(positionSymbol->deepCopy(), {2});
    TIntermTyped *w = new TIntermSwizzle(positionSymbol->deepCopy(), {3});

    TIntermTyped *transformedDepth = z;
    if (compileOptions.addVulkanDepthCorrection)
    {
        TIntermBinary *zPlusW = new TIntermBinary(EOpAdd, z, w->deepCopy());
        TIntermBinary *halfZPlusW =
            new TIntermBinary(EOpMul, zPlusW, CreateFloatNode(0.5, EbpMedium));

        // transformDepth ? (gl_Position.z + gl_Position.w) / 2 : gl_Position.z,
        TIntermTyped *transformDepth = driverUniforms->getTransformDepth();
        transformedDepth = new TIntermTernary(transformDepth, halfZPlusW, z->deepCopy());
    }

    // vec4(...);
    TIntermSequence args = {
        rotatedFlippedXY,
        transformedDepth,
        w,
    };
    TIntermTyped *transformedPosition = TIntermAggregate::CreateConstructor(*vec4Type, &args);

    // Create the function body, which has a single return statement.
    TIntermBlock *body = new TIntermBlock;
    body->appendStatement(new TIntermBranch(EOpReturn, transformedPosition));

    // Declare the function
    TFunction *transformPositionFunction =
        new TFunction(symbolTable, ImmutableString("ANGLETransformPosition"),
                      SymbolType::AngleInternal, vec4Type, true);
    transformPositionFunction->addParameter(positionVar);

    compiler->assignSpirvId(transformPositionFunction->uniqueId(),
                            vk::spirv::kIdTransformPositionFunction);

    TIntermFunctionDefinition *functionDef =
        CreateInternalFunctionDefinitionNode(*transformPositionFunction, body);

    // Insert the function declaration before main().
    const size_t mainIndex = FindMainIndex(root);
    root->insertChildNodes(mainIndex, {functionDef});

    return compiler->validateAST(root);
}

[[nodiscard]] bool InsertFragCoordCorrection(TCompiler *compiler,
                                             const ShCompileOptions &compileOptions,
                                             TIntermBlock *root,
                                             TIntermSequence *insertSequence,
                                             TSymbolTable *symbolTable,
                                             SpecConst *specConst,
                                             const DriverUniform *driverUniforms)
{
    TIntermTyped *flipXY = driverUniforms->getFlipXY(symbolTable, DriverUniformFlip::Fragment);
    TIntermTyped *pivot  = driverUniforms->getHalfRenderArea();

    TIntermTyped *swapXY = specConst->getSwapXY();
    if (swapXY == nullptr)
    {
        swapXY = driverUniforms->getSwapXY();
    }

    const TVariable *fragCoord = static_cast<const TVariable *>(
        symbolTable->findBuiltIn(ImmutableString("gl_FragCoord"), compiler->getShaderVersion()));
    return RotateAndFlipBuiltinVariable(compiler, root, insertSequence, swapXY, flipXY, symbolTable,
                                        fragCoord, kFlippedFragCoordName, pivot);
}

bool HasFramebufferFetch(const TExtensionBehavior &extBehavior,
                         const ShCompileOptions &compileOptions)
{
    return IsExtensionEnabled(extBehavior, TExtension::EXT_shader_framebuffer_fetch) ||
           IsExtensionEnabled(extBehavior, TExtension::EXT_shader_framebuffer_fetch_non_coherent) ||
           IsExtensionEnabled(extBehavior, TExtension::ARM_shader_framebuffer_fetch) ||
           IsExtensionEnabled(extBehavior,
                              TExtension::ARM_shader_framebuffer_fetch_depth_stencil) ||
           IsExtensionEnabled(extBehavior, TExtension::NV_shader_framebuffer_fetch) ||
           (compileOptions.pls.type == ShPixelLocalStorageType::FramebufferFetch &&
            IsExtensionEnabled(extBehavior, TExtension::ANGLE_shader_pixel_local_storage));
}

template <typename Variable>
Variable *FindShaderVariable(std::vector<Variable> *vars, const ImmutableString &name)
{
    for (Variable &var : *vars)
    {
        if (name == var.name)
        {
            return &var;
        }
    }
    UNREACHABLE();
    return nullptr;
}

ShaderVariable *FindIOBlockShaderVariable(std::vector<ShaderVariable> *vars,
                                          const ImmutableString &name)
{
    for (ShaderVariable &var : *vars)
    {
        if (name == var.structOrBlockName)
        {
            return &var;
        }
    }
    UNREACHABLE();
    return nullptr;
}

ShaderVariable *FindUniformFieldShaderVariable(std::vector<ShaderVariable> *vars,
                                               const ImmutableString &name,
                                               const char *prefix)
{
    for (ShaderVariable &var : *vars)
    {
        // The name of the sampler is derived from the uniform name + fields
        // that reach the uniform, concatenated with '_' per RewriteStructSamplers.
        std::string varName = prefix;
        varName += '_';
        varName += var.name;

        if (name == varName)
        {
            return &var;
        }

        ShaderVariable *field = FindUniformFieldShaderVariable(&var.fields, name, varName.c_str());
        if (field != nullptr)
        {
            return field;
        }
    }
    return nullptr;
}

ShaderVariable *FindUniformShaderVariable(std::vector<ShaderVariable> *vars,
                                          const ImmutableString &name)
{
    for (ShaderVariable &var : *vars)
    {
        if (name == var.name)
        {
            return &var;
        }

        // Note: samplers in structs are moved out.  Such samplers will be found in the fields of
        // the struct uniform.
        ShaderVariable *field = FindUniformFieldShaderVariable(&var.fields, name, var.name.c_str());
        if (field != nullptr)
        {
            return field;
        }
    }
    UNREACHABLE();
    return nullptr;
}

void SetSpirvIdInFields(uint32_t id, std::vector<ShaderVariable> *fields)
{
    for (ShaderVariable &field : *fields)
    {
        field.id = id;
        SetSpirvIdInFields(id, &field.fields);
    }
}
}  // anonymous namespace

TranslatorSPIRV::TranslatorSPIRV(sh::GLenum type, ShShaderSpec spec)
    : TCompiler(type, spec, SH_SPIRV_VULKAN_OUTPUT), mFirstUnusedSpirvId(0)
{}

bool TranslatorSPIRV::translateImpl(TIntermBlock *root,
                                    const ShCompileOptions &compileOptions,
                                    PerformanceDiagnostics * /*perfDiagnostics*/,
                                    SpecConst *specConst,
                                    DriverUniform *driverUniforms)
{
    if (getShaderType() == GL_VERTEX_SHADER)
    {
        if (!ShaderBuiltinsWorkaround(this, root, &getSymbolTable(), compileOptions))
        {
            return false;
        }
    }

    // Write out default uniforms into a uniform block assigned to a specific set/binding.
    int defaultUniformCount           = 0;
    int aggregateTypesUsedForUniforms = 0;
    int r32fImageCount                = 0;
    int atomicCounterCount            = 0;
    for (const auto &uniform : getUniforms())
    {
        if (!uniform.isBuiltIn() && uniform.active && !gl::IsOpaqueType(uniform.type))
        {
            ++defaultUniformCount;
        }

        if (uniform.isStruct() || uniform.isArrayOfArrays())
        {
            ++aggregateTypesUsedForUniforms;
        }

        if (uniform.active && gl::IsImageType(uniform.type) && uniform.imageUnitFormat == GL_R32F)
        {
            ++r32fImageCount;
        }

        if (uniform.active && gl::IsAtomicCounterType(uniform.type))
        {
            ++atomicCounterCount;
        }
    }

    // Remove declarations of inactive shader interface variables so SPIR-V transformer doesn't need
    // to replace them.  Note that currently, CollectVariables marks every field of an active
    // uniform that's of struct type as active, i.e. no extracted sampler is inactive, so this can
    // be done before extracting samplers from structs.
    if (!RemoveInactiveInterfaceVariables(this, root, &getSymbolTable(), getAttributes(),
                                          getInputVaryings(), getOutputVariables(), getUniforms(),
                                          getInterfaceBlocks(), true))
    {
        return false;
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
        if (!SeparateStructFromUniformDeclarations(this, root, &getSymbolTable()))
        {
            return false;
        }

        int removedUniformsCount;

        if (!RewriteStructSamplers(this, root, &getSymbolTable(), &removedUniformsCount))
        {
            return false;
        }
        defaultUniformCount -= removedUniformsCount;
    }

    // Replace array of array of opaque uniforms with a flattened array.  This is run after
    // MonomorphizeUnsupportedFunctions and RewriteStructSamplers so that it's not possible for an
    // array of array of opaque type to be partially subscripted and passed to a function.
    if (!RewriteArrayOfArrayOfOpaqueUniforms(this, root, &getSymbolTable()))
    {
        return false;
    }

    if (!FlagSamplersForTexelFetch(this, root, &getSymbolTable(), &mUniforms))
    {
        return false;
    }

    gl::ShaderType packedShaderType = gl::FromGLenum<gl::ShaderType>(getShaderType());

    if (defaultUniformCount > 0)
    {
        if (!DeclareDefaultUniforms(this, root, &getSymbolTable(), packedShaderType))
        {
            return false;
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

    assignSpirvId(
        driverUniforms->getDriverUniformsVariable()->getType().getInterfaceBlock()->uniqueId(),
        vk::spirv::kIdDriverUniformsBlock);

    if (r32fImageCount > 0 && compileOptions.emulateR32fImageAtomicExchange)
    {
        if (!RewriteR32fImages(this, root, &getSymbolTable()))
        {
            return false;
        }
    }

    if (atomicCounterCount > 0)
    {
        // ANGLEUniforms.acbBufferOffsets
        const TIntermTyped *acbBufferOffsets = driverUniforms->getAcbBufferOffsets();
        const TVariable *atomicCounters      = nullptr;
        if (!RewriteAtomicCounters(this, root, &getSymbolTable(), acbBufferOffsets,
                                   &atomicCounters))
        {
            return false;
        }
        assignSpirvId(atomicCounters->getType().getInterfaceBlock()->uniqueId(),
                      vk::spirv::kIdAtomicCounterBlock);
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

    if (packedShaderType != gl::ShaderType::Compute)
    {
        if (!ReplaceGLDepthRangeWithDriverUniform(this, root, driverUniforms, &getSymbolTable()))
        {
            return false;
        }

        // Search for the gl_ClipDistance/gl_CullDistance usage, if its used, we need to do some
        // replacements.
        bool useClipDistance = false;
        bool useCullDistance = false;
        for (const ShaderVariable &outputVarying : mOutputVaryings)
        {
            if (outputVarying.name == "gl_ClipDistance")
            {
                useClipDistance = true;
            }
            else if (outputVarying.name == "gl_CullDistance")
            {
                useCullDistance = true;
            }
        }
        for (const ShaderVariable &inputVarying : mInputVaryings)
        {
            if (inputVarying.name == "gl_ClipDistance")
            {
                useClipDistance = true;
            }
            else if (inputVarying.name == "gl_CullDistance")
            {
                useCullDistance = true;
            }
        }

        if (useClipDistance &&
            !ReplaceClipDistanceAssignments(this, root, &getSymbolTable(), getShaderType(),
                                            driverUniforms->getClipDistancesEnabled()))
        {
            return false;
        }
        if (useCullDistance &&
            !ReplaceCullDistanceAssignments(this, root, &getSymbolTable(), getShaderType()))
        {
            return false;
        }
    }

    if (gl::ShaderTypeSupportsTransformFeedback(packedShaderType))
    {
        if (compileOptions.addVulkanXfbExtensionSupportCode)
        {
            // Add support code for transform feedback extension.
            if (!AddXfbExtensionSupport(this, root, &getSymbolTable(), driverUniforms))
            {
                return false;
            }
        }

        // Add support code for pre-rotation and depth correction in the vertex processing stages.
        if (!AddVertexTransformationSupport(this, compileOptions, root, &getSymbolTable(),
                                            specConst, driverUniforms))
        {
            return false;
        }
    }

    switch (packedShaderType)
    {
        case gl::ShaderType::Fragment:
        {
            bool usesPointCoord    = false;
            bool usesFragCoord     = false;
            bool usesSampleMaskIn  = false;
            bool useSamplePosition = false;

            // Search for the gl_PointCoord usage, if its used, we need to flip the y coordinate.
            for (const ShaderVariable &inputVarying : mInputVaryings)
            {
                if (!inputVarying.isBuiltIn())
                {
                    continue;
                }

                if (inputVarying.name == "gl_SampleMaskIn")
                {
                    usesSampleMaskIn = true;
                    continue;
                }

                if (inputVarying.name == "gl_SamplePosition")
                {
                    useSamplePosition = true;
                    continue;
                }

                if (inputVarying.name == "gl_PointCoord")
                {
                    usesPointCoord = true;
                    break;
                }

                if (inputVarying.name == "gl_FragCoord")
                {
                    usesFragCoord = true;
                    break;
                }
            }

            bool hasGLSampleMask           = false;
            bool hasGLSecondaryFragData    = false;
            const TIntermSymbol *yuvOutput = nullptr;

            for (const ShaderVariable &outputVar : mOutputVariables)
            {
                if (outputVar.name == "gl_SampleMask")
                {
                    ASSERT(!hasGLSampleMask);
                    hasGLSampleMask = true;
                    continue;
                }
                if (outputVar.name == "gl_SecondaryFragDataEXT")
                {
                    ASSERT(!hasGLSecondaryFragData);
                    hasGLSecondaryFragData = true;
                    continue;
                }
                if (outputVar.yuv)
                {
                    // We can only have one yuv output
                    ASSERT(yuvOutput == nullptr);
                    yuvOutput = FindSymbolNode(root, ImmutableString(outputVar.name));
                    continue;
                }
            }

            if (usesPointCoord)
            {
                TIntermTyped *flipNegXY =
                    driverUniforms->getNegFlipXY(&getSymbolTable(), DriverUniformFlip::Fragment);
                TIntermConstantUnion *pivot = CreateFloatNode(0.5f, EbpMedium);
                TIntermTyped *swapXY        = specConst->getSwapXY();
                if (swapXY == nullptr)
                {
                    swapXY = driverUniforms->getSwapXY();
                }
                if (!RotateAndFlipBuiltinVariable(
                        this, root, GetMainSequence(root), swapXY, flipNegXY, &getSymbolTable(),
                        BuiltInVariable::gl_PointCoord(), kFlippedPointCoordName, pivot))
                {
                    return false;
                }
            }

            if (useSamplePosition)
            {
                TIntermTyped *flipXY =
                    driverUniforms->getFlipXY(&getSymbolTable(), DriverUniformFlip::Fragment);
                TIntermConstantUnion *pivot = CreateFloatNode(0.5f, EbpMedium);
                TIntermTyped *swapXY        = specConst->getSwapXY();
                if (swapXY == nullptr)
                {
                    swapXY = driverUniforms->getSwapXY();
                }

                const TVariable *samplePositionBuiltin =
                    static_cast<const TVariable *>(getSymbolTable().findBuiltIn(
                        ImmutableString("gl_SamplePosition"), getShaderVersion()));
                if (!RotateAndFlipBuiltinVariable(this, root, GetMainSequence(root), swapXY, flipXY,
                                                  &getSymbolTable(), samplePositionBuiltin,
                                                  kFlippedPointCoordName, pivot))
                {
                    return false;
                }
            }

            if (usesFragCoord)
            {
                if (!InsertFragCoordCorrection(this, compileOptions, root, GetMainSequence(root),
                                               &getSymbolTable(), specConst, driverUniforms))
                {
                    return false;
                }
            }

            // Emulate gl_FragColor and gl_FragData with normal output variables.
            if (!EmulateFragColorData(this, root, &getSymbolTable(), hasGLSecondaryFragData))
            {
                return false;
            }

            InputAttachmentMap inputAttachmentMap;

            // Emulate framebuffer fetch if used.
            if (HasFramebufferFetch(getExtensionBehavior(), compileOptions))
            {
                if (!EmulateFramebufferFetch(this, root, &inputAttachmentMap))
                {
                    return false;
                }
            }

            // This should be operated after doing ReplaceLastFragData and ReplaceInOutVariables,
            // because they will create the input attachment variables. AddBlendMainCaller will
            // check the existing input attachment variables and if there is no existing input
            // attachment variable then create a new one.
            if (getAdvancedBlendEquations().any() &&
                compileOptions.addAdvancedBlendEquationsEmulation &&
                !EmulateAdvancedBlendEquations(this, root, &getSymbolTable(),
                                               getAdvancedBlendEquations(), driverUniforms,
                                               &inputAttachmentMap))
            {
                return false;
            }

            // Input attachments are potentially added in framebuffer fetch and advanced blend
            // emulation.  Declare their SPIR-V ids.
            assignInputAttachmentIds(inputAttachmentMap);

            if (!RewriteDfdy(this, root, &getSymbolTable(), getShaderVersion(), specConst,
                             driverUniforms))
            {
                return false;
            }

            if (!RewriteInterpolateAtOffset(this, root, &getSymbolTable(), getShaderVersion(),
                                            specConst, driverUniforms))
            {
                return false;
            }

            if (usesSampleMaskIn && !RewriteSampleMaskIn(this, root, &getSymbolTable()))
            {
                return false;
            }

            if (hasGLSampleMask)
            {
                TIntermTyped *numSamples = driverUniforms->getNumSamples();
                if (!RewriteSampleMask(this, root, &getSymbolTable(), numSamples))
                {
                    return false;
                }
            }

            {
                const TVariable *numSamplesVar =
                    static_cast<const TVariable *>(getSymbolTable().findBuiltIn(
                        ImmutableString("gl_NumSamples"), getShaderVersion()));
                TIntermTyped *numSamples = driverUniforms->getNumSamples();
                if (!ReplaceVariableWithTyped(this, root, numSamplesVar, numSamples))
                {
                    return false;
                }
            }

            if (IsExtensionEnabled(getExtensionBehavior(), TExtension::EXT_YUV_target))
            {
                if (!EmulateYUVBuiltIns(this, root, &getSymbolTable()))
                {
                    return false;
                }

                if (!ReswizzleYUVOps(this, root, &getSymbolTable(), yuvOutput))
                {
                    return false;
                }
            }

            if (!EmulateDithering(this, compileOptions, root, &getSymbolTable(), specConst,
                                  driverUniforms))
            {
                return false;
            }

            break;
        }

        case gl::ShaderType::Vertex:
        {
            if (compileOptions.addVulkanXfbEmulationSupportCode)
            {
                // Add support code for transform feedback emulation.  Only applies to vertex shader
                // as tessellation and geometry shader transform feedback capture require
                // VK_EXT_transform_feedback.
                if (!AddXfbEmulationSupport(this, root, &getSymbolTable(), driverUniforms))
                {
                    return false;
                }
            }

            break;
        }

        case gl::ShaderType::Geometry:
            if (!ClampGLLayer(this, root, &getSymbolTable(), driverUniforms))
            {
                return false;
            }
            break;

        case gl::ShaderType::TessControl:
        {
            if (!ReplaceGLBoundingBoxWithGlobal(this, root, &getSymbolTable(), getShaderVersion()))
            {
                return false;
            }
            break;
        }

        case gl::ShaderType::TessEvaluation:
            break;

        case gl::ShaderType::Compute:
            break;

        default:
            UNREACHABLE();
            break;
    }

    specConst->declareSpecConsts(root);
    mValidateASTOptions.validateSpecConstReferences = true;

    // Gather specialization constant usage bits so that we can feedback to context.
    mSpecConstUsageBits = specConst->getSpecConstUsageBits();

    if (!validateAST(root))
    {
        return false;
    }

    // Make sure function call validation is not accidentally left off anywhere.
    ASSERT(mValidateASTOptions.validateFunctionCall);
    ASSERT(mValidateASTOptions.validateNoRawFunctionCalls);

    // Declare the implicitly defined gl_PerVertex I/O blocks if not already.  This will help SPIR-V
    // generation treat them mostly like usual I/O blocks.
    const TVariable *inputPerVertex  = nullptr;
    const TVariable *outputPerVertex = nullptr;
    if (!DeclarePerVertexBlocks(this, root, &getSymbolTable(), &inputPerVertex, &outputPerVertex))
    {
        return false;
    }

    if (inputPerVertex)
    {
        assignSpirvId(inputPerVertex->getType().getInterfaceBlock()->uniqueId(),
                      vk::spirv::kIdInputPerVertexBlock);
    }
    if (outputPerVertex)
    {
        assignSpirvId(outputPerVertex->getType().getInterfaceBlock()->uniqueId(),
                      vk::spirv::kIdOutputPerVertexBlock);
        assignSpirvId(outputPerVertex->uniqueId(), vk::spirv::kIdOutputPerVertexVar);
    }

    // Now that all transformations are done, assign SPIR-V ids to whatever shader variable is still
    // present in the shader in some form.  This should be the last thing done in this function.
    assignSpirvIds(root);

    return true;
}

bool TranslatorSPIRV::translate(TIntermBlock *root,
                                const ShCompileOptions &compileOptions,
                                PerformanceDiagnostics *perfDiagnostics)
{
    mUniqueToSpirvIdMap.clear();
    mFirstUnusedSpirvId = 0;

    SpecConst specConst(&getSymbolTable(), compileOptions, getShaderType());

    DriverUniform driverUniforms(DriverUniformMode::InterfaceBlock);
    DriverUniformExtended driverUniformsExt(DriverUniformMode::InterfaceBlock);

    const bool useExtendedDriverUniforms = compileOptions.addVulkanXfbEmulationSupportCode;

    DriverUniform *uniforms = useExtendedDriverUniforms ? &driverUniformsExt : &driverUniforms;

    if (!translateImpl(root, compileOptions, perfDiagnostics, &specConst, uniforms))
    {
        return false;
    }

    return OutputSPIRV(this, root, compileOptions, mUniqueToSpirvIdMap, mFirstUnusedSpirvId);
}

bool TranslatorSPIRV::shouldFlattenPragmaStdglInvariantAll()
{
    // Not necessary.
    return false;
}

void TranslatorSPIRV::assignSpirvId(TSymbolUniqueId uniqueId, uint32_t spirvId)
{
    ASSERT(mUniqueToSpirvIdMap.find(uniqueId.get()) == mUniqueToSpirvIdMap.end());
    mUniqueToSpirvIdMap[uniqueId.get()] = spirvId;
}

void TranslatorSPIRV::assignInputAttachmentIds(const InputAttachmentMap &inputAttachmentMap)
{
    for (auto &iter : inputAttachmentMap.color)
    {
        const uint32_t index = iter.first;
        const TVariable *var = iter.second;
        ASSERT(var != nullptr);

        assignSpirvId(var->uniqueId(), vk::spirv::kIdInputAttachment0 + index);

        const MetadataFlags flag = static_cast<MetadataFlags>(
            static_cast<uint32_t>(MetadataFlags::HasInputAttachment0) + index);
        mMetadataFlags.set(flag);
    }

    if (inputAttachmentMap.depth != nullptr)
    {
        assignSpirvId(inputAttachmentMap.depth->uniqueId(), vk::spirv::kIdDepthInputAttachment);
        mMetadataFlags.set(MetadataFlags::HasDepthInputAttachment);
    }

    if (inputAttachmentMap.stencil != nullptr)
    {
        assignSpirvId(inputAttachmentMap.stencil->uniqueId(), vk::spirv::kIdStencilInputAttachment);
        mMetadataFlags.set(MetadataFlags::HasStencilInputAttachment);
    }
}

void TranslatorSPIRV::assignSpirvIds(TIntermBlock *root)
{
    // Match the declarations with collected variables and assign a new id to each, starting from
    // the first unreserved id.  This makes sure that the reserved ids for internal variables and
    // ids for shader variables form a minimal contiguous range.  The Vulkan backend takes advantage
    // of this fact for optimal hashing.
    mFirstUnusedSpirvId = vk::spirv::kIdFirstUnreserved;

    for (TIntermNode *node : *root->getSequence())
    {
        TIntermDeclaration *decl = node->getAsDeclarationNode();
        if (decl == nullptr)
        {
            continue;
        }

        TIntermSymbol *symbol = decl->getSequence()->front()->getAsSymbolNode();
        if (symbol == nullptr)
        {
            continue;
        }

        const TType &type          = symbol->getType();
        const TQualifier qualifier = type.getQualifier();

        // Skip internal symbols, which already have a reserved id.
        const TSymbolUniqueId uniqueId =
            type.isInterfaceBlock() ? type.getInterfaceBlock()->uniqueId() : symbol->uniqueId();
        if (mUniqueToSpirvIdMap.find(uniqueId.get()) != mUniqueToSpirvIdMap.end())
        {
            continue;
        }

        uint32_t *variableId                = nullptr;
        std::vector<ShaderVariable> *fields = nullptr;
        if (type.isInterfaceBlock())
        {
            if (IsVaryingIn(qualifier))
            {
                ShaderVariable *varying =
                    FindIOBlockShaderVariable(&mInputVaryings, type.getInterfaceBlock()->name());
                variableId = &varying->id;
                fields     = &varying->fields;
            }
            else if (IsVaryingOut(qualifier))
            {
                ShaderVariable *varying =
                    FindIOBlockShaderVariable(&mOutputVaryings, type.getInterfaceBlock()->name());
                variableId = &varying->id;
                fields     = &varying->fields;
            }
            else if (IsStorageBuffer(qualifier))
            {
                InterfaceBlock *block =
                    FindShaderVariable(&mShaderStorageBlocks, type.getInterfaceBlock()->name());
                variableId = &block->id;
            }
            else
            {
                InterfaceBlock *block =
                    FindShaderVariable(&mUniformBlocks, type.getInterfaceBlock()->name());
                variableId = &block->id;
            }
        }
        else if (qualifier == EvqUniform)
        {
            ShaderVariable *uniform = FindUniformShaderVariable(&mUniforms, symbol->getName());
            variableId              = &uniform->id;
        }
        else if (qualifier == EvqAttribute || qualifier == EvqVertexIn)
        {
            ShaderVariable *attribute = FindShaderVariable(&mAttributes, symbol->getName());
            variableId                = &attribute->id;
        }
        else if (IsShaderIn(qualifier))
        {
            ShaderVariable *varying = FindShaderVariable(&mInputVaryings, symbol->getName());
            variableId              = &varying->id;
            fields                  = &varying->fields;
        }
        else if (qualifier == EvqFragmentOut)
        {
            // webgl_FragColor, webgl_FragData, webgl_SecondaryFragColor and webgl_SecondaryFragData
            // are recorded with their original names (starting with gl_)
            ImmutableString name(symbol->getName());
            if (angle::BeginsWith(name.data(), "webgl_") &&
                symbol->variable().symbolType() == SymbolType::AngleInternal)
            {
                name = ImmutableString(name.data() + 3, name.length() - 3);
            }

            ShaderVariable *output = FindShaderVariable(&mOutputVariables, name);
            variableId             = &output->id;
        }
        else if (IsShaderOut(qualifier))
        {
            ShaderVariable *varying = FindShaderVariable(&mOutputVaryings, symbol->getName());
            variableId              = &varying->id;
            fields                  = &varying->fields;
        }

        if (variableId == nullptr)
        {
            continue;
        }

        ASSERT(variableId != nullptr);
        assignSpirvId(uniqueId, mFirstUnusedSpirvId);
        *variableId = mFirstUnusedSpirvId;

        // Propagate the id to the first field of structs/blocks too.  The front-end gathers
        // varyings as fields, and the transformer needs to infer the variable id (of struct type)
        // just by looking at the fields.
        if (fields != nullptr)
        {
            SetSpirvIdInFields(mFirstUnusedSpirvId, fields);
        }

        ++mFirstUnusedSpirvId;
    }
}
}  // namespace sh
