#version 310 es


struct S {
  int arr[4];
};

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  int inner;
} v;
int foo() {
  int src[4] = int[4](0, 0, 0, 0);
  int dst[4] = int[4](0, 0, 0, 0);
  S dst_struct = S(int[4](0, 0, 0, 0));
  int dst_array[2][4] = int[2][4](int[4](0, 0, 0, 0), int[4](0, 0, 0, 0));
  dst_struct.arr = src;
  dst_array[1u] = src;
  dst = src;
  dst_struct.arr = src;
  dst_array[0u] = src;
  return ((dst[0u] + dst_struct.arr[0u]) + dst_array[0u][0u]);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = foo();
}
