#version 310 es
precision highp float;
precision highp int;


struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

layout(binding = 0, std430)
buffer f_S_block_ssbo {
  int inner;
} v;
bool continue_execution = true;
layout(location = 0) out vec4 main_loc0_Output;
vec4 main_inner() {
  if (false) {
    continue_execution = false;
  }
  atomic_compare_exchange_result_i32 v_1 = atomic_compare_exchange_result_i32(0, false);
  if (continue_execution) {
    int v_2 = atomicCompSwap(v.inner, 0, 1);
    v_1 = atomic_compare_exchange_result_i32(v_2, (v_2 == 0));
  }
  int old_value = v_1.old_value;
  vec4 v_3 = vec4(float(old_value));
  if (!(continue_execution)) {
    discard;
  }
  return v_3;
}
void main() {
  main_loc0_Output = main_inner();
}
