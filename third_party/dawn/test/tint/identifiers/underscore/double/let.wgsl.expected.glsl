#version 310 es

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  int inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int a = 1;
  int v_1 = a;
  int b = a;
  int v_2 = v_1;
  v.inner = (((a + v_1) + b) + v_2);
}
