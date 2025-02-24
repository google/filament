//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// UtilsHLSL.cpp:
//   Utility methods for GLSL to HLSL translation.
//

#include "compiler/translator/hlsl/UtilsHLSL.h"

#include "common/utilities.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/hlsl/StructureHLSL.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

// Parameter types are only added to function names if they are ambiguous according to the
// native HLSL compiler. Other parameter types are not added to function names to avoid
// making function names longer.
bool FunctionParameterNeedsDisambiguation(const TType &paramType)
{
    if (paramType.getObjectSize() == 4 && paramType.getBasicType() == EbtFloat)
    {
        // Disambiguation is needed for float2x2 and float4 parameters. These are the only
        // built-in types that HLSL thinks are identical. float2x3 and float3x2 are different
        // types, for example.
        return true;
    }

    if (paramType.getBasicType() == EbtUInt || paramType.getBasicType() == EbtInt)
    {
        // The HLSL compiler can't always tell the difference between int and uint types when an
        // expression is passed as a function parameter
        return true;
    }

    if (paramType.getBasicType() == EbtStruct)
    {
        // Disambiguation is needed for struct parameters, since HLSL thinks that structs with
        // the same fields but a different name are identical.
        ASSERT(paramType.getStruct()->symbolType() != SymbolType::Empty);
        return true;
    }

    return false;
}

void DisambiguateFunctionNameForParameterType(const TType &paramType,
                                              TString *disambiguatingStringOut)
{
    if (FunctionParameterNeedsDisambiguation(paramType))
    {
        *disambiguatingStringOut += "_" + TypeString(paramType);
    }
}

}  // anonymous namespace

const char *SamplerString(const TBasicType type)
{
    if (IsShadowSampler(type))
    {
        return "SamplerComparisonState";
    }
    else
    {
        return "SamplerState";
    }
}

const char *SamplerString(HLSLTextureGroup type)
{
    if (type >= HLSL_COMPARISON_SAMPLER_GROUP_BEGIN && type <= HLSL_COMPARISON_SAMPLER_GROUP_END)
    {
        return "SamplerComparisonState";
    }
    else
    {
        return "SamplerState";
    }
}

HLSLTextureGroup TextureGroup(const TBasicType type, TLayoutImageInternalFormat imageInternalFormat)

