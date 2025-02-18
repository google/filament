//
// fragment_main
//
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


RWByteAddressBuffer prevent_dce : register(u0);
Texture2D<float4> arg_0_plane0 : register(t0, space1);
Texture2D<float4> arg_0_plane1 : register(t2, space1);
cbuffer cbuffer_arg_0_params : register(b3, space1) {
  uint4 arg_0_params[17];
};
SamplerState arg_1 : register(s1, space1);
float3 tint_GammaCorrection(float3 v, tint_GammaTransferParams params) {
  float3 v_1 = float3((params.G).xxx);
  float3 v_2 = float3((params.D).xxx);
  float3 v_3 = float3(sign(v));
  return (((abs(v) < v_2)) ? ((v_3 * ((params.C * abs(v)) + params.F))) : ((v_3 * (pow(((params.A * abs(v)) + params.B), v_1) + params.E))));
}

float4 tint_TextureSampleExternal(Texture2D<float4> plane_0, Texture2D<float4> plane_1, tint_ExternalTextureParams params, SamplerState tint_sampler, float2 coords) {
  float2 v_4 = mul(float3(coords, 1.0f), params.sampleTransform);
  float3 v_5 = (0.0f).xxx;
  float v_6 = 0.0f;
  if ((params.numPlanes == 1u)) {
    float4 v_7 = plane_0.SampleLevel(tint_sampler, clamp(v_4, params.samplePlane0RectMin, params.samplePlane0RectMax), 0.0f);
    v_5 = v_7.xyz;
    v_6 = v_7.w;
  } else {
    v_5 = mul(params.yuvToRgbConversionMatrix, float4(plane_0.SampleLevel(tint_sampler, clamp(v_4, params.samplePlane0RectMin, params.samplePlane0RectMax), 0.0f).x, plane_1.SampleLevel(tint_sampler, clamp(v_4, params.samplePlane1RectMin, params.samplePlane1RectMax), 0.0f).xy, 1.0f));
    v_6 = 1.0f;
  }
  float3 v_8 = v_5;
  float3 v_9 = (0.0f).xxx;
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    tint_GammaTransferParams v_10 = params.gammaDecodeParams;
    tint_GammaTransferParams v_11 = params.gammaEncodeParams;
    v_9 = tint_GammaCorrection(mul(tint_GammaCorrection(v_8, v_10), params.gamutConversionMatrix), v_11);
  } else {
    v_9 = v_8;
  }
  return float4(v_9, v_6);
}

float3x2 v_12(uint start_byte_offset) {
  uint4 v_13 = arg_0_params[(start_byte_offset / 16u)];
  float2 v_14 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_13.zw) : (v_13.xy)));
  uint4 v_15 = arg_0_params[((8u + start_byte_offset) / 16u)];
  float2 v_16 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_15.zw) : (v_15.xy)));
  uint4 v_17 = arg_0_params[((16u + start_byte_offset) / 16u)];
  return float3x2(v_14, v_16, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_17.zw) : (v_17.xy))));
}

