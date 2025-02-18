#version 310 es

void deref_const() {
  ivec3 a = ivec3(0);
  int b = a.x;
  a.x = 42;
}
void no_deref_const() {
  ivec3 a = ivec3(0);
  int b = a.x;
  a.x = 42;
}
void deref_let() {
  ivec3 a = ivec3(0);
  int i = 0;
  int b = a[min(uint(i), 2u)];
  a.x = 42;
}
void no_deref_let() {
  ivec3 a = ivec3(0);
  int i = 0;
  int b = a[min(uint(i), 2u)];
  a.x = 42;
}
void deref_var() {
  ivec3 a = ivec3(0);
  int i = 0;
  int b = a[min(uint(i), 2u)];
  a.x = 42;
}
void no_deref_var() {
  ivec3 a = ivec3(0);
  int i = 0;
  int b = a[min(uint(i), 2u)];
  a.x = 42;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  deref_const();
  no_deref_const();
  deref_let();
  no_deref_let();
  deref_var();
  no_deref_var();
}
