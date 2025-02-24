//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SpecializationConst.cpp: Add code to generate AST node for various specialization constants.
//

#include "compiler/translator/tree_util/SpecializationConstant.h"
#include "common/PackedEnums.h"
#include "common/angleutils.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermNode_util.h"

namespace sh
{

namespace
{
// Specialization constant names
constexpr ImmutableString kSurfaceRotationSpecConstVarName =
    ImmutableString("ANGLESurfaceRotation");
constexpr ImmutableString kDitherSpecConstVarName = ImmutableString("ANGLEDither");

const TType *MakeSpecConst(const TType &type, vk::SpecializationConstantId id)
{
    // Create a new type with the EvqSpecConst qualifier
    TType *specConstType = new TType(type);
    specConstType->setQualifier(EvqSpecConst);

    // Set the constant_id of the spec const
    TLayoutQualifier layoutQualifier = TLayoutQualifier::Create();
    layoutQualifier.location         = static_cast<int>(id);
    specConstType->setLayoutQualifier(layoutQualifier);

    return specConstType;
}
}  // anonymous namespace

SpecConst::SpecConst(TSymbolTable *symbolTable,
                     const ShCompileOptions &compileOptions,
                     GLenum shaderType)
    : mSymbolTable(symbolTable),
      mCompileOptions(compileOptions),
      mSurfaceRotationVar(nullptr),
      mDitherVar(nullptr)
{
    if (shaderType == GL_FRAGMENT_SHADER || shaderType == GL_COMPUTE_SHADER)
    {
        return;
    }

    // Mark SpecConstUsage::Rotation unconditionally.  gl_Position is always rotated.
    if (mCompileOptions.useSpecializationConstant)
    {
        mUsageBits.set(vk::SpecConstUsage::Rotation);
    }
}

SpecConst::~SpecConst() {}

void SpecConst::declareSpecConsts(TIntermBlock *root)
{
    // Add specialization constant declarations.  The default value of the specialization
    // constant is irrelevant, as it will be set when creating the pipeline.
    // Only emit specialized const declaration if it has been referenced.
    if (mSurfaceRotationVar != nullptr)
    {
        TIntermDeclaration *decl = new TIntermDeclaration();
        decl->appendDeclarator(
            new TIntermBinary(EOpInitialize, getRotation(), CreateBoolNode(false)));

        root->insertStatement(0, decl);
    }

    if (mDitherVar != nullptr)
    {
        TIntermDeclaration *decl = new TIntermDeclaration();
        decl->appendDeclarator(new TIntermBinary(EOpInitialize, getDither(), CreateUIntNode(0)));

        root->insertStatement(0, decl);
    }
}

TIntermSymbol *SpecConst::getRotation()
{
    if (mSurfaceRotationVar == nullptr)
    {
        const TType *type = MakeSpecConst(*StaticType::GetBasic<EbtBool, EbpUndefined>(),
                                          vk::SpecializationConstantId::SurfaceRotation);

        mSurfaceRotationVar = new TVariable(mSymbolTable, kSurfaceRotationSpecConstVarName, type,
                                            SymbolType::AngleInternal);
    }
    return new TIntermSymbol(mSurfaceRotationVar);
}

TIntermTyped *SpecConst::getSwapXY()
{
    if (!mCompileOptions.useSpecializationConstant)
    {
        return nullptr;
    }
    mUsageBits.set(vk::SpecConstUsage::Rotation);
    return getRotation();
}

TIntermTyped *SpecConst::getDither()
{
    if (mDitherVar == nullptr)
    {
        const TType *type = MakeSpecConst(*StaticType::GetBasic<EbtUInt, EbpHigh>(),
                                          vk::SpecializationConstantId::Dither);

        mDitherVar =
            new TVariable(mSymbolTable, kDitherSpecConstVarName, type, SymbolType::AngleInternal);
        mUsageBits.set(vk::SpecConstUsage::Dither);
    }
    return new TIntermSymbol(mDitherVar);
}
}  // namespace sh
