#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec2 inner;
} v;
vec2 dpdxFine_9631de() {
  vec2 res = dFdx(vec2(1.0f));
  return res;
}
void main() {
  v.inner = dpdxFine_9631de();
}
