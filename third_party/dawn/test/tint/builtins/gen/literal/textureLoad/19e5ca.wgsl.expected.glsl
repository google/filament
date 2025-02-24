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
layout(binding = 0, r8) uniform highp readonly image2DArray arg_0;
vec4 textureLoad_19e5ca() {
  uint v_1 = min(1u, (uint(imageSize(arg_0).z) - 1u));
  uvec2 v_2 = (uvec2(imageSize(arg_0).xy) - uvec2(1u));
  ivec2 v_3 = ivec2(min(uvec2(ivec2(1)), v_2));
  vec4 res = imageLoad(arg_0, ivec3(v_3, int(v_1)));
  return res;
}
void main() {
  v.inner = textureLoad_19e5ca();
}
//
// compute_main
//
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 0, r8) uniform highp readonly image2DArray arg_0;
vec4 textureLoad_19e5ca() {
  uint v_1 = min(1u, (uint(imageSize(arg_0).z) - 1u));
  uvec2 v_2 = (uvec2(imageSize(arg_0).xy) - uvec2(1u));
  ivec2 v_3 = ivec2(min(uvec2(ivec2(1)), v_2));
  vec4 res = imageLoad(arg_0, ivec3(v_3, int(v_1)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_19e5ca();
}
//
// vertex_main
//
#version 460


struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

layout(binding = 0, r8) uniform highp readonly image2DArray arg_0;
layout(location = 0) flat out vec4 tint_interstage_location0;
vec4 textureLoad_19e5ca() {
  uint v = min(1u, (uint(imageSize(arg_0).z) - 1u));
  uvec2 v_1 = (uvec2(imageSize(arg_0).xy) - uvec2(1u));
  ivec2 v_2 = ivec2(min(uvec2(ivec2(1)), v_1));
  vec4 res = imageLoad(arg_0, ivec3(v_2, int(v)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_3 = VertexOutput(vec4(0.0f), vec4(0.0f));
  v_3.pos = vec4(0.0f);
  v_3.prevent_dce = textureLoad_19e5ca();
  return v_3;
}
void main() {
  VertexOutput v_4 = vertex_main_inner();
  gl_Position = vec4(v_4.pos.x, -(v_4.pos.y), ((2.0f * v_4.pos.z) - v_4.pos.w), v_4.pos.w);
  tint_interstage_location0 = v_4.prevent_dce;
  gl_PointSize = 1.0f;
}
