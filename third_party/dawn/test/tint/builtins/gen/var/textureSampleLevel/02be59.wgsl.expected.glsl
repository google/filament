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
uniform highp sampler2DShadow arg_0_arg_1;
float textureSampleLevel_02be59() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  int v_1 = arg_3;
  vec3 v_2 = vec3(arg_2, 0.0f);
  float res = textureLod(arg_0_arg_1, v_2, float(v_1));
  return res;
}
void main() {
  v.inner = textureSampleLevel_02be59();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  float inner;
} v;
uniform highp sampler2DShadow arg_0_arg_1;
float textureSampleLevel_02be59() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  int v_1 = arg_3;
  vec3 v_2 = vec3(arg_2, 0.0f);
  float res = textureLod(arg_0_arg_1, v_2, float(v_1));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureSampleLevel_02be59();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  float prevent_dce;
};

uniform highp sampler2DShadow arg_0_arg_1;
layout(location = 0) flat out float tint_interstage_location0;
float textureSampleLevel_02be59() {
  vec2 arg_2 = vec2(1.0f);
  int arg_3 = 1;
  int v = arg_3;
  vec3 v_1 = vec3(arg_2, 0.0f);
  float res = textureLod(arg_0_arg_1, v_1, float(v));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_2 = VertexOutput(vec4(0.0f), 0.0f);
  v_2.pos = vec4(0.0f);
  v_2.prevent_dce = textureSampleLevel_02be59();
  return v_2;
}
void main() {
  VertexOutput v_3 = vertex_main_inner();
  gl_Position = vec4(v_3.pos.x, -(v_3.pos.y), ((2.0f * v_3.pos.z) - v_3.pos.w), v_3.pos.w);
  tint_interstage_location0 = v_3.prevent_dce;
  gl_PointSize = 1.0f;
}
