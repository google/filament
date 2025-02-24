#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v;
vec4 fwidthFine_68f4ef() {
  vec4 res = fwidth(vec4(1.0f));
  return res;
}
void main() {
  v.inner = fwidthFine_68f4ef();
}
