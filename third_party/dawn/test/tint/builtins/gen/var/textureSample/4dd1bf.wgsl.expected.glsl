#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v;
uniform highp samplerCubeArray arg_0_arg_1;
vec4 textureSample_4dd1bf() {
  vec3 arg_2 = vec3(1.0f);
  int arg_3 = 1;
  vec3 v_1 = arg_2;
  vec4 res = texture(arg_0_arg_1, vec4(v_1, float(arg_3)));
  return res;
}
void main() {
  v.inner = textureSample_4dd1bf();
}
