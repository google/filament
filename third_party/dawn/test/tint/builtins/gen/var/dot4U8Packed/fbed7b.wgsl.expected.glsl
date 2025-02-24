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
uint dot4U8Packed_fbed7b() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v_1 = arg_0;
  uint v_2 = arg_1;
  uvec4 v_3 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_4 = (uvec4(v_1) >> v_3);
  uvec4 v_5 = (v_4 & uvec4(255u));
  uvec4 v_6 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_7 = (uvec4(v_2) >> v_6);
  uint res = tint_int_dot(v_5, (v_7 & uvec4(255u)));
  return res;
}
void main() {
  v.inner = dot4U8Packed_fbed7b();
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
uint dot4U8Packed_fbed7b() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v_1 = arg_0;
  uint v_2 = arg_1;
  uvec4 v_3 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_4 = (uvec4(v_1) >> v_3);
  uvec4 v_5 = (v_4 & uvec4(255u));
  uvec4 v_6 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_7 = (uvec4(v_2) >> v_6);
  uint res = tint_int_dot(v_5, (v_7 & uvec4(255u)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = dot4U8Packed_fbed7b();
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
uint dot4U8Packed_fbed7b() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v = arg_0;
  uint v_1 = arg_1;
  uvec4 v_2 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_3 = (uvec4(v) >> v_2);
  uvec4 v_4 = (v_3 & uvec4(255u));
  uvec4 v_5 = uvec4(0u, 8u, 16u, 24u);
  uvec4 v_6 = (uvec4(v_1) >> v_5);
  uint res = tint_int_dot(v_4, (v_6 & uvec4(255u)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_7 = VertexOutput(vec4(0.0f), 0u);
  v_7.pos = vec4(0.0f);
  v_7.prevent_dce = dot4U8Packed_fbed7b();
  return v_7;
}
void main() {
  VertexOutput v_8 = vertex_main_inner();
  gl_Position = vec4(v_8.pos.x, -(v_8.pos.y), ((2.0f * v_8.pos.z) - v_8.pos.w), v_8.pos.w);
  tint_interstage_location0 = v_8.prevent_dce;
  gl_PointSize = 1.0f;
}
