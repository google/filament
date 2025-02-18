uint2 tint_ftou(float2 v) {
  return ((v <= (4294967040.0f).xx) ? ((v < (0.0f).xx) ? (0u).xx : uint2(v)) : (4294967295u).xx);
}

struct GammaTransferParams {
  float G;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  uint padding;
};
struct ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  float3x4 yuvToRgbConversionMatrix;
  GammaTransferParams gammaDecodeParams;
  GammaTransferParams gammaEncodeParams;
  float3x3 gamutConversionMatrix;
  float3x2 sampleTransform;
  float3x2 loadTransform;
  float2 samplePlane0RectMin;
  float2 samplePlane0RectMax;
  float2 samplePlane1RectMin;
  float2 samplePlane1RectMax;
  uint2 apparentSize;
  float2 plane1CoordFactor;
};

Texture2D<float4> ext_tex_plane_1 : register(t1);
cbuffer cbuffer_ext_tex_params : register(b2) {
  uint4 ext_tex_params[17];
};
Texture2D<float4> t : register(t0);

float3 gammaCorrection(float3 v, GammaTransferParams params) {
  bool3 cond = (abs(v) < float3((params.D).xxx));
  float3 t = (float3(sign(v)) * ((params.C * abs(v)) + params.F));
  float3 f = (float3(sign(v)) * (pow(((params.A * abs(v)) + params.B), float3((params.G).xxx)) + params.E));
  return (cond ? t : f);
}

float4 textureLoadExternal(Texture2D<float4> plane0, Texture2D<float4> plane1, int2 coord, ExternalTextureParams params) {
  uint2 clampedCoords = min(uint2(coord), params.apparentSize);
  uint2 plane0_clamped = tint_ftou(round(mul(float3(float2(clampedCoords), 1.0f), params.loadTransform)));
  float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = plane0.Load(uint3(plane0_clamped, uint(0))).rgba;
  } else {
    uint2 plane1_clamped = tint_ftou((float2(plane0_clamped) * params.plane1CoordFactor));
    color = float4(mul(params.yuvToRgbConversionMatrix, float4(plane0.Load(uint3(plane0_clamped, uint(0))).r, plane1.Load(uint3(plane1_clamped, uint(0))).rg, 1.0f)), 1.0f);
  }
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    color = float4(gammaCorrection(color.rgb, params.gammaDecodeParams), color.a);
    color = float4(mul(color.rgb, params.gamutConversionMatrix), color.a);
    color = float4(gammaCorrection(color.rgb, params.gammaEncodeParams), color.a);
  }
  return color;
}

float3x4 ext_tex_params_load_2(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x4(asfloat(ext_tex_params[scalar_offset / 4]), asfloat(ext_tex_params[scalar_offset_1 / 4]), asfloat(ext_tex_params[scalar_offset_2 / 4]));
}

GammaTransferParams ext_tex_params_load_4(uint offset) {
  const uint scalar_offset_3 = ((offset + 0u)) / 4;
  const uint scalar_offset_4 = ((offset + 4u)) / 4;
  const uint scalar_offset_5 = ((offset + 8u)) / 4;
  const uint scalar_offset_6 = ((offset + 12u)) / 4;
  const uint scalar_offset_7 = ((offset + 16u)) / 4;
  const uint scalar_offset_8 = ((offset + 20u)) / 4;
  const uint scalar_offset_9 = ((offset + 24u)) / 4;
  const uint scalar_offset_10 = ((offset + 28u)) / 4;
  GammaTransferParams tint_symbol = {asfloat(ext_tex_params[scalar_offset_3 / 4][scalar_offset_3 % 4]), asfloat(ext_tex_params[scalar_offset_4 / 4][scalar_offset_4 % 4]), asfloat(ext_tex_params[scalar_offset_5 / 4][scalar_offset_5 % 4]), asfloat(ext_tex_params[scalar_offset_6 / 4][scalar_offset_6 % 4]), asfloat(ext_tex_params[scalar_offset_7 / 4][scalar_offset_7 % 4]), asfloat(ext_tex_params[scalar_offset_8 / 4][scalar_offset_8 % 4]), asfloat(ext_tex_params[scalar_offset_9 / 4][scalar_offset_9 % 4]), ext_tex_params[scalar_offset_10 / 4][scalar_offset_10 % 4]};
  return tint_symbol;
}

