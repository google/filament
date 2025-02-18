#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  f16vec3 inner_col0;
  f16vec3 inner_col1;
} v_1;
void a(f16mat2x3 m) {
}
void b(f16vec3 v) {
}
void c(float16_t f_1) {
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  a(f16mat2x3(v_1.inner_col0, v_1.inner_col1));
  b(f16mat2x3(v_1.inner_col0, v_1.inner_col1)[1u]);
  b(f16mat2x3(v_1.inner_col0, v_1.inner_col1)[1u].zxy);
  c(f16mat2x3(v_1.inner_col0, v_1.inner_col1)[1u].x);
  c(f16mat2x3(v_1.inner_col0, v_1.inner_col1)[1u].zxy.x);
}
