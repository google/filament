//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

Texture2D<float4> TextureF  : register(t0);
Texture2D<uint4>  TextureUI : register(t0);
Texture3D<float4> TextureF_3D : register(t0);
Texture2DArray<float4> TextureF_2DArray : register(t0);

SamplerState Sampler        : register(s0);

// Notation:
// PM: premultiply, UM: unmulitply, PT: passthrough
// F: float, U: uint

// Float to float LUMA
float4 PS_FtoF_PM_LUMA_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    color.rgb = color.r * color.a;
    color.a = 1.0f;
    return color;
}
float4 PS_FtoF_UM_LUMA_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb = color.r / color.a;
    }
    color.a = 1.0f;
    return color;
}

// Float to float LUMAALPHA
float4 PS_FtoF_PM_LUMAALPHA_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    color.rgb = color.r * color.a;
    return color;
}

float4 PS_FtoF_UM_LUMAALPHA_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb = color.r / color.a;
    }
    return color;
}

// Float to float RGBA
float4 PS_FtoF_PM_RGBA_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    return color;
}

float4 PS_FtoF_UM_RGBA_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    return color;
}

// Float to float RGB
float4 PS_FtoF_PM_RGB_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    color.a = 1.0f;
    return color;
}

float4 PS_FtoF_UM_RGB_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    color.a = 1.0f;
    return color;
}

float4 PS_FtoF_PM_RGBA_4444_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    color = round(color * float4(15, 15, 15, 15)) / float4(15, 15, 15, 15);
    return color;
}

float4 PS_FtoF_UM_RGBA_4444_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    color = round(color * float4(15, 15, 15, 15)) / float4(15, 15, 15, 15);
    return color;
}

float4 PS_FtoF_PM_RGB_565_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    color.rgb = round(color.rgb * float3(31, 63, 31)) / float3(31, 63, 31);
    color.a = 1.0f;
    return color;
}

float4 PS_FtoF_UM_RGB_565_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    color.rgb = round(color.rgb * float3(31, 63, 31)) / float3(31, 63, 31);
    color.a = 1.0f;
    return color;
}

float4 PS_FtoF_PM_RGBA_5551_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    color = round(color * float4(31, 31, 31, 1)) / float4(31, 31, 31, 1);
    return color;
}

float4 PS_FtoF_UM_RGBA_5551_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    color = round(color * float4(31, 31, 31, 1)) / float4(31, 31, 31, 1);
    return color;
}

// Float to uint RGBA
uint4 PS_FtoU_PT_RGBA_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    return uint4(color * 255);
}

uint4 PS_FtoU_PM_RGBA_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    return uint4(color * 255);
}

uint4 PS_FtoU_UM_RGBA_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    return uint4(color * 255);
}

// Float to uint RGB
uint4 PS_FtoU_PT_RGB_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    return uint4(color.rgb * 255, 1);
}

uint4 PS_FtoU_PM_RGB_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    return uint4(color.rgb * 255, 1);
}

uint4 PS_FtoU_UM_RGB_2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    return uint4(color.rgb * 255, 1);
}

// Texture3D
// Float to float LUMA
float4 PS_FtoF_PM_LUMA_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    color.rgb = color.r * color.a;
    color.a = 1.0f;
    return color;
}

float4 PS_FtoF_UM_LUMA_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb = color.r / color.a;
    }
    color.a = 1.0f;
    return color;
}

// Float to float LUMAALPHA
float4 PS_FtoF_PM_LUMAALPHA_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    color.rgb = color.r * color.a;
    return color;
}

float4 PS_FtoF_UM_LUMAALPHA_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb = color.r / color.a;
    }
    return color;
}

// Float to float RGBA
float4 PS_FtoF_PM_RGBA_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    return color;
}

float4 PS_FtoF_UM_RGBA_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    return color;
}

// Float to float RGB
float4 PS_FtoF_PM_RGB_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    color.a = 1.0f;
    return color;
}

