#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec2 inner;
} v;
vec2 fwidthFine_ff6aa0() {
  vec2 arg_0 = vec2(1.0f);
  vec2 res = fwidth(arg_0);
  return res;
}
void main() {
  v.inner = fwidthFine_ff6aa0();
}
