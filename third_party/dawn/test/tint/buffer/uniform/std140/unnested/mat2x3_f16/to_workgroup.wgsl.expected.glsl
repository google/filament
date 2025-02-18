#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_std140_1_ubo {
  f16vec3 inner_col0;
  f16vec3 inner_col1;
} v;
shared f16mat2x3 w;
void f_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    w = f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf));
  }
  barrier();
  w = f16mat2x3(v.inner_col0, v.inner_col1);
  w[1u] = f16mat2x3(v.inner_col0, v.inner_col1)[0u];
  w[1u] = f16mat2x3(v.inner_col0, v.inner_col1)[0u].zxy;
  w[0u].y = f16mat2x3(v.inner_col0, v.inner_col1)[1u].x;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
