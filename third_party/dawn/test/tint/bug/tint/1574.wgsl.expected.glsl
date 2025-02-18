#version 310 es


struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

layout(binding = 0, std430)
buffer a_u32_block_1_ssbo {
  uint inner;
} v;
layout(binding = 1, std430)
buffer a_i32_block_1_ssbo {
  int inner;
} v_1;
shared uint b_u32;
shared int b_i32;
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    atomicExchange(b_u32, 0u);
    atomicExchange(b_i32, 0);
  }
  barrier();
  uint value = 42u;
  uint v_2 = atomicCompSwap(v.inner, 0u, value);
  atomic_compare_exchange_result_u32 r1 = atomic_compare_exchange_result_u32(v_2, (v_2 == 0u));
  uint v_3 = atomicCompSwap(v.inner, 0u, value);
  atomic_compare_exchange_result_u32 r2 = atomic_compare_exchange_result_u32(v_3, (v_3 == 0u));
  uint v_4 = atomicCompSwap(v.inner, 0u, value);
  atomic_compare_exchange_result_u32 r3 = atomic_compare_exchange_result_u32(v_4, (v_4 == 0u));
  int value_1 = 42;
  int v_5 = atomicCompSwap(v_1.inner, 0, value_1);
  atomic_compare_exchange_result_i32 r1_1 = atomic_compare_exchange_result_i32(v_5, (v_5 == 0));
  int v_6 = atomicCompSwap(v_1.inner, 0, value_1);
  atomic_compare_exchange_result_i32 r2_1 = atomic_compare_exchange_result_i32(v_6, (v_6 == 0));
  int v_7 = atomicCompSwap(v_1.inner, 0, value_1);
  atomic_compare_exchange_result_i32 r3_1 = atomic_compare_exchange_result_i32(v_7, (v_7 == 0));
  uint value_2 = 42u;
  uint v_8 = atomicCompSwap(b_u32, 0u, value_2);
  atomic_compare_exchange_result_u32 r1_2 = atomic_compare_exchange_result_u32(v_8, (v_8 == 0u));
  uint v_9 = atomicCompSwap(b_u32, 0u, value_2);
  atomic_compare_exchange_result_u32 r2_2 = atomic_compare_exchange_result_u32(v_9, (v_9 == 0u));
  uint v_10 = atomicCompSwap(b_u32, 0u, value_2);
  atomic_compare_exchange_result_u32 r3_2 = atomic_compare_exchange_result_u32(v_10, (v_10 == 0u));
  int value_3 = 42;
  int v_11 = atomicCompSwap(b_i32, 0, value_3);
  atomic_compare_exchange_result_i32 r1_3 = atomic_compare_exchange_result_i32(v_11, (v_11 == 0));
  int v_12 = atomicCompSwap(b_i32, 0, value_3);
  atomic_compare_exchange_result_i32 r2_3 = atomic_compare_exchange_result_i32(v_12, (v_12 == 0));
  int v_13 = atomicCompSwap(b_i32, 0, value_3);
  atomic_compare_exchange_result_i32 r3_3 = atomic_compare_exchange_result_i32(v_13, (v_13 == 0));
}
layout(local_size_x = 16, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
