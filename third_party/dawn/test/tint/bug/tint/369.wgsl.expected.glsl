#version 310 es


struct S {
  mat2 m;
};

struct S_std140 {
  vec2 m_col0;
  vec2 m_col1;
};

layout(binding = 0, std430)
buffer SSBO_block_1_ssbo {
  S inner;
} v;
layout(binding = 0, std140)
uniform UBO_block_std140_1_ubo {
  S_std140 inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
