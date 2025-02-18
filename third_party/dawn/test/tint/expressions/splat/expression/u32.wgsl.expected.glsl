#version 310 es

void f() {
  uvec2 v2 = uvec2(3u);
  uvec3 v3 = uvec3(3u);
  uvec4 v4 = uvec4(3u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
