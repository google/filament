struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

struct main_outputs {
  float4 tint_symbol : SV_Target0;
};


RWByteAddressBuffer S : register(u0);
float4 main_inner() {
  if (false) {
    discard;
  }
  int v = int(0);
  S.InterlockedCompareExchange(int(0u), int(0), int(1), v);
  int v_1 = v;
  atomic_compare_exchange_result_i32 v_2 = {v_1, (v_1 == int(0))};
  int old_value = v_2.old_value;
  return float4((float(old_value)).xxxx);
}

main_outputs main() {
  main_outputs v_3 = {main_inner()};
  return v_3;
}

