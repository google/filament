#version 310 es


struct str {
  int i;
};

str P[4] = str[4](str(0), str(0), str(0), str(0));
void func(inout str pointer) {
  pointer = str(0);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  func(P[2u]);
}
