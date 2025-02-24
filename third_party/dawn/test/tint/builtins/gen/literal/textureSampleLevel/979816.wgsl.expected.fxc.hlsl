builtins/gen/literal/textureSampleLevel/979816.wgsl:28:24 warning: use of deprecated builtin
  var res: vec4<f32> = textureSampleLevel(arg_0, arg_1, vec2<f32>(1.f));
                       ^^^^^^^^^^^^^^^^^^

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
  uint flipY;
  float2x2 rotationMatrix;
};

Texture2D<float4> ext_tex_plane_1 : register(t2, space1);
cbuffer cbuffer_ext_tex_params : register(b3, space1) {
  uint4 ext_tex_params[13];
};
Texture2D<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

float3 gammaCorrection(float3 v, GammaTransferParams params) {
  const bool3 cond = (abs(v) < float3((params.D).xxx));
  const float3 t = (sign(v) * ((params.C * abs(v)) + params.F));
  const float3 f = (sign(v) * (pow(((params.A * abs(v)) + params.B), float3((params.G).xxx)) + params.E));
  return (cond ? t : f);
}

float4 textureSampleExternal(Texture2D<float4> plane0, Texture2D<float4> plane1, SamplerState smp, float2 coord, ExternalTextureParams params) {
  float3 color = float3(0.0f, 0.0f, 0.0f);
  if ((params.numPlanes == 1u)) {
    color = plane0.SampleLevel(smp, coord, 0.0f).rgb;
  } else {
    color = mul(params.yuvToRgbConversionMatrix, float4(plane0.SampleLevel(smp, coord, 0.0f).r, plane1.SampleLevel(smp, coord, 0.0f).rg, 1.0f));
  }
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    color = gammaCorrection(color, params.gammaDecodeParams);
    color = mul(color, params.gamutConversionMatrix);
    color = gammaCorrection(color, params.gammaEncodeParams);
  }
  return float4(color, 1.0f);
}

float3x4 tint_symbol_3(uint4 buffer[13], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]));
}

GammaTransferParams tint_symbol_5(uint4 buffer[13], uint offset) {
  const uint scalar_offset_3 = ((offset + 0u)) / 4;
  const uint scalar_offset_4 = ((offset + 4u)) / 4;
  const uint scalar_offset_5 = ((offset + 8u)) / 4;
  const uint scalar_offset_6 = ((offset + 12u)) / 4;
  const uint scalar_offset_7 = ((offset + 16u)) / 4;
  const uint scalar_offset_8 = ((offset + 20u)) / 4;
  const uint scalar_offset_9 = ((offset + 24u)) / 4;
  const uint scalar_offset_10 = ((offset + 28u)) / 4;
  const GammaTransferParams tint_symbol_11 = {asfloat(buffer[scalar_offset_3 / 4][scalar_offset_3 % 4]), asfloat(buffer[scalar_offset_4 / 4][scalar_offset_4 % 4]), asfloat(buffer[scalar_offset_5 / 4][scalar_offset_5 % 4]), asfloat(buffer[scalar_offset_6 / 4][scalar_offset_6 % 4]), asfloat(buffer[scalar_offset_7 / 4][scalar_offset_7 % 4]), asfloat(buffer[scalar_offset_8 / 4][scalar_offset_8 % 4]), asfloat(buffer[scalar_offset_9 / 4][scalar_offset_9 % 4]), buffer[scalar_offset_10 / 4][scalar_offset_10 % 4]};
  return tint_symbol_11;
}

float3x3 tint_symbol_7(uint4 buffer[13], uint offset) {
  const uint scalar_offset_11 = ((offset + 0u)) / 4;
  const uint scalar_offset_12 = ((offset + 16u)) / 4;
  const uint scalar_offset_13 = ((offset + 32u)) / 4;
  return float3x3(asfloat(buffer[scalar_offset_11 / 4].xyz), asfloat(buffer[scalar_offset_12 / 4].xyz), asfloat(buffer[scalar_offset_13 / 4].xyz));
}

float2x2 tint_symbol_9(uint4 buffer[13], uint offset) {
  const uint scalar_offset_14 = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset_14 / 4];
  const uint scalar_offset_15 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset_15 / 4];
  return float2x2(asfloat(((scalar_offset_14 & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_15 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
}

ExternalTextureParams tint_symbol_1(uint4 buffer[13], uint offset) {
  const uint scalar_offset_16 = ((offset + 0u)) / 4;
  const uint scalar_offset_17 = ((offset + 4u)) / 4;
  const uint scalar_offset_18 = ((offset + 176u)) / 4;
  const ExternalTextureParams tint_symbol_12 = {buffer[scalar_offset_16 / 4][scalar_offset_16 % 4], buffer[scalar_offset_17 / 4][scalar_offset_17 % 4], tint_symbol_3(buffer, (offset + 16u)), tint_symbol_5(buffer, (offset + 64u)), tint_symbol_5(buffer, (offset + 96u)), tint_symbol_7(buffer, (offset + 128u)), buffer[scalar_offset_18 / 4][scalar_offset_18 % 4], tint_symbol_9(buffer, (offset + 184u))};
  return tint_symbol_12;
}

void textureSampleLevel_979816() {
  float4 res = textureSampleExternal(arg_0, ext_tex_plane_1, arg_1, (1.0f).xx, tint_symbol_1(ext_tex_params, 0u));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureSampleLevel_979816();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureSampleLevel_979816();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleLevel_979816();
  return;
}
