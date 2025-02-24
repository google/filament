#version 310 es
precision highp float;
precision highp int;


struct S_std140 {
  vec3 matrix_col0;
  uint tint_pad_0;
  vec3 matrix_col1;
  uint tint_pad_1;
  vec3 matrix_col2;
  uint tint_pad_2;
  vec3 vector;
  uint tint_pad_3;
};

layout(binding = 0, std140)
uniform f_data_block_std140_ubo {
  S_std140 inner;
} v;
void main() {
  vec3 v_1 = v.inner.vector;
  vec3 x = (v_1 * mat3(v.inner.matrix_col0, v.inner.matrix_col1, v.inner.matrix_col2));
}
