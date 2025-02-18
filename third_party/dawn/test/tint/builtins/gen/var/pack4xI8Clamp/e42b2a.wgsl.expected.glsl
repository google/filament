//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uint inner;
} v;
uint tint_int_dot(uvec4 x, uvec4 y) {
  return ((((x.x * y.x) + (x.y * y.y)) + (x.z * y.z)) + (x.w * y.w));
}
uint pack4xI8Clamp_e42b2a() {
  ivec4 arg_0 = ivec4(1);
  ivec4 v_1 = arg_0;
  uvec4 v_2 = uvec4(0u, 8u, 16u, 24u);
  ivec4 v_3 = ivec4(-128);
  uvec4 v_4 = uvec4(clamp(v_1, v_3, ivec4(127)));
  uvec4 v_5 = ((v_4 & uvec4(255u)) << v_2);
  uint res = tint_int_dot(v_5, uvec4(1u));
  return res;
}
void main() {
  v.inner = pack4xI8Clamp_e42b2a();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
uint tint_int_dot(uvec4 x, uvec4 y) {
  return ((((x.x * y.x) + (x.y * y.y)) + (x.z * y.z)) + (x.w * y.w));
}
uint pack4xI8Clamp_e42b2a() {
  ivec4 arg_0 = ivec4(1);
  ivec4 v_1 = arg_0;
  uvec4 v_2 = uvec4(0u, 8u, 16u, 24u);
  ivec4 v_3 = ivec4(-128);
  uvec4 v_4 = uvec4(clamp(v_1, v_3, ivec4(127)));
  uvec4 v_5 = ((v_4 & uvec4(255u)) << v_2);
  uint res = tint_int_dot(v_5, uvec4(1u));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = pack4xI8Clamp_e42b2a();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

layout(location = 0) flat out uint tint_interstage_location0;
uint tint_int_dot(uvec4 x, uvec4 y) {
  return ((((x.x * y.x) + (x.y * y.y)) + (x.z * y.z)) + (x.w * y.w));
}
uint pack4xI8Clamp_e42b2a() {
  ivec4 arg_0 = ivec4(1);
  ivec4 v = arg_0;
  uvec4 v_1 = uvec4(0u, 8u, 16u, 24u);
  ivec4 v_2 = ivec4(-128);
  uvec4 v_3 = uvec4(clamp(v, v_2, ivec4(127)));
  uvec4 v_4 = ((v_3 & uvec4(255u)) << v_1);
  uint res = tint_int_dot(v_4, uvec4(1u));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_5 = VertexOutput(vec4(0.0f), 0u);
  v_5.pos = vec4(0.0f);
  v_5.prevent_dce = pack4xI8Clamp_e42b2a();
  return v_5;
}
void main() {
  VertexOutput v_6 = vertex_main_inner();
  gl_Position = vec4(v_6.pos.x, -(v_6.pos.y), ((2.0f * v_6.pos.z) - v_6.pos.w), v_6.pos.w);
  tint_interstage_location0 = v_6.prevent_dce;
  gl_PointSize = 1.0f;
}
