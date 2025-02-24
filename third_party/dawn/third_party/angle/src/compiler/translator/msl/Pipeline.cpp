//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/msl/Pipeline.h"
#include "compiler/translator/tree_util/BuiltIn.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

#define VARIANT_NAME(variant, base) (variant == Variant::Modified ? base "Mod" : base)

bool Pipeline::uses(const TVariable &var) const
{
    if (var.symbolType() == SymbolType::Empty)
    {
        return false;
    }

    if (globalInstanceVar)
    {
        return &var == globalInstanceVar;
    }

    const TType &nodeType      = var.getType();
    const TQualifier qualifier = nodeType.getQualifier();

    switch (type)
    {
        case Type::VertexIn:
            switch (qualifier)
            {
                case TQualifier::EvqAttribute:
                case TQualifier::EvqVertexIn:
                    return true;
                default:
                    return false;
            }

        case Type::VertexOut:
            switch (qualifier)
            {
                case TQualifier::EvqVaryingOut:
                case TQualifier::EvqVertexOut:
                case TQualifier::EvqPosition:
                case TQualifier::EvqPointSize:
                case TQualifier::EvqClipDistance:
                case TQualifier::EvqSmoothOut:
                case TQualifier::EvqFlatOut:
                case TQualifier::EvqNoPerspectiveOut:
                case TQualifier::EvqCentroidOut:
                case TQualifier::EvqSampleOut:
                case TQualifier::EvqNoPerspectiveCentroidOut:
                case TQualifier::EvqNoPerspectiveSampleOut:
                    return true;
                default:
                    return false;
            }

        case Type::FragmentIn:
            switch (qualifier)
            {
                case TQualifier::EvqVaryingIn:
                case TQualifier::EvqFragmentIn:
                case TQualifier::EvqSmoothIn:
                case TQualifier::EvqFlatIn:
                case TQualifier::EvqNoPerspectiveIn:
                case TQualifier::EvqCentroidIn:
                case TQualifier::EvqSampleIn:
                case TQualifier::EvqNoPerspectiveCentroidIn:
                case TQualifier::EvqNoPerspectiveSampleIn:
                    return true;
                default:
                    return false;
            }

        case Type::FragmentOut:
            switch (qualifier)
            {
                case TQualifier::EvqFragmentOut:
                case TQualifier::EvqFragmentInOut:
                case TQualifier::EvqFragColor:
                case TQualifier::EvqFragData:
                case TQualifier::EvqFragDepth:
                case TQualifier::EvqSecondaryFragColorEXT:
                case TQualifier::EvqSecondaryFragDataEXT:
                    return true;
                case TQualifier::EvqSampleMask:
                    return var.symbolType() == SymbolType::AngleInternal;
                default:
                    return false;
            }

        case Type::UserUniforms:
            switch (qualifier)
            {
                case TQualifier::EvqUniform:
                    return true;
                default:
                    return false;
            }

        case Type::NonConstantGlobals:
            switch (qualifier)
            {
                case TQualifier::EvqGlobal:
                case TQualifier::EvqSamplePosition:
                    return true;
                case TQualifier::EvqSampleMaskIn:
                case TQualifier::EvqSampleMask:
                    return var.symbolType() == SymbolType::BuiltIn;
                case TQualifier::EvqUniform:
                    return var.name() == "gl_NumSamples";
                default:
                    return false;
            }

        case Type::InvocationVertexGlobals:
            switch (qualifier)
            {
                case TQualifier::EvqVertexID:
                    return true;
                default:
                    return false;
            }

        case Type::InvocationFragmentGlobals:
            switch (qualifier)
            {
                case TQualifier::EvqFragCoord:
                case TQualifier::EvqPointCoord:
                case TQualifier::EvqFrontFacing:
                case TQualifier::EvqSampleID:
                    return true;
                case TQualifier::EvqSampleMaskIn:
                    return var.symbolType() == SymbolType::AngleInternal;
                default:
                    return false;
            }

        case Type::UniformBuffer:
            switch (qualifier)
            {
                case TQualifier::EvqBuffer:
                    return true;
                default:
                    return false;
            }
        case Type::AngleUniforms:
            UNREACHABLE();  // globalInstanceVar should be non-null and thus never reach here.
            return false;

        case Type::Texture:
            return IsSampler(nodeType.getBasicType());

        case Type::Image:
            return IsImage(nodeType.getBasicType());

        case Type::InstanceId:
            return Name(var) == Name(*BuiltInVariable::gl_InstanceID());
    }
}

