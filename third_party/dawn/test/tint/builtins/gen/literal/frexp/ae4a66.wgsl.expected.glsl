//
// fragment_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require
precision highp float;
precision highp int;


struct frexp_result_vec3_f16 {
  f16vec3 member_0;
  ivec3 member_1;
};

void frexp_ae4a66() {
  frexp_result_vec3_f16 res = frexp_result_vec3_f16(f16vec3(0.5hf), ivec3(1));
}
void main() {
  frexp_ae4a66();
}
//
// compute_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct frexp_result_vec3_f16 {
  f16vec3 member_0;
  ivec3 member_1;
};

void frexp_ae4a66() {
  frexp_result_vec3_f16 res = frexp_result_vec3_f16(f16vec3(0.5hf), ivec3(1));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_ae4a66();
}
//
// vertex_main
//
#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct frexp_result_vec3_f16 {
  f16vec3 member_0;
  ivec3 member_1;
};

struct VertexOutput {
  vec4 pos;
};

void frexp_ae4a66() {
  frexp_result_vec3_f16 res = frexp_result_vec3_f16(f16vec3(0.5hf), ivec3(1));
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f));
  v.pos = vec4(0.0f);
  frexp_ae4a66();
  return v;
}
void main() {
  vec4 v_1 = vertex_main_inner().pos;
  gl_Position = vec4(v_1.x, -(v_1.y), ((2.0f * v_1.z) - v_1.w), v_1.w);
  gl_PointSize = 1.0f;
}
