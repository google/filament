#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct S_std140 {
  f16vec3 matrix_col0;
  f16vec3 matrix_col1;
  f16vec3 matrix_col2;
  f16vec3 vector;
};

layout(binding = 0, std140)
uniform f_data_block_std140_ubo {
  S_std140 inner;
} v;
void main() {
  f16mat3 v_1 = f16mat3(v.inner.matrix_col0, v.inner.matrix_col1, v.inner.matrix_col2);
  f16vec3 x = (v_1 * v.inner.vector);
}
