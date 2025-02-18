#version 310 es

layout(binding = 0, std430)
buffer a_block_1_ssbo {
  uvec4 inner;
} v;
void v_1() {
  v.inner.y = (v.inner.y + 1u);
  v.inner.z = (v.inner.z + 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
