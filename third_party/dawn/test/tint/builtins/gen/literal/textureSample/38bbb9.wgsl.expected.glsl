#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
uniform highp sampler2DShadow arg_0_arg_1;
float textureSample_38bbb9() {
  float res = texture(arg_0_arg_1, vec3(vec2(1.0f), 0.0f));
  return res;
}
void main() {
  v.inner = textureSample_38bbb9();
}
