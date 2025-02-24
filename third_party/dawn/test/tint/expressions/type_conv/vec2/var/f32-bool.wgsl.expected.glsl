#version 310 es

vec2 u = vec2(1.0f);
void f() {
  bvec2 v = bvec2(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
