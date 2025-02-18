#version 310 es

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  vec2 inner_col0;
  vec2 inner_col1;
  vec2 inner_col2;
  vec2 inner_col3;
} v_1;
void a(mat4x2 m) {
}
void b(vec2 v) {
}
void c(float f_1) {
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(mat4x2(v_1.inner_col0, v_1.inner_col1, v_1.inner_col2, v_1.inner_col3));
  b(mat4x2(v_1.inner_col0, v_1.inner_col1, v_1.inner_col2, v_1.inner_col3)[1u]);
  b(mat4x2(v_1.inner_col0, v_1.inner_col1, v_1.inner_col2, v_1.inner_col3)[1u].yx);
  c(mat4x2(v_1.inner_col0, v_1.inner_col1, v_1.inner_col2, v_1.inner_col3)[1u].x);
  c(mat4x2(v_1.inner_col0, v_1.inner_col1, v_1.inner_col2, v_1.inner_col3)[1u].yx.x);
}
