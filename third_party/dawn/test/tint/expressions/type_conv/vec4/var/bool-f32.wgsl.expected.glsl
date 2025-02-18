#version 310 es

bvec4 u = bvec4(true);
void f() {
  vec4 v = vec4(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
