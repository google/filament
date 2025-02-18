//
// fragment_main
//
struct x__atomic_compare_exchange_resultu32 {
  uint old_value;
  bool exchanged;
};

struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};


RWByteAddressBuffer sb_rw : register(u0);
void atomicCompareExchangeWeak_63d8e6() {
  x__atomic_compare_exchange_resultu32 res = (x__atomic_compare_exchange_resultu32)0;
  uint v = 0u;
  sb_rw.InterlockedCompareExchange(0u, 1u, 1u, v);
  uint v_1 = v;
  atomic_compare_exchange_result_u32 v_2 = {v_1, (v_1 == 1u)};
  uint old_value_1 = v_2.old_value;
  uint x_17 = old_value_1;
  x__atomic_compare_exchange_resultu32 v_3 = {x_17, (x_17 == 1u)};
  res = v_3;
}

void fragment_main_1() {
  atomicCompareExchangeWeak_63d8e6();
}

void fragment_main() {
  fragment_main_1();
}

//
// compute_main
//
struct x__atomic_compare_exchange_resultu32 {
  uint old_value;
  bool exchanged;
};

struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};


RWByteAddressBuffer sb_rw : register(u0);
void atomicCompareExchangeWeak_63d8e6() {
  x__atomic_compare_exchange_resultu32 res = (x__atomic_compare_exchange_resultu32)0;
  uint v = 0u;
  sb_rw.InterlockedCompareExchange(0u, 1u, 1u, v);
  uint v_1 = v;
  atomic_compare_exchange_result_u32 v_2 = {v_1, (v_1 == 1u)};
  uint old_value_1 = v_2.old_value;
  uint x_17 = old_value_1;
  x__atomic_compare_exchange_resultu32 v_3 = {x_17, (x_17 == 1u)};
  res = v_3;
}

void compute_main_1() {
  atomicCompareExchangeWeak_63d8e6();
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
}

