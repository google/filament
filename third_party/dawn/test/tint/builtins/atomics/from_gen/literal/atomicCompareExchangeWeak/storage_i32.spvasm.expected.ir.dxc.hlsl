//
// fragment_main
//
struct x__atomic_compare_exchange_resulti32 {
  int old_value;
  bool exchanged;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};


RWByteAddressBuffer sb_rw : register(u0);
void atomicCompareExchangeWeak_1bd40a() {
  x__atomic_compare_exchange_resulti32 res = (x__atomic_compare_exchange_resulti32)0;
  int v = int(0);
  sb_rw.InterlockedCompareExchange(int(0u), int(1), int(1), v);
  int v_1 = v;
  atomic_compare_exchange_result_i32 v_2 = {v_1, (v_1 == int(1))};
  int old_value_1 = v_2.old_value;
  int x_19 = old_value_1;
  x__atomic_compare_exchange_resulti32 v_3 = {x_19, (x_19 == int(1))};
  res = v_3;
}

void fragment_main_1() {
  atomicCompareExchangeWeak_1bd40a();
}

void fragment_main() {
  fragment_main_1();
}

//
// compute_main
//
struct x__atomic_compare_exchange_resulti32 {
  int old_value;
  bool exchanged;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};


RWByteAddressBuffer sb_rw : register(u0);
void atomicCompareExchangeWeak_1bd40a() {
  x__atomic_compare_exchange_resulti32 res = (x__atomic_compare_exchange_resulti32)0;
  int v = int(0);
  sb_rw.InterlockedCompareExchange(int(0u), int(1), int(1), v);
  int v_1 = v;
  atomic_compare_exchange_result_i32 v_2 = {v_1, (v_1 == int(1))};
  int old_value_1 = v_2.old_value;
  int x_19 = old_value_1;
  x__atomic_compare_exchange_resulti32 v_3 = {x_19, (x_19 == int(1))};
  res = v_3;
}

void compute_main_1() {
  atomicCompareExchangeWeak_1bd40a();
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
}

