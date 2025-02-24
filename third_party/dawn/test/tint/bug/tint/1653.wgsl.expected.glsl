#version 310 es

vec4 vs_main_inner(uint in_vertex_index) {
  return vec4[3](vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f))[min(in_vertex_index, 2u)];
}
void main() {
  vec4 v = vs_main_inner(uint(gl_VertexID));
  gl_Position = vec4(v.x, -(v.y), ((2.0f * v.z) - v.w), v.w);
  gl_PointSize = 1.0f;
}
