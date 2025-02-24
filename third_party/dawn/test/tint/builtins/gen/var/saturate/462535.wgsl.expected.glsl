//
// fragment_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  f16vec3 inner;
} v;
f16vec3 saturate_462535() {
  f16vec3 arg_0 = f16vec3(2.0hf);
  f16vec3 res = clamp(arg_0, f16vec3(0.0hf), f16vec3(1.0hf));
  return res;
}
void main() {
  v.inner = saturate_462535();
}
//
// compute_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  f16vec3 inner;
} v;
f16vec3 saturate_462535() {
  f16vec3 arg_0 = f16vec3(2.0hf);
  f16vec3 res = clamp(arg_0, f16vec3(0.0hf), f16vec3(1.0hf));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = saturate_462535();
}
//
// vertex_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct VertexOutput {
  vec4 pos;
  f16vec3 prevent_dce;
};

layout(location = 0) flat out f16vec3 tint_interstage_location0;
f16vec3 saturate_462535() {
  f16vec3 arg_0 = f16vec3(2.0hf);
  f16vec3 res = clamp(arg_0, f16vec3(0.0hf), f16vec3(1.0hf));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), f16vec3(0.0hf));
  v.pos = vec4(0.0f);
  v.prevent_dce = saturate_462535();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = vec4(v_1.pos.x, -(v_1.pos.y), ((2.0f * v_1.pos.z) - v_1.pos.w), v_1.pos.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
