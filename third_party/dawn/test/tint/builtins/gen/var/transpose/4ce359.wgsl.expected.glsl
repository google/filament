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
int transpose_4ce359() {
  mat2x4 arg_0 = mat2x4(vec4(1.0f), vec4(1.0f));
  mat4x2 res = transpose(arg_0);
  return mix(0, 1, (res[0u].x == 0.0f));
}
void main() {
  v.inner = transpose_4ce359();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  int inner;
} v;
int transpose_4ce359() {
  mat2x4 arg_0 = mat2x4(vec4(1.0f), vec4(1.0f));
  mat4x2 res = transpose(arg_0);
  return mix(0, 1, (res[0u].x == 0.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = transpose_4ce359();
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
int transpose_4ce359() {
  mat2x4 arg_0 = mat2x4(vec4(1.0f), vec4(1.0f));
  mat4x2 res = transpose(arg_0);
  return mix(0, 1, (res[0u].x == 0.0f));
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), 0);
  v.pos = vec4(0.0f);
  v.prevent_dce = transpose_4ce359();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = vec4(v_1.pos.x, -(v_1.pos.y), ((2.0f * v_1.pos.z) - v_1.pos.w), v_1.pos.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
