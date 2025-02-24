#version 310 es

void f() {
  uint v = 3u;
  uvec2 v2 = uvec2(v);
  uvec3 v3 = uvec3(v);
  uvec4 v4 = uvec4(v);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
