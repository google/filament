#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
uniform highp samplerCubeShadow arg_0_arg_1;
float textureSampleCompare_63fb83() {
  float res = texture(arg_0_arg_1, vec4(vec3(1.0f), 1.0f));
  return res;
}
void main() {
  v.inner = textureSampleCompare_63fb83();
}