float4 PS_FtoF_UM_RGB_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    color.a = 1.0f;
    return color;
}

float4 PS_FtoF_PM_RGBA_4444_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    color = round(color * float4(15, 15, 15, 15)) / float4(15, 15, 15, 15);
    return color;
}

float4 PS_FtoF_UM_RGBA_4444_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    color = round(color * float4(15, 15, 15, 15)) / float4(15, 15, 15, 15);
    return color;
}

float4 PS_FtoF_PM_RGB_565_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    color.rgb = round(color.rgb * float3(31, 63, 31)) / float3(31, 63, 31);
    color.a = 1.0f;
    return color;
}

float4 PS_FtoF_UM_RGB_565_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    color.rgb = round(color.rgb * float3(31, 63, 31)) / float3(31, 63, 31);
    color.a = 1.0f;
    return color;
}

float4 PS_FtoF_PM_RGBA_5551_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    color = round(color * float4(31, 31, 31, 1)) / float4(31, 31, 31, 1);
    return color;
}

float4 PS_FtoF_UM_RGBA_5551_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    color = round(color * float4(31, 31, 31, 1)) / float4(31, 31, 31, 1);
    return color;
}

// Float to uint RGBA
uint4 PS_FtoU_PT_RGBA_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    return uint4(color * 255);
}

uint4 PS_FtoU_PM_RGBA_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    return uint4(color * 255);
}

uint4 PS_FtoU_UM_RGBA_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    return uint4(color * 255);
}

// Float to uint RGB
uint4 PS_FtoU_PT_RGB_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    return uint4(color.rgb * 255, 1);
}

uint4 PS_FtoU_PM_RGB_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    return uint4(color.rgb * 255, 1);
}

uint4 PS_FtoU_UM_RGB_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    return uint4(color.rgb * 255, 1);
}

// Float to int RGBA
int4 PS_FtoI_PT_RGBA_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    color = round(color * float4(127, 127, 127, 127)) / float4(127, 127, 127, 127);
    return int4(color * 127);
}

int4 PS_FtoI_PM_RGBA_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    color = round(color * float4(127, 127, 127, 127)) / float4(127, 127, 127, 127);
    return int4(color * 127);
}

int4 PS_FtoI_UM_RGBA_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    color = round(color * float4(127, 127, 127, 127)) / float4(127, 127, 127, 127);
    return int4(color * 127);
}

// Float to int RGB
int4 PS_FtoI_PT_RGB_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    color = round(color * float4(127, 127, 127, 127)) / float4(127, 127, 127, 127);
    return int4(color.rgb * 127, 1);
}

int4 PS_FtoI_PM_RGB_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    color.rgb *= color.a;
    color = round(color * float4(127, 127, 127, 127)) / float4(127, 127, 127, 127);
    return int4(color.rgb * 127, 1);
}

int4 PS_FtoI_UM_RGB_3D(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD) : SV_TARGET0
{
    float4 color = TextureF_3D.Sample(Sampler, inTexCoord).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    color = round(color * float4(127, 127, 127, 127)) / float4(127, 127, 127, 127);
    return int4(color.rgb * 127, 1);
}

// Texture2DArray
// Float to float LUMA
float4 PS_FtoF_PM_LUMA_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    color.rgb = color.r * color.a;
    color.a = 1.0f;
    return color;
}

float4 PS_FtoF_UM_LUMA_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    if (color.a > 0.0f)
    {
        color.rgb = color.r / color.a;
    }
    color.a = 1.0f;
    return color;
}

// Float to float LUMAALPHA
float4 PS_FtoF_PM_LUMAALPHA_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    color.rgb = color.r * color.a;
    return color;
}

float4 PS_FtoF_UM_LUMAALPHA_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    if (color.a > 0.0f)
    {
        color.rgb = color.r / color.a;
    }
    return color;
}

