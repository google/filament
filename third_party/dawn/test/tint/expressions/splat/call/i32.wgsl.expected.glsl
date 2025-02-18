#version 310 es

int get_i32() {
  return 1;
}
void f() {
  ivec2 v2 = ivec2(get_i32());
  ivec3 v3 = ivec3(get_i32());
  ivec4 v4 = ivec4(get_i32());
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
