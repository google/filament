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
vec4 textureLoad_af0507() {
  uint v_1 = min(1u, (uint(imageSize(arg_0).z) - 1u));
  uvec2 v_2 = (uvec2(imageSize(arg_0).xy) - uvec2(1u));
  ivec2 v_3 = ivec2(min(uvec2(ivec2(1)), v_2));
  vec4 res = imageLoad(arg_0, ivec3(v_3, int(v_1)));
  return res;
}
void main() {
  v.inner = textureLoad_af0507();
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
vec4 textureLoad_af0507() {
  uint v_1 = min(1u, (uint(imageSize(arg_0).z) - 1u));
  uvec2 v_2 = (uvec2(imageSize(arg_0).xy) - uvec2(1u));
  ivec2 v_3 = ivec2(min(uvec2(ivec2(1)), v_2));
  vec4 res = imageLoad(arg_0, ivec3(v_3, int(v_1)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_af0507();
}
