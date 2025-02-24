struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};
RWByteAddressBuffer a : register(u0);

struct tint_symbol {
  int value : SV_Target0;
};

atomic_compare_exchange_result_i32 aatomicCompareExchangeWeak(uint offset, int compare, int value) {
  atomic_compare_exchange_result_i32 result=(atomic_compare_exchange_result_i32)0;
  a.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


int foo_inner() {
  if (true) {
    discard;
    int x = 0;
    atomic_compare_exchange_result_i32 result = aatomicCompareExchangeWeak(0u, 0, 1);
    if (result.exchanged) {
      x = result.old_value;
    }
    return x;
  }
  int unused;
  return unused;
}

tint_symbol foo() {
  int inner_result = foo_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
