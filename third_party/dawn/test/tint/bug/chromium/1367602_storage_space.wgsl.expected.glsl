#version 310 es


struct A {
  float a[1000000];
};

layout(binding = 0, std430)
buffer v_block_1_ssbo {
  int inner[1000000];
} v_1;
layout(binding = 1, std430)
buffer b_block_1_ssbo {
  A inner;
} v_2;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
