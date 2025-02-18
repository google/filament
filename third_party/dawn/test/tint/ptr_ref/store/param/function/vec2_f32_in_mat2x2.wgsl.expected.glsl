#version 310 es

void func(inout vec2 pointer) {
  pointer = vec2(0.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat2 F = mat2(vec2(0.0f), vec2(0.0f));
  func(F[1u]);
}
