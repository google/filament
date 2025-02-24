#version 310 es

void workgroupBarrier_a17f7f() {
  barrier();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  workgroupBarrier_a17f7f();
}
