#version 310 es

void d() {
}
void c() {
  d();
}
void b() {
  c();
}
void a() {
  b();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
