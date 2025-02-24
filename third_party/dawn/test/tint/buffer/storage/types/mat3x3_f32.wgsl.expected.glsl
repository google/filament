#version 310 es

layout(binding = 0, std430)
buffer in_block_1_ssbo {
  mat3 inner;
} v;
layout(binding = 1, std430)
buffer out_block_1_ssbo {
  mat3 inner;
} v_1;
void tint_store_and_preserve_padding(mat3 value_param) {
  v_1.inner[0u] = value_param[0u];
  v_1.inner[1u] = value_param[1u];
  v_1.inner[2u] = value_param[2u];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_store_and_preserve_padding(v.inner);
}
