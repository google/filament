#version 310 es

layout(binding = 0, std140)
uniform m_block_1_ubo {
  mat3x4 inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3x4 l_m = v.inner;
  vec4 l_m_1 = v.inner[1u];
}
