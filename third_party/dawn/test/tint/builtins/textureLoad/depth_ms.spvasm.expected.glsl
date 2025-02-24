//
// vertex_main
//
#version 310 es


struct vertex_main_out {
  vec4 tint_symbol_1_1;
};

vec4 tint_symbol_1 = vec4(0.0f);
uniform highp sampler2DMS arg_0;
void textureLoad_6273b1() {
  float res = 0.0f;
  uvec2 v = (uvec2(textureSize(arg_0)) - uvec2(1u));
  res = vec4(texelFetch(arg_0, ivec2(min(uvec2(ivec2(0)), v)), 1).x, 0.0f, 0.0f, 0.0f).x;
}
void tint_symbol_2(vec4 tint_symbol) {
  tint_symbol_1 = tint_symbol;
}
void vertex_main_1() {
  textureLoad_6273b1();
  tint_symbol_2(vec4(0.0f));
}
vertex_main_out vertex_main_inner() {
  vertex_main_1();
  return vertex_main_out(tint_symbol_1);
}
void main() {
  vec4 v_1 = vertex_main_inner().tint_symbol_1_1;
  gl_Position = vec4(v_1.x, -(v_1.y), ((2.0f * v_1.z) - v_1.w), v_1.w);
  gl_PointSize = 1.0f;
}
//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2DMS arg_0;
void textureLoad_6273b1() {
  float res = 0.0f;
  uvec2 v = (uvec2(textureSize(arg_0)) - uvec2(1u));
  res = vec4(texelFetch(arg_0, ivec2(min(uvec2(ivec2(0)), v)), 1).x, 0.0f, 0.0f, 0.0f).x;
}
void fragment_main_1() {
  textureLoad_6273b1();
}
void main() {
  fragment_main_1();
}
//
// compute_main
//
#version 310 es

uniform highp sampler2DMS arg_0;
void textureLoad_6273b1() {
  float res = 0.0f;
  uvec2 v = (uvec2(textureSize(arg_0)) - uvec2(1u));
  res = vec4(texelFetch(arg_0, ivec2(min(uvec2(ivec2(0)), v)), 1).x, 0.0f, 0.0f, 0.0f).x;
}
void compute_main_1() {
  textureLoad_6273b1();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_1();
}