{
    switch (type)
    {
        case EbtSampler2D:
        case EbtSamplerVideoWEBGL:
            return HLSL_TEXTURE_2D;
        case EbtSamplerCube:
            return HLSL_TEXTURE_CUBE;
        case EbtSamplerExternalOES:
            return HLSL_TEXTURE_2D;
        case EbtSampler2DArray:
            return HLSL_TEXTURE_2D_ARRAY;
        case EbtSampler3D:
            return HLSL_TEXTURE_3D;
        case EbtSampler2DMS:
            return HLSL_TEXTURE_2D_MS;
        case EbtSampler2DMSArray:
            return HLSL_TEXTURE_2D_MS_ARRAY;
        case EbtSamplerBuffer:
            return HLSL_TEXTURE_BUFFER;
        case EbtISampler2D:
            return HLSL_TEXTURE_2D_INT4;
        case EbtISamplerBuffer:
            return HLSL_TEXTURE_BUFFER_INT4;
        case EbtISampler3D:
            return HLSL_TEXTURE_3D_INT4;
        case EbtISamplerCube:
            return HLSL_TEXTURE_2D_ARRAY_INT4;
        case EbtISampler2DArray:
            return HLSL_TEXTURE_2D_ARRAY_INT4;
        case EbtISampler2DMS:
            return HLSL_TEXTURE_2D_MS_INT4;
        case EbtISampler2DMSArray:
            return HLSL_TEXTURE_2D_MS_ARRAY_INT4;
        case EbtUSampler2D:
            return HLSL_TEXTURE_2D_UINT4;
        case EbtUSampler3D:
            return HLSL_TEXTURE_3D_UINT4;
        case EbtUSamplerCube:
            return HLSL_TEXTURE_2D_ARRAY_UINT4;
        case EbtUSamplerBuffer:
            return HLSL_TEXTURE_BUFFER_UINT4;
        case EbtUSampler2DArray:
            return HLSL_TEXTURE_2D_ARRAY_UINT4;
        case EbtUSampler2DMS:
            return HLSL_TEXTURE_2D_MS_UINT4;
        case EbtUSampler2DMSArray:
            return HLSL_TEXTURE_2D_MS_ARRAY_UINT4;
        case EbtSampler2DShadow:
            return HLSL_TEXTURE_2D_COMPARISON;
        case EbtSamplerCubeShadow:
            return HLSL_TEXTURE_CUBE_COMPARISON;
        case EbtSampler2DArrayShadow:
            return HLSL_TEXTURE_2D_ARRAY_COMPARISON;
        case EbtImage2D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32F:
                case EiifRGBA16F:
                case EiifR32F:
                    return HLSL_TEXTURE_2D;
                case EiifRGBA8:
                    return HLSL_TEXTURE_2D_UNORM;
                case EiifRGBA8_SNORM:
                    return HLSL_TEXTURE_2D_SNORM;
                default:
                    UNREACHABLE();
                    return HLSL_TEXTURE_UNKNOWN;
            }
        }
        case EbtIImage2D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32I:
                case EiifRGBA16I:
                case EiifRGBA8I:
                case EiifR32I:
                    return HLSL_TEXTURE_2D_INT4;
                default:
                    UNREACHABLE();
                    return HLSL_TEXTURE_UNKNOWN;
            }
        }
        case EbtUImage2D:
        {
            switch (imageInternalFormat)
            {

                case EiifRGBA32UI:
                case EiifRGBA16UI:
                case EiifRGBA8UI:
                case EiifR32UI:
                    return HLSL_TEXTURE_2D_UINT4;
                default:
                    UNREACHABLE();
                    return HLSL_TEXTURE_UNKNOWN;
            }
        }
        case EbtImage3D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32F:
                case EiifRGBA16F:
                case EiifR32F:
                    return HLSL_TEXTURE_3D;
                case EiifRGBA8:
                    return HLSL_TEXTURE_3D_UNORM;
                case EiifRGBA8_SNORM:
                    return HLSL_TEXTURE_3D_SNORM;
                default:
                    UNREACHABLE();
                    return HLSL_TEXTURE_UNKNOWN;
            }
        }
        case EbtIImage3D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32I:
                case EiifRGBA16I:
                case EiifRGBA8I:
                case EiifR32I:
                    return HLSL_TEXTURE_3D_INT4;
                default:
                    UNREACHABLE();
                    return HLSL_TEXTURE_UNKNOWN;
            }
        }
        case EbtUImage3D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32UI:
                case EiifRGBA16UI:
                case EiifRGBA8UI:
                case EiifR32UI:
                    return HLSL_TEXTURE_3D_UINT4;
                default:
                    UNREACHABLE();
                    return HLSL_TEXTURE_UNKNOWN;
            }
        }
        case EbtImage2DArray:
        case EbtImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32F:
                case EiifRGBA16F:
                case EiifR32F:
                    return HLSL_TEXTURE_2D_ARRAY;
                case EiifRGBA8:
                    return HLSL_TEXTURE_2D_ARRAY_UNORN;
                case EiifRGBA8_SNORM:
                    return HLSL_TEXTURE_2D_ARRAY_SNORM;
                default:
                    UNREACHABLE();
                    return HLSL_TEXTURE_UNKNOWN;
            }
        }
        case EbtIImage2DArray:
        case EbtIImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32I:
                case EiifRGBA16I:
                case EiifRGBA8I:
                case EiifR32I:
                    return HLSL_TEXTURE_2D_ARRAY_INT4;
                default:
                    UNREACHABLE();
                    return HLSL_TEXTURE_UNKNOWN;
            }
        }
        case EbtUImage2DArray:
        case EbtUImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32UI:
                case EiifRGBA16UI:
                case EiifRGBA8UI:
                case EiifR32UI:
                    return HLSL_TEXTURE_2D_ARRAY_UINT4;
                default:
                    UNREACHABLE();
                    return HLSL_TEXTURE_UNKNOWN;
            }
        }
        case EbtImageBuffer:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32F:
                case EiifRGBA16F:
                case EiifR32F:
                    return HLSL_TEXTURE_BUFFER;
                case EiifRGBA8:
                    return HLSL_TEXTURE_BUFFER_UNORM;
                case EiifRGBA8_SNORM:
                    return HLSL_TEXTURE_BUFFER_SNORM;
                default:
                    UNREACHABLE();
                    return HLSL_TEXTURE_UNKNOWN;
            }
        }
        case EbtUImageBuffer:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32UI:
                case EiifRGBA16UI:
                case EiifRGBA8UI:
                case EiifR32UI:
                    return HLSL_TEXTURE_BUFFER_UINT4;
                default:
                    UNREACHABLE();
                    return HLSL_TEXTURE_UNKNOWN;
            }
        }
        case EbtIImageBuffer:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32I:
                case EiifRGBA16I:
                case EiifRGBA8I:
                case EiifR32I:
                    return HLSL_TEXTURE_BUFFER_INT4;
                default:
                    UNREACHABLE();
                    return HLSL_TEXTURE_UNKNOWN;
            }
        }
        default:
            UNREACHABLE();
            return HLSL_TEXTURE_UNKNOWN;
    }
}

