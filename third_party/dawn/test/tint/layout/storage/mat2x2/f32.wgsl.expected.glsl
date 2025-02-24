#version 310 es


struct SSBO {
  mat2 m;
};

layout(binding = 0, std430)
buffer ssbo_block_1_ssbo {
  SSBO inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2 v = v_1.inner.m;
  v_1.inner.m = v;
}
