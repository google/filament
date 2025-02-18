//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v;
layout(binding = 0, rgba8) uniform highp readonly image2DArray arg_0;
vec4 textureLoad_89620b() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  ivec2 v_1 = arg_1;
  int v_2 = arg_2;
  uint v_3 = (uint(imageSize(arg_0).z) - 1u);
  uint v_4 = min(uint(v_2), v_3);
  uvec2 v_5 = (uvec2(imageSize(arg_0).xy) - uvec2(1u));
  ivec2 v_6 = ivec2(min(uvec2(v_1), v_5));
  vec4 res = imageLoad(arg_0, ivec3(v_6, int(v_4))).zyxw;
  return res;
}
void main() {
  v.inner = textureLoad_89620b();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 0, rgba8) uniform highp readonly image2DArray arg_0;
vec4 textureLoad_89620b() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  ivec2 v_1 = arg_1;
  int v_2 = arg_2;
  uint v_3 = (uint(imageSize(arg_0).z) - 1u);
  uint v_4 = min(uint(v_2), v_3);
  uvec2 v_5 = (uvec2(imageSize(arg_0).xy) - uvec2(1u));
  ivec2 v_6 = ivec2(min(uvec2(v_1), v_5));
  vec4 res = imageLoad(arg_0, ivec3(v_6, int(v_4))).zyxw;
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_89620b();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

layout(binding = 0, rgba8) uniform highp readonly image2DArray arg_0;
layout(location = 0) flat out vec4 tint_interstage_location0;
vec4 textureLoad_89620b() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  ivec2 v = arg_1;
  int v_1 = arg_2;
  uint v_2 = (uint(imageSize(arg_0).z) - 1u);
  uint v_3 = min(uint(v_1), v_2);
  uvec2 v_4 = (uvec2(imageSize(arg_0).xy) - uvec2(1u));
  ivec2 v_5 = ivec2(min(uvec2(v), v_4));
  vec4 res = imageLoad(arg_0, ivec3(v_5, int(v_3))).zyxw;
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_6 = VertexOutput(vec4(0.0f), vec4(0.0f));
  v_6.pos = vec4(0.0f);
  v_6.prevent_dce = textureLoad_89620b();
  return v_6;
}
void main() {
  VertexOutput v_7 = vertex_main_inner();
  gl_Position = vec4(v_7.pos.x, -(v_7.pos.y), ((2.0f * v_7.pos.z) - v_7.pos.w), v_7.pos.w);
  tint_interstage_location0 = v_7.prevent_dce;
  gl_PointSize = 1.0f;
}
