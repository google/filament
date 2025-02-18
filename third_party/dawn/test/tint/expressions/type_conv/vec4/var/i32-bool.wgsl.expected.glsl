#version 310 es

ivec4 u = ivec4(1);
void f() {
  bvec4 v = bvec4(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
