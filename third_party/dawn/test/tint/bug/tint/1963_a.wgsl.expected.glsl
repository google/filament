#version 310 es

void X(vec2 a, vec2 b) {
}
vec2 Y() {
  return vec2(0.0f);
}
void f() {
  vec2 v = vec2(0.0f);
  X(vec2(0.0f), v);
  X(vec2(0.0f), Y());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
