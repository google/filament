struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

struct foo_outputs {
  int tint_symbol : SV_Target0;
};


RWByteAddressBuffer a : register(u0);
static bool continue_execution = true;
int foo_inner() {
  continue_execution = false;
  int x = int(0);
  atomic_compare_exchange_result_i32 v = (atomic_compare_exchange_result_i32)0;
  if (continue_execution) {
    int v_1 = int(0);
    a.InterlockedCompareExchange(int(0u), int(0), int(1), v_1);
    int v_2 = v_1;
    atomic_compare_exchange_result_i32 v_3 = {v_2, (v_2 == int(0))};
    v = v_3;
  }
  atomic_compare_exchange_result_i32 result = v;
  if (result.exchanged) {
    x = result.old_value;
  }
  return x;
}

foo_outputs foo() {
  foo_outputs v_4 = {foo_inner()};
  if (!(continue_execution)) {
    discard;
  }
  return v_4;
}

