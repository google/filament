//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

void tan_ae26ae() {
  vec3 res = vec3(1.55740773677825927734f);
}
void main() {
  tan_ae26ae();
}
//
// compute_main
//
#version 310 es

void tan_ae26ae() {
  vec3 res = vec3(1.55740773677825927734f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tan_ae26ae();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
};

void tan_ae26ae() {
  vec3 res = vec3(1.55740773677825927734f);
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f));
  v.pos = vec4(0.0f);
  tan_ae26ae();
  return v;
}
void main() {
  vec4 v_1 = vertex_main_inner().pos;
  gl_Position = vec4(v_1.x, -(v_1.y), ((2.0f * v_1.z) - v_1.w), v_1.w);
  gl_PointSize = 1.0f;
}