const char *TextureString(const HLSLTextureGroup textureGroup)
{
    switch (textureGroup)
    {
        case HLSL_TEXTURE_2D:
            return "Texture2D<float4>";
        case HLSL_TEXTURE_CUBE:
            return "TextureCube<float4>";
        case HLSL_TEXTURE_2D_ARRAY:
            return "Texture2DArray<float4>";
        case HLSL_TEXTURE_3D:
            return "Texture3D<float4>";
        case HLSL_TEXTURE_2D_UNORM:
            return "Texture2D<unorm float4>";
        case HLSL_TEXTURE_CUBE_UNORM:
            return "TextureCube<unorm float4>";
        case HLSL_TEXTURE_2D_ARRAY_UNORN:
            return "Texture2DArray<unorm float4>";
        case HLSL_TEXTURE_3D_UNORM:
            return "Texture3D<unorm float4>";
        case HLSL_TEXTURE_2D_SNORM:
            return "Texture2D<snorm float4>";
        case HLSL_TEXTURE_CUBE_SNORM:
            return "TextureCube<snorm float4>";
        case HLSL_TEXTURE_2D_ARRAY_SNORM:
            return "Texture2DArray<snorm float4>";
        case HLSL_TEXTURE_3D_SNORM:
            return "Texture3D<snorm float4>";
        case HLSL_TEXTURE_2D_MS:
            return "Texture2DMS<float4>";
        case HLSL_TEXTURE_2D_MS_ARRAY:
            return "Texture2DMSArray<float4>";
        case HLSL_TEXTURE_2D_INT4:
            return "Texture2D<int4>";
        case HLSL_TEXTURE_3D_INT4:
            return "Texture3D<int4>";
        case HLSL_TEXTURE_2D_ARRAY_INT4:
            return "Texture2DArray<int4>";
        case HLSL_TEXTURE_2D_MS_INT4:
            return "Texture2DMS<int4>";
        case HLSL_TEXTURE_2D_MS_ARRAY_INT4:
            return "Texture2DMSArray<int4>";
        case HLSL_TEXTURE_2D_UINT4:
            return "Texture2D<uint4>";
        case HLSL_TEXTURE_3D_UINT4:
            return "Texture3D<uint4>";
        case HLSL_TEXTURE_2D_ARRAY_UINT4:
            return "Texture2DArray<uint4>";
        case HLSL_TEXTURE_2D_MS_UINT4:
            return "Texture2DMS<uint4>";
        case HLSL_TEXTURE_2D_MS_ARRAY_UINT4:
            return "Texture2DMSArray<uint4>";
        case HLSL_TEXTURE_2D_COMPARISON:
            return "Texture2D";
        case HLSL_TEXTURE_CUBE_COMPARISON:
            return "TextureCube";
        case HLSL_TEXTURE_2D_ARRAY_COMPARISON:
            return "Texture2DArray";
        case HLSL_TEXTURE_BUFFER:
            return "Buffer<float4>";
        case HLSL_TEXTURE_BUFFER_INT4:
            return "Buffer<int4>";
        case HLSL_TEXTURE_BUFFER_UINT4:
            return "Buffer<uint4>";
        case HLSL_TEXTURE_BUFFER_UNORM:
            return "Buffer<unorm float4>";
        case HLSL_TEXTURE_BUFFER_SNORM:
            return "Buffer<snorm float4>";
        default:
            UNREACHABLE();
    }

    return "<unknown read texture type>";
}

const char *TextureString(const TBasicType type, TLayoutImageInternalFormat imageInternalFormat)
{
    return TextureString(TextureGroup(type, imageInternalFormat));
}

