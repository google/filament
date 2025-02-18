//
// fragment_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct modf_result_f16 {
  float16_t member_0;
  float16_t whole;
};

void modf_8dbbbf() {
  float16_t arg_0 = -1.5hf;
  modf_result_f16 v = modf_result_f16(0.0hf, 0.0hf);
  v.member_0 = modf(arg_0, v.whole);
  modf_result_f16 res = v;
}
void main() {
  modf_8dbbbf();
}
//
// compute_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct modf_result_f16 {
  float16_t member_0;
  float16_t whole;
};

void modf_8dbbbf() {
  float16_t arg_0 = -1.5hf;
  modf_result_f16 v = modf_result_f16(0.0hf, 0.0hf);
  v.member_0 = modf(arg_0, v.whole);
  modf_result_f16 res = v;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_8dbbbf();
}
//
// vertex_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct modf_result_f16 {
  float16_t member_0;
  float16_t whole;
};

struct VertexOutput {
  vec4 pos;
};

void modf_8dbbbf() {
  float16_t arg_0 = -1.5hf;
  modf_result_f16 v = modf_result_f16(0.0hf, 0.0hf);
  v.member_0 = modf(arg_0, v.whole);
  modf_result_f16 res = v;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_1 = VertexOutput(vec4(0.0f));
  v_1.pos = vec4(0.0f);
  modf_8dbbbf();
  return v_1;
}
void main() {
  vec4 v_2 = vertex_main_inner().pos;
  gl_Position = vec4(v_2.x, -(v_2.y), ((2.0f * v_2.z) - v_2.w), v_2.w);
  gl_PointSize = 1.0f;
}
