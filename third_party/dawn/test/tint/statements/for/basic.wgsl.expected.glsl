#version 310 es

void some_loop_body() {
}
void f() {
  {
    int i = 0;
    while(true) {
      if ((i < 5)) {
      } else {
        break;
      }
      some_loop_body();
      {
        i = (i + 1);
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
