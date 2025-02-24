#version 310 es

layout(binding = 0, std430)
buffer out_block_1_ssbo {
  mat3 inner;
} v;
void tint_store_and_preserve_padding(mat3 value_param) {
  v.inner[0u] = value_param[0u];
  v.inner[1u] = value_param[1u];
  v.inner[2u] = value_param[2u];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3 m = mat3(vec3(0.0f), vec3(0.0f), vec3(0.0f));
  tint_store_and_preserve_padding(mat3(m));
}
