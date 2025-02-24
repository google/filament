#version 310 es

shared int i;
void v() {
  i = (i - 1);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
