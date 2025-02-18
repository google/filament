//
// fragment_main
//
#version 460
#extension GL_EXT_texture_shadow_lod: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
uniform highp samplerCubeArrayShadow arg_0_arg_1;
float textureSampleLevel_ae5e39() {
  vec3 arg_2 = vec3(1.0f);
  int arg_3 = 1;
  int arg_4 = 1;
  vec3 v_1 = arg_2;
  int v_2 = arg_4;
  vec4 v_3 = vec4(v_1, float(arg_3));
  float res = textureLod(arg_0_arg_1, v_3, 0.0f, float(v_2));
  return res;
}
void main() {
  v.inner = textureSampleLevel_ae5e39();
}
//
// compute_main
//
#version 460
#extension GL_EXT_texture_shadow_lod: require

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  float inner;
} v;
uniform highp samplerCubeArrayShadow arg_0_arg_1;
float textureSampleLevel_ae5e39() {
  vec3 arg_2 = vec3(1.0f);
  int arg_3 = 1;
  int arg_4 = 1;
  vec3 v_1 = arg_2;
  int v_2 = arg_4;
  vec4 v_3 = vec4(v_1, float(arg_3));
  float res = textureLod(arg_0_arg_1, v_3, 0.0f, float(v_2));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureSampleLevel_ae5e39();
}
//
// vertex_main
//
#version 460
#extension GL_EXT_texture_shadow_lod: require


struct VertexOutput {
  vec4 pos;
  float prevent_dce;
};

uniform highp samplerCubeArrayShadow arg_0_arg_1;
layout(location = 0) flat out float tint_interstage_location0;
float textureSampleLevel_ae5e39() {
  vec3 arg_2 = vec3(1.0f);
  int arg_3 = 1;
  int arg_4 = 1;
  vec3 v = arg_2;
  int v_1 = arg_4;
  vec4 v_2 = vec4(v, float(arg_3));
  float res = textureLod(arg_0_arg_1, v_2, 0.0f, float(v_1));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_3 = VertexOutput(vec4(0.0f), 0.0f);
  v_3.pos = vec4(0.0f);
  v_3.prevent_dce = textureSampleLevel_ae5e39();
  return v_3;
}
void main() {
  VertexOutput v_4 = vertex_main_inner();
  gl_Position = vec4(v_4.pos.x, -(v_4.pos.y), ((2.0f * v_4.pos.z) - v_4.pos.w), v_4.pos.w);
  tint_interstage_location0 = v_4.prevent_dce;
  gl_PointSize = 1.0f;
}
