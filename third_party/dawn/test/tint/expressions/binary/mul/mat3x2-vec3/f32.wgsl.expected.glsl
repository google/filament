#version 310 es
precision highp float;
precision highp int;


struct S_std140 {
  vec2 matrix_col0;
  vec2 matrix_col1;
  vec2 matrix_col2;
  uint tint_pad_0;
  uint tint_pad_1;
  vec3 vector;
  uint tint_pad_2;
};

layout(binding = 0, std140)
uniform f_data_block_std140_ubo {
  S_std140 inner;
} v;
void main() {
  mat3x2 v_1 = mat3x2(v.inner.matrix_col0, v.inner.matrix_col1, v.inner.matrix_col2);
  vec2 x = (v_1 * v.inner.vector);
}
