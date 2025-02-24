#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  f16vec3 inner;
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  f16vec3 inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16vec3 x = v.inner;
  v_1.inner = x;
}
