#version 310 es


struct B {
  int b;
};

struct A {
  int a;
};

B f(A a) {
  return B(0);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
