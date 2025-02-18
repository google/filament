#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
float fwidthFine_f1742d() {
  float res = fwidth(1.0f);
  return res;
}
void main() {
  v.inner = fwidthFine_f1742d();
}
