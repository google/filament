#version 310 es


struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

layout(binding = 0, std430)
buffer a_block_1_ssbo {
  int inner;
} v_1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int v_2 = atomicCompSwap(v_1.inner, 1, 1);
  int v = atomic_compare_exchange_result_i32(v_2, (v_2 == 1)).old_value;
}
