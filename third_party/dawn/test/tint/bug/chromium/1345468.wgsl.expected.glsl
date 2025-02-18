#version 310 es

void f() {
  int i = 1;
  vec2 a = mat4x2(vec2(0.0f), vec2(0.0f), vec2(4.0f, 0.0f), vec2(0.0f))[min(uint(i), 3u)];
  int b = ivec2(0, 1)[min(uint(i), 1u)];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
