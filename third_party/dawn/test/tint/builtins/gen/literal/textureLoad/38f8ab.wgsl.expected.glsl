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
uniform highp isampler2DMS arg_0;
ivec4 textureLoad_38f8ab() {
  uvec2 v_1 = (uvec2(textureSize(arg_0)) - uvec2(1u));
  ivec2 v_2 = ivec2(min(uvec2(ivec2(1)), v_1));
  ivec4 res = texelFetch(arg_0, v_2, int(1u));
  return res;
}
void main() {
  v.inner = textureLoad_38f8ab();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  ivec4 inner;
} v;
uniform highp isampler2DMS arg_0;
ivec4 textureLoad_38f8ab() {
  uvec2 v_1 = (uvec2(textureSize(arg_0)) - uvec2(1u));
  ivec2 v_2 = ivec2(min(uvec2(ivec2(1)), v_1));
  ivec4 res = texelFetch(arg_0, v_2, int(1u));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_38f8ab();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec4 prevent_dce;
};

uniform highp isampler2DMS arg_0;
layout(location = 0) flat out ivec4 tint_interstage_location0;
ivec4 textureLoad_38f8ab() {
  uvec2 v = (uvec2(textureSize(arg_0)) - uvec2(1u));
  ivec2 v_1 = ivec2(min(uvec2(ivec2(1)), v));
  ivec4 res = texelFetch(arg_0, v_1, int(1u));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_2 = VertexOutput(vec4(0.0f), ivec4(0));
  v_2.pos = vec4(0.0f);
  v_2.prevent_dce = textureLoad_38f8ab();
  return v_2;
}
void main() {
  VertexOutput v_3 = vertex_main_inner();
  gl_Position = vec4(v_3.pos.x, -(v_3.pos.y), ((2.0f * v_3.pos.z) - v_3.pos.w), v_3.pos.w);
  tint_interstage_location0 = v_3.prevent_dce;
  gl_PointSize = 1.0f;
}
