Texture2D<float4> TextureF  : register(t0);
Texture2DMS<float4> TextureF_MS: register(t0);
Texture2D<uint4>  TextureUI : register(t0);
Texture2D<int4>   TextureI  : register(t0);

SamplerState Sampler        : register(s0);

void VS_Passthrough2D( in float2  inPosition :    POSITION,  in float2  inTexCoord : TEXCOORD0,
                    out float4 outPosition : SV_POSITION, out float2 outTexCoord : TEXCOORD0)
{
    outPosition = float4(inPosition, 0.0f, 1.0f);
    outTexCoord = inTexCoord;
}

float PS_PassthroughDepth2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_DEPTH
{
    return TextureF.Sample(Sampler, inTexCoord).r;
}

float4 PS_PassthroughRGBA2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return TextureF.Sample(Sampler, inTexCoord).rgba;
}

float4 PS_PassthroughA2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return float4(0.0f, 0.0f, 0.0f, TextureF.Sample(Sampler, inTexCoord).a);
}

float4 PS_PassthroughRGBA2DMS(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0, in uint inSampleIndex : SV_SAMPLEINDEX) : SV_TARGET0
{
    return TextureF_MS.sample[inSampleIndex][inTexCoord].rgba;
}

uint4 PS_PassthroughRGBA2DUI(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint2 size;
    TextureUI.GetDimensions(size.x, size.y);

    return TextureUI.Load(int3(size * inTexCoord, 0)).rgba;
}

int4 PS_PassthroughRGBA2DI(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint2 size;
    TextureI.GetDimensions(size.x, size.y);

    return TextureI.Load(int3(size * inTexCoord, 0)).rgba;
}

float4 PS_PassthroughRGB2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return float4(TextureF.Sample(Sampler, inTexCoord).rgb, 1.0f);
}

uint4 PS_PassthroughRGB2DUI(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint2 size;
    TextureUI.GetDimensions(size.x, size.y);

    return uint4(TextureUI.Load(int3(size * inTexCoord, 0)).rgb, 0);
}

int4 PS_PassthroughRGB2DI(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint2 size;
    TextureI.GetDimensions(size.x, size.y);

    return int4(TextureI.Load(int3(size * inTexCoord, 0)).rgb, 0);
}

float4 PS_PassthroughRG2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return float4(TextureF.Sample(Sampler, inTexCoord).rg, 0.0f, 1.0f);
}

uint4 PS_PassthroughRG2DUI(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint2 size;
    TextureUI.GetDimensions(size.x, size.y);

    return uint4(TextureUI.Load(int3(size * inTexCoord, 0)).rg, 0, 0);
}

int4 PS_PassthroughRG2DI(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint2 size;
    TextureI.GetDimensions(size.x, size.y);

    return int4(TextureI.Load(int3(size * inTexCoord, 0)).rg, 0, 0);
}

float4 PS_PassthroughR2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return float4(TextureF.Sample(Sampler, inTexCoord).r, 0.0f, 0.0f, 1.0f);
}

uint4 PS_PassthroughR2DUI(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint2 size;
    TextureUI.GetDimensions(size.x, size.y);

    return uint4(TextureUI.Load(int3(size * inTexCoord, 0)).r, 0, 0, 0);
}

int4 PS_PassthroughR2DI(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    uint2 size;
    TextureI.GetDimensions(size.x, size.y);

    return int4(TextureI.Load(int3(size * inTexCoord, 0)).r, 0, 0, 0);
}

float4 PS_PassthroughLum2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return float4(TextureF.Sample(Sampler, inTexCoord).rrr, 1.0f);
}

float4 PS_PassthroughLumAlpha2D(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return TextureF.Sample(Sampler, inTexCoord).rrra;
}

float4 PS_PassthroughRGBA2D_4444(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return round(TextureF.Sample(Sampler, inTexCoord) * float4(15, 15, 15, 15)) / float4(15, 15, 15, 15);
}

float4 PS_PassthroughRGB2D_565(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return float4(round(TextureF.Sample(Sampler, inTexCoord).rgb * float3(31, 63, 31)) / float3(31, 63, 31), 1.0f);
}

float4 PS_PassthroughRGBA2D_5551(in float4 inPosition : SV_POSITION, in float2 inTexCoord : TEXCOORD0) : SV_TARGET0
{
    return round(TextureF.Sample(Sampler, inTexCoord) * float4(31, 31, 31, 1)) / float4(31, 31, 31, 1);
}
