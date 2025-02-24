//
// fragment_main
//
struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};
struct x__atomic_compare_exchange_resulti32 {
  int old_value;
  bool exchanged;
};

RWByteAddressBuffer sb_rw : register(u0);

atomic_compare_exchange_result_i32 sb_rwatomicCompareExchangeWeak(uint offset, int compare, int value) {
  atomic_compare_exchange_result_i32 result=(atomic_compare_exchange_result_i32)0;
  sb_rw.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


void atomicCompareExchangeWeak_1bd40a() {
  x__atomic_compare_exchange_resulti32 res = (x__atomic_compare_exchange_resulti32)0;
  atomic_compare_exchange_result_i32 tint_symbol = sb_rwatomicCompareExchangeWeak(0u, 1, 1);
  int old_value_1 = tint_symbol.old_value;
  int x_19 = old_value_1;
  x__atomic_compare_exchange_resulti32 tint_symbol_1 = {x_19, (x_19 == 1)};
  res = tint_symbol_1;
  return;
}

void fragment_main_1() {
  atomicCompareExchangeWeak_1bd40a();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}
//
// compute_main
//
struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};
struct x__atomic_compare_exchange_resulti32 {
  int old_value;
  bool exchanged;
};

RWByteAddressBuffer sb_rw : register(u0);

atomic_compare_exchange_result_i32 sb_rwatomicCompareExchangeWeak(uint offset, int compare, int value) {
  atomic_compare_exchange_result_i32 result=(atomic_compare_exchange_result_i32)0;
  sb_rw.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


void atomicCompareExchangeWeak_1bd40a() {
  x__atomic_compare_exchange_resulti32 res = (x__atomic_compare_exchange_resulti32)0;
  atomic_compare_exchange_result_i32 tint_symbol = sb_rwatomicCompareExchangeWeak(0u, 1, 1);
  int old_value_1 = tint_symbol.old_value;
  int x_19 = old_value_1;
  x__atomic_compare_exchange_resulti32 tint_symbol_1 = {x_19, (x_19 == 1)};
  res = tint_symbol_1;
  return;
}

void compute_main_1() {
  atomicCompareExchangeWeak_1bd40a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
