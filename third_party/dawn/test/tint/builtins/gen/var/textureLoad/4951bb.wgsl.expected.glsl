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
vec4 textureLoad_4951bb() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  uvec2 v_1 = arg_1;
  int v_2 = arg_2;
  uint v_3 = (uint(imageSize(arg_0).z) - 1u);
  uint v_4 = min(uint(v_2), v_3);
  ivec2 v_5 = ivec2(min(v_1, (uvec2(imageSize(arg_0).xy) - uvec2(1u))));
  vec4 res = imageLoad(arg_0, ivec3(v_5, int(v_4)));
  return res;
}
void main() {
  v.inner = textureLoad_4951bb();
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
vec4 textureLoad_4951bb() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  uvec2 v_1 = arg_1;
  int v_2 = arg_2;
  uint v_3 = (uint(imageSize(arg_0).z) - 1u);
  uint v_4 = min(uint(v_2), v_3);
  ivec2 v_5 = ivec2(min(v_1, (uvec2(imageSize(arg_0).xy) - uvec2(1u))));
  vec4 res = imageLoad(arg_0, ivec3(v_5, int(v_4)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_4951bb();
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
vec4 textureLoad_4951bb() {
  uvec2 arg_1 = uvec2(1u);
  int arg_2 = 1;
  uvec2 v = arg_1;
  int v_1 = arg_2;
  uint v_2 = (uint(imageSize(arg_0).z) - 1u);
  uint v_3 = min(uint(v_1), v_2);
  ivec2 v_4 = ivec2(min(v, (uvec2(imageSize(arg_0).xy) - uvec2(1u))));
  vec4 res = imageLoad(arg_0, ivec3(v_4, int(v_3)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_5 = VertexOutput(vec4(0.0f), vec4(0.0f));
  v_5.pos = vec4(0.0f);
  v_5.prevent_dce = textureLoad_4951bb();
  return v_5;
}
void main() {
  VertexOutput v_6 = vertex_main_inner();
  gl_Position = vec4(v_6.pos.x, -(v_6.pos.y), ((2.0f * v_6.pos.z) - v_6.pos.w), v_6.pos.w);
  tint_interstage_location0 = v_6.prevent_dce;
  gl_PointSize = 1.0f;
}
