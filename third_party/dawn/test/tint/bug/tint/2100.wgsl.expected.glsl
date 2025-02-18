#version 310 es


struct S_std140 {
  mat4 matrix_view;
  vec3 matrix_normal_col0;
  uint tint_pad_0;
  vec3 matrix_normal_col1;
  uint tint_pad_1;
  vec3 matrix_normal_col2;
  uint tint_pad_2;
};

layout(binding = 0, std140)
uniform v_buffer_block_std140_ubo {
  S_std140 inner;
} v;
vec4 main_inner() {
  float x = v.inner.matrix_view[0u].z;
  return vec4(x, 0.0f, 0.0f, 1.0f);
}
void main() {
  vec4 v_1 = main_inner();
  gl_Position = vec4(v_1.x, -(v_1.y), ((2.0f * v_1.z) - v_1.w), v_1.w);
  gl_PointSize = 1.0f;
}
