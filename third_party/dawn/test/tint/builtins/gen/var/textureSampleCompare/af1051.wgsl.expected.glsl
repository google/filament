#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
uniform highp sampler2DArrayShadow arg_0_arg_1;
float textureSampleCompare_af1051() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  float arg_4 = 1.0f;
  vec2 v_1 = arg_2;
  float v_2 = arg_4;
  vec4 v_3 = vec4(v_1, float(arg_3), v_2);
  vec2 v_4 = dFdx(v_1);
  float res = textureGradOffset(arg_0_arg_1, v_3, v_4, dFdy(v_1), ivec2(1));
  return res;
}
void main() {
  v.inner = textureSampleCompare_af1051();
}
