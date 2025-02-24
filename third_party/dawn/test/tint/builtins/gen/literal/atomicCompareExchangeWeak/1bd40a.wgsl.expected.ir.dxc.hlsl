//
// fragment_main
//
struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};


RWByteAddressBuffer sb_rw : register(u0);
void atomicCompareExchangeWeak_1bd40a() {
  int v = int(0);
  sb_rw.InterlockedCompareExchange(int(0u), int(1), int(1), v);
  int v_1 = v;
  atomic_compare_exchange_result_i32 res = {v_1, (v_1 == int(1))};
}

void fragment_main() {
  atomicCompareExchangeWeak_1bd40a();
}

//
// compute_main
//
struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};


RWByteAddressBuffer sb_rw : register(u0);
void atomicCompareExchangeWeak_1bd40a() {
  int v = int(0);
  sb_rw.InterlockedCompareExchange(int(0u), int(1), int(1), v);
  int v_1 = v;
  atomic_compare_exchange_result_i32 res = {v_1, (v_1 == int(1))};
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicCompareExchangeWeak_1bd40a();
}