Name Pipeline::getStructTypeName(Variant variant) const
{
    const char *name;
    switch (type)
    {
        case Type::VertexIn:
            name = VARIANT_NAME(variant, "VertexIn");
            break;
        case Type::VertexOut:
            name = VARIANT_NAME(variant, "VertexOut");
            break;
        case Type::FragmentIn:
            name = VARIANT_NAME(variant, "FragmentIn");
            break;
        case Type::FragmentOut:
            name = VARIANT_NAME(variant, "FragmentOut");
            break;
        case Type::UserUniforms:
            name = VARIANT_NAME(variant, "UserUniforms");
            break;
        case Type::AngleUniforms:
            name = VARIANT_NAME(variant, "AngleUniforms");
            break;
        case Type::NonConstantGlobals:
            name = VARIANT_NAME(variant, "NonConstGlobals");
            break;
        case Type::InvocationVertexGlobals:
            name = VARIANT_NAME(variant, "InvocationVertexGlobals");
            break;
        case Type::InvocationFragmentGlobals:
            name = VARIANT_NAME(variant, "InvocationFragmentGlobals");
            break;
        case Type::Texture:
            name = VARIANT_NAME(variant, "TextureEnvs");
            break;
        case Type::Image:
            name = VARIANT_NAME(variant, "Images");
            break;
        case Type::InstanceId:
            name = VARIANT_NAME(variant, "InstanceId");
            break;
        case Type::UniformBuffer:
            name = VARIANT_NAME(variant, "UniformBuffer");
    }
    return Name(name);
}

Name Pipeline::getStructInstanceName(Variant variant) const
{
    const char *name;
    switch (type)
    {
        case Type::VertexIn:
            name = VARIANT_NAME(variant, "vertexIn");
            break;
        case Type::VertexOut:
            // Used by name in compiler/translator/tree_ops/msl/RewriteOutArgs.cpp
            name = VARIANT_NAME(variant, "vertexOut");
            break;
        case Type::FragmentIn:
            name = VARIANT_NAME(variant, "fragmentIn");
            break;
        case Type::FragmentOut:
            // Used by name in compiler/translator/tree_ops/msl/RewriteOutArgs.cpp
            name = VARIANT_NAME(variant, "fragmentOut");
            break;
        case Type::UserUniforms:
            name = VARIANT_NAME(variant, "userUniforms");
            break;
        case Type::AngleUniforms:
            name = VARIANT_NAME(variant, "angleUniforms");
            break;
        case Type::NonConstantGlobals:
            // Used by name in compiler/translator/tree_ops/msl/RewriteOutArgs.cpp
            name = VARIANT_NAME(variant, "nonConstGlobals");
            break;
        case Type::InvocationVertexGlobals:
            name = VARIANT_NAME(variant, "invocationVertexGlobals");
            break;
        case Type::InvocationFragmentGlobals:
            name = VARIANT_NAME(variant, "invocationFragmentGlobals");
            break;
        case Type::Texture:
            name = VARIANT_NAME(variant, "textureEnvs");
            break;
        case Type::Image:
            name = VARIANT_NAME(variant, "images");
            break;
        case Type::InstanceId:
            name = VARIANT_NAME(variant, "instanceId");
            break;
        case Type::UniformBuffer:
            name = VARIANT_NAME(variant, "uniformBuffer");
    }
    return Name(name);
}

static bool AllowPacking(Pipeline::Type type)
{
    return false;
}

static bool AllowPadding(Pipeline::Type type)
{
    using Type = Pipeline::Type;

    switch (type)
    {
        case Type::VertexIn:
        case Type::VertexOut:
        case Type::FragmentIn:
        case Type::FragmentOut:
        case Type::AngleUniforms:
        case Type::NonConstantGlobals:
        case Type::InvocationVertexGlobals:
        case Type::InvocationFragmentGlobals:
            return true;

        case Type::UserUniforms:
        case Type::Texture:
        case Type::Image:
        case Type::InstanceId:
        case Type::UniformBuffer:
            return false;
    }
}
enum Compare
{
    LT,
    LTE,
    EQ,
    GTE,
    GT,
};

template <typename T>
static bool CompareBy(Compare op, const T &x, const T &y)
{
    switch (op)
    {
        case LT:
            return x < y;
        case LTE:
            return x <= y;
        case EQ:
            return x == y;
        case GTE:
            return x >= y;
        case GT:
            return x > y;
    }
}

template <TBasicType BT, Compare Cmp, uint8_t MatchDim, uint8_t NewDim>
static uint8_t SaturateVectorOf(const TField &field)
{
    static_assert(NewDim >= MatchDim, "");

    const TType &type = *field.type();
    ASSERT(type.isScalar() || type.isVector());

    const bool cond = type.getBasicType() == BT && !type.isArray() &&
                      CompareBy(Cmp, type.getNominalSize(), MatchDim) &&
                      type.getQualifier() != TQualifier::EvqFragDepth;

    if (cond)
    {
        return NewDim;
    }
    return 0;
}

