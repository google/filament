#version 310 es

layout(binding = 0, std140)
uniform m_block_std140_1_ubo {
  vec2 inner_col0;
  vec2 inner_col1;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2 v_1 = mat2(v.inner_col0, v.inner_col1);
  mat2 l_m = v_1;
  vec2 l_m_1 = v_1[1u];
}
