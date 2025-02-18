struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

struct main_outputs {
  float4 tint_symbol : SV_Target0;
};


RWByteAddressBuffer S : register(u0);
static bool continue_execution = true;
float4 main_inner() {
  if (false) {
    continue_execution = false;
  }
  atomic_compare_exchange_result_i32 v = (atomic_compare_exchange_result_i32)0;
  if (continue_execution) {
    int v_1 = int(0);
    S.InterlockedCompareExchange(int(0u), int(0), int(1), v_1);
    int v_2 = v_1;
    atomic_compare_exchange_result_i32 v_3 = {v_2, (v_2 == int(0))};
    v = v_3;
  }
  atomic_compare_exchange_result_i32 v_4 = v;
  int old_value = v_4.old_value;
  return float4((float(old_value)).xxxx);
}

main_outputs main() {
  main_outputs v_5 = {main_inner()};
  if (!(continue_execution)) {
    discard;
  }
  return v_5;
}

