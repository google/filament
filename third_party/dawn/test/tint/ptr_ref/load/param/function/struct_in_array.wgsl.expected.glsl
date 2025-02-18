#version 310 es


struct str {
  int i;
};

str func(inout str pointer) {
  return pointer;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  str F[4] = str[4](str(0), str(0), str(0), str(0));
  str r = func(F[2u]);
}
