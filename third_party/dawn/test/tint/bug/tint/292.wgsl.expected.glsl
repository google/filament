#version 310 es

vec4 main_inner() {
  vec3 light = vec3(1.20000004768371582031f, 1.0f, 2.0f);
  vec3 negative_light = -(light);
  return vec4(0.0f);
}
void main() {
  vec4 v = main_inner();
  gl_Position = vec4(v.x, -(v.y), ((2.0f * v.z) - v.w), v.w);
  gl_PointSize = 1.0f;
}
