//
// fragment_main
//
struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};


RWByteAddressBuffer sb_rw : register(u0);
void atomicCompareExchangeWeak_1bd40a() {
  int arg_1 = int(1);
  int arg_2 = int(1);
  int v = arg_1;
  int v_1 = arg_2;
  int v_2 = int(0);
  sb_rw.InterlockedCompareExchange(int(0u), v, v_1, v_2);
  int v_3 = v_2;
  atomic_compare_exchange_result_i32 res = {v_3, (v_3 == v)};
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
  int arg_1 = int(1);
  int arg_2 = int(1);
  int v = arg_1;
  int v_1 = arg_2;
  int v_2 = int(0);
  sb_rw.InterlockedCompareExchange(int(0u), v, v_1, v_2);
  int v_3 = v_2;
  atomic_compare_exchange_result_i32 res = {v_3, (v_3 == v)};
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicCompareExchangeWeak_1bd40a();
}

