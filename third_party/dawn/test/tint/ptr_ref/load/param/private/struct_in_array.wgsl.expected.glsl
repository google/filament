#version 310 es


struct str {
  int i;
};

str P[4] = str[4](str(0), str(0), str(0), str(0));
str func(inout str pointer) {
  return pointer;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  str r = func(P[2u]);
}
