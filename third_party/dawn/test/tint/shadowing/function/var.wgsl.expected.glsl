#version 310 es


struct a {
  int a;
};

void f() {
  a a_1 = a(0);
  a b = a_1;
  a a_2 = a(0);
  a b_1 = a_2;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
