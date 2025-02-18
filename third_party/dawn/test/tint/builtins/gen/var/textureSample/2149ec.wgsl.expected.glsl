#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v;
uniform highp sampler3D arg_0_arg_1;
vec4 textureSample_2149ec() {
  vec3 arg_2 = vec3(1.0f);
  vec4 res = textureOffset(arg_0_arg_1, arg_2, ivec3(1));
  return res;
}
void main() {
  v.inner = textureSample_2149ec();
}
