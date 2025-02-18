#version 310 es

float v() {
  return 0.40000000596046447754f;
}
layout(local_size_x = 2, local_size_y = 1, local_size_z = 1) in;
void main() {
  float a = v();
}
