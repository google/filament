#version 310 es


struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};

layout(binding = 0, std430)
buffer a_block_1_ssbo {
  uint inner;
} v;
layout(local_size_x = 16, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint value = 42u;
  uint v_1 = atomicCompSwap(v.inner, 0u, value);
  atomic_compare_exchange_result_u32 result = atomic_compare_exchange_result_u32(v_1, (v_1 == 0u));
}
