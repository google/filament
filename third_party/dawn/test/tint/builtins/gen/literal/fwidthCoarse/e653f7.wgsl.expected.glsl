#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec2 inner;
} v;
vec2 fwidthCoarse_e653f7() {
  vec2 res = fwidth(vec2(1.0f));
  return res;
}
void main() {
  v.inner = fwidthCoarse_e653f7();
}
