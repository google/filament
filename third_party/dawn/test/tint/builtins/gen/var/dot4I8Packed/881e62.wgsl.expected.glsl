//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  int inner;
} v;
int tint_int_dot(ivec4 x, ivec4 y) {
  return ((((x.x * y.x) + (x.y * y.y)) + (x.z * y.z)) + (x.w * y.w));
}
int dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v_1 = arg_0;
  uint v_2 = arg_1;
  uvec4 v_3 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_4 = ivec4((uvec4(v_1) << v_3));
  ivec4 v_5 = (v_4 >> uvec4(24u));
  uvec4 v_6 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_7 = ivec4((uvec4(v_2) << v_6));
  int res = tint_int_dot(v_5, (v_7 >> uvec4(24u)));
  return res;
}
void main() {
  v.inner = dot4I8Packed_881e62();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  int inner;
} v;
int tint_int_dot(ivec4 x, ivec4 y) {
  return ((((x.x * y.x) + (x.y * y.y)) + (x.z * y.z)) + (x.w * y.w));
}
int dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v_1 = arg_0;
  uint v_2 = arg_1;
  uvec4 v_3 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_4 = ivec4((uvec4(v_1) << v_3));
  ivec4 v_5 = (v_4 >> uvec4(24u));
  uvec4 v_6 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_7 = ivec4((uvec4(v_2) << v_6));
  int res = tint_int_dot(v_5, (v_7 >> uvec4(24u)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = dot4I8Packed_881e62();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

layout(location = 0) flat out int tint_interstage_location0;
int tint_int_dot(ivec4 x, ivec4 y) {
  return ((((x.x * y.x) + (x.y * y.y)) + (x.z * y.z)) + (x.w * y.w));
}
int dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  uint v = arg_0;
  uint v_1 = arg_1;
  uvec4 v_2 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_3 = ivec4((uvec4(v) << v_2));
  ivec4 v_4 = (v_3 >> uvec4(24u));
  uvec4 v_5 = uvec4(24u, 16u, 8u, 0u);
  ivec4 v_6 = ivec4((uvec4(v_1) << v_5));
  int res = tint_int_dot(v_4, (v_6 >> uvec4(24u)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_7 = VertexOutput(vec4(0.0f), 0);
  v_7.pos = vec4(0.0f);
  v_7.prevent_dce = dot4I8Packed_881e62();
  return v_7;
}
void main() {
  VertexOutput v_8 = vertex_main_inner();
  gl_Position = vec4(v_8.pos.x, -(v_8.pos.y), ((2.0f * v_8.pos.z) - v_8.pos.w), v_8.pos.w);
  tint_interstage_location0 = v_8.prevent_dce;
  gl_PointSize = 1.0f;
}
