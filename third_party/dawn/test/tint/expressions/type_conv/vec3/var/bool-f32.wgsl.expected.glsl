#version 310 es

bvec3 u = bvec3(true);
void f() {
  vec3 v = vec3(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