ModifyStructConfig Pipeline::externalStructModifyConfig() const
{
    using Pred   = ModifyStructConfig::Predicate;
    using SatVec = ModifyStructConfig::SaturateVector;

    ModifyStructConfig config(
        isPipelineOut() ? ConvertType::OriginalToModified : ConvertType::ModifiedToOriginal,
        AllowPacking(type), AllowPadding(type));

    config.externalAddressSpace = externalAddressSpace();

    switch (type)
    {
        case Type::VertexIn:
            config.inlineArray        = Pred::True;
            config.splitMatrixColumns = Pred::True;
            config.inlineStruct       = Pred::True;
            break;

        case Type::VertexOut:
            config.inlineArray = [](const TField &field) -> bool {
                // Clip distance output uses float[n] type instead of metal::array.
                return field.type()->getQualifier() != TQualifier::EvqClipDistance;
            };
            config.splitMatrixColumns = Pred::True;
            config.inlineStruct       = Pred::True;
            break;

        case Type::FragmentIn:
            config.inlineArray        = Pred::True;
            config.splitMatrixColumns = Pred::True;
            config.inlineStruct       = Pred::True;
            break;

        case Type::FragmentOut:
            config.inlineArray            = Pred::True;
            config.splitMatrixColumns     = Pred::True;
            config.inlineStruct           = Pred::True;
            config.saturateScalarOrVector = [](const TField &field) -> uint8_t {
                if (field.type()->getQualifier() == TQualifier::EvqSampleMask)
                {
                    return 1;
                }
                if (uint8_t s = SaturateVectorOf<TBasicType::EbtInt, LT, 4, 4>(field))
                {
                    return s;
                }
                if (uint8_t s = SaturateVectorOf<TBasicType::EbtUInt, LT, 4, 4>(field))
                {
                    return s;
                }
                if (uint8_t s = SaturateVectorOf<TBasicType::EbtFloat, LT, 4, 4>(field))
                {
                    return s;
                }
                return 0;
            };
            break;
        case Type::UserUniforms:
            config.promoteBoolToUint            = Pred::False;
            config.saturateMatrixRows           = SatVec::DontSaturate;
            config.saturateScalarOrVectorArrays = SatVec::DontSaturate;
            config.recurseStruct                = Pred::True;
            break;

        case Type::AngleUniforms:
            config.initialBlockStorage = TLayoutBlockStorage::EbsStd430;  // XXX: Correct?
            break;

        case Type::NonConstantGlobals:
            break;
        case Type::UniformBuffer:
            config.promoteBoolToUint            = Pred::False;
            config.saturateMatrixRows           = SatVec::DontSaturate;
            config.saturateScalarOrVectorArrays = SatVec::DontSaturate;
            config.recurseStruct                = Pred::True;
            break;
        case Type::InvocationVertexGlobals:
        case Type::InvocationFragmentGlobals:
        case Type::Texture:
        case Type::Image:
        case Type::InstanceId:
            break;
    }

    return config;
}

bool Pipeline::alwaysRequiresLocalVariableDeclarationInMain() const
{
    switch (type)
    {
        case Type::VertexIn:
        case Type::FragmentIn:
        case Type::UserUniforms:
        case Type::AngleUniforms:
        case Type::UniformBuffer:
        case Type::Image:
            return false;

        case Type::VertexOut:
        case Type::FragmentOut:
        case Type::NonConstantGlobals:
        case Type::InvocationVertexGlobals:
        case Type::InvocationFragmentGlobals:
        case Type::Texture:
        case Type::InstanceId:
            return true;
    }
}

bool Pipeline::isPipelineOut() const
{
    switch (type)
    {
        case Type::VertexIn:
        case Type::FragmentIn:
        case Type::UserUniforms:
        case Type::AngleUniforms:
        case Type::NonConstantGlobals:
        case Type::InvocationVertexGlobals:
        case Type::InvocationFragmentGlobals:
        case Type::Texture:
        case Type::Image:
        case Type::InstanceId:
        case Type::UniformBuffer:
            return false;

        case Type::VertexOut:
        case Type::FragmentOut:
            return true;
    }
}

AddressSpace Pipeline::externalAddressSpace() const
{
    switch (type)
    {
        case Type::VertexIn:
        case Type::FragmentIn:
        case Type::NonConstantGlobals:
        case Type::InvocationVertexGlobals:
        case Type::InvocationFragmentGlobals:
        case Type::Texture:
        case Type::Image:
        case Type::InstanceId:
        case Type::FragmentOut:
        case Type::VertexOut:
            return AddressSpace::Thread;

        case Type::UserUniforms:
        case Type::AngleUniforms:
        case Type::UniformBuffer:
            return AddressSpace::Constant;
    }
}

bool PipelineStructs::matches(const TStructure &s, bool internal, bool external) const
{
    PipelineScoped<TStructure> ps[] = {
        fragmentIn,
        fragmentOut,
        vertexIn,
        vertexOut,
        userUniforms,
        /* angleUniforms, */
        nonConstantGlobals,
        invocationVertexGlobals,
        invocationFragmentGlobals,
        uniformBuffers,
        texture,
        instanceId,
    };
    for (const auto &p : ps)
    {
        if (internal && p.internal == &s)
        {
            return true;
        }
        if (external && p.external == &s)
        {
            return true;
        }
    }
    return false;
}