// Float to float RGBA
float4 PS_FtoF_PM_RGBA_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    color.rgb *= color.a;
    return color;
}

float4 PS_FtoF_UM_RGBA_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    return color;
}

// Float to float RGB
float4 PS_FtoF_PM_RGB_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    color.rgb *= color.a;
    color.a = 1.0f;
    return color;
}

float4 PS_FtoF_UM_RGB_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    color.a = 1.0f;
    return color;
}

float4 PS_FtoF_PM_RGBA_4444_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    color.rgb *= color.a;
    color = round(color * float4(15, 15, 15, 15)) / float4(15, 15, 15, 15);
    return color;
}

float4 PS_FtoF_UM_RGBA_4444_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    color = round(color * float4(15, 15, 15, 15)) / float4(15, 15, 15, 15);
    return color;
}

float4 PS_FtoF_PM_RGB_565_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    color.rgb *= color.a;
    color.rgb = round(color.rgb * float3(31, 63, 31)) / float3(31, 63, 31);
    color.a = 1.0f;
    return color;
}

float4 PS_FtoF_UM_RGB_565_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    color.rgb = round(color.rgb * float3(31, 63, 31)) / float3(31, 63, 31);
    color.a = 1.0f;
    return color;
}

float4 PS_FtoF_PM_RGBA_5551_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    color.rgb *= color.a;
    color = round(color * float4(31, 31, 31, 1)) / float4(31, 31, 31, 1);
    return color;
}

float4 PS_FtoF_UM_RGBA_5551_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    color = round(color * float4(31, 31, 31, 1)) / float4(31, 31, 31, 1);
    return color;
}

// Float to uint RGBA
uint4 PS_FtoU_PT_RGBA_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    return uint4(color * 255);
}

uint4 PS_FtoU_PM_RGBA_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    color.rgb *= color.a;
    return uint4(color * 255);
}

uint4 PS_FtoU_UM_RGBA_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    return uint4(color * 255);
}

// Float to uint RGB
uint4 PS_FtoU_PT_RGB_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    return uint4(color.rgb * 255, 1);
}

uint4 PS_FtoU_PM_RGB_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    color.rgb *= color.a;
    return uint4(color.rgb * 255, 1);
}

uint4 PS_FtoU_UM_RGB_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    return uint4(color.rgb * 255, 1);
}

// Float to int RGBA
int4 PS_FtoI_PT_RGBA_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    color = round(color * float4(127, 127, 127, 127)) / float4(127, 127, 127, 127);
    return int4(color * 127);
}

int4 PS_FtoI_PM_RGBA_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    color.rgb *= color.a;
    color = round(color * float4(127, 127, 127, 127)) / float4(127, 127, 127, 127);
    return int4(color * 127);
}

int4 PS_FtoI_UM_RGBA_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    color = round(color * float4(127, 127, 127, 127)) / float4(127, 127, 127, 127);
    return int4(color * 127);
}

// Float to int RGB
int4 PS_FtoI_PT_RGB_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    color = round(color * float4(127, 127, 127, 127)) / float4(127, 127, 127, 127);
    return int4(color.rgb * 127, 1);
}

int4 PS_FtoI_PM_RGB_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    color.rgb *= color.a;
    color = round(color * float4(127, 127, 127, 127)) / float4(127, 127, 127, 127);
    return int4(color.rgb * 127, 1);
}

int4 PS_FtoI_UM_RGB_2DArray(in float4 inPosition : SV_POSITION, in uint inLayer : SV_RENDERTARGETARRAYINDEX, in float3 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    float4 color = TextureF_2DArray.Sample(Sampler, float3(inTexCoord.xy, inLayer)).rgba;
    if (color.a > 0.0f)
    {
        color.rgb /= color.a;
    }
    color = round(color * float4(127, 127, 127, 127)) / float4(127, 127, 127, 127);
    return int4(color.rgb * 127, 1);
}
