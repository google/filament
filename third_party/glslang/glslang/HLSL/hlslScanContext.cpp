//
// Copyright (C) 2016 Google, Inc.
// Copyright (C) 2016 LunarG, Inc.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of Google, Inc., nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

//
// HLSL scanning, leveraging the scanning done by the preprocessor.
//

#include <cstring>
#include <unordered_map>
#include <unordered_set>

#include "../Include/Types.h"
#include "../MachineIndependent/SymbolTable.h"
#include "../MachineIndependent/ParseHelper.h"
#include "hlslScanContext.h"
#include "hlslTokens.h"

// preprocessor includes
#include "../MachineIndependent/preprocessor/PpContext.h"
#include "../MachineIndependent/preprocessor/PpTokens.h"

namespace {

struct str_eq
{
    bool operator()(const char* lhs, const char* rhs) const
    {
        return strcmp(lhs, rhs) == 0;
    }
};

struct str_hash
{
    size_t operator()(const char* str) const
    {
        // djb2
        unsigned long hash = 5381;
        int c;

        while ((c = *str++) != 0)
            hash = ((hash << 5) + hash) + c;

        return hash;
    }
};

// A single global usable by all threads, by all versions, by all languages.
const std::unordered_map<const char*, glslang::EHlslTokenClass, str_hash, str_eq> KeywordMap {
    {"static",glslang::EHTokStatic},
    {"const",glslang::EHTokConst},
    {"unorm",glslang::EHTokUnorm},
    {"snorm",glslang::EHTokSNorm},
    {"extern",glslang::EHTokExtern},
    {"uniform",glslang::EHTokUniform},
    {"volatile",glslang::EHTokVolatile},
    {"precise",glslang::EHTokPrecise},
    {"shared",glslang::EHTokShared},
    {"groupshared",glslang::EHTokGroupShared},
    {"linear",glslang::EHTokLinear},
    {"centroid",glslang::EHTokCentroid},
    {"nointerpolation",glslang::EHTokNointerpolation},
    {"noperspective",glslang::EHTokNoperspective},
    {"sample",glslang::EHTokSample},
    {"row_major",glslang::EHTokRowMajor},
    {"column_major",glslang::EHTokColumnMajor},
    {"packoffset",glslang::EHTokPackOffset},
    {"in",glslang::EHTokIn},
    {"out",glslang::EHTokOut},
    {"inout",glslang::EHTokInOut},
    {"layout",glslang::EHTokLayout},
    {"globallycoherent",glslang::EHTokGloballyCoherent},
    {"inline",glslang::EHTokInline},

    {"point",glslang::EHTokPoint},
    {"line",glslang::EHTokLine},
    {"triangle",glslang::EHTokTriangle},
    {"lineadj",glslang::EHTokLineAdj},
    {"triangleadj",glslang::EHTokTriangleAdj},

    {"PointStream",glslang::EHTokPointStream},
    {"LineStream",glslang::EHTokLineStream},
    {"TriangleStream",glslang::EHTokTriangleStream},

    {"InputPatch",glslang::EHTokInputPatch},
    {"OutputPatch",glslang::EHTokOutputPatch},

    {"Buffer",glslang::EHTokBuffer},
    {"vector",glslang::EHTokVector},
    {"matrix",glslang::EHTokMatrix},

    {"void",glslang::EHTokVoid},
    {"string",glslang::EHTokString},
    {"bool",glslang::EHTokBool},
    {"int",glslang::EHTokInt},
    {"uint",glslang::EHTokUint},
    {"uint64_t",glslang::EHTokUint64},
    {"dword",glslang::EHTokDword},
    {"half",glslang::EHTokHalf},
    {"float",glslang::EHTokFloat},
    {"double",glslang::EHTokDouble},
    {"min16float",glslang::EHTokMin16float},
    {"min10float",glslang::EHTokMin10float},
    {"min16int",glslang::EHTokMin16int},
    {"min12int",glslang::EHTokMin12int},
    {"min16uint",glslang::EHTokMin16uint},

    {"bool1",glslang::EHTokBool1},
    {"bool2",glslang::EHTokBool2},
    {"bool3",glslang::EHTokBool3},
    {"bool4",glslang::EHTokBool4},
    {"float1",glslang::EHTokFloat1},
    {"float2",glslang::EHTokFloat2},
    {"float3",glslang::EHTokFloat3},
    {"float4",glslang::EHTokFloat4},
    {"int1",glslang::EHTokInt1},
    {"int2",glslang::EHTokInt2},
    {"int3",glslang::EHTokInt3},
    {"int4",glslang::EHTokInt4},
    {"double1",glslang::EHTokDouble1},
    {"double2",glslang::EHTokDouble2},
    {"double3",glslang::EHTokDouble3},
    {"double4",glslang::EHTokDouble4},
    {"uint1",glslang::EHTokUint1},
    {"uint2",glslang::EHTokUint2},
    {"uint3",glslang::EHTokUint3},
    {"uint4",glslang::EHTokUint4},

    {"half1",glslang::EHTokHalf1},
    {"half2",glslang::EHTokHalf2},
    {"half3",glslang::EHTokHalf3},
    {"half4",glslang::EHTokHalf4},
    {"min16float1",glslang::EHTokMin16float1},
    {"min16float2",glslang::EHTokMin16float2},
    {"min16float3",glslang::EHTokMin16float3},
    {"min16float4",glslang::EHTokMin16float4},
    {"min10float1",glslang::EHTokMin10float1},
    {"min10float2",glslang::EHTokMin10float2},
    {"min10float3",glslang::EHTokMin10float3},
    {"min10float4",glslang::EHTokMin10float4},
    {"min16int1",glslang::EHTokMin16int1},
    {"min16int2",glslang::EHTokMin16int2},
    {"min16int3",glslang::EHTokMin16int3},
    {"min16int4",glslang::EHTokMin16int4},
    {"min12int1",glslang::EHTokMin12int1},
    {"min12int2",glslang::EHTokMin12int2},
    {"min12int3",glslang::EHTokMin12int3},
    {"min12int4",glslang::EHTokMin12int4},
    {"min16uint1",glslang::EHTokMin16uint1},
    {"min16uint2",glslang::EHTokMin16uint2},
    {"min16uint3",glslang::EHTokMin16uint3},
    {"min16uint4",glslang::EHTokMin16uint4},

    {"bool1x1",glslang::EHTokBool1x1},
    {"bool1x2",glslang::EHTokBool1x2},
    {"bool1x3",glslang::EHTokBool1x3},
    {"bool1x4",glslang::EHTokBool1x4},
    {"bool2x1",glslang::EHTokBool2x1},
    {"bool2x2",glslang::EHTokBool2x2},
    {"bool2x3",glslang::EHTokBool2x3},
    {"bool2x4",glslang::EHTokBool2x4},
    {"bool3x1",glslang::EHTokBool3x1},
    {"bool3x2",glslang::EHTokBool3x2},
    {"bool3x3",glslang::EHTokBool3x3},
    {"bool3x4",glslang::EHTokBool3x4},
    {"bool4x1",glslang::EHTokBool4x1},
    {"bool4x2",glslang::EHTokBool4x2},
    {"bool4x3",glslang::EHTokBool4x3},
    {"bool4x4",glslang::EHTokBool4x4},
    {"int1x1",glslang::EHTokInt1x1},
    {"int1x2",glslang::EHTokInt1x2},
    {"int1x3",glslang::EHTokInt1x3},
    {"int1x4",glslang::EHTokInt1x4},
    {"int2x1",glslang::EHTokInt2x1},
    {"int2x2",glslang::EHTokInt2x2},
    {"int2x3",glslang::EHTokInt2x3},
    {"int2x4",glslang::EHTokInt2x4},
    {"int3x1",glslang::EHTokInt3x1},
    {"int3x2",glslang::EHTokInt3x2},
    {"int3x3",glslang::EHTokInt3x3},
    {"int3x4",glslang::EHTokInt3x4},
    {"int4x1",glslang::EHTokInt4x1},
    {"int4x2",glslang::EHTokInt4x2},
    {"int4x3",glslang::EHTokInt4x3},
    {"int4x4",glslang::EHTokInt4x4},
    {"uint1x1",glslang::EHTokUint1x1},
    {"uint1x2",glslang::EHTokUint1x2},
    {"uint1x3",glslang::EHTokUint1x3},
    {"uint1x4",glslang::EHTokUint1x4},
    {"uint2x1",glslang::EHTokUint2x1},
    {"uint2x2",glslang::EHTokUint2x2},
    {"uint2x3",glslang::EHTokUint2x3},
    {"uint2x4",glslang::EHTokUint2x4},
    {"uint3x1",glslang::EHTokUint3x1},
    {"uint3x2",glslang::EHTokUint3x2},
    {"uint3x3",glslang::EHTokUint3x3},
    {"uint3x4",glslang::EHTokUint3x4},
    {"uint4x1",glslang::EHTokUint4x1},
    {"uint4x2",glslang::EHTokUint4x2},
    {"uint4x3",glslang::EHTokUint4x3},
    {"uint4x4",glslang::EHTokUint4x4},
    {"bool1x1",glslang::EHTokBool1x1},
    {"bool1x2",glslang::EHTokBool1x2},
    {"bool1x3",glslang::EHTokBool1x3},
    {"bool1x4",glslang::EHTokBool1x4},
    {"bool2x1",glslang::EHTokBool2x1},
    {"bool2x2",glslang::EHTokBool2x2},
    {"bool2x3",glslang::EHTokBool2x3},
    {"bool2x4",glslang::EHTokBool2x4},
    {"bool3x1",glslang::EHTokBool3x1},
    {"bool3x2",glslang::EHTokBool3x2},
    {"bool3x3",glslang::EHTokBool3x3},
    {"bool3x4",glslang::EHTokBool3x4},
    {"bool4x1",glslang::EHTokBool4x1},
    {"bool4x2",glslang::EHTokBool4x2},
    {"bool4x3",glslang::EHTokBool4x3},
    {"bool4x4",glslang::EHTokBool4x4},
    {"float1x1",glslang::EHTokFloat1x1},
    {"float1x2",glslang::EHTokFloat1x2},
    {"float1x3",glslang::EHTokFloat1x3},
    {"float1x4",glslang::EHTokFloat1x4},
    {"float2x1",glslang::EHTokFloat2x1},
    {"float2x2",glslang::EHTokFloat2x2},
    {"float2x3",glslang::EHTokFloat2x3},
    {"float2x4",glslang::EHTokFloat2x4},
    {"float3x1",glslang::EHTokFloat3x1},
    {"float3x2",glslang::EHTokFloat3x2},
    {"float3x3",glslang::EHTokFloat3x3},
    {"float3x4",glslang::EHTokFloat3x4},
    {"float4x1",glslang::EHTokFloat4x1},
    {"float4x2",glslang::EHTokFloat4x2},
    {"float4x3",glslang::EHTokFloat4x3},
    {"float4x4",glslang::EHTokFloat4x4},
    {"half1x1",glslang::EHTokHalf1x1},
    {"half1x2",glslang::EHTokHalf1x2},
    {"half1x3",glslang::EHTokHalf1x3},
    {"half1x4",glslang::EHTokHalf1x4},
    {"half2x1",glslang::EHTokHalf2x1},
    {"half2x2",glslang::EHTokHalf2x2},
    {"half2x3",glslang::EHTokHalf2x3},
    {"half2x4",glslang::EHTokHalf2x4},
    {"half3x1",glslang::EHTokHalf3x1},
    {"half3x2",glslang::EHTokHalf3x2},
    {"half3x3",glslang::EHTokHalf3x3},
    {"half3x4",glslang::EHTokHalf3x4},
    {"half4x1",glslang::EHTokHalf4x1},
    {"half4x2",glslang::EHTokHalf4x2},
    {"half4x3",glslang::EHTokHalf4x3},
    {"half4x4",glslang::EHTokHalf4x4},
    {"double1x1",glslang::EHTokDouble1x1},
    {"double1x2",glslang::EHTokDouble1x2},
    {"double1x3",glslang::EHTokDouble1x3},
    {"double1x4",glslang::EHTokDouble1x4},
    {"double2x1",glslang::EHTokDouble2x1},
    {"double2x2",glslang::EHTokDouble2x2},
    {"double2x3",glslang::EHTokDouble2x3},
    {"double2x4",glslang::EHTokDouble2x4},
    {"double3x1",glslang::EHTokDouble3x1},
    {"double3x2",glslang::EHTokDouble3x2},
    {"double3x3",glslang::EHTokDouble3x3},
    {"double3x4",glslang::EHTokDouble3x4},
    {"double4x1",glslang::EHTokDouble4x1},
    {"double4x2",glslang::EHTokDouble4x2},
    {"double4x3",glslang::EHTokDouble4x3},
    {"double4x4",glslang::EHTokDouble4x4},
    {"min16float1x1",glslang::EHTokMin16float1x1},
    {"min16float1x2",glslang::EHTokMin16float1x2},
    {"min16float1x3",glslang::EHTokMin16float1x3},
    {"min16float1x4",glslang::EHTokMin16float1x4},
    {"min16float2x1",glslang::EHTokMin16float2x1},
    {"min16float2x2",glslang::EHTokMin16float2x2},
    {"min16float2x3",glslang::EHTokMin16float2x3},
    {"min16float2x4",glslang::EHTokMin16float2x4},
    {"min16float3x1",glslang::EHTokMin16float3x1},
    {"min16float3x2",glslang::EHTokMin16float3x2},
    {"min16float3x3",glslang::EHTokMin16float3x3},
    {"min16float3x4",glslang::EHTokMin16float3x4},
    {"min16float4x1",glslang::EHTokMin16float4x1},
    {"min16float4x2",glslang::EHTokMin16float4x2},
    {"min16float4x3",glslang::EHTokMin16float4x3},
    {"min16float4x4",glslang::EHTokMin16float4x4},
    {"min10float1x1",glslang::EHTokMin10float1x1},
    {"min10float1x2",glslang::EHTokMin10float1x2},
    {"min10float1x3",glslang::EHTokMin10float1x3},
    {"min10float1x4",glslang::EHTokMin10float1x4},
    {"min10float2x1",glslang::EHTokMin10float2x1},
    {"min10float2x2",glslang::EHTokMin10float2x2},
    {"min10float2x3",glslang::EHTokMin10float2x3},
    {"min10float2x4",glslang::EHTokMin10float2x4},
    {"min10float3x1",glslang::EHTokMin10float3x1},
    {"min10float3x2",glslang::EHTokMin10float3x2},
    {"min10float3x3",glslang::EHTokMin10float3x3},
    {"min10float3x4",glslang::EHTokMin10float3x4},
    {"min10float4x1",glslang::EHTokMin10float4x1},
    {"min10float4x2",glslang::EHTokMin10float4x2},
    {"min10float4x3",glslang::EHTokMin10float4x3},
    {"min10float4x4",glslang::EHTokMin10float4x4},
    {"min16int1x1",glslang::EHTokMin16int1x1},
    {"min16int1x2",glslang::EHTokMin16int1x2},
    {"min16int1x3",glslang::EHTokMin16int1x3},
    {"min16int1x4",glslang::EHTokMin16int1x4},
    {"min16int2x1",glslang::EHTokMin16int2x1},
    {"min16int2x2",glslang::EHTokMin16int2x2},
    {"min16int2x3",glslang::EHTokMin16int2x3},
    {"min16int2x4",glslang::EHTokMin16int2x4},
    {"min16int3x1",glslang::EHTokMin16int3x1},
    {"min16int3x2",glslang::EHTokMin16int3x2},
    {"min16int3x3",glslang::EHTokMin16int3x3},
    {"min16int3x4",glslang::EHTokMin16int3x4},
    {"min16int4x1",glslang::EHTokMin16int4x1},
    {"min16int4x2",glslang::EHTokMin16int4x2},
    {"min16int4x3",glslang::EHTokMin16int4x3},
    {"min16int4x4",glslang::EHTokMin16int4x4},
    {"min12int1x1",glslang::EHTokMin12int1x1},
    {"min12int1x2",glslang::EHTokMin12int1x2},
    {"min12int1x3",glslang::EHTokMin12int1x3},
    {"min12int1x4",glslang::EHTokMin12int1x4},
    {"min12int2x1",glslang::EHTokMin12int2x1},
    {"min12int2x2",glslang::EHTokMin12int2x2},
    {"min12int2x3",glslang::EHTokMin12int2x3},
    {"min12int2x4",glslang::EHTokMin12int2x4},
    {"min12int3x1",glslang::EHTokMin12int3x1},
    {"min12int3x2",glslang::EHTokMin12int3x2},
    {"min12int3x3",glslang::EHTokMin12int3x3},
    {"min12int3x4",glslang::EHTokMin12int3x4},
    {"min12int4x1",glslang::EHTokMin12int4x1},
    {"min12int4x2",glslang::EHTokMin12int4x2},
    {"min12int4x3",glslang::EHTokMin12int4x3},
    {"min12int4x4",glslang::EHTokMin12int4x4},
    {"min16uint1x1",glslang::EHTokMin16uint1x1},
    {"min16uint1x2",glslang::EHTokMin16uint1x2},
    {"min16uint1x3",glslang::EHTokMin16uint1x3},
    {"min16uint1x4",glslang::EHTokMin16uint1x4},
    {"min16uint2x1",glslang::EHTokMin16uint2x1},
    {"min16uint2x2",glslang::EHTokMin16uint2x2},
    {"min16uint2x3",glslang::EHTokMin16uint2x3},
    {"min16uint2x4",glslang::EHTokMin16uint2x4},
    {"min16uint3x1",glslang::EHTokMin16uint3x1},
    {"min16uint3x2",glslang::EHTokMin16uint3x2},
    {"min16uint3x3",glslang::EHTokMin16uint3x3},
    {"min16uint3x4",glslang::EHTokMin16uint3x4},
    {"min16uint4x1",glslang::EHTokMin16uint4x1},
    {"min16uint4x2",glslang::EHTokMin16uint4x2},
    {"min16uint4x3",glslang::EHTokMin16uint4x3},
    {"min16uint4x4",glslang::EHTokMin16uint4x4},

    {"sampler",glslang::EHTokSampler},
    {"sampler1D",glslang::EHTokSampler1d},
    {"sampler2D",glslang::EHTokSampler2d},
    {"sampler3D",glslang::EHTokSampler3d},
    {"samplerCUBE",glslang::EHTokSamplerCube},
    {"sampler_state",glslang::EHTokSamplerState},
    {"SamplerState",glslang::EHTokSamplerState},
    {"SamplerComparisonState",glslang::EHTokSamplerComparisonState},
    {"texture",glslang::EHTokTexture},
    {"Texture1D",glslang::EHTokTexture1d},
    {"Texture1DArray",glslang::EHTokTexture1darray},
    {"Texture2D",glslang::EHTokTexture2d},
    {"Texture2DArray",glslang::EHTokTexture2darray},
    {"Texture3D",glslang::EHTokTexture3d},
    {"TextureCube",glslang::EHTokTextureCube},
    {"TextureCubeArray",glslang::EHTokTextureCubearray},
    {"Texture2DMS",glslang::EHTokTexture2DMS},
    {"Texture2DMSArray",glslang::EHTokTexture2DMSarray},
    {"RWTexture1D",glslang::EHTokRWTexture1d},
    {"RWTexture1DArray",glslang::EHTokRWTexture1darray},
    {"RWTexture2D",glslang::EHTokRWTexture2d},
    {"RWTexture2DArray",glslang::EHTokRWTexture2darray},
    {"RWTexture3D",glslang::EHTokRWTexture3d},
    {"RWBuffer",glslang::EHTokRWBuffer},
    {"SubpassInput",glslang::EHTokSubpassInput},
    {"SubpassInputMS",glslang::EHTokSubpassInputMS},

    {"AppendStructuredBuffer",glslang::EHTokAppendStructuredBuffer},
    {"ByteAddressBuffer",glslang::EHTokByteAddressBuffer},
    {"ConsumeStructuredBuffer",glslang::EHTokConsumeStructuredBuffer},
    {"RWByteAddressBuffer",glslang::EHTokRWByteAddressBuffer},
    {"RWStructuredBuffer",glslang::EHTokRWStructuredBuffer},
    {"StructuredBuffer",glslang::EHTokStructuredBuffer},
    {"TextureBuffer",glslang::EHTokTextureBuffer},

    {"class",glslang::EHTokClass},
    {"struct",glslang::EHTokStruct},
    {"cbuffer",glslang::EHTokCBuffer},
    {"ConstantBuffer",glslang::EHTokConstantBuffer},
    {"tbuffer",glslang::EHTokTBuffer},
    {"typedef",glslang::EHTokTypedef},
    {"this",glslang::EHTokThis},
    {"namespace",glslang::EHTokNamespace},

    {"true",glslang::EHTokBoolConstant},
    {"false",glslang::EHTokBoolConstant},

    {"for",glslang::EHTokFor},
    {"do",glslang::EHTokDo},
    {"while",glslang::EHTokWhile},
    {"break",glslang::EHTokBreak},
    {"continue",glslang::EHTokContinue},
    {"if",glslang::EHTokIf},
    {"else",glslang::EHTokElse},
    {"discard",glslang::EHTokDiscard},
    {"return",glslang::EHTokReturn},
    {"switch",glslang::EHTokSwitch},
    {"case",glslang::EHTokCase},
    {"default",glslang::EHTokDefault},
};

const std::unordered_set<const char*, str_hash, str_eq> ReservedSet {
    "auto",
    "catch",
    "char",
    "const_cast",
    "enum",
    "explicit",
    "friend",
    "goto",
    "long",
    "mutable",
    "new",
    "operator",
    "private",
    "protected",
    "public",
    "reinterpret_cast",
    "short",
    "signed",
    "sizeof",
    "static_cast",
    "template",
    "throw",
    "try",
    "typename",
    "union",
    "unsigned",
    "using",
    "virtual",
};
std::unordered_map<const char*, glslang::TBuiltInVariable, str_hash, str_eq> SemanticMap { 

    // in DX9, all outputs had to have a semantic associated with them, that was either consumed
    // by the system or was a specific register assignment
    // in DX10+, only semantics with the SV_ prefix have any meaning beyond decoration
    // Fxc will only accept DX9 style semantics in compat mode
    // Also, in DX10 if a SV value is present as the input of a stage, but isn't appropriate for that
    // stage, it would just be ignored as it is likely there as part of an output struct from one stage
    // to the next
#if 0
    (*SemanticMap)["PSIZE"] = EbvPointSize;
    (*SemanticMap)["FOG"] =   EbvFogFragCoord;
    (*SemanticMap)["DEPTH"] = EbvFragDepth;
    (*SemanticMap)["VFACE"] = EbvFace;
    (*SemanticMap)["VPOS"] =  EbvFragCoord;
#endif

    {"SV_POSITION",glslang::EbvPosition},
    {"SV_VERTEXID",glslang::EbvVertexIndex},
    {"SV_VIEWPORTARRAYINDEX",glslang::EbvViewportIndex},
    {"SV_TESSFACTOR",glslang::EbvTessLevelOuter},
    {"SV_SAMPLEINDEX",glslang::EbvSampleId},
    {"SV_RENDERTARGETARRAYINDEX",glslang::EbvLayer},
    {"SV_PRIMITIVEID",glslang::EbvPrimitiveId},
    {"SV_OUTPUTCONTROLPOINTID",glslang::EbvInvocationId},
    {"SV_ISFRONTFACE",glslang::EbvFace},
    {"SV_VIEWID",glslang::EbvViewIndex},
    {"SV_INSTANCEID",glslang::EbvInstanceIndex},
    {"SV_INSIDETESSFACTOR",glslang::EbvTessLevelInner},
    {"SV_GSINSTANCEID",glslang::EbvInvocationId},
    {"SV_DISPATCHTHREADID",glslang::EbvGlobalInvocationId},
    {"SV_GROUPTHREADID",glslang::EbvLocalInvocationId},
    {"SV_GROUPINDEX",glslang::EbvLocalInvocationIndex},
    {"SV_GROUPID",glslang::EbvWorkGroupId},
    {"SV_DOMAINLOCATION",glslang::EbvTessCoord},
    {"SV_DEPTH",glslang::EbvFragDepth},
    {"SV_COVERAGE",glslang::EbvSampleMask},
    {"SV_DEPTHGREATEREQUAL",glslang::EbvFragDepthGreater},
    {"SV_DEPTHLESSEQUAL",glslang::EbvFragDepthLesser},
    {"SV_STENCILREF", glslang::EbvFragStencilRef},
};
}

