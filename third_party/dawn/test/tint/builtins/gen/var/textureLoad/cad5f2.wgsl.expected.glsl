//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uvec4 inner;
} v;
uniform highp usampler2DMS arg_0;
uvec4 textureLoad_cad5f2() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  ivec2 v_1 = arg_1;
  uint v_2 = arg_2;
  uvec2 v_3 = (uvec2(textureSize(arg_0)) - uvec2(1u));
  ivec2 v_4 = ivec2(min(uvec2(v_1), v_3));
  uvec4 res = texelFetch(arg_0, v_4, int(v_2));
  return res;
}
void main() {
  v.inner = textureLoad_cad5f2();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
uniform highp usampler2DMS arg_0;
uvec4 textureLoad_cad5f2() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  ivec2 v_1 = arg_1;
  uint v_2 = arg_2;
  uvec2 v_3 = (uvec2(textureSize(arg_0)) - uvec2(1u));
  ivec2 v_4 = ivec2(min(uvec2(v_1), v_3));
  uvec4 res = texelFetch(arg_0, v_4, int(v_2));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_cad5f2();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec4 prevent_dce;
};

uniform highp usampler2DMS arg_0;
layout(location = 0) flat out uvec4 tint_interstage_location0;
uvec4 textureLoad_cad5f2() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  ivec2 v = arg_1;
  uint v_1 = arg_2;
  uvec2 v_2 = (uvec2(textureSize(arg_0)) - uvec2(1u));
  ivec2 v_3 = ivec2(min(uvec2(v), v_2));
  uvec4 res = texelFetch(arg_0, v_3, int(v_1));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_4 = VertexOutput(vec4(0.0f), uvec4(0u));
  v_4.pos = vec4(0.0f);
  v_4.prevent_dce = textureLoad_cad5f2();
  return v_4;
}
void main() {
  VertexOutput v_5 = vertex_main_inner();
  gl_Position = vec4(v_5.pos.x, -(v_5.pos.y), ((2.0f * v_5.pos.z) - v_5.pos.w), v_5.pos.w);
  tint_interstage_location0 = v_5.prevent_dce;
  gl_PointSize = 1.0f;
}
