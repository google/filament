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
layout(binding = 0, rg32f) uniform highp image2DArray arg_0;
vec4 textureLoad_482627() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  uvec2 v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = min(v_2, (uint(imageSize(arg_0).z) - 1u));
  ivec2 v_4 = ivec2(min(v_1, (uvec2(imageSize(arg_0).xy) - uvec2(1u))));
  vec4 res = imageLoad(arg_0, ivec3(v_4, int(v_3)));
  return res;
}
void main() {
  v.inner = textureLoad_482627();
}
//
// compute_main
//
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 0, rg32f) uniform highp image2DArray arg_0;
vec4 textureLoad_482627() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  uvec2 v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = min(v_2, (uint(imageSize(arg_0).z) - 1u));
  ivec2 v_4 = ivec2(min(v_1, (uvec2(imageSize(arg_0).xy) - uvec2(1u))));
  vec4 res = imageLoad(arg_0, ivec3(v_4, int(v_3)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_482627();
}
