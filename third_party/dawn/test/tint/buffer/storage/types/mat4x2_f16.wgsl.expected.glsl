#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer in_block_1_ssbo {
  f16mat4x2 inner;
} v;
layout(binding = 1, std430)
buffer out_block_1_ssbo {
  f16mat4x2 inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v_1.inner = v.inner;
}
