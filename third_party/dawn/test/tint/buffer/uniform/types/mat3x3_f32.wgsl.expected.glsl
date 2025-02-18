#version 310 es

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  vec3 inner_col0;
  uint tint_pad_0;
  vec3 inner_col1;
  uint tint_pad_1;
  vec3 inner_col2;
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  mat3 inner;
} v_1;
void tint_store_and_preserve_padding(mat3 value_param) {
  v_1.inner[0u] = value_param[0u];
  v_1.inner[1u] = value_param[1u];
  v_1.inner[2u] = value_param[2u];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3 x = mat3(v.inner_col0, v.inner_col1, v.inner_col2);
  tint_store_and_preserve_padding(x);
}
