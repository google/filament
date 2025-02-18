#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v;
vec4 dpdyCoarse_445d24() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = dFdy(arg_0);
  return res;
}
void main() {
  v.inner = dpdyCoarse_445d24();
}
