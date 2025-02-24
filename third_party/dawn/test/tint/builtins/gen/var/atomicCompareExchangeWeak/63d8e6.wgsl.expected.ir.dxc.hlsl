//
// fragment_main
//
struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};


RWByteAddressBuffer sb_rw : register(u0);
void atomicCompareExchangeWeak_63d8e6() {
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint v = arg_1;
  uint v_1 = 0u;
  sb_rw.InterlockedCompareExchange(0u, v, arg_2, v_1);
  uint v_2 = v_1;
  atomic_compare_exchange_result_u32 res = {v_2, (v_2 == v)};
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
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint v = arg_1;
  uint v_1 = 0u;
  sb_rw.InterlockedCompareExchange(0u, v, arg_2, v_1);
  uint v_2 = v_1;
  atomic_compare_exchange_result_u32 res = {v_2, (v_2 == v)};
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicCompareExchangeWeak_63d8e6();
}

