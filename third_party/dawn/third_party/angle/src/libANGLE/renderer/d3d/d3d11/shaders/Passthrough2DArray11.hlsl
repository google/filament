//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

Texture2DArray<float4> TextureF  : register(t0);
Texture2DArray<uint4>  TextureUI : register(t0);
Texture2DArray<int4>   TextureI  : register(t0);

SamplerState      Sampler   : register(s0);

float4 PS_PassthroughRGBA2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return TextureF.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
}

uint4 PS_PassthroughRGBA2DArrayUI(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint3 size;
    TextureUI.GetDimensions(size.x, size.y, size.z);

    int4 location = int4(size.xy * inTexCoord.xy, inLayer, 0);

    return TextureUI.Load(location).rgba;
}

int4 PS_PassthroughRGBA2DArrayI(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint3 size;
    TextureI.GetDimensions(size.x, size.y, size.z);

    int4 location = int4(size.xy * inTexCoord.xy, inLayer, 0);

    return TextureI.Load(location).rgba;
}

float4 PS_PassthroughRGB2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return float4(TextureF.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgb, 1.0f);
}

uint4 PS_PassthroughRGB2DArrayUI(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint3 size;
    TextureUI.GetDimensions(size.x, size.y, size.z);

    int4 location = int4(size.xy * inTexCoord.xy, inLayer, 0);

    return uint4(TextureUI.Load(location).rgb, 1.0f);
}

int4 PS_PassthroughRGB2DArrayI(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint3 size;
    TextureI.GetDimensions(size.x, size.y, size.z);

    int4 location = int4(size.xy * inTexCoord.xy, inLayer, 0);

    return int4(TextureI.Load(location).rgb, 1.0f);
}

float4 PS_PassthroughRG2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return float4(TextureF.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rg, 0.0f, 1.0f);
}

uint4 PS_PassthroughRG2DArrayUI(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint3 size;
    TextureUI.GetDimensions(size.x, size.y, size.z);

    int4 location = int4(size.xy * inTexCoord.xy, inLayer, 0);

    return uint4(TextureUI.Load(location).rg, 0.0f, 1.0f);
}

int4 PS_PassthroughRG2DArrayI(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint3 size;
    TextureI.GetDimensions(size.x, size.y, size.z);

    int4 location = int4(size.xy * inTexCoord.xy, inLayer, 0);

    return int4(TextureI.Load(location).rg, 0.0f, 1.0f);
}

float4 PS_PassthroughR2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return float4(TextureF.Sample(Sampler, float3(inTexCoord.xy, inLayer)).r, 0.0f, 0.0f, 1.0f);
}

uint4 PS_PassthroughR2DArrayUI(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint3 size;
    TextureUI.GetDimensions(size.x, size.y, size.z);

    int4 location = int4(size.xy * inTexCoord.xy, inLayer, 0);

    return uint4(TextureUI.Load(location).r, 0.0f, 0.0f, 1.0f);
}

int4 PS_PassthroughR2DArrayI(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint3 size;
    TextureI.GetDimensions(size.x, size.y, size.z);

    int4 location = int4(size.xy * inTexCoord.xy, inLayer, 0);

    return int4(TextureI.Load(location).r, 0.0f, 0.0f, 1.0f);
}

float4 PS_PassthroughLum2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return float4(TextureF.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rrr, 1.0f);
}

float4 PS_PassthroughLumAlpha2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return TextureF.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rrra;
}

float4 PS_PassthroughRGBA2DArray_4444(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return round(TextureF.Sample(Sampler, float3(inTexCoord.xy, inLayer)) * float4(15, 15, 15, 15)) / float4(15, 15, 15, 15);
}

float4 PS_PassthroughRGB2DArray_565(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return float4(round(TextureF.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgb * float3(31, 63, 31)) / float3(31, 63, 31), 1.0f);
}

float4 PS_PassthroughRGBA2DArray_5551(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return round(TextureF.Sample(Sampler, float3(inTexCoord.xy, inLayer)) * float4(31, 31, 31, 1)) / float4(31, 31, 31, 1);
}
