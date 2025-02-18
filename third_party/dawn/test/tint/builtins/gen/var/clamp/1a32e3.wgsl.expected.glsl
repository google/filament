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
ivec4 clamp_1a32e3() {
  ivec4 arg_0 = ivec4(1);
  ivec4 arg_1 = ivec4(1);
  ivec4 arg_2 = ivec4(1);
  ivec4 res = min(max(arg_0, arg_1), arg_2);
  return res;
}
void main() {
  v.inner = clamp_1a32e3();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  ivec4 inner;
} v;
ivec4 clamp_1a32e3() {
  ivec4 arg_0 = ivec4(1);
  ivec4 arg_1 = ivec4(1);
  ivec4 arg_2 = ivec4(1);
  ivec4 res = min(max(arg_0, arg_1), arg_2);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = clamp_1a32e3();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec4 prevent_dce;
};

layout(location = 0) flat out ivec4 tint_interstage_location0;
ivec4 clamp_1a32e3() {
  ivec4 arg_0 = ivec4(1);
  ivec4 arg_1 = ivec4(1);
  ivec4 arg_2 = ivec4(1);
  ivec4 res = min(max(arg_0, arg_1), arg_2);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), ivec4(0));
  v.pos = vec4(0.0f);
  v.prevent_dce = clamp_1a32e3();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = vec4(v_1.pos.x, -(v_1.pos.y), ((2.0f * v_1.pos.z) - v_1.pos.w), v_1.pos.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
