#version 310 es

void v() {
  int i = 1;
  int b = ivec2(1)[min(uint(i), 1u)];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
