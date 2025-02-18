//
// fragment_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uint inner;
} v;
layout(binding = 1, std430)
buffer f_SB_RO_ssbo {
  float16_t arg_0[];
} sb_ro;
uint arrayLength_9ea96d() {
  uint res = uint(sb_ro.arg_0.length());
  return res;
}
void main() {
  v.inner = arrayLength_9ea96d();
}
//
// compute_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
layout(binding = 1, std430)
buffer SB_RO_1_ssbo {
  float16_t arg_0[];
} sb_ro;
uint arrayLength_9ea96d() {
  uint res = uint(sb_ro.arg_0.length());
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = arrayLength_9ea96d();
}
//
// vertex_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

layout(binding = 1, std430)
buffer v_SB_RO_ssbo {
  float16_t arg_0[];
} sb_ro;
layout(location = 0) flat out uint tint_interstage_location0;
uint arrayLength_9ea96d() {
  uint res = uint(sb_ro.arg_0.length());
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), 0u);
  v.pos = vec4(0.0f);
  v.prevent_dce = arrayLength_9ea96d();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = vec4(v_1.pos.x, -(v_1.pos.y), ((2.0f * v_1.pos.z) - v_1.pos.w), v_1.pos.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
