struct tint_GammaTransferParams {
  float G;
  float A;
  float B;
  float C;
  float D;
  float E;
  float F;
  uint padding;
};

struct tint_ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  float3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
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


Texture2D<float4> t_plane0 : register(t0);
Texture2D<float4> t_plane1 : register(t1);
cbuffer cbuffer_t_params : register(b2) {
  uint4 t_params[17];
};
uint2 tint_v2f32_to_v2u32(float2 value) {
  return (((value <= (4294967040.0f).xx)) ? ((((value >= (0.0f).xx)) ? (uint2(value)) : ((0u).xx))) : ((4294967295u).xx));
}

float3 tint_GammaCorrection(float3 v, tint_GammaTransferParams params) {
  float3 v_1 = float3((params.G).xxx);
  float3 v_2 = float3((params.D).xxx);
  float3 v_3 = float3(sign(v));
  return (((abs(v) < v_2)) ? ((v_3 * ((params.C * abs(v)) + params.F))) : ((v_3 * (pow(((params.A * abs(v)) + params.B), v_1) + params.E))));
}

float4 tint_TextureLoadExternal(Texture2D<float4> plane_0, Texture2D<float4> plane_1, tint_ExternalTextureParams params, uint2 coords) {
  float2 v_4 = round(mul(float3(float2(min(coords, params.apparentSize)), 1.0f), params.loadTransform));
  uint2 v_5 = tint_v2f32_to_v2u32(v_4);
  float3 v_6 = (0.0f).xxx;
  float v_7 = 0.0f;
  if ((params.numPlanes == 1u)) {
    int2 v_8 = int2(v_5);
    float4 v_9 = plane_0.Load(int3(v_8, int(0u)));
    v_6 = v_9.xyz;
    v_7 = v_9.w;
  } else {
    int2 v_10 = int2(v_5);
    float v_11 = plane_0.Load(int3(v_10, int(0u))).x;
    int2 v_12 = int2(tint_v2f32_to_v2u32((v_4 * params.plane1CoordFactor)));
    v_6 = mul(params.yuvToRgbConversionMatrix, float4(v_11, plane_1.Load(int3(v_12, int(0u))).xy, 1.0f));
    v_7 = 1.0f;
  }
  float3 v_13 = v_6;
  float3 v_14 = (0.0f).xxx;
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    tint_GammaTransferParams v_15 = params.gammaDecodeParams;
    tint_GammaTransferParams v_16 = params.gammaEncodeParams;
    v_14 = tint_GammaCorrection(mul(tint_GammaCorrection(v_13, v_15), params.gamutConversionMatrix), v_16);
  } else {
    v_14 = v_13;
  }
  return float4(v_14, v_7);
}

float3x2 v_17(uint start_byte_offset) {
  uint4 v_18 = t_params[(start_byte_offset / 16u)];
  float2 v_19 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_18.zw) : (v_18.xy)));
  uint4 v_20 = t_params[((8u + start_byte_offset) / 16u)];
  float2 v_21 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_20.zw) : (v_20.xy)));
  uint4 v_22 = t_params[((16u + start_byte_offset) / 16u)];
  return float3x2(v_19, v_21, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_22.zw) : (v_22.xy))));
}

float3x3 v_23(uint start_byte_offset) {
  return float3x3(asfloat(t_params[(start_byte_offset / 16u)].xyz), asfloat(t_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(t_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_GammaTransferParams v_24(uint start_byte_offset) {
  tint_GammaTransferParams v_25 = {asfloat(t_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]), asfloat(t_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]), asfloat(t_params[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]), asfloat(t_params[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]), asfloat(t_params[((16u + start_byte_offset) / 16u)][(((16u + start_byte_offset) % 16u) / 4u)]), asfloat(t_params[((20u + start_byte_offset) / 16u)][(((20u + start_byte_offset) % 16u) / 4u)]), asfloat(t_params[((24u + start_byte_offset) / 16u)][(((24u + start_byte_offset) % 16u) / 4u)]), t_params[((28u + start_byte_offset) / 16u)][(((28u + start_byte_offset) % 16u) / 4u)]};
  return v_25;
}

float3x4 v_26(uint start_byte_offset) {
  return float3x4(asfloat(t_params[(start_byte_offset / 16u)]), asfloat(t_params[((16u + start_byte_offset) / 16u)]), asfloat(t_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_27(uint start_byte_offset) {
  uint v_28 = t_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)];
  uint v_29 = t_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)];
  float3x4 v_30 = v_26((16u + start_byte_offset));
  tint_GammaTransferParams v_31 = v_24((64u + start_byte_offset));
  tint_GammaTransferParams v_32 = v_24((96u + start_byte_offset));
  float3x3 v_33 = v_23((128u + start_byte_offset));
  float3x2 v_34 = v_17((176u + start_byte_offset));
  float3x2 v_35 = v_17((200u + start_byte_offset));
  uint4 v_36 = t_params[((224u + start_byte_offset) / 16u)];
  float2 v_37 = asfloat(((((((224u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_36.zw) : (v_36.xy)));
  uint4 v_38 = t_params[((232u + start_byte_offset) / 16u)];
  float2 v_39 = asfloat(((((((232u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_38.zw) : (v_38.xy)));
  uint4 v_40 = t_params[((240u + start_byte_offset) / 16u)];
  float2 v_41 = asfloat(((((((240u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_40.zw) : (v_40.xy)));
  uint4 v_42 = t_params[((248u + start_byte_offset) / 16u)];
  float2 v_43 = asfloat(((((((248u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_42.zw) : (v_42.xy)));
  uint4 v_44 = t_params[((256u + start_byte_offset) / 16u)];
  uint2 v_45 = ((((((256u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_44.zw) : (v_44.xy));
  uint4 v_46 = t_params[((264u + start_byte_offset) / 16u)];
  tint_ExternalTextureParams v_47 = {v_28, v_29, v_30, v_31, v_32, v_33, v_34, v_35, v_37, v_39, v_41, v_43, v_45, asfloat(((((((264u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_46.zw) : (v_46.xy)))};
  return v_47;
}

[numthreads(1, 1, 1)]
void i() {
  tint_ExternalTextureParams v_48 = v_27(0u);
  float4 r = tint_TextureLoadExternal(t_plane0, t_plane1, v_48, uint2((int(0)).xx));
}

