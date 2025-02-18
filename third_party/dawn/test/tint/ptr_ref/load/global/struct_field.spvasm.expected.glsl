#version 310 es


struct S {
  int i;
};

S V = S(0);
void main_1() {
  int i = 0;
  i = V.i;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_1();
}
