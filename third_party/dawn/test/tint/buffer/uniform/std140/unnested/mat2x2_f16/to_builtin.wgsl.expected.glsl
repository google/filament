#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  f16vec2 inner_col0;
  f16vec2 inner_col1;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat2 t = transpose(f16mat2(v.inner_col0, v.inner_col1));
  float16_t l = length(f16mat2(v.inner_col0, v.inner_col1)[1u]);
  float16_t a = abs(f16mat2(v.inner_col0, v.inner_col1)[0u].yx.x);
}
