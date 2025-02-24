#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec3 inner;
} v;
vec3 fwidthFine_523fdc() {
  vec3 res = fwidth(vec3(1.0f));
  return res;
}
void main() {
  v.inner = fwidthFine_523fdc();
}
