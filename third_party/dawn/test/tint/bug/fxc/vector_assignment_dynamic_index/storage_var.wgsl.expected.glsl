#version 310 es

layout(binding = 0, std140)
uniform i_block_1_ubo {
  uint inner;
} v;
layout(binding = 1, std430)
buffer v1_block_1_ssbo {
  vec3 inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.inner[min(v.inner, 2u)] = 1.0f;
}
