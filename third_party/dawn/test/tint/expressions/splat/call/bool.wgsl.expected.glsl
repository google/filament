#version 310 es

bool get_bool() {
  return true;
}
void f() {
  bvec2 v2 = bvec2(get_bool());
  bvec3 v3 = bvec3(get_bool());
  bvec4 v4 = bvec4(get_bool());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
