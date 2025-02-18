#version 310 es

void func(inout vec4 pointer) {
  pointer = vec4(0.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec4 F = vec4(0.0f);
  func(F);
}
