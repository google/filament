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
layout(binding = 3, std140)
uniform f_arg_0_params_block_std140_ubo {
  tint_ExternalTextureParams_std140 inner;
} v_2;
uniform highp sampler2D arg_0_plane0_arg_1;
uniform highp sampler2D arg_0_plane1_arg_1;
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_3 = vec3(params.G);
  return mix((sign(v) * (pow(((params.A * abs(v)) + params.B), v_3) + params.E)), (sign(v) * ((params.C * abs(v)) + params.F)), lessThan(abs(v), vec3(params.D)));
}
vec4 tint_TextureSampleExternal(tint_ExternalTextureParams params, vec2 coords) {
  vec2 v_4 = (params.sampleTransform * vec3(coords, 1.0f));
  vec3 v_5 = vec3(0.0f);
  float v_6 = 0.0f;
  if ((params.numPlanes == 1u)) {
    vec4 v_7 = textureLod(arg_0_plane0_arg_1, clamp(v_4, params.samplePlane0RectMin, params.samplePlane0RectMax), 0.0f);
    v_5 = v_7.xyz;
    v_6 = v_7.w;
  } else {
    v_5 = (vec4(textureLod(arg_0_plane0_arg_1, clamp(v_4, params.samplePlane0RectMin, params.samplePlane0RectMax), 0.0f).x, textureLod(arg_0_plane1_arg_1, clamp(v_4, params.samplePlane1RectMin, params.samplePlane1RectMax), 0.0f).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_6 = 1.0f;
  }
  vec3 v_8 = v_5;
  vec3 v_9 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_9 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_8, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_9 = v_8;
  }
  return vec4(v_9, v_6);
}
tint_ExternalTextureParams tint_convert_tint_ExternalTextureParams(tint_ExternalTextureParams_std140 tint_input) {
  mat3 v_10 = mat3(tint_input.gamutConversionMatrix_col0, tint_input.gamutConversionMatrix_col1, tint_input.gamutConversionMatrix_col2);
  mat3x2 v_11 = mat3x2(tint_input.sampleTransform_col0, tint_input.sampleTransform_col1, tint_input.sampleTransform_col2);
  return tint_ExternalTextureParams(tint_input.numPlanes, tint_input.doYuvToRgbConversionOnly, tint_input.yuvToRgbConversionMatrix, tint_input.gammaDecodeParams, tint_input.gammaEncodeParams, v_10, v_11, mat3x2(tint_input.loadTransform_col0, tint_input.loadTransform_col1, tint_input.loadTransform_col2), tint_input.samplePlane0RectMin, tint_input.samplePlane0RectMax, tint_input.samplePlane1RectMin, tint_input.samplePlane1RectMax, tint_input.apparentSize, tint_input.plane1CoordFactor);
}
vec4 textureSampleBaseClampToEdge_7c04e6() {
  vec2 arg_2 = vec2(1.0f);
  tint_ExternalTextureParams v_12 = tint_convert_tint_ExternalTextureParams(v_2.inner);
  vec4 res = tint_TextureSampleExternal(v_12, arg_2);
  return res;
}
void main() {
  v_1.inner = textureSampleBaseClampToEdge_7c04e6();
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
layout(binding = 3, std140)
uniform arg_0_params_block_std140_1_ubo {
  tint_ExternalTextureParams_std140 inner;
} v_2;
uniform highp sampler2D arg_0_plane0_arg_1;
uniform highp sampler2D arg_0_plane1_arg_1;
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_3 = vec3(params.G);
  return mix((sign(v) * (pow(((params.A * abs(v)) + params.B), v_3) + params.E)), (sign(v) * ((params.C * abs(v)) + params.F)), lessThan(abs(v), vec3(params.D)));
}
vec4 tint_TextureSampleExternal(tint_ExternalTextureParams params, vec2 coords) {
  vec2 v_4 = (params.sampleTransform * vec3(coords, 1.0f));
  vec3 v_5 = vec3(0.0f);
  float v_6 = 0.0f;
  if ((params.numPlanes == 1u)) {
    vec4 v_7 = textureLod(arg_0_plane0_arg_1, clamp(v_4, params.samplePlane0RectMin, params.samplePlane0RectMax), 0.0f);
    v_5 = v_7.xyz;
    v_6 = v_7.w;
  } else {
    v_5 = (vec4(textureLod(arg_0_plane0_arg_1, clamp(v_4, params.samplePlane0RectMin, params.samplePlane0RectMax), 0.0f).x, textureLod(arg_0_plane1_arg_1, clamp(v_4, params.samplePlane1RectMin, params.samplePlane1RectMax), 0.0f).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_6 = 1.0f;
  }
  vec3 v_8 = v_5;
  vec3 v_9 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_9 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_8, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_9 = v_8;
  }
  return vec4(v_9, v_6);
}
tint_ExternalTextureParams tint_convert_tint_ExternalTextureParams(tint_ExternalTextureParams_std140 tint_input) {
  mat3 v_10 = mat3(tint_input.gamutConversionMatrix_col0, tint_input.gamutConversionMatrix_col1, tint_input.gamutConversionMatrix_col2);
  mat3x2 v_11 = mat3x2(tint_input.sampleTransform_col0, tint_input.sampleTransform_col1, tint_input.sampleTransform_col2);
  return tint_ExternalTextureParams(tint_input.numPlanes, tint_input.doYuvToRgbConversionOnly, tint_input.yuvToRgbConversionMatrix, tint_input.gammaDecodeParams, tint_input.gammaEncodeParams, v_10, v_11, mat3x2(tint_input.loadTransform_col0, tint_input.loadTransform_col1, tint_input.loadTransform_col2), tint_input.samplePlane0RectMin, tint_input.samplePlane0RectMax, tint_input.samplePlane1RectMin, tint_input.samplePlane1RectMax, tint_input.apparentSize, tint_input.plane1CoordFactor);
}
vec4 textureSampleBaseClampToEdge_7c04e6() {
  vec2 arg_2 = vec2(1.0f);
  tint_ExternalTextureParams v_12 = tint_convert_tint_ExternalTextureParams(v_2.inner);
  vec4 res = tint_TextureSampleExternal(v_12, arg_2);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.inner = textureSampleBaseClampToEdge_7c04e6();
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

layout(binding = 3, std140)
uniform v_arg_0_params_block_std140_ubo {
  tint_ExternalTextureParams_std140 inner;
} v_1;
uniform highp sampler2D arg_0_plane0_arg_1;
uniform highp sampler2D arg_0_plane1_arg_1;
layout(location = 0) flat out vec4 tint_interstage_location0;
vec3 tint_GammaCorrection(vec3 v, tint_GammaTransferParams params) {
  vec3 v_2 = vec3(params.G);
  return mix((sign(v) * (pow(((params.A * abs(v)) + params.B), v_2) + params.E)), (sign(v) * ((params.C * abs(v)) + params.F)), lessThan(abs(v), vec3(params.D)));
}
vec4 tint_TextureSampleExternal(tint_ExternalTextureParams params, vec2 coords) {
  vec2 v_3 = (params.sampleTransform * vec3(coords, 1.0f));
  vec3 v_4 = vec3(0.0f);
  float v_5 = 0.0f;
  if ((params.numPlanes == 1u)) {
    vec4 v_6 = textureLod(arg_0_plane0_arg_1, clamp(v_3, params.samplePlane0RectMin, params.samplePlane0RectMax), 0.0f);
    v_4 = v_6.xyz;
    v_5 = v_6.w;
  } else {
    v_4 = (vec4(textureLod(arg_0_plane0_arg_1, clamp(v_3, params.samplePlane0RectMin, params.samplePlane0RectMax), 0.0f).x, textureLod(arg_0_plane1_arg_1, clamp(v_3, params.samplePlane1RectMin, params.samplePlane1RectMax), 0.0f).xy, 1.0f) * params.yuvToRgbConversionMatrix);
    v_5 = 1.0f;
  }
  vec3 v_7 = v_4;
  vec3 v_8 = vec3(0.0f);
  if ((params.doYuvToRgbConversionOnly == 0u)) {
    v_8 = tint_GammaCorrection((params.gamutConversionMatrix * tint_GammaCorrection(v_7, params.gammaDecodeParams)), params.gammaEncodeParams);
  } else {
    v_8 = v_7;
  }
  return vec4(v_8, v_5);
}
tint_ExternalTextureParams tint_convert_tint_ExternalTextureParams(tint_ExternalTextureParams_std140 tint_input) {
  mat3 v_9 = mat3(tint_input.gamutConversionMatrix_col0, tint_input.gamutConversionMatrix_col1, tint_input.gamutConversionMatrix_col2);
  mat3x2 v_10 = mat3x2(tint_input.sampleTransform_col0, tint_input.sampleTransform_col1, tint_input.sampleTransform_col2);
  return tint_ExternalTextureParams(tint_input.numPlanes, tint_input.doYuvToRgbConversionOnly, tint_input.yuvToRgbConversionMatrix, tint_input.gammaDecodeParams, tint_input.gammaEncodeParams, v_9, v_10, mat3x2(tint_input.loadTransform_col0, tint_input.loadTransform_col1, tint_input.loadTransform_col2), tint_input.samplePlane0RectMin, tint_input.samplePlane0RectMax, tint_input.samplePlane1RectMin, tint_input.samplePlane1RectMax, tint_input.apparentSize, tint_input.plane1CoordFactor);
}
vec4 textureSampleBaseClampToEdge_7c04e6() {
  vec2 arg_2 = vec2(1.0f);
  tint_ExternalTextureParams v_11 = tint_convert_tint_ExternalTextureParams(v_1.inner);
  vec4 res = tint_TextureSampleExternal(v_11, arg_2);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_12 = VertexOutput(vec4(0.0f), vec4(0.0f));
  v_12.pos = vec4(0.0f);
  v_12.prevent_dce = textureSampleBaseClampToEdge_7c04e6();
  return v_12;
}
void main() {
  VertexOutput v_13 = vertex_main_inner();
  gl_Position = vec4(v_13.pos.x, -(v_13.pos.y), ((2.0f * v_13.pos.z) - v_13.pos.w), v_13.pos.w);
  tint_interstage_location0 = v_13.prevent_dce;
  gl_PointSize = 1.0f;
}
