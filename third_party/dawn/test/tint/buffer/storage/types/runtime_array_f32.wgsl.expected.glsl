#version 310 es

layout(binding = 0, std430)
buffer in_block_1_ssbo {
  float inner[];
} v;
layout(binding = 1, std430)
buffer out_block_1_ssbo {
  float inner[];
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_2 = (uint(v_1.inner.length()) - 1u);
  uint v_3 = min(uint(0), v_2);
  uint v_4 = (uint(v.inner.length()) - 1u);
  uint v_5 = min(uint(0), v_4);
  v_1.inner[v_3] = v.inner[v_5];
}
