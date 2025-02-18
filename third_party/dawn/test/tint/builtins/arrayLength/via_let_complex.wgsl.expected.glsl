#version 310 es

layout(binding = 0, std430)
buffer S_1_ssbo {
  int a[];
} G;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint l1 = uint(G.a.length());
}
