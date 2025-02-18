//
// fragment_main
//
struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};


RWByteAddressBuffer sb_rw : register(u0);
void atomicCompareExchangeWeak_63d8e6() {
  uint v = 0u;
  sb_rw.InterlockedCompareExchange(0u, 1u, 1u, v);
  uint v_1 = v;
  atomic_compare_exchange_result_u32 res = {v_1, (v_1 == 1u)};
}

void fragment_main() {
  atomicCompareExchangeWeak_63d8e6();
}

//
// compute_main
//
struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};


RWByteAddressBuffer sb_rw : register(u0);
void atomicCompareExchangeWeak_63d8e6() {
  uint v = 0u;
  sb_rw.InterlockedCompareExchange(0u, 1u, 1u, v);
  uint v_1 = v;
  atomic_compare_exchange_result_u32 res = {v_1, (v_1 == 1u)};
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicCompareExchangeWeak_63d8e6();
}

