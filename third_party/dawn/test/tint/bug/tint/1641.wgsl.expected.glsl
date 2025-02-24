#version 310 es


struct Normals {
  vec3 f;
};

vec4 main_inner() {
  int zero = 0;
  return vec4(Normals[1](Normals(vec3(0.0f, 0.0f, 1.0f)))[min(uint(zero), 0u)].f, 1.0f);
}
void main() {
  vec4 v = main_inner();
  gl_Position = vec4(v.x, -(v.y), ((2.0f * v.z) - v.w), v.w);
  gl_PointSize = 1.0f;
}
