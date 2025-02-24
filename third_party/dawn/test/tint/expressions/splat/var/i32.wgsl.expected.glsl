#version 310 es

void f() {
  int v = 3;
  ivec2 v2 = ivec2(v);
  ivec3 v3 = ivec3(v);
  ivec4 v4 = ivec4(v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
