#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
uniform highp samplerCubeArrayShadow arg_0_arg_1;
float textureSampleCompare_a3ca7e() {
  float res = texture(arg_0_arg_1, vec4(vec3(1.0f), float(1)), 1.0f);
  return res;
}
void main() {
  v.inner = textureSampleCompare_a3ca7e();
}
