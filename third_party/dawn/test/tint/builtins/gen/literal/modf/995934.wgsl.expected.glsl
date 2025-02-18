//
// fragment_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct modf_result_vec4_f16 {
  f16vec4 member_0;
  f16vec4 whole;
};

void modf_995934() {
  modf_result_vec4_f16 res = modf_result_vec4_f16(f16vec4(-0.5hf), f16vec4(-1.0hf));
}
void main() {
  modf_995934();
}
//
// compute_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct modf_result_vec4_f16 {
  f16vec4 member_0;
  f16vec4 whole;
};

void modf_995934() {
  modf_result_vec4_f16 res = modf_result_vec4_f16(f16vec4(-0.5hf), f16vec4(-1.0hf));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_995934();
}
//
// vertex_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct modf_result_vec4_f16 {
  f16vec4 member_0;
  f16vec4 whole;
};

struct VertexOutput {
  vec4 pos;
};

void modf_995934() {
  modf_result_vec4_f16 res = modf_result_vec4_f16(f16vec4(-0.5hf), f16vec4(-1.0hf));
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f));
  v.pos = vec4(0.0f);
  modf_995934();
  return v;
}
void main() {
  vec4 v_1 = vertex_main_inner().pos;
  gl_Position = vec4(v_1.x, -(v_1.y), ((2.0f * v_1.z) - v_1.w), v_1.w);
  gl_PointSize = 1.0f;
}
