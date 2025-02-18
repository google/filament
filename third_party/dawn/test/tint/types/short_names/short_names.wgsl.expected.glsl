#version 310 es

vec4 main_inner(uint VertexIndex) {
  return vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
void main() {
  vec4 v = main_inner(uint(gl_VertexID));
  gl_Position = vec4(v.x, -(v.y), ((2.0f * v.z) - v.w), v.w);
  gl_PointSize = 1.0f;
}
