//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec2 inner;
} v;
vec2 normalize_fc2ef1() {
  vec2 res = vec2(0.70710676908493041992f);
  return res;
}
void main() {
  v.inner = normalize_fc2ef1();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec2 inner;
} v;
vec2 normalize_fc2ef1() {
  vec2 res = vec2(0.70710676908493041992f);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = normalize_fc2ef1();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  vec2 prevent_dce;
};

layout(location = 0) flat out vec2 tint_interstage_location0;
vec2 normalize_fc2ef1() {
  vec2 res = vec2(0.70710676908493041992f);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), vec2(0.0f));
  v.pos = vec4(0.0f);
  v.prevent_dce = normalize_fc2ef1();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = vec4(v_1.pos.x, -(v_1.pos.y), ((2.0f * v_1.pos.z) - v_1.pos.w), v_1.pos.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
