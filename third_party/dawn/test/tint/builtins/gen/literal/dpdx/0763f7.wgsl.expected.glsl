#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec3 inner;
} v;
vec3 dpdx_0763f7() {
  vec3 res = dFdx(vec3(1.0f));
  return res;
}
void main() {
  v.inner = dpdx_0763f7();
}
