#version 310 es

void f() {
  bvec2 v2 = bvec2(true);
  bvec3 v3 = bvec3(true);
  bvec4 v4 = bvec4(true);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
