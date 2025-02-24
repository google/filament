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
Texture2D<float4> arg_0_plane1 : register(t1, space1);
cbuffer cbuffer_arg_0_params : register(b2, space1) {
  uint4 arg_0_params[17];
};
float3x2 v(uint start_byte_offset) {
  uint4 v_1 = arg_0_params[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint4 v_3 = arg_0_params[((8u + start_byte_offset) / 16u)];
  float2 v_4 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_3.zw) : (v_3.xy)));
  uint4 v_5 = arg_0_params[((16u + start_byte_offset) / 16u)];
  return float3x2(v_2, v_4, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy))));
}

float3x3 v_6(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_GammaTransferParams v_7(uint start_byte_offset) {
  tint_GammaTransferParams v_8 = {asfloat(arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]), asfloat(arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)][(((16u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((20u + start_byte_offset) / 16u)][(((20u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((24u + start_byte_offset) / 16u)][(((24u + start_byte_offset) % 16u) / 4u)]), arg_0_params[((28u + start_byte_offset) / 16u)][(((28u + start_byte_offset) % 16u) / 4u)]};
  return v_8;
}

float3x4 v_9(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_10(uint start_byte_offset) {
  uint v_11 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)];
  uint v_12 = arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)];
  float3x4 v_13 = v_9((16u + start_byte_offset));
  tint_GammaTransferParams v_14 = v_7((64u + start_byte_offset));
  tint_GammaTransferParams v_15 = v_7((96u + start_byte_offset));
  float3x3 v_16 = v_6((128u + start_byte_offset));
  float3x2 v_17 = v((176u + start_byte_offset));
  float3x2 v_18 = v((200u + start_byte_offset));
  uint4 v_19 = arg_0_params[((224u + start_byte_offset) / 16u)];
  float2 v_20 = asfloat(((((((224u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_19.zw) : (v_19.xy)));
  uint4 v_21 = arg_0_params[((232u + start_byte_offset) / 16u)];
  float2 v_22 = asfloat(((((((232u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_21.zw) : (v_21.xy)));
  uint4 v_23 = arg_0_params[((240u + start_byte_offset) / 16u)];
  float2 v_24 = asfloat(((((((240u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_23.zw) : (v_23.xy)));
  uint4 v_25 = arg_0_params[((248u + start_byte_offset) / 16u)];
  float2 v_26 = asfloat(((((((248u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_25.zw) : (v_25.xy)));
  uint4 v_27 = arg_0_params[((256u + start_byte_offset) / 16u)];
  uint2 v_28 = ((((((256u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_27.zw) : (v_27.xy));
  uint4 v_29 = arg_0_params[((264u + start_byte_offset) / 16u)];
  tint_ExternalTextureParams v_30 = {v_11, v_12, v_13, v_14, v_15, v_16, v_17, v_18, v_20, v_22, v_24, v_26, v_28, asfloat(((((((264u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_29.zw) : (v_29.xy)))};
  return v_30;
}

uint2 textureDimensions_cdc6c9() {
  tint_ExternalTextureParams v_31 = v_10(0u);
  uint2 res = (v_31.apparentSize + (1u).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, textureDimensions_cdc6c9());
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
Texture2D<float4> arg_0_plane1 : register(t1, space1);
cbuffer cbuffer_arg_0_params : register(b2, space1) {
  uint4 arg_0_params[17];
};
float3x2 v(uint start_byte_offset) {
  uint4 v_1 = arg_0_params[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint4 v_3 = arg_0_params[((8u + start_byte_offset) / 16u)];
  float2 v_4 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_3.zw) : (v_3.xy)));
  uint4 v_5 = arg_0_params[((16u + start_byte_offset) / 16u)];
  return float3x2(v_2, v_4, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy))));
}

float3x3 v_6(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_GammaTransferParams v_7(uint start_byte_offset) {
  tint_GammaTransferParams v_8 = {asfloat(arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]), asfloat(arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)][(((16u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((20u + start_byte_offset) / 16u)][(((20u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((24u + start_byte_offset) / 16u)][(((24u + start_byte_offset) % 16u) / 4u)]), arg_0_params[((28u + start_byte_offset) / 16u)][(((28u + start_byte_offset) % 16u) / 4u)]};
  return v_8;
}

float3x4 v_9(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_10(uint start_byte_offset) {
  uint v_11 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)];
  uint v_12 = arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)];
  float3x4 v_13 = v_9((16u + start_byte_offset));
  tint_GammaTransferParams v_14 = v_7((64u + start_byte_offset));
  tint_GammaTransferParams v_15 = v_7((96u + start_byte_offset));
  float3x3 v_16 = v_6((128u + start_byte_offset));
  float3x2 v_17 = v((176u + start_byte_offset));
  float3x2 v_18 = v((200u + start_byte_offset));
  uint4 v_19 = arg_0_params[((224u + start_byte_offset) / 16u)];
  float2 v_20 = asfloat(((((((224u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_19.zw) : (v_19.xy)));
  uint4 v_21 = arg_0_params[((232u + start_byte_offset) / 16u)];
  float2 v_22 = asfloat(((((((232u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_21.zw) : (v_21.xy)));
  uint4 v_23 = arg_0_params[((240u + start_byte_offset) / 16u)];
  float2 v_24 = asfloat(((((((240u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_23.zw) : (v_23.xy)));
  uint4 v_25 = arg_0_params[((248u + start_byte_offset) / 16u)];
  float2 v_26 = asfloat(((((((248u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_25.zw) : (v_25.xy)));
  uint4 v_27 = arg_0_params[((256u + start_byte_offset) / 16u)];
  uint2 v_28 = ((((((256u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_27.zw) : (v_27.xy));
  uint4 v_29 = arg_0_params[((264u + start_byte_offset) / 16u)];
  tint_ExternalTextureParams v_30 = {v_11, v_12, v_13, v_14, v_15, v_16, v_17, v_18, v_20, v_22, v_24, v_26, v_28, asfloat(((((((264u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_29.zw) : (v_29.xy)))};
  return v_30;
}

uint2 textureDimensions_cdc6c9() {
  tint_ExternalTextureParams v_31 = v_10(0u);
  uint2 res = (v_31.apparentSize + (1u).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, textureDimensions_cdc6c9());
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
  uint2 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint2 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


Texture2D<float4> arg_0_plane0 : register(t0, space1);
Texture2D<float4> arg_0_plane1 : register(t1, space1);
cbuffer cbuffer_arg_0_params : register(b2, space1) {
  uint4 arg_0_params[17];
};
float3x2 v(uint start_byte_offset) {
  uint4 v_1 = arg_0_params[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint4 v_3 = arg_0_params[((8u + start_byte_offset) / 16u)];
  float2 v_4 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_3.zw) : (v_3.xy)));
  uint4 v_5 = arg_0_params[((16u + start_byte_offset) / 16u)];
  return float3x2(v_2, v_4, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy))));
}

float3x3 v_6(uint start_byte_offset) {
  return float3x3(asfloat(arg_0_params[(start_byte_offset / 16u)].xyz), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)].xyz), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)].xyz));
}

tint_GammaTransferParams v_7(uint start_byte_offset) {
  tint_GammaTransferParams v_8 = {asfloat(arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]), asfloat(arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)][(((16u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((20u + start_byte_offset) / 16u)][(((20u + start_byte_offset) % 16u) / 4u)]), asfloat(arg_0_params[((24u + start_byte_offset) / 16u)][(((24u + start_byte_offset) % 16u) / 4u)]), arg_0_params[((28u + start_byte_offset) / 16u)][(((28u + start_byte_offset) % 16u) / 4u)]};
  return v_8;
}

float3x4 v_9(uint start_byte_offset) {
  return float3x4(asfloat(arg_0_params[(start_byte_offset / 16u)]), asfloat(arg_0_params[((16u + start_byte_offset) / 16u)]), asfloat(arg_0_params[((32u + start_byte_offset) / 16u)]));
}

tint_ExternalTextureParams v_10(uint start_byte_offset) {
  uint v_11 = arg_0_params[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)];
  uint v_12 = arg_0_params[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)];
  float3x4 v_13 = v_9((16u + start_byte_offset));
  tint_GammaTransferParams v_14 = v_7((64u + start_byte_offset));
  tint_GammaTransferParams v_15 = v_7((96u + start_byte_offset));
  float3x3 v_16 = v_6((128u + start_byte_offset));
  float3x2 v_17 = v((176u + start_byte_offset));
  float3x2 v_18 = v((200u + start_byte_offset));
  uint4 v_19 = arg_0_params[((224u + start_byte_offset) / 16u)];
  float2 v_20 = asfloat(((((((224u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_19.zw) : (v_19.xy)));
  uint4 v_21 = arg_0_params[((232u + start_byte_offset) / 16u)];
  float2 v_22 = asfloat(((((((232u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_21.zw) : (v_21.xy)));
  uint4 v_23 = arg_0_params[((240u + start_byte_offset) / 16u)];
  float2 v_24 = asfloat(((((((240u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_23.zw) : (v_23.xy)));
  uint4 v_25 = arg_0_params[((248u + start_byte_offset) / 16u)];
  float2 v_26 = asfloat(((((((248u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_25.zw) : (v_25.xy)));
  uint4 v_27 = arg_0_params[((256u + start_byte_offset) / 16u)];
  uint2 v_28 = ((((((256u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_27.zw) : (v_27.xy));
  uint4 v_29 = arg_0_params[((264u + start_byte_offset) / 16u)];
  tint_ExternalTextureParams v_30 = {v_11, v_12, v_13, v_14, v_15, v_16, v_17, v_18, v_20, v_22, v_24, v_26, v_28, asfloat(((((((264u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_29.zw) : (v_29.xy)))};
  return v_30;
}

uint2 textureDimensions_cdc6c9() {
  tint_ExternalTextureParams v_31 = v_10(0u);
  uint2 res = (v_31.apparentSize + (1u).xx);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_32 = (VertexOutput)0;
  v_32.pos = (0.0f).xxxx;
  v_32.prevent_dce = textureDimensions_cdc6c9();
  VertexOutput v_33 = v_32;
  return v_33;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_34 = vertex_main_inner();
  vertex_main_outputs v_35 = {v_34.prevent_dce, v_34.pos};
  return v_35;
}

