//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

void atanh_7f2874() {
  vec3 res = vec3(0.54930615425109863281f);
}
void main() {
  atanh_7f2874();
}
//
// compute_main
//
#version 310 es

void atanh_7f2874() {
  vec3 res = vec3(0.54930615425109863281f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  atanh_7f2874();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
};

void atanh_7f2874() {
  vec3 res = vec3(0.54930615425109863281f);
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f));
  v.pos = vec4(0.0f);
  atanh_7f2874();
  return v;
}
void main() {
  vec4 v_1 = vertex_main_inner().pos;
  gl_Position = vec4(v_1.x, -(v_1.y), ((2.0f * v_1.z) - v_1.w), v_1.w);
  gl_PointSize = 1.0f;
}
