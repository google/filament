#version 310 es


struct S {
  int i;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  S inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
