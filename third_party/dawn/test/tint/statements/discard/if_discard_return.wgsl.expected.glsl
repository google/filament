#version 310 es

bool continue_execution = true;
void f(bool cond) {
  if (cond) {
    continue_execution = false;
    return;
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
