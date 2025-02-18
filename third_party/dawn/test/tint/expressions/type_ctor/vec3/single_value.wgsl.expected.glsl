#version 310 es

vec3 v() {
  return vec3(0.0f);
}
void f() {
  vec3 a = vec3(1.0f);
  vec3 b = vec3(a);
  vec3 c = vec3(v());
  vec3 d = vec3((a * 2.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
