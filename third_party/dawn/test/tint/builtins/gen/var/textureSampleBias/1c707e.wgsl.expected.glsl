#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v;
uniform highp sampler2DArray arg_0_arg_1;
vec4 textureSampleBias_1c707e() {
  vec2 arg_2 = vec2(1.0f);
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  vec2 v_1 = arg_2;
  float v_2 = clamp(arg_4, -16.0f, 15.9899997711181640625f);
  vec4 res = texture(arg_0_arg_1, vec3(v_1, float(arg_3)), v_2);
  return res;
}
void main() {
  v.inner = textureSampleBias_1c707e();
}
