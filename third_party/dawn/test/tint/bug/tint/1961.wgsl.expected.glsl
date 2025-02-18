#version 310 es

void f() {
  bool x = false;
  bool y = false;
  bool v = false;
  if (x) {
    v = true;
  } else {
    v = false;
  }
  if (v) {
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
