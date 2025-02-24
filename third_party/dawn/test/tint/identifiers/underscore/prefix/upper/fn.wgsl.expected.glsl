#version 310 es

void A() {
}
void _A() {
}
void B() {
  A();
}
void _B() {
  _A();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  B();
  _B();
}
