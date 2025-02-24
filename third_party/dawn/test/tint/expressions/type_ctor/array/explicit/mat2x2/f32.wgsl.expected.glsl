#version 310 es

mat2 arr[2] = mat2[2](mat2(vec2(1.0f, 2.0f), vec2(3.0f, 4.0f)), mat2(vec2(5.0f, 6.0f), vec2(7.0f, 8.0f)));
void f() {
  mat2 v[2] = arr;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
