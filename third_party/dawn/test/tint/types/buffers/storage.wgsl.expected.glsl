#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_weights_block_ssbo {
  float inner[];
} v;
void main() {
  uint v_1 = (uint(v.inner.length()) - 1u);
  uint v_2 = min(uint(0), v_1);
  float a = v.inner[v_2];
}
