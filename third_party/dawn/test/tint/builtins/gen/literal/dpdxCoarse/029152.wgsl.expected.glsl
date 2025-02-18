#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
float dpdxCoarse_029152() {
  float res = dFdx(1.0f);
  return res;
}
void main() {
  v.inner = dpdxCoarse_029152();
}