const char *TextureGroupSuffix(const HLSLTextureGroup type)
{
    switch (type)
    {
        case HLSL_TEXTURE_2D:
            return "2D";
        case HLSL_TEXTURE_CUBE:
            return "Cube";
        case HLSL_TEXTURE_2D_ARRAY:
            return "2DArray";
        case HLSL_TEXTURE_3D:
            return "3D";
        case HLSL_TEXTURE_2D_UNORM:
            return "2D_unorm_float4_";
        case HLSL_TEXTURE_CUBE_UNORM:
            return "Cube_unorm_float4_";
        case HLSL_TEXTURE_2D_ARRAY_UNORN:
            return "2DArray_unorm_float4_";
        case HLSL_TEXTURE_3D_UNORM:
            return "3D_unorm_float4_";
        case HLSL_TEXTURE_2D_SNORM:
            return "2D_snorm_float4_";
        case HLSL_TEXTURE_CUBE_SNORM:
            return "Cube_snorm_float4_";
        case HLSL_TEXTURE_2D_ARRAY_SNORM:
            return "2DArray_snorm_float4_";
        case HLSL_TEXTURE_3D_SNORM:
            return "3D_snorm_float4_";
        case HLSL_TEXTURE_2D_MS:
            return "2DMS";
        case HLSL_TEXTURE_2D_MS_ARRAY:
            return "2DMSArray";
        case HLSL_TEXTURE_2D_INT4:
            return "2D_int4_";
        case HLSL_TEXTURE_3D_INT4:
            return "3D_int4_";
        case HLSL_TEXTURE_2D_ARRAY_INT4:
            return "2DArray_int4_";
        case HLSL_TEXTURE_2D_MS_INT4:
            return "2DMS_int4_";
        case HLSL_TEXTURE_2D_MS_ARRAY_INT4:
            return "2DMSArray_int4_";
        case HLSL_TEXTURE_2D_UINT4:
            return "2D_uint4_";
        case HLSL_TEXTURE_3D_UINT4:
            return "3D_uint4_";
        case HLSL_TEXTURE_2D_ARRAY_UINT4:
            return "2DArray_uint4_";
        case HLSL_TEXTURE_2D_MS_UINT4:
            return "2DMS_uint4_";
        case HLSL_TEXTURE_2D_MS_ARRAY_UINT4:
            return "2DMSArray_uint4_";
        case HLSL_TEXTURE_2D_COMPARISON:
            return "2D_comparison";
        case HLSL_TEXTURE_CUBE_COMPARISON:
            return "Cube_comparison";
        case HLSL_TEXTURE_2D_ARRAY_COMPARISON:
            return "2DArray_comparison";
        case HLSL_TEXTURE_BUFFER:
            return "Buffer";
        case HLSL_TEXTURE_BUFFER_INT4:
            return "Buffer_int4_";
        case HLSL_TEXTURE_BUFFER_UINT4:
            return "Buffer_uint4_";
        case HLSL_TEXTURE_BUFFER_UNORM:
            return "Buffer_unorm_float4_";
        case HLSL_TEXTURE_BUFFER_SNORM:
            return "Buffer_snorm_float4_";
        default:
            UNREACHABLE();
    }

    return "<unknown texture type>";
}

const char *TextureGroupSuffix(const TBasicType type,
                               TLayoutImageInternalFormat imageInternalFormat)
{
    return TextureGroupSuffix(TextureGroup(type, imageInternalFormat));
}

const char *TextureTypeSuffix(const TBasicType type, TLayoutImageInternalFormat imageInternalFormat)
{
    switch (type)
    {
        case EbtISamplerCube:
            return "Cube_int4_";
        case EbtUSamplerCube:
            return "Cube_uint4_";
        case EbtSamplerExternalOES:
            return "_External";
        case EbtImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32F:
                case EiifRGBA16F:
                case EiifR32F:
                    return "Cube_float4_";
                case EiifRGBA8:
                    return "Cube_unorm_float4_";
                case EiifRGBA8_SNORM:
                    return "Cube_snorm_float4_";
                default:
                    UNREACHABLE();
            }
            break;
        }
        case EbtIImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32I:
                case EiifRGBA16I:
                case EiifRGBA8I:
                case EiifR32I:
                    return "Cube_int4_";
                default:
                    UNREACHABLE();
            }
            break;
        }
        case EbtUImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32UI:
                case EiifRGBA16UI:
                case EiifRGBA8UI:
                case EiifR32UI:
                    return "Cube_uint4_";
                default:
                    UNREACHABLE();
            }
            break;
        }
        default:
            // All other types are identified by their group suffix
            return TextureGroupSuffix(type, imageInternalFormat);
    }
    UNREACHABLE();
    return "_TTS_invalid_";
}

HLSLRWTextureGroup RWTextureGroup(const TBasicType type,
                                  TLayoutImageInternalFormat imageInternalFormat)

