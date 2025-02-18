#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
uniform highp sampler2DArrayShadow arg_0_arg_1;
float textureSample_4703d0() {
  vec4 v_1 = vec4(vec2(1.0f), float(1u), 0.0f);
  vec2 v_2 = dFdx(vec2(1.0f));
  float res = textureGradOffset(arg_0_arg_1, v_1, v_2, dFdy(vec2(1.0f)), ivec2(1));
  return res;
}
void main() {
  v.inner = textureSample_4703d0();
}
