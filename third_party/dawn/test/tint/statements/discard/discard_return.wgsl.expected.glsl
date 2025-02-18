#version 310 es

bool continue_execution = true;
void f() {
  continue_execution = false;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
