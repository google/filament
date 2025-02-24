#version 310 es

vec4 v() {
  return vec4(0.0f);
}
void f() {
  vec4 a = vec4(1.0f);
  vec4 b = vec4(a);
  vec4 c = vec4(v());
  vec4 d = vec4((a * 2.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
