//
// fragment_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float16_t inner;
} v;
float16_t smoothstep_586e12() {
  float16_t arg_0 = 2.0hf;
  float16_t arg_1 = 4.0hf;
  float16_t arg_2 = 3.0hf;
  float16_t v_1 = arg_0;
  float16_t v_2 = clamp(((arg_2 - v_1) / (arg_1 - v_1)), 0.0hf, 1.0hf);
  float16_t res = (v_2 * (v_2 * (3.0hf - (2.0hf * v_2))));
  return res;
}
void main() {
  v.inner = smoothstep_586e12();
}
//
// compute_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  float16_t inner;
} v;
float16_t smoothstep_586e12() {
  float16_t arg_0 = 2.0hf;
  float16_t arg_1 = 4.0hf;
  float16_t arg_2 = 3.0hf;
  float16_t v_1 = arg_0;
  float16_t v_2 = clamp(((arg_2 - v_1) / (arg_1 - v_1)), 0.0hf, 1.0hf);
  float16_t res = (v_2 * (v_2 * (3.0hf - (2.0hf * v_2))));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = smoothstep_586e12();
}
//
// vertex_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct VertexOutput {
  vec4 pos;
  float16_t prevent_dce;
};

layout(location = 0) flat out float16_t tint_interstage_location0;
float16_t smoothstep_586e12() {
  float16_t arg_0 = 2.0hf;
  float16_t arg_1 = 4.0hf;
  float16_t arg_2 = 3.0hf;
  float16_t v = arg_0;
  float16_t v_1 = clamp(((arg_2 - v) / (arg_1 - v)), 0.0hf, 1.0hf);
  float16_t res = (v_1 * (v_1 * (3.0hf - (2.0hf * v_1))));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_2 = VertexOutput(vec4(0.0f), 0.0hf);
  v_2.pos = vec4(0.0f);
  v_2.prevent_dce = smoothstep_586e12();
  return v_2;
}
void main() {
  VertexOutput v_3 = vertex_main_inner();
  gl_Position = vec4(v_3.pos.x, -(v_3.pos.y), ((2.0f * v_3.pos.z) - v_3.pos.w), v_3.pos.w);
  tint_interstage_location0 = v_3.prevent_dce;
  gl_PointSize = 1.0f;
}
