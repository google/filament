#version 310 es
precision highp float;
precision highp int;


struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

layout(binding = 0, std430)
buffer f_a_block_ssbo {
  int inner;
} v;
bool continue_execution = true;
layout(location = 0) out int foo_loc0_Output;
int foo_inner() {
  continue_execution = false;
  int x = 0;
  atomic_compare_exchange_result_i32 v_1 = atomic_compare_exchange_result_i32(0, false);
  if (continue_execution) {
    int v_2 = atomicCompSwap(v.inner, 0, 1);
    v_1 = atomic_compare_exchange_result_i32(v_2, (v_2 == 0));
  }
  atomic_compare_exchange_result_i32 result = v_1;
  if (result.exchanged) {
    x = result.old_value;
  }
  if (!(continue_execution)) {
    discard;
  }
  return x;
}
void main() {
  foo_loc0_Output = foo_inner();
}
