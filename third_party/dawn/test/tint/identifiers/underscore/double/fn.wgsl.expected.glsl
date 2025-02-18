#version 310 es

void a() {
}
void v() {
}
void b() {
  a();
}
void v_1() {
  v();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  b();
  v_1();
}