namespace glslang {

// Wrapper for tokenizeClass() to get everything inside the token.
void HlslScanContext::tokenize(HlslToken& token)
{
    EHlslTokenClass tokenClass = tokenizeClass(token);
    token.tokenClass = tokenClass;
}

glslang::TBuiltInVariable HlslScanContext::mapSemantic(const char* upperCase)
{
    auto it = SemanticMap.find(upperCase);
    if (it != SemanticMap.end())
        return it->second;
    else
        return glslang::EbvNone;
}

//
// Fill in token information for the next token, except for the token class.
// Returns the enum value of the token class of the next token found.
// Return 0 (EndOfTokens) on end of input.
//
EHlslTokenClass HlslScanContext::tokenizeClass(HlslToken& token)
{
    do {
        parserToken = &token;
        TPpToken ppToken;
        int token = ppContext.tokenize(ppToken);
        if (token == EndOfInput)
            return EHTokNone;

        tokenText = ppToken.name;
        loc = ppToken.loc;
        parserToken->loc = loc;
        switch (token) {
        case ';':                       return EHTokSemicolon;
        case ',':                       return EHTokComma;
        case ':':                       return EHTokColon;
        case '=':                       return EHTokAssign;
        case '(':                       return EHTokLeftParen;
        case ')':                       return EHTokRightParen;
        case '.':                       return EHTokDot;
        case '!':                       return EHTokBang;
        case '-':                       return EHTokDash;
        case '~':                       return EHTokTilde;
        case '+':                       return EHTokPlus;
        case '*':                       return EHTokStar;
        case '/':                       return EHTokSlash;
        case '%':                       return EHTokPercent;
        case '<':                       return EHTokLeftAngle;
        case '>':                       return EHTokRightAngle;
        case '|':                       return EHTokVerticalBar;
        case '^':                       return EHTokCaret;
        case '&':                       return EHTokAmpersand;
        case '?':                       return EHTokQuestion;
        case '[':                       return EHTokLeftBracket;
        case ']':                       return EHTokRightBracket;
        case '{':                       return EHTokLeftBrace;
        case '}':                       return EHTokRightBrace;
        case '\\':
            parseContext.error(loc, "illegal use of escape character", "\\", "");
            break;

        case PPAtomAddAssign:          return EHTokAddAssign;
        case PPAtomSubAssign:          return EHTokSubAssign;
        case PPAtomMulAssign:          return EHTokMulAssign;
        case PPAtomDivAssign:          return EHTokDivAssign;
        case PPAtomModAssign:          return EHTokModAssign;

        case PpAtomRight:              return EHTokRightOp;
        case PpAtomLeft:               return EHTokLeftOp;

        case PpAtomRightAssign:        return EHTokRightAssign;
        case PpAtomLeftAssign:         return EHTokLeftAssign;
        case PpAtomAndAssign:          return EHTokAndAssign;
        case PpAtomOrAssign:           return EHTokOrAssign;
        case PpAtomXorAssign:          return EHTokXorAssign;

        case PpAtomAnd:                return EHTokAndOp;
        case PpAtomOr:                 return EHTokOrOp;
        case PpAtomXor:                return EHTokXorOp;

        case PpAtomEQ:                 return EHTokEqOp;
        case PpAtomGE:                 return EHTokGeOp;
        case PpAtomNE:                 return EHTokNeOp;
        case PpAtomLE:                 return EHTokLeOp;

        case PpAtomDecrement:          return EHTokDecOp;
        case PpAtomIncrement:          return EHTokIncOp;

        case PpAtomColonColon:         return EHTokColonColon;

        case PpAtomConstInt:           parserToken->i = ppToken.ival;       return EHTokIntConstant;
        case PpAtomConstUint:          parserToken->i = ppToken.ival;       return EHTokUintConstant;
        case PpAtomConstFloat16:       parserToken->d = ppToken.dval;       return EHTokFloat16Constant;
        case PpAtomConstFloat:         parserToken->d = ppToken.dval;       return EHTokFloatConstant;
        case PpAtomConstDouble:        parserToken->d = ppToken.dval;       return EHTokDoubleConstant;
        case PpAtomIdentifier:
        {
            EHlslTokenClass token = tokenizeIdentifier();
            return token;
        }

        case PpAtomConstString: {
            parserToken->string = NewPoolTString(tokenText);
            return EHTokStringConstant;
        }

        case EndOfInput:               return EHTokNone;

        default:
            if (token < PpAtomMaxSingle) {
                char buf[2];
                buf[0] = (char)token;
                buf[1] = 0;
                parseContext.error(loc, "unexpected token", buf, "");
            } else if (tokenText[0] != 0)
                parseContext.error(loc, "unexpected token", tokenText, "");
            else
                parseContext.error(loc, "unexpected token", "", "");
            break;
        }
    } while (true);
}

EHlslTokenClass HlslScanContext::tokenizeIdentifier()
{
    if (ReservedSet.find(tokenText) != ReservedSet.end())
        return reservedWord();

    auto it = KeywordMap.find(tokenText);
    if (it == KeywordMap.end()) {
        // Should have an identifier of some sort
        return identifierOrType();
    }
    keyword = it->second;

    switch (keyword) {

    // qualifiers
    case EHTokStatic:
    case EHTokConst:
    case EHTokSNorm:
    case EHTokUnorm:
    case EHTokExtern:
    case EHTokUniform:
    case EHTokVolatile:
    case EHTokShared:
    case EHTokGroupShared:
    case EHTokLinear:
    case EHTokCentroid:
    case EHTokNointerpolation:
    case EHTokNoperspective:
    case EHTokSample:
    case EHTokRowMajor:
    case EHTokColumnMajor:
    case EHTokPackOffset:
    case EHTokIn:
    case EHTokOut:
    case EHTokInOut:
    case EHTokPrecise:
    case EHTokLayout:
    case EHTokGloballyCoherent:
    case EHTokInline:
        return keyword;

    // primitive types
    case EHTokPoint:
    case EHTokLine:
    case EHTokTriangle:
    case EHTokLineAdj:
    case EHTokTriangleAdj:
        return keyword;

    // stream out types
    case EHTokPointStream:
    case EHTokLineStream:
    case EHTokTriangleStream:
        return keyword;

    // Tessellation patches
    case EHTokInputPatch:
    case EHTokOutputPatch:
        return keyword;

    case EHTokBuffer:
    case EHTokVector:
    case EHTokMatrix:
        return keyword;

    // scalar types
    case EHTokVoid:
    case EHTokString:
    case EHTokBool:
    case EHTokInt:
    case EHTokUint:
    case EHTokUint64:
    case EHTokDword:
    case EHTokHalf:
    case EHTokFloat:
    case EHTokDouble:
    case EHTokMin16float:
    case EHTokMin10float:
    case EHTokMin16int:
    case EHTokMin12int:
    case EHTokMin16uint:

    // vector types
    case EHTokBool1:
    case EHTokBool2:
    case EHTokBool3:
    case EHTokBool4:
    case EHTokFloat1:
    case EHTokFloat2:
    case EHTokFloat3:
    case EHTokFloat4:
    case EHTokInt1:
    case EHTokInt2:
    case EHTokInt3:
    case EHTokInt4:
    case EHTokDouble1:
    case EHTokDouble2:
    case EHTokDouble3:
    case EHTokDouble4:
    case EHTokUint1:
    case EHTokUint2:
    case EHTokUint3:
    case EHTokUint4:
    case EHTokHalf1:
    case EHTokHalf2:
    case EHTokHalf3:
    case EHTokHalf4:
    case EHTokMin16float1:
    case EHTokMin16float2:
    case EHTokMin16float3:
    case EHTokMin16float4:
    case EHTokMin10float1:
    case EHTokMin10float2:
    case EHTokMin10float3:
    case EHTokMin10float4:
    case EHTokMin16int1:
    case EHTokMin16int2:
    case EHTokMin16int3:
    case EHTokMin16int4:
    case EHTokMin12int1:
    case EHTokMin12int2:
    case EHTokMin12int3:
    case EHTokMin12int4:
    case EHTokMin16uint1:
    case EHTokMin16uint2:
    case EHTokMin16uint3:
    case EHTokMin16uint4:

    // matrix types
    case EHTokBool1x1:
    case EHTokBool1x2:
    case EHTokBool1x3:
    case EHTokBool1x4:
    case EHTokBool2x1:
    case EHTokBool2x2:
    case EHTokBool2x3:
    case EHTokBool2x4:
    case EHTokBool3x1:
    case EHTokBool3x2:
    case EHTokBool3x3:
    case EHTokBool3x4:
    case EHTokBool4x1:
    case EHTokBool4x2:
    case EHTokBool4x3:
    case EHTokBool4x4:
    case EHTokInt1x1:
    case EHTokInt1x2:
    case EHTokInt1x3:
    case EHTokInt1x4:
    case EHTokInt2x1:
    case EHTokInt2x2:
    case EHTokInt2x3:
    case EHTokInt2x4:
    case EHTokInt3x1:
    case EHTokInt3x2:
    case EHTokInt3x3:
    case EHTokInt3x4:
    case EHTokInt4x1:
    case EHTokInt4x2:
    case EHTokInt4x3:
    case EHTokInt4x4:
    case EHTokUint1x1:
    case EHTokUint1x2:
    case EHTokUint1x3:
    case EHTokUint1x4:
    case EHTokUint2x1:
    case EHTokUint2x2:
    case EHTokUint2x3:
    case EHTokUint2x4:
    case EHTokUint3x1:
    case EHTokUint3x2:
    case EHTokUint3x3:
    case EHTokUint3x4:
    case EHTokUint4x1:
    case EHTokUint4x2:
    case EHTokUint4x3:
    case EHTokUint4x4:
    case EHTokFloat1x1:
    case EHTokFloat1x2:
    case EHTokFloat1x3:
    case EHTokFloat1x4:
    case EHTokFloat2x1:
    case EHTokFloat2x2:
    case EHTokFloat2x3:
    case EHTokFloat2x4:
    case EHTokFloat3x1:
    case EHTokFloat3x2:
    case EHTokFloat3x3:
    case EHTokFloat3x4:
    case EHTokFloat4x1:
    case EHTokFloat4x2:
    case EHTokFloat4x3:
    case EHTokFloat4x4:
    case EHTokHalf1x1:
    case EHTokHalf1x2:
    case EHTokHalf1x3:
    case EHTokHalf1x4:
    case EHTokHalf2x1:
    case EHTokHalf2x2:
    case EHTokHalf2x3:
    case EHTokHalf2x4:
    case EHTokHalf3x1:
    case EHTokHalf3x2:
    case EHTokHalf3x3:
    case EHTokHalf3x4:
    case EHTokHalf4x1:
    case EHTokHalf4x2:
    case EHTokHalf4x3:
    case EHTokHalf4x4:
    case EHTokDouble1x1:
    case EHTokDouble1x2:
    case EHTokDouble1x3:
    case EHTokDouble1x4:
    case EHTokDouble2x1:
    case EHTokDouble2x2:
    case EHTokDouble2x3:
    case EHTokDouble2x4:
    case EHTokDouble3x1:
    case EHTokDouble3x2:
    case EHTokDouble3x3:
    case EHTokDouble3x4:
    case EHTokDouble4x1:
    case EHTokDouble4x2:
    case EHTokDouble4x3:
    case EHTokDouble4x4:
    case EHTokMin16float1x1:
    case EHTokMin16float1x2:
    case EHTokMin16float1x3:
    case EHTokMin16float1x4:
    case EHTokMin16float2x1:
    case EHTokMin16float2x2:
    case EHTokMin16float2x3:
    case EHTokMin16float2x4:
    case EHTokMin16float3x1:
    case EHTokMin16float3x2:
    case EHTokMin16float3x3:
    case EHTokMin16float3x4:
    case EHTokMin16float4x1:
    case EHTokMin16float4x2:
    case EHTokMin16float4x3:
    case EHTokMin16float4x4:
    case EHTokMin10float1x1:
    case EHTokMin10float1x2:
    case EHTokMin10float1x3:
    case EHTokMin10float1x4:
    case EHTokMin10float2x1:
    case EHTokMin10float2x2:
    case EHTokMin10float2x3:
    case EHTokMin10float2x4:
    case EHTokMin10float3x1:
    case EHTokMin10float3x2:
    case EHTokMin10float3x3:
    case EHTokMin10float3x4:
    case EHTokMin10float4x1:
    case EHTokMin10float4x2:
    case EHTokMin10float4x3:
    case EHTokMin10float4x4:
    case EHTokMin16int1x1:
    case EHTokMin16int1x2:
    case EHTokMin16int1x3:
    case EHTokMin16int1x4:
    case EHTokMin16int2x1:
    case EHTokMin16int2x2:
    case EHTokMin16int2x3:
    case EHTokMin16int2x4:
    case EHTokMin16int3x1:
    case EHTokMin16int3x2:
    case EHTokMin16int3x3:
    case EHTokMin16int3x4:
    case EHTokMin16int4x1:
    case EHTokMin16int4x2:
    case EHTokMin16int4x3:
    case EHTokMin16int4x4:
    case EHTokMin12int1x1:
    case EHTokMin12int1x2:
    case EHTokMin12int1x3:
    case EHTokMin12int1x4:
    case EHTokMin12int2x1:
    case EHTokMin12int2x2:
    case EHTokMin12int2x3:
    case EHTokMin12int2x4:
    case EHTokMin12int3x1:
    case EHTokMin12int3x2:
    case EHTokMin12int3x3:
    case EHTokMin12int3x4:
    case EHTokMin12int4x1:
    case EHTokMin12int4x2:
    case EHTokMin12int4x3:
    case EHTokMin12int4x4:
    case EHTokMin16uint1x1:
    case EHTokMin16uint1x2:
    case EHTokMin16uint1x3:
    case EHTokMin16uint1x4:
    case EHTokMin16uint2x1:
    case EHTokMin16uint2x2:
    case EHTokMin16uint2x3:
    case EHTokMin16uint2x4:
    case EHTokMin16uint3x1:
    case EHTokMin16uint3x2:
    case EHTokMin16uint3x3:
    case EHTokMin16uint3x4:
    case EHTokMin16uint4x1:
    case EHTokMin16uint4x2:
    case EHTokMin16uint4x3:
    case EHTokMin16uint4x4:
        return keyword;

    // texturing types
    case EHTokSampler:
    case EHTokSampler1d:
    case EHTokSampler2d:
    case EHTokSampler3d:
    case EHTokSamplerCube:
    case EHTokSamplerState:
    case EHTokSamplerComparisonState:
    case EHTokTexture:
    case EHTokTexture1d:
    case EHTokTexture1darray:
    case EHTokTexture2d:
    case EHTokTexture2darray:
    case EHTokTexture3d:
    case EHTokTextureCube:
    case EHTokTextureCubearray:
    case EHTokTexture2DMS:
    case EHTokTexture2DMSarray:
    case EHTokRWTexture1d:
    case EHTokRWTexture1darray:
    case EHTokRWTexture2d:
    case EHTokRWTexture2darray:
    case EHTokRWTexture3d:
    case EHTokRWBuffer:
    case EHTokAppendStructuredBuffer:
    case EHTokByteAddressBuffer:
    case EHTokConsumeStructuredBuffer:
    case EHTokRWByteAddressBuffer:
    case EHTokRWStructuredBuffer:
    case EHTokStructuredBuffer:
    case EHTokTextureBuffer:
    case EHTokSubpassInput:
    case EHTokSubpassInputMS:
        return keyword;

    // variable, user type, ...
    case EHTokClass:
    case EHTokStruct:
    case EHTokTypedef:
    case EHTokCBuffer:
    case EHTokConstantBuffer:
    case EHTokTBuffer:
    case EHTokThis:
    case EHTokNamespace:
        return keyword;

    case EHTokBoolConstant:
        if (strcmp("true", tokenText) == 0)
            parserToken->b = true;
        else
            parserToken->b = false;
        return keyword;

    // control flow
    case EHTokFor:
    case EHTokDo:
    case EHTokWhile:
    case EHTokBreak:
    case EHTokContinue:
    case EHTokIf:
    case EHTokElse:
    case EHTokDiscard:
    case EHTokReturn:
    case EHTokCase:
    case EHTokSwitch:
    case EHTokDefault:
        return keyword;

    default:
        parseContext.infoSink.info.message(EPrefixInternalError, "Unknown glslang keyword", loc);
        return EHTokNone;
    }
}

EHlslTokenClass HlslScanContext::identifierOrType()
{
    parserToken->string = NewPoolTString(tokenText);

    return EHTokIdentifier;
}

// Give an error for use of a reserved symbol.
// However, allow built-in declarations to use reserved words, to allow
// extension support before the extension is enabled.
EHlslTokenClass HlslScanContext::reservedWord()
{
    if (! parseContext.symbolTable.atBuiltInLevel())
        parseContext.error(loc, "Reserved word.", tokenText, "", "");

    return EHTokNone;
}

} // end namespace glslang
