#version 310 es

layout(binding = 0, std430)
buffer output_block_1_ssbo {
  uint inner[2];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  if (false) {
    v.inner[0u] = 1u;
  }
  if (false) {
    v.inner[1u] = 1u;
  }
}
