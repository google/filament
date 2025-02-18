//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;


struct frexp_result_f32 {
  float member_0;
  int member_1;
};

void frexp_bee870() {
  frexp_result_f32 res = frexp_result_f32(0.5f, 1);
}
void main() {
  frexp_bee870();
}
//
// compute_main
//
#version 310 es


struct frexp_result_f32 {
  float member_0;
  int member_1;
};

void frexp_bee870() {
  frexp_result_f32 res = frexp_result_f32(0.5f, 1);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_bee870();
}
//
// vertex_main
//
#version 310 es


struct frexp_result_f32 {
  float member_0;
  int member_1;
};

struct VertexOutput {
  vec4 pos;
};

void frexp_bee870() {
  frexp_result_f32 res = frexp_result_f32(0.5f, 1);
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f));
  v.pos = vec4(0.0f);
  frexp_bee870();
  return v;
}
void main() {
  vec4 v_1 = vertex_main_inner().pos;
  gl_Position = vec4(v_1.x, -(v_1.y), ((2.0f * v_1.z) - v_1.w), v_1.w);
  gl_PointSize = 1.0f;
}
