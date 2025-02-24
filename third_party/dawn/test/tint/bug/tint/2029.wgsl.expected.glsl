#version 310 es

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  ivec3 inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = ivec3(1);
}
