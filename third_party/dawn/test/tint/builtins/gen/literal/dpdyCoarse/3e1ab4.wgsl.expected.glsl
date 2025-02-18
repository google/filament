#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec2 inner;
} v;
vec2 dpdyCoarse_3e1ab4() {
  vec2 res = dFdy(vec2(1.0f));
  return res;
}
void main() {
  v.inner = dpdyCoarse_3e1ab4();
}
