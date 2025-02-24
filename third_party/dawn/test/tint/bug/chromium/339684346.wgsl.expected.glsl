#version 310 es

void a(inout int x) {
}
void b(inout int x) {
  a(x);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
