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
  uint arg_1 = 0u;
  uint arg_2 = 0u;
  x__atomic_compare_exchange_resultu32 res = (x__atomic_compare_exchange_resultu32)0;
  arg_1 = 1u;
  arg_2 = 1u;
  uint x_21 = arg_2;
  uint x_22 = arg_1;
  uint v = 0u;
  sb_rw.InterlockedCompareExchange(0u, x_22, x_21, v);
  uint v_1 = v;
  atomic_compare_exchange_result_u32 v_2 = {v_1, (v_1 == x_22)};
  uint old_value_1 = v_2.old_value;
  uint x_23 = old_value_1;
  x__atomic_compare_exchange_resultu32 v_3 = {x_23, (x_23 == x_21)};
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
  uint arg_1 = 0u;
  uint arg_2 = 0u;
  x__atomic_compare_exchange_resultu32 res = (x__atomic_compare_exchange_resultu32)0;
  arg_1 = 1u;
  arg_2 = 1u;
  uint x_21 = arg_2;
  uint x_22 = arg_1;
  uint v = 0u;
  sb_rw.InterlockedCompareExchange(0u, x_22, x_21, v);
  uint v_1 = v;
  atomic_compare_exchange_result_u32 v_2 = {v_1, (v_1 == x_22)};
  uint old_value_1 = v_2.old_value;
  uint x_23 = old_value_1;
  x__atomic_compare_exchange_resultu32 v_3 = {x_23, (x_23 == x_21)};
  res = v_3;
}

void compute_main_1() {
  atomicCompareExchangeWeak_63d8e6();
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
}

