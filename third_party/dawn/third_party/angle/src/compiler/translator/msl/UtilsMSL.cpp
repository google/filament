//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#if defined(_MSC_VER)
#    pragma warning(disable : 4718)
#endif
#include "compiler/translator/msl/UtilsMSL.h"
#include <algorithm>
#include <climits>
#include "compiler/translator/HashNames.h"
#include "compiler/translator/ImmutableString.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/Types.h"

namespace sh
{

const char *getBasicMetalString(const TType *t)
{
    switch (t->getBasicType())
    {
        case EbtVoid:
            return "void";
        case EbtFloat:
            return "float";
        case EbtInt:
            return "int";
        case EbtUInt:
            return "uint32_t";
        case EbtBool:
            return "bool";
        case EbtYuvCscStandardEXT:
            return "yuvCscStandardEXT";
        case EbtSampler2D:
            return "sampler2D";
        case EbtSampler3D:
            return "sampler3D";
        case EbtSamplerCube:
            return "samplerCube";
        case EbtSamplerExternalOES:
            return "samplerExternalOES";
        case EbtSamplerExternal2DY2YEXT:
            return "__samplerExternal2DY2YEXT";
        case EbtSampler2DRect:
            return "sampler2DRect";
        case EbtSampler2DArray:
            return "sampler2DArray";
        case EbtSampler2DMS:
            return "sampler2DMS";
        case EbtSampler2DMSArray:
            return "sampler2DMSArray";
        case EbtSamplerCubeArray:
            return "samplerCubeArray";
        case EbtISampler2D:
            return "isampler2D";
        case EbtISampler3D:
            return "isampler3D";
        case EbtISamplerCube:
            return "isamplerCube";
        case EbtISampler2DArray:
            return "isampler2DArray";
        case EbtISampler2DMS:
            return "isampler2DMS";
        case EbtISampler2DMSArray:
            return "isampler2DMSArray";
        case EbtISamplerCubeArray:
            return "isamplerCubeArray";
        case EbtUSampler2D:
            return "usampler2D";
        case EbtUSampler3D:
            return "usampler3D";
        case EbtUSamplerCube:
            return "usamplerCube";
        case EbtUSampler2DArray:
            return "usampler2DArray";
        case EbtUSampler2DMS:
            return "usampler2DMS";
        case EbtUSampler2DMSArray:
            return "usampler2DMSArray";
        case EbtUSamplerCubeArray:
            return "usamplerCubeArray";
        case EbtSampler2DShadow:
            return "sampler2DShadow";
        case EbtSamplerCubeShadow:
            return "samplerCubeShadow";
        case EbtSampler2DArrayShadow:
            return "sampler2DArrayShadow";
        case EbtSamplerCubeArrayShadow:
            return "samplerCubeArrayShadow";
        case EbtStruct:
            return "structure";
        case EbtInterfaceBlock:
            return "interface block";
        case EbtImage2D:
            return "texture2d";
        case EbtIImage2D:
            return "texture2d<int>";
        case EbtUImage2D:
            return "texture2d<uint32_t>";
        case EbtImage3D:
            return "texture3d";
        case EbtIImage3D:
            return "texture3d<int>";
        case EbtUImage3D:
            return "texture3d<uint32_t>";
        case EbtImage2DArray:
            if (t->isUnsizedArray())
            {
                return "array_ref<texture2d>";
            }
            else
            {
                return "NOT_IMPLEMENTED";
            }
        case EbtIImage2DArray:
            if (t->isUnsizedArray())
            {
                return "array_ref<texture2d<int>>";
            }
            else
            {
                return "NOT_IMPLEMENTED";
            }
        case EbtUImage2DArray:
            if (t->isUnsizedArray())
            {
                return "array_ref<texture2d<uint32_t>>";
            }
            else
            {
                return "NOT_IMPLEMENTED";
            }
        case EbtImageCube:
            return "texturecube";
        case EbtIImageCube:
            return "texturecube<int>";
        case EbtUImageCube:
            return "texturecube<uint32_t>";
        case EbtImageCubeArray:
            if (t->isUnsizedArray())
            {
                return "array_ref<texturecube>";
            }
            else
            {
                return "NOT_IMPLEMENTED";
            }
        case EbtIImageCubeArray:
            if (t->isUnsizedArray())
            {
                return "array_ref<texturecube<int>>";
            }
            else
            {
                return "NOT_IMPLEMENTED";
            }
        case EbtUImageCubeArray:
            if (t->isUnsizedArray())
            {
                return "array_ref<texturecube<uint32_t>>";
            }
            else
            {
                return "NOT_IMPLEMENTED";
            }
        case EbtAtomicCounter:
            return "atomic_uint";
        case EbtSamplerVideoWEBGL:
            return "$samplerVideoWEBGL";
        default:
            UNREACHABLE();
            return "unknown type";
    }
}

const char *getBuiltInMetalTypeNameString(const TType *t)
{
    if (t->isMatrix())
    {
        switch (t->getCols())
        {
            case 2:
                switch (t->getRows())
                {
                    case 2:
                        return "float2";
                    case 3:
                        return "float2x3";
                    case 4:
                        return "float2x4";
                    default:
                        UNREACHABLE();
                        return nullptr;
                }
            case 3:
                switch (t->getRows())
                {
                    case 2:
                        return "float3x2";
                    case 3:
                        return "float3";
                    case 4:
                        return "float3x4";
                    default:
                        UNREACHABLE();
                        return nullptr;
                }
            case 4:
                switch (t->getRows())
                {
                    case 2:
                        return "float4x2";
                    case 3:
                        return "float4x3";
                    case 4:
                        return "float4";
                    default:
                        UNREACHABLE();
                        return nullptr;
                }
            default:
                UNREACHABLE();
                return nullptr;
        }
    }
    if (t->isVector())
    {
        switch (t->getBasicType())
        {
            case EbtFloat:
                switch (t->getNominalSize())
                {
                    case 2:
                        return "float2";
                    case 3:
                        return "float3";
                    case 4:
                        return "float4";
                    default:
                        UNREACHABLE();
                        return nullptr;
                }
            case EbtInt:
                switch (t->getNominalSize())
                {
                    case 2:
                        return "int2";
                    case 3:
                        return "int3";
                    case 4:
                        return "int4";
                    default:
                        UNREACHABLE();
                        return nullptr;
                }
            case EbtBool:
                switch (t->getNominalSize())
                {
                    case 2:
                        return "bool2";
                    case 3:
                        return "bool3";
                    case 4:
                        return "bool4";
                    default:
                        UNREACHABLE();
                        return nullptr;
                }
            case EbtUInt:
                switch (t->getNominalSize())
                {
                    case 2:
                        return "uint2";
                    case 3:
                        return "uint3";
                    case 4:
                        return "uint4";
                    default:
                        UNREACHABLE();
                        return nullptr;
                }
            default:
                UNREACHABLE();
                return nullptr;
        }
    }
    ASSERT(t->getBasicType() != EbtStruct);
    ASSERT(t->getBasicType() != EbtInterfaceBlock);
    return getBasicMetalString(t);
}

ImmutableString GetMetalTypeName(const TType &type, ShHashFunction64 hashFunction, NameMap *nameMap)
{
    if (type.getBasicType() == EbtStruct)
        return HashName(type.getStruct(), hashFunction, nameMap);
    else
        return ImmutableString(getBuiltInMetalTypeNameString(&type));
}

}  // namespace sh
