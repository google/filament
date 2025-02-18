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

layout(binding = 2, std140)
uniform t_params_block_std140_1_ubo {
  tint_ExternalTextureParams_std140 inner;
} v_1;
uniform highp sampler2D t_plane0;
uniform highp sampler2D t_plane1;
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
    vec4 v_8 = texelFetch(t_plane0, v_7, int(0u));
    v_5 = v_8.xyz;
    v_6 = v_8.w;
  } else {
    ivec2 v_9 = ivec2(v_4);
    float v_10 = texelFetch(t_plane0, v_9, int(0u)).x;
    ivec2 v_11 = ivec2(uvec2((v_3 * params.plane1CoordFactor)));
    v_5 = (vec4(v_10, texelFetch(t_plane1, v_11, int(0u)).xy, 1.0f) * params.yuvToRgbConversionMatrix);
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
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_ExternalTextureParams v_16 = tint_convert_tint_ExternalTextureParams(v_1.inner);
  vec4 r = tint_TextureLoadExternal(v_16, min(uvec2(ivec2(0)), ((v_16.apparentSize + uvec2(1u)) - uvec2(1u))));
}
