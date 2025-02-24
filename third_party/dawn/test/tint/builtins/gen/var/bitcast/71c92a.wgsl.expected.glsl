//
// fragment_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  f16vec4 inner;
} v;
f16vec4 tint_bitcast_to_f16(ivec2 src) {
  uvec2 v_1 = uvec2(src);
  return f16vec4(unpackFloat2x16(v_1.x), unpackFloat2x16(v_1.y));
}
f16vec4 bitcast_71c92a() {
  ivec2 arg_0 = ivec2(1);
  f16vec4 res = tint_bitcast_to_f16(arg_0);
  return res;
}
void main() {
  v.inner = bitcast_71c92a();
}
//
// compute_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  f16vec4 inner;
} v;
f16vec4 tint_bitcast_to_f16(ivec2 src) {
  uvec2 v_1 = uvec2(src);
  return f16vec4(unpackFloat2x16(v_1.x), unpackFloat2x16(v_1.y));
}
f16vec4 bitcast_71c92a() {
  ivec2 arg_0 = ivec2(1);
  f16vec4 res = tint_bitcast_to_f16(arg_0);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = bitcast_71c92a();
}
//
// vertex_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct VertexOutput {
  vec4 pos;
  f16vec4 prevent_dce;
};

layout(location = 0) flat out f16vec4 tint_interstage_location0;
f16vec4 tint_bitcast_to_f16(ivec2 src) {
  uvec2 v = uvec2(src);
  return f16vec4(unpackFloat2x16(v.x), unpackFloat2x16(v.y));
}
f16vec4 bitcast_71c92a() {
  ivec2 arg_0 = ivec2(1);
  f16vec4 res = tint_bitcast_to_f16(arg_0);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_1 = VertexOutput(vec4(0.0f), f16vec4(0.0hf));
  v_1.pos = vec4(0.0f);
  v_1.prevent_dce = bitcast_71c92a();
  return v_1;
}
void main() {
  VertexOutput v_2 = vertex_main_inner();
  gl_Position = vec4(v_2.pos.x, -(v_2.pos.y), ((2.0f * v_2.pos.z) - v_2.pos.w), v_2.pos.w);
  tint_interstage_location0 = v_2.prevent_dce;
  gl_PointSize = 1.0f;
}