float3x3 v_18(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_GammaTransferParams v_19(uint start_byte_offset) {
  tint_GammaTransferParams v_20 = {asfloat(arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]), asfloat(arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)][(((16u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((20u + start_byte_offset) / 16u)][(((20u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((24u + start_byte_offset) / 16u)][(((24u + start_byte_offset) % 16u) / 4u)]), arg_0_params[((28u + start_byte_offset) / 16u)][(((28u + start_byte_offset) % 16u) / 4u)]};
  return v_20;
}

float3x4 v_21(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_22(uint start_byte_offset) {
  uint v_23 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)];
  uint v_24 = arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)];
  float3x4 v_25 = v_21((16u + start_byte_offset));
  tint_GammaTransferParams v_26 = v_19((64u + start_byte_offset));
  tint_GammaTransferParams v_27 = v_19((96u + start_byte_offset));
  float3x3 v_28 = v_18((128u + start_byte_offset));
  float3x2 v_29 = v_12((176u + start_byte_offset));
  float3x2 v_30 = v_12((200u + start_byte_offset));
  uint4 v_31 = arg_0_params[((224u + start_byte_offset) / 16u)];
  float2 v_32 = asfloat(((((((224u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_31.zw) : (v_31.xy)));
  uint4 v_33 = arg_0_params[((232u + start_byte_offset) / 16u)];
  float2 v_34 = asfloat(((((((232u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_33.zw) : (v_33.xy)));
  uint4 v_35 = arg_0_params[((240u + start_byte_offset) / 16u)];
  float2 v_36 = asfloat(((((((240u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_35.zw) : (v_35.xy)));
  uint4 v_37 = arg_0_params[((248u + start_byte_offset) / 16u)];
  float2 v_38 = asfloat(((((((248u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_37.zw) : (v_37.xy)));
  uint4 v_39 = arg_0_params[((256u + start_byte_offset) / 16u)];
  uint2 v_40 = ((((((256u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_39.zw) : (v_39.xy));
  uint4 v_41 = arg_0_params[((264u + start_byte_offset) / 16u)];
  tint_ExternalTextureParams v_42 = {v_23, v_24, v_25, v_26, v_27, v_28, v_29, v_30, v_32, v_34, v_36, v_38, v_40, asfloat(((((((264u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_41.zw) : (v_41.xy)))};
  return v_42;
}

float4 textureSampleBaseClampToEdge_7c04e6() {
  tint_ExternalTextureParams v_43 = v_22(0u);
  float4 res = tint_TextureSampleExternal(arg_0_plane0, arg_0_plane1, v_43, arg_1, (1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureSampleBaseClampToEdge_7c04e6()));
}

//
// compute_main
//
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


RWByteAddressBuffer prevent_dce : register(u0);
Texture2D<float4> arg_0_plane0 : register(t0, space1);
Texture2D<float4> arg_0_plane1 : register(t2, space1);
cbuffer cbuffer_arg_0_params : register(b3, space1) {
  uint4 arg_0_params[17];
};
SamplerState arg_1 : register(s1, space1);
float3 tint_GammaCorrection(float3 v, tint_GammaTransferParams params) {
  float3 v_1 = float3((params.G).xxx);
  float3 v_2 = float3((params.D).xxx);
  float3 v_3 = float3(sign(v));
  return (((abs(v) < v_2)) ? ((v_3 * ((params.C * abs(v)) + params.F))) : ((v_3 * (pow(((params.A * abs(v)) + params.B), v_1) + params.E))));
}

float4 tint_TextureSampleExternal(Texture2D<float4> plane_0, Texture2D<float4> plane_1, tint_ExternalTextureParams params, SamplerState tint_sampler, float2 coords) {
  float2 v_4 = mul(float3(coords, 1.0f), params.sampleTransform);
  float3 v_5 = (0.0f).xxx;
  float v_6 = 0.0f;
  if ((params.numPlanes == 1u)) {
    float4 v_7 = plane_0.SampleLevel(tint_sampler, clamp(v_4, params.samplePlane0RectMin, params.samplePlane0RectMax), 0.0f);
    v_5 = v_7.xyz;
    v_6 = v_7.w;
  } else {
    v_5 = mul(params.yuvToRgbConversionMatrix, float4(plane_0.SampleLevel(tint_sampler, clamp(v_4, params.samplePlane0RectMin, params.samplePlane0RectMax), 0.0f).x, plane_1.SampleLevel(tint_sampler, clamp(v_4, params.samplePlane1RectMin, params.samplePlane1RectMax), 0.0f).xy, 1.0f));
    v_6 = 1.0f;
  }
  float3 v_8 = v_5;
  float3 v_9 = (0.0f).xxx;
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    tint_GammaTransferParams v_10 = params.gammaDecodeParams;
    tint_GammaTransferParams v_11 = params.gammaEncodeParams;
    v_9 = tint_GammaCorrection(mul(tint_GammaCorrection(v_8, v_10), params.gamutConversionMatrix), v_11);
  } else {
    v_9 = v_8;
  }
  return float4(v_9, v_6);
}

float3x2 v_12(uint start_byte_offset) {
  uint4 v_13 = arg_0_params[(start_byte_offset / 16u)];
  float2 v_14 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_13.zw) : (v_13.xy)));
  uint4 v_15 = arg_0_params[((8u + start_byte_offset) / 16u)];
  float2 v_16 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_15.zw) : (v_15.xy)));
  uint4 v_17 = arg_0_params[((16u + start_byte_offset) / 16u)];
  return float3x2(v_14, v_16, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_17.zw) : (v_17.xy))));
}

float3x3 v_18(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_GammaTransferParams v_19(uint start_byte_offset) {
  tint_GammaTransferParams v_20 = {asfloat(arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]), asfloat(arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)][(((16u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((20u + start_byte_offset) / 16u)][(((20u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((24u + start_byte_offset) / 16u)][(((24u + start_byte_offset) % 16u) / 4u)]), arg_0_params[((28u + start_byte_offset) / 16u)][(((28u + start_byte_offset) % 16u) / 4u)]};
  return v_20;
}

float3x4 v_21(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_22(uint start_byte_offset) {
  uint v_23 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)];
  uint v_24 = arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)];
  float3x4 v_25 = v_21((16u + start_byte_offset));
  tint_GammaTransferParams v_26 = v_19((64u + start_byte_offset));
  tint_GammaTransferParams v_27 = v_19((96u + start_byte_offset));
  float3x3 v_28 = v_18((128u + start_byte_offset));
  float3x2 v_29 = v_12((176u + start_byte_offset));
  float3x2 v_30 = v_12((200u + start_byte_offset));
  uint4 v_31 = arg_0_params[((224u + start_byte_offset) / 16u)];
  float2 v_32 = asfloat(((((((224u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_31.zw) : (v_31.xy)));
  uint4 v_33 = arg_0_params[((232u + start_byte_offset) / 16u)];
  float2 v_34 = asfloat(((((((232u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_33.zw) : (v_33.xy)));
  uint4 v_35 = arg_0_params[((240u + start_byte_offset) / 16u)];
  float2 v_36 = asfloat(((((((240u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_35.zw) : (v_35.xy)));
  uint4 v_37 = arg_0_params[((248u + start_byte_offset) / 16u)];
  float2 v_38 = asfloat(((((((248u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_37.zw) : (v_37.xy)));
  uint4 v_39 = arg_0_params[((256u + start_byte_offset) / 16u)];
  uint2 v_40 = ((((((256u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_39.zw) : (v_39.xy));
  uint4 v_41 = arg_0_params[((264u + start_byte_offset) / 16u)];
  tint_ExternalTextureParams v_42 = {v_23, v_24, v_25, v_26, v_27, v_28, v_29, v_30, v_32, v_34, v_36, v_38, v_40, asfloat(((((((264u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_41.zw) : (v_41.xy)))};
  return v_42;
}

float4 textureSampleBaseClampToEdge_7c04e6() {
  tint_ExternalTextureParams v_43 = v_22(0u);
  float4 res = tint_TextureSampleExternal(arg_0_plane0, arg_0_plane1, v_43, arg_1, (1.0f).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureSampleBaseClampToEdge_7c04e6()));
}

//
// vertex_main
//
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

struct VertexOutput {
  float4 pos;
  float4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


Texture2D<float4> arg_0_plane0 : register(t0, space1);
Texture2D<float4> arg_0_plane1 : register(t2, space1);
cbuffer cbuffer_arg_0_params : register(b3, space1) {
  uint4 arg_0_params[17];
};
SamplerState arg_1 : register(s1, space1);
float3 tint_GammaCorrection(float3 v, tint_GammaTransferParams params) {
  float3 v_1 = float3((params.G).xxx);
  float3 v_2 = float3((params.D).xxx);
  float3 v_3 = float3(sign(v));
  return (((abs(v) < v_2)) ? ((v_3 * ((params.C * abs(v)) + params.F))) : ((v_3 * (pow(((params.A * abs(v)) + params.B), v_1) + params.E))));
}

float4 tint_TextureSampleExternal(Texture2D<float4> plane_0, Texture2D<float4> plane_1, tint_ExternalTextureParams params, SamplerState tint_sampler, float2 coords) {
  float2 v_4 = mul(float3(coords, 1.0f), params.sampleTransform);
  float3 v_5 = (0.0f).xxx;
  float v_6 = 0.0f;
  if ((params.numPlanes == 1u)) {
    float4 v_7 = plane_0.SampleLevel(tint_sampler, clamp(v_4, params.samplePlane0RectMin, params.samplePlane0RectMax), 0.0f);
    v_5 = v_7.xyz;
    v_6 = v_7.w;
  } else {
    v_5 = mul(params.yuvToRgbConversionMatrix, float4(plane_0.SampleLevel(tint_sampler, clamp(v_4, params.samplePlane0RectMin, params.samplePlane0RectMax), 0.0f).x, plane_1.SampleLevel(tint_sampler, clamp(v_4, params.samplePlane1RectMin, params.samplePlane1RectMax), 0.0f).xy, 1.0f));
    v_6 = 1.0f;
  }
  float3 v_8 = v_5;
  float3 v_9 = (0.0f).xxx;
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    tint_GammaTransferParams v_10 = params.gammaDecodeParams;
    tint_GammaTransferParams v_11 = params.gammaEncodeParams;
    v_9 = tint_GammaCorrection(mul(tint_GammaCorrection(v_8, v_10), params.gamutConversionMatrix), v_11);
  } else {
    v_9 = v_8;
  }
  return float4(v_9, v_6);
}

float3x2 v_12(uint start_byte_offset) {
  uint4 v_13 = arg_0_params[(start_byte_offset / 16u)];
  float2 v_14 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_13.zw) : (v_13.xy)));
  uint4 v_15 = arg_0_params[((8u + start_byte_offset) / 16u)];
  float2 v_16 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_15.zw) : (v_15.xy)));
  uint4 v_17 = arg_0_params[((16u + start_byte_offset) / 16u)];
  return float3x2(v_14, v_16, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_17.zw) : (v_17.xy))));
}

float3x3 v_18(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_GammaTransferParams v_19(uint start_byte_offset) {
  tint_GammaTransferParams v_20 = {asfloat(arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]), asfloat(arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)][(((16u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((20u + start_byte_offset) / 16u)][(((20u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((24u + start_byte_offset) / 16u)][(((24u + start_byte_offset) % 16u) / 4u)]), arg_0_params[((28u + start_byte_offset) / 16u)][(((28u + start_byte_offset) % 16u) / 4u)]};
  return v_20;
}

float3x4 v_21(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_22(uint start_byte_offset) {
  uint v_23 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)];
  uint v_24 = arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)];
  float3x4 v_25 = v_21((16u + start_byte_offset));
  tint_GammaTransferParams v_26 = v_19((64u + start_byte_offset));
  tint_GammaTransferParams v_27 = v_19((96u + start_byte_offset));
  float3x3 v_28 = v_18((128u + start_byte_offset));
  float3x2 v_29 = v_12((176u + start_byte_offset));
  float3x2 v_30 = v_12((200u + start_byte_offset));
  uint4 v_31 = arg_0_params[((224u + start_byte_offset) / 16u)];
  float2 v_32 = asfloat(((((((224u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_31.zw) : (v_31.xy)));
  uint4 v_33 = arg_0_params[((232u + start_byte_offset) / 16u)];
  float2 v_34 = asfloat(((((((232u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_33.zw) : (v_33.xy)));
  uint4 v_35 = arg_0_params[((240u + start_byte_offset) / 16u)];
  float2 v_36 = asfloat(((((((240u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_35.zw) : (v_35.xy)));
  uint4 v_37 = arg_0_params[((248u + start_byte_offset) / 16u)];
  float2 v_38 = asfloat(((((((248u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_37.zw) : (v_37.xy)));
  uint4 v_39 = arg_0_params[((256u + start_byte_offset) / 16u)];
  uint2 v_40 = ((((((256u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_39.zw) : (v_39.xy));
  uint4 v_41 = arg_0_params[((264u + start_byte_offset) / 16u)];
  tint_ExternalTextureParams v_42 = {v_23, v_24, v_25, v_26, v_27, v_28, v_29, v_30, v_32, v_34, v_36, v_38, v_40, asfloat(((((((264u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_41.zw) : (v_41.xy)))};
  return v_42;
}

float4 textureSampleBaseClampToEdge_7c04e6() {
  tint_ExternalTextureParams v_43 = v_22(0u);
  float4 res = tint_TextureSampleExternal(arg_0_plane0, arg_0_plane1, v_43, arg_1, (1.0f).xx);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_44 = (VertexOutput)0;
  v_44.pos = (0.0f).xxxx;
  v_44.prevent_dce = textureSampleBaseClampToEdge_7c04e6();
  VertexOutput v_45 = v_44;
  return v_45;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_46 = vertex_main_inner();
  vertex_main_outputs v_47 = {v_46.prevent_dce, v_46.pos};
  return v_47;
}