float3x3 ext_tex_params_load_6(uint offset) {
  const uint scalar_offset_11 = ((offset + 0u)) / 4;
  const uint scalar_offset_12 = ((offset + 16u)) / 4;
  const uint scalar_offset_13 = ((offset + 32u)) / 4;
  return float3x3(asfloat(ext_tex_params[scalar_offset_11 / 4].xyz), asfloat(ext_tex_params[scalar_offset_12 / 4].xyz), asfloat(ext_tex_params[scalar_offset_13 / 4].xyz));
}

float3x2 ext_tex_params_load_8(uint offset) {
  const uint scalar_offset_14 = ((offset + 0u)) / 4;
  uint4 ubo_load = ext_tex_params[scalar_offset_14 / 4];
  const uint scalar_offset_15 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = ext_tex_params[scalar_offset_15 / 4];
  const uint scalar_offset_16 = ((offset + 16u)) / 4;
  uint4 ubo_load_2 = ext_tex_params[scalar_offset_16 / 4];
  return float3x2(asfloat(((scalar_offset_14 & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_15 & 2) ? ubo_load_1.zw : ubo_load_1.xy)), asfloat(((scalar_offset_16 & 2) ? ubo_load_2.zw : ubo_load_2.xy)));
}

ExternalTextureParams ext_tex_params_load(uint offset) {
  const uint scalar_offset_17 = ((offset + 0u)) / 4;
  const uint scalar_offset_18 = ((offset + 4u)) / 4;
  const uint scalar_offset_19 = ((offset + 224u)) / 4;
  uint4 ubo_load_3 = ext_tex_params[scalar_offset_19 / 4];
  const uint scalar_offset_20 = ((offset + 232u)) / 4;
  uint4 ubo_load_4 = ext_tex_params[scalar_offset_20 / 4];
  const uint scalar_offset_21 = ((offset + 240u)) / 4;
  uint4 ubo_load_5 = ext_tex_params[scalar_offset_21 / 4];
  const uint scalar_offset_22 = ((offset + 248u)) / 4;
  uint4 ubo_load_6 = ext_tex_params[scalar_offset_22 / 4];
  const uint scalar_offset_23 = ((offset + 256u)) / 4;
  uint4 ubo_load_7 = ext_tex_params[scalar_offset_23 / 4];
  const uint scalar_offset_24 = ((offset + 264u)) / 4;
  uint4 ubo_load_8 = ext_tex_params[scalar_offset_24 / 4];
  ExternalTextureParams tint_symbol_1 = {ext_tex_params[scalar_offset_17 / 4][scalar_offset_17 % 4], ext_tex_params[scalar_offset_18 / 4][scalar_offset_18 % 4], ext_tex_params_load_2((offset + 16u)), ext_tex_params_load_4((offset + 64u)), ext_tex_params_load_4((offset + 96u)), ext_tex_params_load_6((offset + 128u)), ext_tex_params_load_8((offset + 176u)), ext_tex_params_load_8((offset + 200u)), asfloat(((scalar_offset_19 & 2) ? ubo_load_3.zw : ubo_load_3.xy)), asfloat(((scalar_offset_20 & 2) ? ubo_load_4.zw : ubo_load_4.xy)), asfloat(((scalar_offset_21 & 2) ? ubo_load_5.zw : ubo_load_5.xy)), asfloat(((scalar_offset_22 & 2) ? ubo_load_6.zw : ubo_load_6.xy)), ((scalar_offset_23 & 2) ? ubo_load_7.zw : ubo_load_7.xy), asfloat(((scalar_offset_24 & 2) ? ubo_load_8.zw : ubo_load_8.xy))};
  return tint_symbol_1;
}

[numthreads(1, 1, 1)]
void i() {
  float4 r = textureLoadExternal(t, ext_tex_plane_1, (0).xx, ext_tex_params_load(0u));
  return;
}
