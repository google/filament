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
int select_fb7e53() {
  bvec2 arg_0 = bvec2(true);
  bvec2 arg_1 = bvec2(true);
  bool arg_2 = true;
  bvec2 v_1 = arg_0;
  bvec2 v_2 = arg_1;
  bvec2 res = mix(v_1, v_2, bvec2(arg_2));
  return mix(0, 1, all(equal(res, bvec2(false))));
}
void main() {
  v.inner = select_fb7e53();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  int inner;
} v;
int select_fb7e53() {
  bvec2 arg_0 = bvec2(true);
  bvec2 arg_1 = bvec2(true);
  bool arg_2 = true;
  bvec2 v_1 = arg_0;
  bvec2 v_2 = arg_1;
  bvec2 res = mix(v_1, v_2, bvec2(arg_2));
  return mix(0, 1, all(equal(res, bvec2(false))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = select_fb7e53();
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
int select_fb7e53() {
  bvec2 arg_0 = bvec2(true);
  bvec2 arg_1 = bvec2(true);
  bool arg_2 = true;
  bvec2 v = arg_0;
  bvec2 v_1 = arg_1;
  bvec2 res = mix(v, v_1, bvec2(arg_2));
  return mix(0, 1, all(equal(res, bvec2(false))));
}
VertexOutput vertex_main_inner() {
  VertexOutput v_2 = VertexOutput(vec4(0.0f), 0);
  v_2.pos = vec4(0.0f);
  v_2.prevent_dce = select_fb7e53();
  return v_2;
}
void main() {
  VertexOutput v_3 = vertex_main_inner();
  gl_Position = vec4(v_3.pos.x, -(v_3.pos.y), ((2.0f * v_3.pos.z) - v_3.pos.w), v_3.pos.w);
  tint_interstage_location0 = v_3.prevent_dce;
  gl_PointSize = 1.0f;
}
