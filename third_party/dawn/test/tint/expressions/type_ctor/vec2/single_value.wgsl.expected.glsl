#version 310 es

vec2 v() {
  return vec2(0.0f);
}
void f() {
  vec2 a = vec2(1.0f);
  vec2 b = vec2(a);
  vec2 c = vec2(v());
  vec2 d = vec2((a * 2.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
