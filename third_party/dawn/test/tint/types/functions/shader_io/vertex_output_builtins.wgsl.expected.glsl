#version 310 es

vec4 main_inner() {
  return vec4(1.0f, 2.0f, 3.0f, 4.0f);
}
void main() {
  vec4 v = main_inner();
  gl_Position = vec4(v.x, -(v.y), ((2.0f * v.z) - v.w), v.w);
  gl_PointSize = 1.0f;
}
