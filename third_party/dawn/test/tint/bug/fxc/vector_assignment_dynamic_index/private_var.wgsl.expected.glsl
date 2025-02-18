#version 310 es

layout(binding = 0, std140)
uniform i_block_1_ubo {
  uint inner;
} v;
vec3 v1 = vec3(0.0f);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v1[min(v.inner, 2u)] = 1.0f;
}
