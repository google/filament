//
// fragment_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uint inner;
} v;
layout(binding = 1, std430)
buffer f_SB_RW_ssbo {
  float16_t arg_0[];
} sb_rw;
uint arrayLength_fd35e2() {
  uint res = uint(sb_rw.arg_0.length());
  return res;
}
void main() {
  v.inner = arrayLength_fd35e2();
}
//
// compute_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
layout(binding = 1, std430)
buffer SB_RW_1_ssbo {
  float16_t arg_0[];
} sb_rw;
uint arrayLength_fd35e2() {
  uint res = uint(sb_rw.arg_0.length());
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = arrayLength_fd35e2();
}
