#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec3 inner;
} v;
vec3 dpdyFine_1fb7ab() {
  vec3 res = dFdy(vec3(1.0f));
  return res;
}
void main() {
  v.inner = dpdyFine_1fb7ab();
}
