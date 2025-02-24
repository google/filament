//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TextureFunctionHLSL: Class for writing implementations of ESSL texture functions into HLSL
// output. Some of the implementations are straightforward and just call the HLSL equivalent of the
// ESSL texture function, others do more work to emulate ESSL texture sampling or size query
// behavior.
//

#include "compiler/translator/hlsl/TextureFunctionHLSL.h"

#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/hlsl/UtilsHLSL.h"

namespace sh
{

namespace
{

void OutputIntTexCoordWrap(TInfoSinkBase &out,
                           const char *wrapMode,
                           const char *size,
                           const ImmutableString &texCoord,
                           const char *texCoordOffset,
                           const char *texCoordOutName)
{
    // GLES 3.0.4 table 3.22 specifies how the wrap modes work. We don't use the formulas verbatim
    // but rather use equivalent formulas that map better to HLSL.
    out << "int " << texCoordOutName << ";\n";
    out << "float " << texCoordOutName << "Offset = " << texCoord << " + float(" << texCoordOffset
        << ") / " << size << ";\n";
    out << "bool " << texCoordOutName << "UseBorderColor = false;\n";

    // CLAMP_TO_EDGE / D3D11_TEXTURE_ADDRESS_CLAMP == 3
    out << "if (" << wrapMode << " == 3)\n";
    out << "{\n";
    out << "    " << texCoordOutName << " = clamp(int(floor(" << size << " * " << texCoordOutName
        << "Offset)), 0, int(" << size << ") - 1);\n";
    out << "}\n";

    // CLAMP_TO_BORDER / D3D11_TEXTURE_ADDRESS_BORDER == 4
    out << "else if (" << wrapMode << " == 4)\n";
    out << "{\n";
    out << "    int texCoordInt = int(floor(" << size << " * " << texCoordOutName << "Offset));\n";
    out << "    " << texCoordOutName << " = clamp(texCoordInt, 0, int(" << size << ") - 1);\n";
    out << "    " << texCoordOutName << "UseBorderColor = (texCoordInt != " << texCoordOutName
        << ");\n";
    out << "}\n";

    // MIRRORED_REPEAT / D3D11_TEXTURE_ADDRESS_MIRROR == 2
    out << "else if (" << wrapMode << " == 2)\n";
    out << "{\n";
    out << "    float coordWrapped = 1.0 - abs(frac(abs(" << texCoordOutName
        << "Offset) * 0.5) * 2.0 - 1.0);\n";
    out << "    " << texCoordOutName << " = min(int(floor(" << size << " * coordWrapped)), int("
        << size << ") - 1);\n";
    out << "}\n";

    // MIRROR_CLAMP_TO_EDGE_EXT / D3D11_TEXTURE_ADDRESS_MIRROR_ONCE == 5
    out << "else if (" << wrapMode << " == 5)\n";
    out << "{\n";
    out << "    " << texCoordOutName << " = min(int(floor(" << size << " * abs(" << texCoordOutName
        << "Offset))), int(" << size << ") - 1);\n";
    out << "}\n";

    // REPEAT / D3D11_TEXTURE_ADDRESS_WRAP == 1
    out << "else\n";
    out << "{\n";
    out << "    " << texCoordOutName << " = int(floor(" << size << " * frac(" << texCoordOutName
        << "Offset)));\n";
    out << "}\n";
}

void OutputIntTexCoordWraps(TInfoSinkBase &out,
                            const TextureFunctionHLSL::TextureFunction &textureFunction,
                            ImmutableString *texCoordX,
                            ImmutableString *texCoordY,
                            ImmutableString *texCoordZ)
{
    // Convert from normalized floating-point to integer
    out << "int wrapS = samplerMetadata[samplerIndex].wrapModes & 0x7;\n";
    if (textureFunction.offset)
    {
        OutputIntTexCoordWrap(out, "wrapS", "width", *texCoordX, "offset.x", "tix");
    }
    else
    {
        OutputIntTexCoordWrap(out, "wrapS", "width", *texCoordX, "0", "tix");
    }
    *texCoordX = ImmutableString("tix");
    out << "int wrapT = (samplerMetadata[samplerIndex].wrapModes >> 3) & 0x7;\n";
    if (textureFunction.offset)
    {
        OutputIntTexCoordWrap(out, "wrapT", "height", *texCoordY, "offset.y", "tiy");
    }
    else
    {
        OutputIntTexCoordWrap(out, "wrapT", "height", *texCoordY, "0", "tiy");
    }
    *texCoordY = ImmutableString("tiy");

    bool tizAvailable = false;

    if (IsSamplerArray(textureFunction.sampler))
    {
        *texCoordZ = ImmutableString("int(max(0, min(layers - 1, floor(0.5 + t.z))))");
    }
    else if (!IsSamplerCube(textureFunction.sampler) && !IsSampler2D(textureFunction.sampler))
    {
        out << "int wrapR = (samplerMetadata[samplerIndex].wrapModes >> 6) & 0x7;\n";
        if (textureFunction.offset)
        {
            OutputIntTexCoordWrap(out, "wrapR", "depth", *texCoordZ, "offset.z", "tiz");
        }
        else
        {
            OutputIntTexCoordWrap(out, "wrapR", "depth", *texCoordZ, "0", "tiz");
        }
        *texCoordZ   = ImmutableString("tiz");
        tizAvailable = true;
    }

    out << "bool useBorderColor = tixUseBorderColor || tiyUseBorderColor"
        << (tizAvailable ? " || tizUseBorderColor" : "") << ";\n";
}

void OutputHLSL4SampleFunctionPrefix(TInfoSinkBase &out,
                                     const TextureFunctionHLSL::TextureFunction &textureFunction,
                                     const ImmutableString &textureReference,
                                     const ImmutableString &samplerReference)
{
    out << textureReference;
    if (IsIntegerSampler(textureFunction.sampler) ||
        textureFunction.method == TextureFunctionHLSL::TextureFunction::FETCH)
    {
        out << ".Load(";
        return;
    }

    if (IsShadowSampler(textureFunction.sampler))
    {
        switch (textureFunction.method)
        {
            case TextureFunctionHLSL::TextureFunction::IMPLICIT:
            case TextureFunctionHLSL::TextureFunction::BIAS:
            case TextureFunctionHLSL::TextureFunction::LOD:
                out << ".SampleCmp(";
                break;
            case TextureFunctionHLSL::TextureFunction::LOD0:
            case TextureFunctionHLSL::TextureFunction::LOD0BIAS:
            case TextureFunctionHLSL::TextureFunction::GRAD:
                out << ".SampleCmpLevelZero(";
                break;
            default:
                UNREACHABLE();
        }
    }
    else
    {
        switch (textureFunction.method)
        {
            case TextureFunctionHLSL::TextureFunction::IMPLICIT:
                out << ".Sample(";
                break;
            case TextureFunctionHLSL::TextureFunction::BIAS:
                out << ".SampleBias(";
                break;
            case TextureFunctionHLSL::TextureFunction::LOD:
            case TextureFunctionHLSL::TextureFunction::LOD0:
            case TextureFunctionHLSL::TextureFunction::LOD0BIAS:
                out << ".SampleLevel(";
                break;
            case TextureFunctionHLSL::TextureFunction::GRAD:
                out << ".SampleGrad(";
                break;
            default:
                UNREACHABLE();
        }
    }
    out << samplerReference << ", ";
}

const char *GetSamplerCoordinateTypeString(
    const TextureFunctionHLSL::TextureFunction &textureFunction,
    int hlslCoords)
{
    // Gather[Red|Green|Blue|Alpha] accepts float texture coordinates on textures in integer or
    // unsigned integer formats.
    // https://docs.microsoft.com/en-us/windows/desktop/direct3dhlsl/dx-graphics-hlsl-to-gather
    if ((IsIntegerSampler(textureFunction.sampler) &&
         textureFunction.method != TextureFunctionHLSL::TextureFunction::GATHER) ||
        textureFunction.method == TextureFunctionHLSL::TextureFunction::FETCH)
    {
        switch (hlslCoords)
        {
            case 1:
                return "int";
            case 2:
                if (IsSampler2DMS(textureFunction.sampler))
                {
                    return "int2";
                }
                else
                {
                    return "int3";
                }
            case 3:
                if (IsSampler2DMSArray(textureFunction.sampler))
                {
                    return "int3";
                }
                else
                {
                    return "int4";
                }
            default:
                UNREACHABLE();
        }
    }
    else
    {
        switch (hlslCoords)
        {
            case 1:
                return "float";
            case 2:
                return "float2";
            case 3:
                return "float3";
            case 4:
                return "float4";
            default:
                UNREACHABLE();
        }
    }
    return "";
}

int GetHLSLCoordCount(const TextureFunctionHLSL::TextureFunction &textureFunction,
                      ShShaderOutput outputType)
{
    if (outputType == SH_HLSL_3_0_OUTPUT)
    {
        int hlslCoords = 2;
        switch (textureFunction.sampler)
        {
            case EbtSamplerBuffer:
                hlslCoords = 1;
                break;
            case EbtSampler2D:
            case EbtSamplerExternalOES:
            case EbtSampler2DMS:
            case EbtSamplerVideoWEBGL:
                hlslCoords = 2;
                break;
            case EbtSamplerCube:
                hlslCoords = 3;
                break;
            default:
                UNREACHABLE();
        }

        switch (textureFunction.method)
        {
            case TextureFunctionHLSL::TextureFunction::IMPLICIT:
            case TextureFunctionHLSL::TextureFunction::GRAD:
                return hlslCoords;
            case TextureFunctionHLSL::TextureFunction::BIAS:
            case TextureFunctionHLSL::TextureFunction::LOD:
            case TextureFunctionHLSL::TextureFunction::LOD0:
            case TextureFunctionHLSL::TextureFunction::LOD0BIAS:
                return 4;
            default:
                UNREACHABLE();
        }
    }
    else
    {
        if (IsSamplerBuffer(textureFunction.sampler))
        {
            return 1;
        }
        else if (IsSampler3D(textureFunction.sampler) || IsSamplerArray(textureFunction.sampler) ||
                 IsSamplerCube(textureFunction.sampler))
        {
            return 3;
        }
        ASSERT(IsSampler2D(textureFunction.sampler));
        return 2;
    }
    return 0;
}

void OutputTextureFunctionArgumentList(TInfoSinkBase &out,
                                       const TextureFunctionHLSL::TextureFunction &textureFunction,
                                       const ShShaderOutput outputType)
{
    if (outputType == SH_HLSL_3_0_OUTPUT)
    {
        switch (textureFunction.sampler)
        {
            case EbtSampler2D:
            case EbtSamplerVideoWEBGL:
            case EbtSamplerExternalOES:
                out << "sampler2D s";
                break;
            case EbtSamplerCube:
                out << "samplerCUBE s";
                break;
            default:
                UNREACHABLE();
        }
    }
    else
    {
        ASSERT(outputType == SH_HLSL_4_1_OUTPUT);
        // A bug in the D3D compiler causes some nested sampling operations to fail.
        // See http://anglebug.com/42260714
        // TODO(jmadill): Reinstate the const keyword when possible.
        out << /*"const"*/ "uint samplerIndex";
    }

    if (textureFunction.method ==
        TextureFunctionHLSL::TextureFunction::FETCH)  // Integer coordinates
    {
        switch (textureFunction.coords)
        {
            case 1:
                out << ", int t";
                break;
            case 2:
                out << ", int2 t";
                break;
            case 3:
                out << ", int3 t";
                break;
            default:
                UNREACHABLE();
        }
    }
    else  // Floating-point coordinates (except textureSize)
    {
        switch (textureFunction.coords)
        {
            case 0:
                break;  // textureSize(gSampler2DMS sampler)
            case 1:
                out << ", int lod";
                break;  // textureSize()
            case 2:
                out << ", float2 t";
                break;
            case 3:
                out << ", float3 t";
                break;
            case 4:
                out << ", float4 t";
                break;
            default:
                UNREACHABLE();
        }
    }

    if (textureFunction.method == TextureFunctionHLSL::TextureFunction::GRAD)
    {
        switch (textureFunction.sampler)
        {
            case EbtSampler2D:
            case EbtISampler2D:
            case EbtUSampler2D:
            case EbtSampler2DArray:
            case EbtISampler2DArray:
            case EbtUSampler2DArray:
            case EbtSampler2DShadow:
            case EbtSampler2DArrayShadow:
            case EbtSamplerExternalOES:
            case EbtSamplerVideoWEBGL:
                out << ", float2 ddx, float2 ddy";
                break;
            case EbtSampler3D:
            case EbtISampler3D:
            case EbtUSampler3D:
            case EbtSamplerCube:
            case EbtISamplerCube:
            case EbtUSamplerCube:
            case EbtSamplerCubeShadow:
                out << ", float3 ddx, float3 ddy";
                break;
            default:
                UNREACHABLE();
        }
    }

    switch (textureFunction.method)
    {
        case TextureFunctionHLSL::TextureFunction::IMPLICIT:
            break;
        case TextureFunctionHLSL::TextureFunction::BIAS:
            break;  // Comes after the offset parameter
        case TextureFunctionHLSL::TextureFunction::LOD:
            out << ", float lod";
            break;
        case TextureFunctionHLSL::TextureFunction::LOD0:
            break;
        case TextureFunctionHLSL::TextureFunction::LOD0BIAS:
            break;  // Comes after the offset parameter
        case TextureFunctionHLSL::TextureFunction::SIZE:
            break;
        case TextureFunctionHLSL::TextureFunction::FETCH:
            if (IsSampler2DMS(textureFunction.sampler) ||
                IsSampler2DMSArray(textureFunction.sampler))
                out << ", int index";
            else if (!IsSamplerBuffer(textureFunction.sampler))
                out << ", int mip";
            break;
        case TextureFunctionHLSL::TextureFunction::GRAD:
            break;
        case TextureFunctionHLSL::TextureFunction::GATHER:
            break;
        default:
            UNREACHABLE();
    }

    if (textureFunction.method == TextureFunctionHLSL::TextureFunction::GATHER &&
        IsShadowSampler(textureFunction.sampler))
    {
        out << ", float refZ";
    }

    if (textureFunction.offset)
    {
        switch (textureFunction.sampler)
        {
            case EbtSampler3D:
            case EbtISampler3D:
            case EbtUSampler3D:
                out << ", int3 offset";
                break;
            case EbtSampler2D:
            case EbtSampler2DArray:
            case EbtISampler2D:
            case EbtISampler2DArray:
            case EbtUSampler2D:
            case EbtUSampler2DArray:
            case EbtSampler2DShadow:
            case EbtSampler2DArrayShadow:
            case EbtSamplerExternalOES:
            case EbtSamplerVideoWEBGL:
                out << ", int2 offset";
                break;
            default:
                // Offset is not supported for multisampled textures.
                UNREACHABLE();
        }
    }

    if (textureFunction.method == TextureFunctionHLSL::TextureFunction::BIAS ||
        textureFunction.method == TextureFunctionHLSL::TextureFunction::LOD0BIAS)
    {
        out << ", float bias";
    }
    else if (textureFunction.method == TextureFunctionHLSL::TextureFunction::GATHER &&
             !IsShadowSampler(textureFunction.sampler))
    {
        out << ", int comp = 0";
    }
}

void GetTextureReference(TInfoSinkBase &out,
                         const TextureFunctionHLSL::TextureFunction &textureFunction,
                         const ShShaderOutput outputType,
                         ImmutableString *textureReference,
                         ImmutableString *samplerReference)
{
    if (outputType == SH_HLSL_4_1_OUTPUT)
    {
        static const ImmutableString kTexturesStr("textures");
        static const ImmutableString kSamplersStr("samplers");
        static const ImmutableString kSamplerIndexStr("[samplerIndex]");
        static const ImmutableString kTextureIndexStr("[textureIndex]");
        static const ImmutableString kSamplerArrayIndexStr("[samplerArrayIndex]");
        ImmutableString suffix(TextureGroupSuffix(textureFunction.sampler));

        if (TextureGroup(textureFunction.sampler) == HLSL_TEXTURE_2D)
        {
            ImmutableStringBuilder textureRefBuilder(kTexturesStr.length() + suffix.length() +
                                                     kSamplerIndexStr.length());
            textureRefBuilder << kTexturesStr << suffix << kSamplerIndexStr;
            *textureReference = textureRefBuilder;
            ImmutableStringBuilder samplerRefBuilder(kSamplersStr.length() + suffix.length() +
                                                     kSamplerIndexStr.length());
            samplerRefBuilder << kSamplersStr << suffix << kSamplerIndexStr;
            *samplerReference = samplerRefBuilder;
        }
        else
        {
            out << "    const uint textureIndex = samplerIndex - textureIndexOffset"
                << suffix.data() << ";\n";
            ImmutableStringBuilder textureRefBuilder(kTexturesStr.length() + suffix.length() +
                                                     kTextureIndexStr.length());
            textureRefBuilder << kTexturesStr << suffix << kTextureIndexStr;
            *textureReference = textureRefBuilder;

            out << "    const uint samplerArrayIndex = samplerIndex - samplerIndexOffset"
                << suffix.data() << ";\n";
            ImmutableStringBuilder samplerRefBuilder(kSamplersStr.length() + suffix.length() +
                                                     kSamplerArrayIndexStr.length());
            samplerRefBuilder << kSamplersStr << suffix << kSamplerArrayIndexStr;
            *samplerReference = samplerRefBuilder;
        }
    }
    else
    {
        *textureReference = ImmutableString("x");
        *samplerReference = ImmutableString("s");
    }
}

void OutputTextureSizeFunctionBody(TInfoSinkBase &out,
                                   const TextureFunctionHLSL::TextureFunction &textureFunction,
                                   const ImmutableString &textureReference,
                                   bool getDimensionsIgnoresBaseLevel)
{
    if (IsSampler2DMS(textureFunction.sampler))
    {
        out << "    uint width; uint height; uint samples;\n"
            << "    " << textureReference << ".GetDimensions(width, height, samples);\n";
    }
    else if (IsSampler2DMSArray(textureFunction.sampler))
    {
        out << "    uint width; uint height; uint depth; uint samples;\n"
            << "    " << textureReference << ".GetDimensions(width, height, depth, samples);\n";
    }
    else if (IsSamplerBuffer(textureFunction.sampler))
    {
        out << "    uint width;\n" << "    " << textureReference << ".GetDimensions(width);\n";
    }
    else
    {
        if (getDimensionsIgnoresBaseLevel)
        {
            out << "    int baseLevel = samplerMetadata[samplerIndex].baseLevel;\n";
        }
        else
        {
            out << "    int baseLevel = 0;\n";
        }

        if (IsSampler3D(textureFunction.sampler) || IsSamplerArray(textureFunction.sampler) ||
            (IsIntegerSampler(textureFunction.sampler) && IsSamplerCube(textureFunction.sampler)))
        {
            // "depth" stores either the number of layers in an array texture or 3D depth
            out << "    uint width; uint height; uint depth; uint numberOfLevels;\n"
                << "    " << textureReference
                << ".GetDimensions(baseLevel, width, height, depth, numberOfLevels);\n"
                << "    width = max(width >> lod, 1);\n"
                << "    height = max(height >> lod, 1);\n";

            if (!IsSamplerArray(textureFunction.sampler))
            {
                out << "    depth = max(depth >> lod, 1);\n";
            }
        }
        else if (IsSampler2D(textureFunction.sampler) || IsSamplerCube(textureFunction.sampler))
        {
            out << "    uint width; uint height; uint numberOfLevels;\n"
                << "    " << textureReference
                << ".GetDimensions(baseLevel, width, height, numberOfLevels);\n"
                << "    width = max(width >> lod, 1);\n"
                << "    height = max(height >> lod, 1);\n";
        }
        else
            UNREACHABLE();
    }

    const char *returnType = textureFunction.getReturnType();
    if (strcmp(returnType, "int3") == 0)
    {
        out << "    return int3(width, height, depth);\n";
    }
    else if (strcmp(returnType, "int2") == 0)
    {
        out << "    return int2(width, height);\n";
    }
    else
    {
        out << "    return int(width);\n";
    }
}

void ProjectTextureCoordinates(const TextureFunctionHLSL::TextureFunction &textureFunction,
                               ImmutableString *texCoordX,
                               ImmutableString *texCoordY,
                               ImmutableString *texCoordZ)
{
    if (textureFunction.proj)
    {
        ImmutableString proj("");
        switch (textureFunction.coords)
        {
            case 3:
                proj = ImmutableString(" / t.z");
                break;
            case 4:
                proj = ImmutableString(" / t.w");
                break;
            default:
                UNREACHABLE();
        }
        ImmutableStringBuilder texCoordXBuilder(texCoordX->length() + proj.length() + 2u);
        texCoordXBuilder << '(' << *texCoordX << proj << ')';
        *texCoordX = texCoordXBuilder;
        ImmutableStringBuilder texCoordYBuilder(texCoordY->length() + proj.length() + 2u);
        texCoordYBuilder << '(' << *texCoordY << proj << ')';
        *texCoordY = texCoordYBuilder;
        ImmutableStringBuilder texCoordZBuilder(texCoordZ->length() + proj.length() + 2u);
        texCoordZBuilder << '(' << *texCoordZ << proj << ')';
        *texCoordZ = texCoordZBuilder;
    }
}

void OutputIntegerTextureSampleFunctionComputations(
    TInfoSinkBase &out,
    const TextureFunctionHLSL::TextureFunction &textureFunction,
    const ShShaderOutput outputType,
    const ImmutableString &textureReference,
    ImmutableString *texCoordX,
    ImmutableString *texCoordY,
    ImmutableString *texCoordZ,
    bool getDimensionsIgnoresBaseLevel)
{
    if (!IsIntegerSampler(textureFunction.sampler))
    {
        return;
    }

    if (getDimensionsIgnoresBaseLevel)
    {
        out << "    int baseLevel = samplerMetadata[samplerIndex].baseLevel;\n";
    }
    else
    {
        out << "    int baseLevel = 0;\n";
    }

    if (IsSamplerCube(textureFunction.sampler))
    {
        out << "    float width; float height; float layers; float levels;\n";

        out << "    uint mip = 0;\n";

        out << "    " << textureReference
            << ".GetDimensions(baseLevel + mip, width, height, layers, levels);\n";

        out << "    bool xMajor = abs(t.x) >= abs(t.y) && abs(t.x) >= abs(t.z);\n";
        out << "    bool yMajor = abs(t.y) >= abs(t.z) && abs(t.y) > abs(t.x);\n";
        out << "    bool zMajor = abs(t.z) > abs(t.x) && abs(t.z) > abs(t.y);\n";
        out << "    bool negative = (xMajor && t.x < 0.0f) || (yMajor && t.y < 0.0f) || "
               "(zMajor && t.z < 0.0f);\n";

        // FACE_POSITIVE_X = 000b
        // FACE_NEGATIVE_X = 001b
        // FACE_POSITIVE_Y = 010b
        // FACE_NEGATIVE_Y = 011b
        // FACE_POSITIVE_Z = 100b
        // FACE_NEGATIVE_Z = 101b
        out << "    int face = (int)negative + (int)yMajor * 2 + (int)zMajor * 4;\n";

        out << "    float u = xMajor ? -t.z : (yMajor && t.y < 0.0f ? -t.x : t.x);\n";
        out << "    float v = yMajor ? t.z : (negative ? t.y : -t.y);\n";
        out << "    float m = xMajor ? t.x : (yMajor ? t.y : t.z);\n";

        out << "    float3 r = any(t) ? t : float3(1, 0, 0);\n";
        out << "    t.x = (u * 0.5f / m) + 0.5f;\n";
        out << "    t.y = (v * 0.5f / m) + 0.5f;\n";

        // Mip level computation.
        if (textureFunction.method == TextureFunctionHLSL::TextureFunction::IMPLICIT ||
            textureFunction.method == TextureFunctionHLSL::TextureFunction::LOD ||
            textureFunction.method == TextureFunctionHLSL::TextureFunction::GRAD)
        {
            if (textureFunction.method == TextureFunctionHLSL::TextureFunction::IMPLICIT)
            {
                // We would like to calculate tha maximum of how many texels we move in the major
                // face's texture as we move across the screen in any direction. Namely, we want the
                // length of the directional derivative of the function p (defined below), maximized
                // over screen space directions. (For short: we want the norm of Dp.) For
                // simplicity, assume that z-axis is the major axis. By symmetry, we can assume that
                // the positive z direction is major. (The calculated value will be the same even if
                // this is false.) Let r denote the function from screen position to cube texture
                // coordinates. Then p can be written as p = s . P . r, where P(r) = (r.x, r.y)/r.z
                // is the projection onto the major cube face, and s = diag(width, height)/2. (s
                // linearly maps from the cube face into texture space, so that p(r) is in units of
                // texels.) The derivative is
                // Dp(r) = s |1 0 -r.x/r.z|
                //           |0 1 -r.y/r.z| |ddx(r) ddy(r)| / r.z
                //       = |dot(a, ddx(r)) dot(a, ddy(r))|
                //         |dot(b, ddx(r)) dot(b, ddy(r))| / (2 r.z)
                // where a = w * vec3(1, 0, -r.x/r.z)
                //       b = h * vec3(0, 1, -r.y/r.z)
                // We would like to know max(L(x)) over unit vectors x, where L(x) = |Dp(r) x|^2.
                // Since ddx(r) and ddy(r) are unknown, the best we can do is to sample L in some
                // directions and take the maximum across the samples.
                //
                // Some implementations use max(L(n1), L(n2)) where n1 = vec2(1,0) and n2 =
                // vec2(0,1).
                //
                // Some implementations use max(L(n1), L(n2), L(n3), L(n4)),
                // where n3 = (n1 + n2) / |n1 + n2| = (n1 + n2)/sqrt(2)
                //       n4 = (n1 - n2) / |n1 - n2| = (n1 - n2)/sqrt(2).
                // In other words, two samples along the diagonal screen space directions have been
                // added, giving a strictly better estimate of the true maximum.
                //
                // It turns out we can get twice the sample count very cheaply.
                // We can use the linearity of Dp(r) to get these extra samples of L cheaply in
                // terms of the already taken samples, L(n1) and L(n2):
                // Denoting
                // dpx = Dp(r)n1
                // dpy = Dp(r)n2
                // dpxx = dot(dpx, dpx)
                // dpyy = dot(dpy, dpy)
                // dpxy = dot(dpx, dpy)
                // we obtain
                // L(n3) = |Dp(r)n1 + Dp(r)n2|^2/2 = (dpxx + dpyy)/2 + dpxy
                // L(n4) = |Dp(r)n1 - Dp(r)n2|^2/2 = (dpxx + dpyy)/2 - dpxy
                // max(L(n1), L(n2), L(n3), L(n4))
                // = max(max(L(n1), L(n2)), max(L(n3), L(n4)))
                // = max(max(dpxx, dpyy), (dpxx + dpyy)/2 + abs(dpxy))
                // So the extra cost is: one dot, one abs, one add, one multiply-add and one max.
                // (All scalar.)
                //
                // In section 3.8.10.1, the OpenGL ES 3 specification defines the "scale factor",
                // rho. In our terminology, this definition works out to taking sqrt(max(L(n1),
                // L(n2))). Some implementations will use this estimate, here we use the strictly
                // better sqrt(max(L(n1), L(n2), L(n3), L(n4))), since it's not much more expensive
                // to calculate.

                // Swap coordinates such that we can assume that the positive z-axis is major, in
                // what follows.
                out << "    float3 ddxr = xMajor ? ddx(r).yzx : yMajor ? ddx(r).zxy : ddx(r).xyz;\n"
                       "    float3 ddyr = xMajor ? ddy(r).yzx : yMajor ? ddy(r).zxy : ddy(r).xyz;\n"
                       "    r = xMajor ? r.yzx : yMajor ? r.zxy : r.xyz;\n";

                out << "    float2 s = 0.5*float2(width, height);\n"
                       "    float2 dpx = s * (ddxr.xy - ddxr.z*r.xy/r.z)/r.z;\n"
                       "    float2 dpy = s * (ddyr.xy - ddyr.z*r.xy/r.z)/r.z;\n"
                       "    float dpxx = dot(dpx, dpx);\n;"
                       "    float dpyy = dot(dpy, dpy);\n;"
                       "    float dpxy = dot(dpx, dpy);\n"
                       "    float ma = max(dpxx, dpyy);\n"
                       "    float mb = 0.5 * (dpxx + dpyy) + abs(dpxy);\n"
                       "    float mab = max(ma, mb);\n"
                       "    float lod = 0.5f * log2(mab);\n";
            }
            else if (textureFunction.method == TextureFunctionHLSL::TextureFunction::GRAD)
            {
                // ESSL 3.00.6 spec section 8.8: "For the cube version, the partial
                // derivatives of P are assumed to be in the coordinate system used before
                // texture coordinates are projected onto the appropriate cube face."
                // ddx[0] and ddy[0] are the derivatives of t.x passed into the function
                // ddx[1] and ddy[1] are the derivatives of t.y passed into the function
                // ddx[2] and ddy[2] are the derivatives of t.z passed into the function
                // Determine the derivatives of u, v and m
                out << "    float dudx = xMajor ? ddx[2] : (yMajor && t.y < 0.0f ? -ddx[0] "
                       ": ddx[0]);\n"
                       "    float dudy = xMajor ? ddy[2] : (yMajor && t.y < 0.0f ? -ddy[0] "
                       ": ddy[0]);\n"
                       "    float dvdx = yMajor ? ddx[2] : (negative ? ddx[1] : -ddx[1]);\n"
                       "    float dvdy = yMajor ? ddy[2] : (negative ? ddy[1] : -ddy[1]);\n"
                       "    float dmdx = xMajor ? ddx[0] : (yMajor ? ddx[1] : ddx[2]);\n"
                       "    float dmdy = xMajor ? ddy[0] : (yMajor ? ddy[1] : ddy[2]);\n";
                // Now determine the derivatives of the face coordinates, using the
                // derivatives calculated above.
                // d / dx (u(x) * 0.5 / m(x) + 0.5)
                // = 0.5 * (m(x) * u'(x) - u(x) * m'(x)) / m(x)^2
                out << "    float dfacexdx = 0.5f * (m * dudx - u * dmdx) / (m * m);\n"
                       "    float dfaceydx = 0.5f * (m * dvdx - v * dmdx) / (m * m);\n"
                       "    float dfacexdy = 0.5f * (m * dudy - u * dmdy) / (m * m);\n"
                       "    float dfaceydy = 0.5f * (m * dvdy - v * dmdy) / (m * m);\n"
                       "    float2 sizeVec = float2(width, height);\n"
                       "    float2 faceddx = float2(dfacexdx, dfaceydx) * sizeVec;\n"
                       "    float2 faceddy = float2(dfacexdy, dfaceydy) * sizeVec;\n";
                // Optimization: instead of: log2(max(length(faceddx), length(faceddy)))
                // we compute: log2(max(length(faceddx)^2, length(faceddy)^2)) / 2
                out << "    float lengthfaceddx2 = dot(faceddx, faceddx);\n"
                       "    float lengthfaceddy2 = dot(faceddy, faceddy);\n"
                       "    float lod = log2(max(lengthfaceddx2, lengthfaceddy2)) * 0.5f;\n";
            }
            out << "    mip = uint(min(max(round(lod), 0), levels - 1));\n"
                << "    " << textureReference
                << ".GetDimensions(baseLevel + mip, width, height, layers, levels);\n";
        }

        // Convert from normalized floating-point to integer
        static const ImmutableString kXPrefix("int(floor(width * frac(");
        static const ImmutableString kYPrefix("int(floor(height * frac(");
        static const ImmutableString kSuffix(")))");
        ImmutableStringBuilder texCoordXBuilder(kXPrefix.length() + texCoordX->length() +
                                                kSuffix.length());
        texCoordXBuilder << kXPrefix << *texCoordX << kSuffix;
        *texCoordX = texCoordXBuilder;
        ImmutableStringBuilder texCoordYBuilder(kYPrefix.length() + texCoordX->length() +
                                                kSuffix.length());
        texCoordYBuilder << kYPrefix << *texCoordY << kSuffix;
        *texCoordY = texCoordYBuilder;
        *texCoordZ = ImmutableString("face");
    }
    else if (textureFunction.method != TextureFunctionHLSL::TextureFunction::FETCH)
    {
        if (IsSamplerArray(textureFunction.sampler))
        {
            out << "    float width; float height; float layers; float levels;\n";
            if (textureFunction.method == TextureFunctionHLSL::TextureFunction::LOD0)
            {
                out << "    uint mip = 0;\n";
            }
            else if (textureFunction.method == TextureFunctionHLSL::TextureFunction::LOD0BIAS)
            {
                out << "    uint mip = bias;\n";
            }
            else
            {

                out << "    " << textureReference
                    << ".GetDimensions(baseLevel, width, height, layers, levels);\n";
                if (textureFunction.method == TextureFunctionHLSL::TextureFunction::IMPLICIT ||
                    textureFunction.method == TextureFunctionHLSL::TextureFunction::BIAS)
                {
                    out << "    float2 tSized = float2(t.x * width, t.y * height);\n"
                           "    float dx = length(ddx(tSized));\n"
                           "    float dy = length(ddy(tSized));\n"
                           "    float lod = log2(max(dx, dy));\n";

                    if (textureFunction.method == TextureFunctionHLSL::TextureFunction::BIAS)
                    {
                        out << "    lod += bias;\n";
                    }
                }
                else if (textureFunction.method == TextureFunctionHLSL::TextureFunction::GRAD)
                {
                    out << "    float2 sizeVec = float2(width, height);\n"
                           "    float2 sizeDdx = ddx * sizeVec;\n"
                           "    float2 sizeDdy = ddy * sizeVec;\n"
                           "    float lod = log2(max(dot(sizeDdx, sizeDdx), "
                           "dot(sizeDdy, sizeDdy))) * 0.5f;\n";
                }

                out << "    uint mip = uint(min(max(round(lod), 0), levels - 1));\n";
            }

            out << "    " << textureReference
                << ".GetDimensions(baseLevel + mip, width, height, layers, levels);\n";
        }
        else if (IsSampler2D(textureFunction.sampler))
        {
            out << "    float width; float height; float levels;\n";

            if (textureFunction.method == TextureFunctionHLSL::TextureFunction::LOD0)
            {
                out << "    uint mip = 0;\n";
            }
            else if (textureFunction.method == TextureFunctionHLSL::TextureFunction::LOD0BIAS)
            {
                out << "    uint mip = bias;\n";
            }
            else
            {
                out << "    " << textureReference
                    << ".GetDimensions(baseLevel, width, height, levels);\n";

                if (textureFunction.method == TextureFunctionHLSL::TextureFunction::IMPLICIT ||
                    textureFunction.method == TextureFunctionHLSL::TextureFunction::BIAS)
                {
                    out << "    float2 tSized = float2(t.x * width, t.y * height);\n"
                           "    float dx = length(ddx(tSized));\n"
                           "    float dy = length(ddy(tSized));\n"
                           "    float lod = log2(max(dx, dy));\n";

                    if (textureFunction.method == TextureFunctionHLSL::TextureFunction::BIAS)
                    {
                        out << "    lod += bias;\n";
                    }
                }
                else if (textureFunction.method == TextureFunctionHLSL::TextureFunction::GRAD)
                {
                    out << "    float2 sizeVec = float2(width, height);\n"
                           "    float2 sizeDdx = ddx * sizeVec;\n"
                           "    float2 sizeDdy = ddy * sizeVec;\n"
                           "    float lod = log2(max(dot(sizeDdx, sizeDdx), "
                           "dot(sizeDdy, sizeDdy))) * 0.5f;\n";
                }

                out << "    uint mip = uint(min(max(round(lod), 0), levels - 1));\n";
            }

            out << "    " << textureReference
                << ".GetDimensions(baseLevel + mip, width, height, levels);\n";
        }
        else if (IsSampler3D(textureFunction.sampler))
        {
            out << "    float width; float height; float depth; float levels;\n";

            if (textureFunction.method == TextureFunctionHLSL::TextureFunction::LOD0)
            {
                out << "    uint mip = 0;\n";
            }
            else if (textureFunction.method == TextureFunctionHLSL::TextureFunction::LOD0BIAS)
            {
                out << "    uint mip = bias;\n";
            }
            else
            {
                out << "    " << textureReference
                    << ".GetDimensions(baseLevel, width, height, depth, levels);\n";

                if (textureFunction.method == TextureFunctionHLSL::TextureFunction::IMPLICIT ||
                    textureFunction.method == TextureFunctionHLSL::TextureFunction::BIAS)
                {
                    out << "    float3 tSized = float3(t.x * width, t.y * height, t.z * depth);\n"
                           "    float dx = length(ddx(tSized));\n"
                           "    float dy = length(ddy(tSized));\n"
                           "    float lod = log2(max(dx, dy));\n";

                    if (textureFunction.method == TextureFunctionHLSL::TextureFunction::BIAS)
                    {
                        out << "    lod += bias;\n";
                    }
                }
                else if (textureFunction.method == TextureFunctionHLSL::TextureFunction::GRAD)
                {
                    out << "    float3 sizeVec = float3(width, height, depth);\n"
                           "    float3 sizeDdx = ddx * sizeVec;\n"
                           "    float3 sizeDdy = ddy * sizeVec;\n"
                           "    float lod = log2(max(dot(sizeDdx, sizeDdx), dot(sizeDdy, "
                           "sizeDdy))) * 0.5f;\n";
                }

                out << "    uint mip = uint(min(max(round(lod), 0), levels - 1));\n";
            }

            out << "    " << textureReference
                << ".GetDimensions(baseLevel + mip, width, height, depth, levels);\n";
        }
        else
            UNREACHABLE();

        OutputIntTexCoordWraps(out, textureFunction, texCoordX, texCoordY, texCoordZ);
    }
}

void OutputTextureGatherFunctionBody(TInfoSinkBase &out,
                                     const TextureFunctionHLSL::TextureFunction &textureFunction,
                                     ShShaderOutput outputType,
                                     const ImmutableString &textureReference,
                                     const ImmutableString &samplerReference,
                                     const ImmutableString &texCoordX,
                                     const ImmutableString &texCoordY,
                                     const ImmutableString &texCoordZ)
{
    const int hlslCoords = GetHLSLCoordCount(textureFunction, outputType);
    ImmutableString samplerCoordTypeString(
        GetSamplerCoordinateTypeString(textureFunction, hlslCoords));
    ImmutableStringBuilder samplerCoordBuilder(
        samplerCoordTypeString.length() + strlen("(") + texCoordX.length() + strlen(", ") +
        texCoordY.length() + strlen(", ") + texCoordZ.length() + strlen(")"));

    samplerCoordBuilder << samplerCoordTypeString << "(" << texCoordX << ", " << texCoordY;
    if (hlslCoords >= 3)
    {
        if (textureFunction.coords < 3)
        {
            samplerCoordBuilder << ", 0";
        }
        else
        {
            samplerCoordBuilder << ", " << texCoordZ;
        }
    }
    samplerCoordBuilder << ")";

    ImmutableString samplerCoordString(samplerCoordBuilder);

    if (IsShadowSampler(textureFunction.sampler))
    {
        out << "return " << textureReference << ".GatherCmp(" << samplerReference << ", "
            << samplerCoordString << ", refZ";
        if (textureFunction.offset)
        {
            out << ", offset";
        }
        out << ");\n";
        return;
    }

    constexpr std::array<const char *, 4> kHLSLGatherFunctions = {
        {"GatherRed", "GatherGreen", "GatherBlue", "GatherAlpha"}};

    out << "    switch(comp)\n"
           "    {\n";
    for (size_t component = 0; component < kHLSLGatherFunctions.size(); ++component)
    {
        out << "        case " << component << ":\n"
            << "            return " << textureReference << "." << kHLSLGatherFunctions[component]
            << "(" << samplerReference << ", " << samplerCoordString;
        if (textureFunction.offset)
        {
            out << ", offset";
        }
        out << ");\n";
    }

    out << "        default:\n"
           "            return float4(0.0, 0.0, 0.0, 1.0);\n"
           "    }\n";
}

void OutputTextureSampleFunctionReturnStatement(
    TInfoSinkBase &out,
    const TextureFunctionHLSL::TextureFunction &textureFunction,
    const ShShaderOutput outputType,
    const ImmutableString &textureReference,
    const ImmutableString &samplerReference,
    const ImmutableString &texCoordX,
    const ImmutableString &texCoordY,
    const ImmutableString &texCoordZ)
{
    out << "    return ";

    if (IsIntegerSampler(textureFunction.sampler) && !IsSamplerCube(textureFunction.sampler) &&
        textureFunction.method != TextureFunctionHLSL::TextureFunction::FETCH)
    {
        out << " useBorderColor ? ";
        if (IsIntegerSamplerUnsigned(textureFunction.sampler))
        {
            out << "asuint";
        }
        out << "(samplerMetadata[samplerIndex].intBorderColor) : ";
    }

    // HLSL intrinsic
    if (outputType == SH_HLSL_3_0_OUTPUT)
    {
        switch (textureFunction.sampler)
        {
            case EbtSampler2D:
            case EbtSamplerVideoWEBGL:
            case EbtSamplerExternalOES:
                out << "tex2D";
                break;
            case EbtSamplerCube:
                out << "texCUBE";
                break;
            default:
                UNREACHABLE();
        }

        switch (textureFunction.method)
        {
            case TextureFunctionHLSL::TextureFunction::IMPLICIT:
                out << "(" << samplerReference << ", ";
                break;
            case TextureFunctionHLSL::TextureFunction::BIAS:
                out << "bias(" << samplerReference << ", ";
                break;
            case TextureFunctionHLSL::TextureFunction::LOD:
                out << "lod(" << samplerReference << ", ";
                break;
            case TextureFunctionHLSL::TextureFunction::LOD0:
                out << "lod(" << samplerReference << ", ";
                break;
            case TextureFunctionHLSL::TextureFunction::LOD0BIAS:
                out << "lod(" << samplerReference << ", ";
                break;
            case TextureFunctionHLSL::TextureFunction::GRAD:
                out << "grad(" << samplerReference << ", ";
                break;
            default:
                UNREACHABLE();
        }
    }
    else if (outputType == SH_HLSL_4_1_OUTPUT)
    {
        OutputHLSL4SampleFunctionPrefix(out, textureFunction, textureReference, samplerReference);
    }
    else
        UNREACHABLE();

    const int hlslCoords = GetHLSLCoordCount(textureFunction, outputType);
    out << GetSamplerCoordinateTypeString(textureFunction, hlslCoords);

    if (hlslCoords >= 2)
    {
        out << "(" << texCoordX << ", " << texCoordY;
    }
    else if (hlslCoords == 1)
    {
        std::string varName(texCoordX.data());
        if (size_t pos = varName.find_last_of('.') != std::string::npos)
        {
            varName = varName.substr(0, pos);
        }
        out << "(" << varName;
    }
    else
    {
        out << "(";
    }

    if (outputType == SH_HLSL_3_0_OUTPUT)
    {
        if (hlslCoords >= 3)
        {
            if (textureFunction.coords < 3)
            {
                out << ", 0";
            }
            else
            {
                out << ", " << texCoordZ;
            }
        }

        if (hlslCoords == 4)
        {
            switch (textureFunction.method)
            {
                case TextureFunctionHLSL::TextureFunction::BIAS:
                    out << ", bias";
                    break;
                case TextureFunctionHLSL::TextureFunction::LOD:
                    out << ", lod";
                    break;
                case TextureFunctionHLSL::TextureFunction::LOD0:
                    out << ", 0";
                    break;
                case TextureFunctionHLSL::TextureFunction::LOD0BIAS:
                    out << ", bias";
                    break;
                default:
                    UNREACHABLE();
            }
        }

        out << ")";
    }
    else if (outputType == SH_HLSL_4_1_OUTPUT)
    {
        if (hlslCoords >= 3)
        {
            ASSERT(!IsIntegerSampler(textureFunction.sampler) ||
                   !IsSamplerCube(textureFunction.sampler) || texCoordZ == "face");
            out << ", " << texCoordZ;
        }

        if (textureFunction.method == TextureFunctionHLSL::TextureFunction::GRAD)
        {
            if (IsIntegerSampler(textureFunction.sampler))
            {
                out << ", mip)";
            }
            else if (IsShadowSampler(textureFunction.sampler))
            {
                // Compare value
                if (textureFunction.proj)
                {
                    // According to ESSL 3.00.4 sec 8.8 p95 on textureProj:
                    // The resulting third component of P' in the shadow forms is used as
                    // Dref
                    out << "), " << texCoordZ;
                }
                else
                {
                    switch (textureFunction.coords)
                    {
                        case 3:
                            out << "), t.z";
                            break;
                        case 4:
                            out << "), t.w";
                            break;
                        default:
                            UNREACHABLE();
                    }
                }
            }
            else
            {
                out << "), ddx, ddy";
            }
        }
        else if (IsIntegerSampler(textureFunction.sampler) ||
                 textureFunction.method == TextureFunctionHLSL::TextureFunction::FETCH)
        {
            if (IsSampler2DMS(textureFunction.sampler) ||
                IsSampler2DMSArray(textureFunction.sampler))
                out << "), index";
            else if (IsSamplerBuffer(textureFunction.sampler))
                out << ")";
            else
                out << ", mip)";
        }
        else if (IsShadowSampler(textureFunction.sampler))
        {
            // Compare value
            if (textureFunction.proj)
            {
                // According to ESSL 3.00.4 sec 8.8 p95 on textureProj:
                // The resulting third component of P' in the shadow forms is used as Dref
                out << "), " << texCoordZ;
            }
            else
            {
                switch (textureFunction.coords)
                {
                    case 3:
                        out << "), t.z";
                        break;
                    case 4:
                        out << "), t.w";
                        break;
                    default:
                        UNREACHABLE();
                }
            }
        }
        else
        {
            switch (textureFunction.method)
            {
                case TextureFunctionHLSL::TextureFunction::IMPLICIT:
                    out << ")";
                    break;
                case TextureFunctionHLSL::TextureFunction::BIAS:
                    out << "), bias";
                    break;
                case TextureFunctionHLSL::TextureFunction::LOD:
                    out << "), lod";
                    break;
                case TextureFunctionHLSL::TextureFunction::LOD0:
                    out << "), 0";
                    break;
                case TextureFunctionHLSL::TextureFunction::LOD0BIAS:
                    out << "), bias";
                    break;
                default:
                    UNREACHABLE();
            }
        }

        if (textureFunction.offset &&
            (!IsIntegerSampler(textureFunction.sampler) ||
             textureFunction.method == TextureFunctionHLSL::TextureFunction::FETCH))
        {
            out << ", offset";
        }
    }
    else
        UNREACHABLE();

    out << ");\n";  // Close the sample function call and return statement
}

}  // Anonymous namespace

ImmutableString TextureFunctionHLSL::TextureFunction::name() const
{
    static const ImmutableString kGlTextureName("gl_texture");

    ImmutableString suffix(TextureTypeSuffix(this->sampler));

    ImmutableStringBuilder name(kGlTextureName.length() + suffix.length() + 4u + 6u + 5u);

    name << kGlTextureName;

    // We need to include full the sampler type in the function name to make the signature unique
    // on D3D11, where samplers are passed to texture functions as indices.
    name << suffix;

    if (proj)
    {
        name << "Proj";
    }

    if (offset)
    {
        name << "Offset";
    }

    switch (method)
    {
        case IMPLICIT:
            break;
        case BIAS:
            break;  // Extra parameter makes the signature unique
        case LOD:
            name << "Lod";
            break;
        case LOD0:
            name << "Lod0";
            break;
        case LOD0BIAS:
            name << "Lod0";
            break;  // Extra parameter makes the signature unique
        case SIZE:
            name << "Size";
            break;
        case FETCH:
            name << "Fetch";
            break;
        case GRAD:
            name << "Grad";
            break;
        case GATHER:
            name << "Gather";
            break;
        default:
            UNREACHABLE();
    }

    return name;
}

const char *TextureFunctionHLSL::TextureFunction::getReturnType() const
{
    if (method == TextureFunction::SIZE)
    {
        switch (sampler)
        {
            case EbtSampler2D:
            case EbtISampler2D:
            case EbtUSampler2D:
            case EbtSampler2DShadow:
            case EbtSamplerCube:
            case EbtISamplerCube:
            case EbtUSamplerCube:
            case EbtSamplerCubeShadow:
            case EbtSamplerExternalOES:
            case EbtSampler2DMS:
            case EbtISampler2DMS:
            case EbtUSampler2DMS:
            case EbtSamplerVideoWEBGL:
                return "int2";
            case EbtSampler3D:
            case EbtISampler3D:
            case EbtUSampler3D:
            case EbtSampler2DArray:
            case EbtISampler2DArray:
            case EbtUSampler2DArray:
            case EbtSampler2DMSArray:
            case EbtISampler2DMSArray:
            case EbtUSampler2DMSArray:
            case EbtSampler2DArrayShadow:
                return "int3";
            case EbtISamplerBuffer:
            case EbtUSamplerBuffer:
            case EbtSamplerBuffer:
                return "int";
            default:
                UNREACHABLE();
        }
    }
    else  // Sampling function
    {
        switch (sampler)
        {
            case EbtSampler2D:
            case EbtSampler2DMS:
            case EbtSampler2DMSArray:
            case EbtSampler3D:
            case EbtSamplerCube:
            case EbtSampler2DArray:
            case EbtSamplerExternalOES:
            case EbtSamplerVideoWEBGL:
            case EbtSamplerBuffer:
                return "float4";
            case EbtISampler2D:
            case EbtISampler2DMS:
            case EbtISampler2DMSArray:
            case EbtISampler3D:
            case EbtISamplerCube:
            case EbtISampler2DArray:
            case EbtISamplerBuffer:
                return "int4";
            case EbtUSampler2D:
            case EbtUSampler2DMS:
            case EbtUSampler2DMSArray:
            case EbtUSampler3D:
            case EbtUSamplerCube:
            case EbtUSampler2DArray:
            case EbtUSamplerBuffer:
                return "uint4";
            case EbtSampler2DShadow:
            case EbtSamplerCubeShadow:
            case EbtSampler2DArrayShadow:
                if (method == TextureFunctionHLSL::TextureFunction::GATHER)
                {
                    return "float4";
                }
                else
                {
                    return "float";
                }
            default:
                UNREACHABLE();
        }
    }
    return "";
}

bool TextureFunctionHLSL::TextureFunction::operator<(const TextureFunction &rhs) const
{
    return std::tie(sampler, coords, proj, offset, method) <
           std::tie(rhs.sampler, rhs.coords, rhs.proj, rhs.offset, rhs.method);
}

ImmutableString TextureFunctionHLSL::useTextureFunction(const ImmutableString &name,
                                                        TBasicType samplerType,
                                                        int coords,
                                                        size_t argumentCount,
                                                        bool lod0,
                                                        sh::GLenum shaderType)
{
    TextureFunction textureFunction;
    textureFunction.sampler = samplerType;
    textureFunction.coords  = coords;
    textureFunction.method  = TextureFunction::IMPLICIT;
    textureFunction.proj    = false;
    textureFunction.offset  = false;

    if (name == "texture2D" || name == "textureCube" || name == "texture")
    {
        textureFunction.method = TextureFunction::IMPLICIT;
    }
    else if (name == "texture2DProj" || name == "textureProj")
    {
        textureFunction.method = TextureFunction::IMPLICIT;
        textureFunction.proj   = true;
    }
    else if (name == "texture2DLod" || name == "textureCubeLod" || name == "textureLod" ||
             name == "texture2DLodEXT" || name == "textureCubeLodEXT")
    {
        textureFunction.method = TextureFunction::LOD;
    }
    else if (name == "texture2DProjLod" || name == "textureProjLod" ||
             name == "texture2DProjLodEXT")
    {
        textureFunction.method = TextureFunction::LOD;
        textureFunction.proj   = true;
    }
    else if (name == "textureSize")
    {
        textureFunction.method = TextureFunction::SIZE;
    }
    else if (name == "textureOffset")
    {
        textureFunction.method = TextureFunction::IMPLICIT;
        textureFunction.offset = true;
    }
    else if (name == "textureProjOffset")
    {
        textureFunction.method = TextureFunction::IMPLICIT;
        textureFunction.offset = true;
        textureFunction.proj   = true;
    }
    else if (name == "textureLodOffset")
    {
        textureFunction.method = TextureFunction::LOD;
        textureFunction.offset = true;
    }
    else if (name == "textureProjLodOffset")
    {
        textureFunction.method = TextureFunction::LOD;
        textureFunction.proj   = true;
        textureFunction.offset = true;
    }
    else if (name == "texelFetch")
    {
        textureFunction.method = TextureFunction::FETCH;
    }
    else if (name == "texelFetchOffset")
    {
        textureFunction.method = TextureFunction::FETCH;
        textureFunction.offset = true;
    }
    else if (name == "textureGrad" || name == "texture2DGradEXT")
    {
        textureFunction.method = TextureFunction::GRAD;
    }
    else if (name == "textureGradOffset")
    {
        textureFunction.method = TextureFunction::GRAD;
        textureFunction.offset = true;
    }
    else if (name == "textureProjGrad" || name == "texture2DProjGradEXT" ||
             name == "textureCubeGradEXT")
    {
        textureFunction.method = TextureFunction::GRAD;
        textureFunction.proj   = true;
    }
    else if (name == "textureProjGradOffset")
    {
        textureFunction.method = TextureFunction::GRAD;
        textureFunction.proj   = true;
        textureFunction.offset = true;
    }
    else if (name == "textureGather")
    {
        textureFunction.method = TextureFunction::GATHER;
    }
    else if (name == "textureGatherOffset")
    {
        textureFunction.method = TextureFunction::GATHER;
        textureFunction.offset = true;
    }
    else if (name == "textureVideoWEBGL")
    {
        textureFunction.method = TextureFunction::IMPLICIT;
    }
    else
        UNREACHABLE();

    if (textureFunction.method ==
        TextureFunction::IMPLICIT)  // Could require lod 0 or have a bias argument
    {
        size_t mandatoryArgumentCount = 2;  // All functions have sampler and coordinate arguments

        if (textureFunction.offset)
        {
            mandatoryArgumentCount++;
        }

        bool bias = (argumentCount > mandatoryArgumentCount);  // Bias argument is optional

        if (lod0 || shaderType == GL_VERTEX_SHADER || shaderType == GL_COMPUTE_SHADER)
        {
            if (bias)
            {
                textureFunction.method = TextureFunction::LOD0BIAS;
            }
            else
            {
                textureFunction.method = TextureFunction::LOD0;
            }
        }
        else if (bias)
        {
            textureFunction.method = TextureFunction::BIAS;
        }
    }

    mUsesTexture.insert(textureFunction);
    return textureFunction.name();
}

void TextureFunctionHLSL::textureFunctionHeader(TInfoSinkBase &out,
                                                const ShShaderOutput outputType,
                                                bool getDimensionsIgnoresBaseLevel)
{
    for (const TextureFunction &textureFunction : mUsesTexture)
    {
        // Function header
        out << textureFunction.getReturnType() << " " << textureFunction.name() << "(";

        OutputTextureFunctionArgumentList(out, textureFunction, outputType);

        out << ")\n"
               "{\n";

        // In some cases we use a variable to store the texture/sampler objects, but to work around
        // a D3D11 compiler bug related to discard inside a loop that is conditional on texture
        // sampling we need to call the function directly on references to the texture and sampler
        // arrays. The bug was found using dEQP-GLES3.functional.shaders.discard*loop_texture*
        // tests.
        ImmutableString textureReference("");
        ImmutableString samplerReference("");
        GetTextureReference(out, textureFunction, outputType, &textureReference, &samplerReference);

        if (textureFunction.method == TextureFunction::SIZE)
        {
            OutputTextureSizeFunctionBody(out, textureFunction, textureReference,
                                          getDimensionsIgnoresBaseLevel);
        }
        else
        {
            ImmutableString texCoordX("t.x");
            ImmutableString texCoordY("t.y");
            ImmutableString texCoordZ("t.z");
            if (textureFunction.method == TextureFunction::GATHER)
            {
                OutputTextureGatherFunctionBody(out, textureFunction, outputType, textureReference,
                                                samplerReference, texCoordX, texCoordY, texCoordZ);
            }
            else
            {
                ProjectTextureCoordinates(textureFunction, &texCoordX, &texCoordY, &texCoordZ);
                OutputIntegerTextureSampleFunctionComputations(
                    out, textureFunction, outputType, textureReference, &texCoordX, &texCoordY,
                    &texCoordZ, getDimensionsIgnoresBaseLevel);
                OutputTextureSampleFunctionReturnStatement(out, textureFunction, outputType,
                                                           textureReference, samplerReference,
                                                           texCoordX, texCoordY, texCoordZ);
            }
        }

        out << "}\n"
               "\n";
    }
}

}  // namespace sh