{
    switch (type)
    {
        case EbtImage2D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32F:
                case EiifRGBA16F:
                case EiifR32F:
                    return HLSL_RWTEXTURE_2D_FLOAT4;
                case EiifRGBA8:
                    return HLSL_RWTEXTURE_2D_UNORM;
                case EiifRGBA8_SNORM:
                    return HLSL_RWTEXTURE_2D_SNORM;
                default:
                    UNREACHABLE();
            }
            break;
        }
        case EbtIImage2D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32I:
                case EiifRGBA16I:
                case EiifRGBA8I:
                case EiifR32I:
                    return HLSL_RWTEXTURE_2D_INT4;
                default:
                    UNREACHABLE();
            }
            break;
        }
        case EbtUImage2D:
        {
            switch (imageInternalFormat)
            {

                case EiifRGBA32UI:
                case EiifRGBA16UI:
                case EiifRGBA8UI:
                case EiifR32UI:
                    return HLSL_RWTEXTURE_2D_UINT4;
                default:
                    UNREACHABLE();
            }
            break;
        }
        case EbtImage3D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32F:
                case EiifRGBA16F:
                case EiifR32F:
                    return HLSL_RWTEXTURE_3D_FLOAT4;
                case EiifRGBA8:
                    return HLSL_RWTEXTURE_3D_UNORM;
                case EiifRGBA8_SNORM:
                    return HLSL_RWTEXTURE_3D_SNORM;
                default:
                    UNREACHABLE();
            }
            break;
        }
        case EbtIImage3D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32I:
                case EiifRGBA16I:
                case EiifRGBA8I:
                case EiifR32I:
                    return HLSL_RWTEXTURE_3D_INT4;
                default:
                    UNREACHABLE();
            }
            break;
        }
        case EbtUImage3D:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32UI:
                case EiifRGBA16UI:
                case EiifRGBA8UI:
                case EiifR32UI:
                    return HLSL_RWTEXTURE_3D_UINT4;
                default:
                    UNREACHABLE();
            }
            break;
        }
        case EbtImage2DArray:
        case EbtImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32F:
                case EiifRGBA16F:
                case EiifR32F:
                    return HLSL_RWTEXTURE_2D_ARRAY_FLOAT4;
                case EiifRGBA8:
                    return HLSL_RWTEXTURE_2D_ARRAY_UNORN;
                case EiifRGBA8_SNORM:
                    return HLSL_RWTEXTURE_2D_ARRAY_SNORM;
                default:
                    UNREACHABLE();
            }
            break;
        }
        case EbtIImage2DArray:
        case EbtIImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32I:
                case EiifRGBA16I:
                case EiifRGBA8I:
                case EiifR32I:
                    return HLSL_RWTEXTURE_2D_ARRAY_INT4;
                default:
                    UNREACHABLE();
            }
            break;
        }
        case EbtUImage2DArray:
        case EbtUImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32UI:
                case EiifRGBA16UI:
                case EiifRGBA8UI:
                case EiifR32UI:
                    return HLSL_RWTEXTURE_2D_ARRAY_UINT4;
                default:
                    UNREACHABLE();
            }
            break;
        }
        case EbtImageBuffer:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32F:
                case EiifRGBA16F:
                case EiifR32F:
                    return HLSL_RWTEXTURE_BUFFER_FLOAT4;
                case EiifRGBA8:
                    return HLSL_RWTEXTURE_BUFFER_UNORM;
                case EiifRGBA8_SNORM:
                    return HLSL_RWTEXTURE_BUFFER_SNORM;
                default:
                    UNREACHABLE();
            }
            break;
        }
        case EbtIImageBuffer:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32I:
                case EiifRGBA16I:
                case EiifRGBA8I:
                case EiifR32I:
                    return HLSL_RWTEXTURE_BUFFER_INT4;
                default:
                    UNREACHABLE();
            }
            break;
        }
        case EbtUImageBuffer:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32UI:
                case EiifRGBA16UI:
                case EiifRGBA8UI:
                case EiifR32UI:
                    return HLSL_RWTEXTURE_BUFFER_UINT4;
                default:
                    UNREACHABLE();
            }
            break;
        }
        default:
            UNREACHABLE();
    }
    return HLSL_RWTEXTURE_UNKNOWN;
}

