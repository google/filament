#version 310 es


struct S {
  int i;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  S V = S(0);
  int i = V.i;
}
