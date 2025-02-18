//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

void select_3a14be() {
  bool arg_2 = true;
  ivec2 res = mix(ivec2(1), ivec2(1), bvec2(arg_2));
}
void main() {
  select_3a14be();
}
//
// compute_main
//
#version 310 es

void select_3a14be() {
  bool arg_2 = true;
  ivec2 res = mix(ivec2(1), ivec2(1), bvec2(arg_2));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  select_3a14be();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
};

void select_3a14be() {
  bool arg_2 = true;
  ivec2 res = mix(ivec2(1), ivec2(1), bvec2(arg_2));
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f));
  v.pos = vec4(0.0f);
  select_3a14be();
  return v;
}
void main() {
  vec4 v_1 = vertex_main_inner().pos;
  gl_Position = vec4(v_1.x, -(v_1.y), ((2.0f * v_1.z) - v_1.w), v_1.w);
  gl_PointSize = 1.0f;
}
