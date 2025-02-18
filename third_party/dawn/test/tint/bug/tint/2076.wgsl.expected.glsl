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

uniform highp sampler2D randomTexture_plane0;
uniform highp sampler2D randomTexture_plane1;
layout(binding = 4, std140)
uniform randomTexture_params_block_std140_1_ubo {
  tint_ExternalTextureParams_std140 inner;
} v;
uniform highp sampler2D depthTexture;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
