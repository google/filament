#version 310 es


struct S {
  ivec4 a[4];
};

int counter = 0;
int foo() {
  counter = (counter + 1);
  return counter;
}
int bar() {
  counter = (counter + 2);
  return counter;
}
void v() {
  S x = S(ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0)));
  uint v_1 = min(uint(foo()), 3u);
  int v_2 = bar();
  int v_3 = (x.a[v_1][min(uint(v_2), 3u)] + 5);
  x.a[v_1][min(uint(v_2), 3u)] = v_3;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
