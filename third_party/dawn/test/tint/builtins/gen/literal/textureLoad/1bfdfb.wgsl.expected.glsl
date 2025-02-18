//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;


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

struct tint_ExternalTextureParams_std140 {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  uint tint_pad_0;
  uint tint_pad_1;
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  vec3 gamutConversionMatrix_col0;
  uint tint_pad_2;
  vec3 gamutConversionMatrix_col1;
  uint tint_pad_3;
  vec3 gamutConversionMatrix_col2;
  uint tint_pad_4;
  vec2 sampleTransform_col0;
  vec2 sampleTransform_col1;
  vec2 sampleTransform_col2;
  vec2 loadTransform_col0;
  vec2 loadTransform_col1;
  vec2 loadTransform_col2;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 apparentSize;
  vec2 plane1CoordFactor;
};

struct tint_ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat3x2 sampleTransform;
  mat3x2 loadTransform;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 apparentSize;
  vec2 plane1CoordFactor;
};

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v_1;
layout(binding = 2, std140)
uniform f_arg_0_params_block_std140_ubo {
  tint_ExternalTextureParams_std140 inner;
} v_2;
uniform highp sampler2D arg_0_plane0;
uniform highp sampler2D arg_0_plane1;
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_3 = vec3(params.G);
  return mix((sign(v) * (pow(((params.A * abs(v)) + params.B), v_3) + params.E)), (sign(v) * ((params.C * abs(v)) + params.F)), lessThan(abs(v), vec3(params.D)));
}
vec4 tint_TextureLoadExternal(tint_ExternalTextureParams params, uvec2 coords) {
  vec2 v_4 = round((params.loadTransform * vec3(vec2(min(coords, params.apparentSize)), 1.0f)));
  uvec2 v_5 = uvec2(v_4);
  vec3 v_6 = vec3(0.0f);
  float v_7 = 0.0f;
  if ((params.numPlanes == 1u)) {
    ivec2 v_8 = ivec2(v_5);
    vec4 v_9 = texelFetch(arg_0_plane0, v_8, int(0u));
    v_6 = v_9.xyz;
    v_7 = v_9.w;
  } else {
    ivec2 v_10 = ivec2(v_5);
    float v_11 = texelFetch(arg_0_plane0, v_10, int(0u)).x;
    ivec2 v_12 = ivec2(uvec2((v_4 * params.plane1CoordFactor)));
    v_6 = (vec4(v_11, texelFetch(arg_0_plane1, v_12, int(0u)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_7 = 1.0f;
  }
  vec3 v_13 = v_6;
  vec3 v_14 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_14 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_13, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_14 = v_13;
  }
  return vec4(v_14, v_7);
}
tint_ExternalTextureParams tint_convert_tint_ExternalTextureParams(tint_ExternalTextureParams_std140 tint_input) {
  mat3 v_15 = mat3(tint_input.gamutConversionMatrix_col0, tint_input.gamutConversionMatrix_col1, tint_input.gamutConversionMatrix_col2);
  mat3x2 v_16 = mat3x2(tint_input.sampleTransform_col0, tint_input.sampleTransform_col1, tint_input.sampleTransform_col2);
  return tint_ExternalTextureParams(tint_input.numPlanes, tint_input.doYuvToRgbConversionOnly, tint_input.yuvToRgbConversionMatrix, tint_input.gammaDecodeParams, tint_input.gammaEncodeParams, v_15, v_16, mat3x2(tint_input.loadTransform_col0, tint_input.loadTransform_col1, tint_input.loadTransform_col2), tint_input.samplePlane0RectMin, tint_input.samplePlane0RectMax, tint_input.samplePlane1RectMin, tint_input.samplePlane1RectMax, tint_input.apparentSize, tint_input.plane1CoordFactor);
}
vec4 textureLoad_1bfdfb() {
  tint_ExternalTextureParams v_17 = tint_convert_tint_ExternalTextureParams(v_2.inner);
  vec4 res = tint_TextureLoadExternal(v_17, min(uvec2(1u), ((v_17.apparentSize + uvec2(1u)) - uvec2(1u))));
  return res;
}
void main() {
  v_1.inner = textureLoad_1bfdfb();
}
//
// compute_main
//
#version 310 es


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

struct tint_ExternalTextureParams_std140 {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  uint tint_pad_0;
  uint tint_pad_1;
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  vec3 gamutConversionMatrix_col0;
  uint tint_pad_2;
  vec3 gamutConversionMatrix_col1;
  uint tint_pad_3;
  vec3 gamutConversionMatrix_col2;
  uint tint_pad_4;
  vec2 sampleTransform_col0;
  vec2 sampleTransform_col1;
  vec2 sampleTransform_col2;
  vec2 loadTransform_col0;
  vec2 loadTransform_col1;
  vec2 loadTransform_col2;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 apparentSize;
  vec2 plane1CoordFactor;
};

struct tint_ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat3x2 sampleTransform;
  mat3x2 loadTransform;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 apparentSize;
  vec2 plane1CoordFactor;
};

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v_1;
layout(binding = 2, std140)
uniform arg_0_params_block_std140_1_ubo {
  tint_ExternalTextureParams_std140 inner;
} v_2;
uniform highp sampler2D arg_0_plane0;
uniform highp sampler2D arg_0_plane1;
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_3 = vec3(params.G);
  return mix((sign(v) * (pow(((params.A * abs(v)) + params.B), v_3) + params.E)), (sign(v) * ((params.C * abs(v)) + params.F)), lessThan(abs(v), vec3(params.D)));
}
vec4 tint_TextureLoadExternal(tint_ExternalTextureParams params, uvec2 coords) {
  vec2 v_4 = round((params.loadTransform * vec3(vec2(min(coords, params.apparentSize)), 1.0f)));
  uvec2 v_5 = uvec2(v_4);
  vec3 v_6 = vec3(0.0f);
  float v_7 = 0.0f;
  if ((params.numPlanes == 1u)) {
    ivec2 v_8 = ivec2(v_5);
    vec4 v_9 = texelFetch(arg_0_plane0, v_8, int(0u));
    v_6 = v_9.xyz;
    v_7 = v_9.w;
  } else {
    ivec2 v_10 = ivec2(v_5);
    float v_11 = texelFetch(arg_0_plane0, v_10, int(0u)).x;
    ivec2 v_12 = ivec2(uvec2((v_4 * params.plane1CoordFactor)));
    v_6 = (vec4(v_11, texelFetch(arg_0_plane1, v_12, int(0u)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_7 = 1.0f;
  }
  vec3 v_13 = v_6;
  vec3 v_14 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_14 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_13, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_14 = v_13;
  }
  return vec4(v_14, v_7);
}
tint_ExternalTextureParams tint_convert_tint_ExternalTextureParams(tint_ExternalTextureParams_std140 tint_input) {
  mat3 v_15 = mat3(tint_input.gamutConversionMatrix_col0, tint_input.gamutConversionMatrix_col1, tint_input.gamutConversionMatrix_col2);
  mat3x2 v_16 = mat3x2(tint_input.sampleTransform_col0, tint_input.sampleTransform_col1, tint_input.sampleTransform_col2);
  return tint_ExternalTextureParams(tint_input.numPlanes, tint_input.doYuvToRgbConversionOnly, tint_input.yuvToRgbConversionMatrix, tint_input.gammaDecodeParams, tint_input.gammaEncodeParams, v_15, v_16, mat3x2(tint_input.loadTransform_col0, tint_input.loadTransform_col1, tint_input.loadTransform_col2), tint_input.samplePlane0RectMin, tint_input.samplePlane0RectMax, tint_input.samplePlane1RectMin, tint_input.samplePlane1RectMax, tint_input.apparentSize, tint_input.plane1CoordFactor);
}
vec4 textureLoad_1bfdfb() {
  tint_ExternalTextureParams v_17 = tint_convert_tint_ExternalTextureParams(v_2.inner);
  vec4 res = tint_TextureLoadExternal(v_17, min(uvec2(1u), ((v_17.apparentSize + uvec2(1u)) - uvec2(1u))));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.inner = textureLoad_1bfdfb();
}
//
// vertex_main
//
#version 310 es


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

struct tint_ExternalTextureParams_std140 {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  uint tint_pad_0;
  uint tint_pad_1;
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  vec3 gamutConversionMatrix_col0;
  uint tint_pad_2;
  vec3 gamutConversionMatrix_col1;
  uint tint_pad_3;
  vec3 gamutConversionMatrix_col2;
  uint tint_pad_4;
  vec2 sampleTransform_col0;
  vec2 sampleTransform_col1;
  vec2 sampleTransform_col2;
  vec2 loadTransform_col0;
  vec2 loadTransform_col1;
  vec2 loadTransform_col2;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 apparentSize;
  vec2 plane1CoordFactor;
};

struct tint_ExternalTextureParams {
  uint numPlanes;
  uint doYuvToRgbConversionOnly;
  mat3x4 yuvToRgbConversionMatrix;
  tint_GammaTransferParams gammaDecodeParams;
  tint_GammaTransferParams gammaEncodeParams;
  mat3 gamutConversionMatrix;
  mat3x2 sampleTransform;
  mat3x2 loadTransform;
  vec2 samplePlane0RectMin;
  vec2 samplePlane0RectMax;
  vec2 samplePlane1RectMin;
  vec2 samplePlane1RectMax;
  uvec2 apparentSize;
  vec2 plane1CoordFactor;
};

struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

layout(binding = 2, std140)
uniform v_arg_0_params_block_std140_ubo {
  tint_ExternalTextureParams_std140 inner;
} v_1;
uniform highp sampler2D arg_0_plane0;
uniform highp sampler2D arg_0_plane1;
layout(location = 0) flat out vec4 tint_interstage_location0;
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_2 = vec3(params.G);
  return mix((sign(v) * (pow(((params.A * abs(v)) + params.B), v_2) + params.E)), (sign(v) * ((params.C * abs(v)) + params.F)), lessThan(abs(v), vec3(params.D)));
}
vec4 tint_TextureLoadExternal(tint_ExternalTextureParams params, uvec2 coords) {
  vec2 v_3 = round((params.loadTransform * vec3(vec2(min(coords, params.apparentSize)), 1.0f)));
  uvec2 v_4 = uvec2(v_3);
  vec3 v_5 = vec3(0.0f);
  float v_6 = 0.0f;
  if ((params.numPlanes == 1u)) {
    ivec2 v_7 = ivec2(v_4);
    vec4 v_8 = texelFetch(arg_0_plane0, v_7, int(0u));
    v_5 = v_8.xyz;
    v_6 = v_8.w;
  } else {
    ivec2 v_9 = ivec2(v_4);
    float v_10 = texelFetch(arg_0_plane0, v_9, int(0u)).x;
    ivec2 v_11 = ivec2(uvec2((v_3 * params.plane1CoordFactor)));
    v_5 = (vec4(v_10, texelFetch(arg_0_plane1, v_11, int(0u)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_6 = 1.0f;
  }
  vec3 v_12 = v_5;
  vec3 v_13 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_13 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_12, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_13 = v_12;
  }
  return vec4(v_13, v_6);
}
tint_ExternalTextureParams tint_convert_tint_ExternalTextureParams(tint_ExternalTextureParams_std140 tint_input) {
  mat3 v_14 = mat3(tint_input.gamutConversionMatrix_col0, tint_input.gamutConversionMatrix_col1, tint_input.gamutConversionMatrix_col2);
  mat3x2 v_15 = mat3x2(tint_input.sampleTransform_col0, tint_input.sampleTransform_col1, tint_input.sampleTransform_col2);
  return tint_ExternalTextureParams(tint_input.numPlanes, tint_input.doYuvToRgbConversionOnly, tint_input.yuvToRgbConversionMatrix, tint_input.gammaDecodeParams, tint_input.gammaEncodeParams, v_14, v_15, mat3x2(tint_input.loadTransform_col0, tint_input.loadTransform_col1, tint_input.loadTransform_col2), tint_input.samplePlane0RectMin, tint_input.samplePlane0RectMax, tint_input.samplePlane1RectMin, tint_input.samplePlane1RectMax, tint_input.apparentSize, tint_input.plane1CoordFactor);
}
vec4 textureLoad_1bfdfb() {
  tint_ExternalTextureParams v_16 = tint_convert_tint_ExternalTextureParams(v_1.inner);
  vec4 res = tint_TextureLoadExternal(v_16, min(uvec2(1u), ((v_16.apparentSize + uvec2(1u)) - uvec2(1u))));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_17 = VertexOutput(vec4(0.0f), vec4(0.0f));
  v_17.pos = vec4(0.0f);
  v_17.prevent_dce = textureLoad_1bfdfb();
  return v_17;
}
void main() {
  VertexOutput v_18 = vertex_main_inner();
  gl_Position = vec4(v_18.pos.x, -(v_18.pos.y), ((2.0f * v_18.pos.z) - v_18.pos.w), v_18.pos.w);
  tint_interstage_location0 = v_18.prevent_dce;
  gl_PointSize = 1.0f;
}