const char *RWTextureString(const HLSLRWTextureGroup RWTextureGroup)
{
    switch (RWTextureGroup)
    {
        case HLSL_RWTEXTURE_2D_FLOAT4:
            return "RWTexture2D<float4>";
        case HLSL_RWTEXTURE_2D_ARRAY_FLOAT4:
            return "RWTexture2DArray<float4>";
        case HLSL_RWTEXTURE_3D_FLOAT4:
            return "RWTexture3D<float4>";
        case HLSL_RWTEXTURE_2D_UNORM:
            return "RWTexture2D<unorm float4>";
        case HLSL_RWTEXTURE_2D_ARRAY_UNORN:
            return "RWTexture2DArray<unorm float4>";
        case HLSL_RWTEXTURE_3D_UNORM:
            return "RWTexture3D<unorm float4>";
        case HLSL_RWTEXTURE_2D_SNORM:
            return "RWTexture2D<snorm float4>";
        case HLSL_RWTEXTURE_2D_ARRAY_SNORM:
            return "RWTexture2DArray<snorm float4>";
        case HLSL_RWTEXTURE_3D_SNORM:
            return "RWTexture3D<snorm float4>";
        case HLSL_RWTEXTURE_2D_UINT4:
            return "RWTexture2D<uint4>";
        case HLSL_RWTEXTURE_2D_ARRAY_UINT4:
            return "RWTexture2DArray<uint4>";
        case HLSL_RWTEXTURE_3D_UINT4:
            return "RWTexture3D<uint4>";
        case HLSL_RWTEXTURE_2D_INT4:
            return "RWTexture2D<int4>";
        case HLSL_RWTEXTURE_2D_ARRAY_INT4:
            return "RWTexture2DArray<int4>";
        case HLSL_RWTEXTURE_3D_INT4:
            return "RWTexture3D<int4>";
        case HLSL_RWTEXTURE_BUFFER_FLOAT4:
            return "RWBuffer<float4>";
        case HLSL_RWTEXTURE_BUFFER_UNORM:
            return "RWBuffer<unorm float4>";
        case HLSL_RWTEXTURE_BUFFER_SNORM:
            return "RWBuffer<snorm float4>";
        case HLSL_RWTEXTURE_BUFFER_UINT4:
            return "RWBuffer<uint4>";
        case HLSL_RWTEXTURE_BUFFER_INT4:
            return "RWBuffer<int4>";
        default:
            UNREACHABLE();
    }

    return "<unknown read and write texture type>";
}

const char *RWTextureString(const TBasicType type, TLayoutImageInternalFormat imageInternalFormat)
{
    return RWTextureString(RWTextureGroup(type, imageInternalFormat));
}

const char *RWTextureGroupSuffix(const HLSLRWTextureGroup type)
{
    switch (type)
    {
        case HLSL_RWTEXTURE_2D_FLOAT4:
            return "RW2D_float4_";
        case HLSL_RWTEXTURE_2D_ARRAY_FLOAT4:
            return "RW2DArray_float4_";
        case HLSL_RWTEXTURE_3D_FLOAT4:
            return "RW3D_float4_";
        case HLSL_RWTEXTURE_2D_UNORM:
            return "RW2D_unorm_float4_";
        case HLSL_RWTEXTURE_2D_ARRAY_UNORN:
            return "RW2DArray_unorm_float4_";
        case HLSL_RWTEXTURE_3D_UNORM:
            return "RW3D_unorm_float4_";
        case HLSL_RWTEXTURE_2D_SNORM:
            return "RW2D_snorm_float4_";
        case HLSL_RWTEXTURE_2D_ARRAY_SNORM:
            return "RW2DArray_snorm_float4_";
        case HLSL_RWTEXTURE_3D_SNORM:
            return "RW3D_snorm_float4_";
        case HLSL_RWTEXTURE_2D_UINT4:
            return "RW2D_uint4_";
        case HLSL_RWTEXTURE_2D_ARRAY_UINT4:
            return "RW2DArray_uint4_";
        case HLSL_RWTEXTURE_3D_UINT4:
            return "RW3D_uint4_";
        case HLSL_RWTEXTURE_2D_INT4:
            return "RW2D_int4_";
        case HLSL_RWTEXTURE_2D_ARRAY_INT4:
            return "RW2DArray_int4_";
        case HLSL_RWTEXTURE_3D_INT4:
            return "RW3D_int4_";
        case HLSL_RWTEXTURE_BUFFER_FLOAT4:
            return "RWBuffer_float4_";
        case HLSL_RWTEXTURE_BUFFER_UNORM:
            return "RWBuffer_unorm_float4_";
        case HLSL_RWTEXTURE_BUFFER_SNORM:
            return "RWBuffer_snorm_float4_";
        case HLSL_RWTEXTURE_BUFFER_UINT4:
            return "RWBuffer_uint4_";
        case HLSL_RWTEXTURE_BUFFER_INT4:
            return "RWBuffer_int4_";
        default:
            UNREACHABLE();
    }

    return "<unknown read and write resource>";
}

