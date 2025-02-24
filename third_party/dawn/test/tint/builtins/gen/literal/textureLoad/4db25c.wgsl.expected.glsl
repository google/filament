//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
uniform highp sampler2DMS arg_0;
float textureLoad_4db25c() {
  ivec2 v_1 = ivec2(min(uvec2(1u), (uvec2(textureSize(arg_0)) - uvec2(1u))));
  float res = texelFetch(arg_0, v_1, int(1u)).x;
  return res;
}
void main() {
  v.inner = textureLoad_4db25c();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  float inner;
} v;
uniform highp sampler2DMS arg_0;
float textureLoad_4db25c() {
  ivec2 v_1 = ivec2(min(uvec2(1u), (uvec2(textureSize(arg_0)) - uvec2(1u))));
  float res = texelFetch(arg_0, v_1, int(1u)).x;
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_4db25c();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  float prevent_dce;
};

uniform highp sampler2DMS arg_0;
layout(location = 0) flat out float tint_interstage_location0;
float textureLoad_4db25c() {
  ivec2 v = ivec2(min(uvec2(1u), (uvec2(textureSize(arg_0)) - uvec2(1u))));
  float res = texelFetch(arg_0, v, int(1u)).x;
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_1 = VertexOutput(vec4(0.0f), 0.0f);
  v_1.pos = vec4(0.0f);
  v_1.prevent_dce = textureLoad_4db25c();
  return v_1;
}
void main() {
  VertexOutput v_2 = vertex_main_inner();
  gl_Position = vec4(v_2.pos.x, -(v_2.pos.y), ((2.0f * v_2.pos.z) - v_2.pos.w), v_2.pos.w);
  tint_interstage_location0 = v_2.prevent_dce;
  gl_PointSize = 1.0f;
}
