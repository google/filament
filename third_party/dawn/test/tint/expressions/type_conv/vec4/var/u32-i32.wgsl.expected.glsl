#version 310 es

uvec4 u = uvec4(1u);
void f() {
  ivec4 v = ivec4(u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
