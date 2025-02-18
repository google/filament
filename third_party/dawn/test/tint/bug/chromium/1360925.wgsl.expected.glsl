#version 310 es

layout(binding = 0, std430)
buffer G_block_1_ssbo {
  int inner[];
} v;
void n() {
  uint v_1 = uint(v.inner.length());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
