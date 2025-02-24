#version 310 es


struct str {
  int arr[4];
};

str P = str(int[4](0, 0, 0, 0));
void func(inout int pointer[4]) {
  pointer = int[4](0, 0, 0, 0);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  func(P.arr);
}
