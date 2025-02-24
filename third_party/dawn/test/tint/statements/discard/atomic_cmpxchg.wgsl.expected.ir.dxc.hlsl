struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

struct foo_outputs {
  int tint_symbol : SV_Target0;
};


RWByteAddressBuffer a : register(u0);
int foo_inner() {
  discard;
  int x = int(0);
  int v = int(0);
  a.InterlockedCompareExchange(int(0u), int(0), int(1), v);
  int v_1 = v;
  atomic_compare_exchange_result_i32 result = {v_1, (v_1 == int(0))};
  if (result.exchanged) {
    x = result.old_value;
  }
  return x;
}

foo_outputs foo() {
  foo_outputs v_2 = {foo_inner()};
  return v_2;
}

