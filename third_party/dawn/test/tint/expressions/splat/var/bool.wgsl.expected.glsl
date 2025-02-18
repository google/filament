#version 310 es

void f() {
  bool v = true;
  bvec2 v2 = bvec2(v);
  bvec3 v3 = bvec3(v);
  bvec4 v4 = bvec4(v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
