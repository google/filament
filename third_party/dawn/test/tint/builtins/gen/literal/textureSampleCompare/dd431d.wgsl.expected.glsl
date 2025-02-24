#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
uniform highp sampler2DArrayShadow arg_0_arg_1;
float textureSampleCompare_dd431d() {
  float res = texture(arg_0_arg_1, vec4(vec2(1.0f), float(1), 1.0f));
  return res;
}
void main() {
  v.inner = textureSampleCompare_dd431d();
}
