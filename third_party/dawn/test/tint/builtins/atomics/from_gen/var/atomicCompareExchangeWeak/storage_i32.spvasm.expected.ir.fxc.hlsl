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
  int arg_1 = int(0);
  int arg_2 = int(0);
  x__atomic_compare_exchange_resulti32 res = (x__atomic_compare_exchange_resulti32)0;
  arg_1 = int(1);
  arg_2 = int(1);
  int x_23 = arg_2;
  int x_24 = arg_1;
  int v = int(0);
  sb_rw.InterlockedCompareExchange(int(0u), x_24, x_23, v);
  int v_1 = v;
  atomic_compare_exchange_result_i32 v_2 = {v_1, (v_1 == x_24)};
  int old_value_1 = v_2.old_value;
  int x_25 = old_value_1;
  x__atomic_compare_exchange_resulti32 v_3 = {x_25, (x_25 == x_23)};
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
  int arg_1 = int(0);
  int arg_2 = int(0);
  x__atomic_compare_exchange_resulti32 res = (x__atomic_compare_exchange_resulti32)0;
  arg_1 = int(1);
  arg_2 = int(1);
  int x_23 = arg_2;
  int x_24 = arg_1;
  int v = int(0);
  sb_rw.InterlockedCompareExchange(int(0u), x_24, x_23, v);
  int v_1 = v;
  atomic_compare_exchange_result_i32 v_2 = {v_1, (v_1 == x_24)};
  int old_value_1 = v_2.old_value;
  int x_25 = old_value_1;
  x__atomic_compare_exchange_resulti32 v_3 = {x_25, (x_25 == x_23)};
  res = v_3;
}

void compute_main_1() {
  atomicCompareExchangeWeak_1bd40a();
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
}