const char *RWTextureGroupSuffix(const TBasicType type,
                                 TLayoutImageInternalFormat imageInternalFormat)
{
    return RWTextureGroupSuffix(RWTextureGroup(type, imageInternalFormat));
}

const char *RWTextureTypeSuffix(const TBasicType type,
                                TLayoutImageInternalFormat imageInternalFormat)
{
    switch (type)
    {
        case EbtImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32F:
                case EiifRGBA16F:
                case EiifR32F:
                    return "RWCube_float4_";
                case EiifRGBA8:
                    return "RWCube_unorm_float4_";
                case EiifRGBA8_SNORM:
                    return "RWCube_unorm_float4_";
                default:
                    UNREACHABLE();
            }
            break;
        }
        case EbtIImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32I:
                case EiifRGBA16I:
                case EiifRGBA8I:
                case EiifR32I:
                    return "RWCube_int4_";
                default:
                    UNREACHABLE();
            }
            break;
        }
        case EbtUImageCube:
        {
            switch (imageInternalFormat)
            {
                case EiifRGBA32UI:
                case EiifRGBA16UI:
                case EiifRGBA8UI:
                case EiifR32UI:
                    return "RWCube_uint4_";
                default:
                    UNREACHABLE();
            }
            break;
        }
        default:
            // All other types are identified by their group suffix
            return RWTextureGroupSuffix(type, imageInternalFormat);
    }
    UNREACHABLE();
    return "_RWTS_invalid_";
}

TString DecorateField(const ImmutableString &string, const TStructure &structure)
{
    if (structure.symbolType() != SymbolType::BuiltIn)
    {
        return Decorate(string);
    }

    return TString(string.data());
}

TString DecoratePrivate(const ImmutableString &privateText)
{
    return "dx_" + TString(privateText.data());
}

TString Decorate(const ImmutableString &string)
{
    if (!gl::IsBuiltInName(string.data()))
    {
        return "_" + TString(string.data());
    }

    return TString(string.data());
}

TString DecorateVariableIfNeeded(const TVariable &variable)
{
    if (variable.symbolType() == SymbolType::AngleInternal ||
        variable.symbolType() == SymbolType::BuiltIn || variable.symbolType() == SymbolType::Empty)
    {
        // Besides handling internal variables, we generate names for nameless parameters here.
        const ImmutableString &name = variable.name();
        // The name should not have a prefix reserved for user-defined variables or functions.
        ASSERT(!name.beginsWith("f_"));
        ASSERT(!name.beginsWith("_"));
        return TString(name.data());
    }
    // For user defined variables, combine variable name with unique id
    // so variables of the same name in different scopes do not get overwritten.
    else if (variable.symbolType() == SymbolType::UserDefined &&
             variable.getType().getQualifier() == EvqTemporary)
    {
        return Decorate(variable.name()) + str(variable.uniqueId().get());
    }
    else
    {
        return Decorate(variable.name());
    }
}

TString DecorateFunctionIfNeeded(const TFunction *func)
{
    if (func->symbolType() == SymbolType::AngleInternal)
    {
        // The name should not have a prefix reserved for user-defined variables or functions.
        ASSERT(!func->name().beginsWith("f_"));
        ASSERT(!func->name().beginsWith("_"));
        return TString(func->name().data());
    }
    ASSERT(!gl::IsBuiltInName(func->name().data()));
    // Add an additional f prefix to functions so that they're always disambiguated from variables.
    // This is necessary in the corner case where a variable declaration hides a function that it
    // uses in its initializer.
    return "f_" + TString(func->name().data());
}

