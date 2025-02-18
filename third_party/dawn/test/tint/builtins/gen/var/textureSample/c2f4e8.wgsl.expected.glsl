#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
uniform highp samplerCubeArrayShadow arg_0_arg_1;
float textureSample_c2f4e8() {
  vec3 arg_2 = vec3(1.0f);
  int arg_3 = 1;
  vec3 v_1 = arg_2;
  float res = texture(arg_0_arg_1, vec4(v_1, float(arg_3)), 0.0f);
  return res;
}
void main() {
  v.inner = textureSample_c2f4e8();
}
