//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v;
layout(binding = 0, r8) uniform highp image2D arg_0;
vec4 textureLoad_35a5e2() {
  uint v_1 = (uvec2(imageSize(arg_0)).x - 1u);
  vec4 res = imageLoad(arg_0, ivec2(uvec2(min(uint(1), v_1), 0u)));
  return res;
}
void main() {
  v.inner = textureLoad_35a5e2();
}
//
// compute_main
//
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 0, r8) uniform highp image2D arg_0;
vec4 textureLoad_35a5e2() {
  uint v_1 = (uvec2(imageSize(arg_0)).x - 1u);
  vec4 res = imageLoad(arg_0, ivec2(uvec2(min(uint(1), v_1), 0u)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_35a5e2();
}