TString TypeString(const TType &type)
{
    const TStructure *structure = type.getStruct();
    if (structure)
    {
        if (structure->symbolType() != SymbolType::Empty)
        {
            return StructNameString(*structure);
        }
        else  // Nameless structure, define in place
        {
            return StructureHLSL::defineNameless(*structure);
        }
    }
    else if (type.isMatrix())
    {
        uint8_t cols = type.getCols();
        uint8_t rows = type.getRows();
        return "float" + str(cols) + "x" + str(rows);
    }
    else
    {
        switch (type.getBasicType())
        {
            case EbtFloat:
                switch (type.getNominalSize())
                {
                    case 1:
                        return "float";
                    case 2:
                        return "float2";
                    case 3:
                        return "float3";
                    case 4:
                        return "float4";
                }
            case EbtInt:
                switch (type.getNominalSize())
                {
                    case 1:
                        return "int";
                    case 2:
                        return "int2";
                    case 3:
                        return "int3";
                    case 4:
                        return "int4";
                }
            case EbtUInt:
                switch (type.getNominalSize())
                {
                    case 1:
                        return "uint";
                    case 2:
                        return "uint2";
                    case 3:
                        return "uint3";
                    case 4:
                        return "uint4";
                }
            case EbtBool:
                switch (type.getNominalSize())
                {
                    case 1:
                        return "bool";
                    case 2:
                        return "bool2";
                    case 3:
                        return "bool3";
                    case 4:
                        return "bool4";
                }
            case EbtVoid:
                return "void";
            case EbtSampler2D:
            case EbtISampler2D:
            case EbtUSampler2D:
            case EbtSampler2DArray:
            case EbtISampler2DArray:
            case EbtUSampler2DArray:
                return "sampler2D";
            case EbtSamplerCube:
            case EbtISamplerCube:
            case EbtUSamplerCube:
                return "samplerCUBE";
            case EbtSamplerExternalOES:
                return "sampler2D";
            case EbtSamplerVideoWEBGL:
                return "sampler2D";
            case EbtAtomicCounter:
                // Multiple atomic_uints will be implemented as a single RWByteAddressBuffer
                return "RWByteAddressBuffer";
            default:
                break;
        }
    }

    UNREACHABLE();
    return "<unknown type>";
}

TString StructNameString(const TStructure &structure)
{
    if (structure.symbolType() == SymbolType::Empty)
    {
        return "";
    }

    // For structures at global scope we use a consistent
    // translation so that we can link between shader stages.
    if (structure.atGlobalScope())
    {
        return Decorate(structure.name());
    }

    return "ss" + str(structure.uniqueId().get()) + "_" + TString(structure.name().data());
}

TString QualifiedStructNameString(const TStructure &structure,
                                  bool useHLSLRowMajorPacking,
                                  bool useStd140Packing,
                                  bool forcePadding)
{
    if (structure.symbolType() == SymbolType::Empty)
    {
        return "";
    }

    TString prefix = "";

    // Structs packed with row-major matrices in HLSL are prefixed with "rm"
    // GLSL column-major maps to HLSL row-major, and the converse is true

    if (useStd140Packing)
    {
        prefix += "std_";
    }

    if (useHLSLRowMajorPacking)
    {
        prefix += "rm_";
    }

    if (forcePadding)
    {
        prefix += "fp_";
    }

    return prefix + StructNameString(structure);
}

const char *InterpolationString(TQualifier qualifier)
{
    switch (qualifier)
    {
        case EvqVaryingOut:
        case EvqVaryingIn:
        case EvqVertexOut:
        case EvqFragmentIn:
            return "";
        case EvqSmoothOut:
        case EvqSmoothIn:
            return "linear";
        case EvqFlatOut:
        case EvqFlatIn:
            return "nointerpolation";
        case EvqNoPerspectiveOut:
        case EvqNoPerspectiveIn:
            return "noperspective";
        case EvqCentroidOut:
        case EvqCentroidIn:
            return "centroid";
        case EvqSampleOut:
        case EvqSampleIn:
            return "sample";
        case EvqNoPerspectiveCentroidOut:
        case EvqNoPerspectiveCentroidIn:
            return "noperspective centroid";
        case EvqNoPerspectiveSampleOut:
        case EvqNoPerspectiveSampleIn:
            return "noperspective sample";
        default:
            UNREACHABLE();
    }

    return "";
}

const char *QualifierString(TQualifier qualifier)
{
    switch (qualifier)
    {
        case EvqParamIn:
            return "in";
        case EvqParamOut:
            return "inout";  // 'out' results in an HLSL error if not all fields are written, for
                             // GLSL it's undefined
        case EvqParamInOut:
            return "inout";
        case EvqParamConst:
            return "const";
        default:
            UNREACHABLE();
    }

    return "";
}

TString DisambiguateFunctionName(const TFunction *func)
{
    TString disambiguatingString;
    size_t paramCount = func->getParamCount();
    for (size_t i = 0; i < paramCount; ++i)
    {
        DisambiguateFunctionNameForParameterType(func->getParam(i)->getType(),
                                                 &disambiguatingString);
    }
    return disambiguatingString;
}

TString DisambiguateFunctionName(const TIntermSequence *args)
{
    TString disambiguatingString;
    for (TIntermNode *arg : *args)
    {
        ASSERT(arg->getAsTyped());
        DisambiguateFunctionNameForParameterType(arg->getAsTyped()->getType(),
                                                 &disambiguatingString);
    }
    return disambiguatingString;
}

}  // namespace sh
