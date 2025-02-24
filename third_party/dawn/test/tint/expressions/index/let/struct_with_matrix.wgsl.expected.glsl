#version 310 es


struct S {
  int m;
  mat4 n;
};

float f() {
  S a = S(0, mat4(vec4(0.0f), vec4(0.0f), vec4(0.0f), vec4(0.0f)));
  return a.n[2u].y;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
