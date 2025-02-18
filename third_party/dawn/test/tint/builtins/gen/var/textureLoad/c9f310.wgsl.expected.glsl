//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  ivec4 inner;
} v;
layout(binding = 0, r32i) uniform highp iimage2D arg_0;
ivec4 textureLoad_c9f310() {
  int arg_1 = 1;
  int v_1 = arg_1;
  uint v_2 = (uvec2(imageSize(arg_0)).x - 1u);
  ivec4 res = imageLoad(arg_0, ivec2(uvec2(min(uint(v_1), v_2), 0u)));
  return res;
}
void main() {
  v.inner = textureLoad_c9f310();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  ivec4 inner;
} v;
layout(binding = 0, r32i) uniform highp iimage2D arg_0;
ivec4 textureLoad_c9f310() {
  int arg_1 = 1;
  int v_1 = arg_1;
  uint v_2 = (uvec2(imageSize(arg_0)).x - 1u);
  ivec4 res = imageLoad(arg_0, ivec2(uvec2(min(uint(v_1), v_2), 0u)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_c9f310();
}
