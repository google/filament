struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};
struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};
groupshared uint b_u32;
groupshared int b_i32;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    uint atomic_result = 0u;
    InterlockedExchange(b_u32, 0u, atomic_result);
    int atomic_result_1 = 0;
    InterlockedExchange(b_i32, 0, atomic_result_1);
  }
  GroupMemoryBarrierWithGroupSync();
}

RWByteAddressBuffer a_u32 : register(u0);
RWByteAddressBuffer a_i32 : register(u1);

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

atomic_compare_exchange_result_u32 a_u32atomicCompareExchangeWeak(uint offset, uint compare, uint value) {
  atomic_compare_exchange_result_u32 result=(atomic_compare_exchange_result_u32)0;
  a_u32.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


atomic_compare_exchange_result_i32 a_i32atomicCompareExchangeWeak(uint offset, int compare, int value) {
  atomic_compare_exchange_result_i32 result=(atomic_compare_exchange_result_i32)0;
  a_i32.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


void main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  {
    uint value = 42u;
    atomic_compare_exchange_result_u32 r1 = a_u32atomicCompareExchangeWeak(0u, 0u, value);
    atomic_compare_exchange_result_u32 r2 = a_u32atomicCompareExchangeWeak(0u, 0u, value);
    atomic_compare_exchange_result_u32 r3 = a_u32atomicCompareExchangeWeak(0u, 0u, value);
  }
  {
    int value = 42;
    atomic_compare_exchange_result_i32 r1 = a_i32atomicCompareExchangeWeak(0u, 0, value);
    atomic_compare_exchange_result_i32 r2 = a_i32atomicCompareExchangeWeak(0u, 0, value);
    atomic_compare_exchange_result_i32 r3 = a_i32atomicCompareExchangeWeak(0u, 0, value);
  }
  {
    uint value = 42u;
    atomic_compare_exchange_result_u32 atomic_result_2 = (atomic_compare_exchange_result_u32)0;
    uint atomic_compare_value = 0u;
    InterlockedCompareExchange(b_u32, atomic_compare_value, value, atomic_result_2.old_value);
    atomic_result_2.exchanged = atomic_result_2.old_value == atomic_compare_value;
    atomic_compare_exchange_result_u32 r1 = atomic_result_2;
    atomic_compare_exchange_result_u32 atomic_result_3 = (atomic_compare_exchange_result_u32)0;
    uint atomic_compare_value_1 = 0u;
    InterlockedCompareExchange(b_u32, atomic_compare_value_1, value, atomic_result_3.old_value);
    atomic_result_3.exchanged = atomic_result_3.old_value == atomic_compare_value_1;
    atomic_compare_exchange_result_u32 r2 = atomic_result_3;
    atomic_compare_exchange_result_u32 atomic_result_4 = (atomic_compare_exchange_result_u32)0;
    uint atomic_compare_value_2 = 0u;
    InterlockedCompareExchange(b_u32, atomic_compare_value_2, value, atomic_result_4.old_value);
    atomic_result_4.exchanged = atomic_result_4.old_value == atomic_compare_value_2;
    atomic_compare_exchange_result_u32 r3 = atomic_result_4;
  }
  {
    int value = 42;
    atomic_compare_exchange_result_i32 atomic_result_5 = (atomic_compare_exchange_result_i32)0;
    int atomic_compare_value_3 = 0;
    InterlockedCompareExchange(b_i32, atomic_compare_value_3, value, atomic_result_5.old_value);
    atomic_result_5.exchanged = atomic_result_5.old_value == atomic_compare_value_3;
    atomic_compare_exchange_result_i32 r1 = atomic_result_5;
    atomic_compare_exchange_result_i32 atomic_result_6 = (atomic_compare_exchange_result_i32)0;
    int atomic_compare_value_4 = 0;
    InterlockedCompareExchange(b_i32, atomic_compare_value_4, value, atomic_result_6.old_value);
    atomic_result_6.exchanged = atomic_result_6.old_value == atomic_compare_value_4;
    atomic_compare_exchange_result_i32 r2 = atomic_result_6;
    atomic_compare_exchange_result_i32 atomic_result_7 = (atomic_compare_exchange_result_i32)0;
    int atomic_compare_value_5 = 0;
    InterlockedCompareExchange(b_i32, atomic_compare_value_5, value, atomic_result_7.old_value);
    atomic_result_7.exchanged = atomic_result_7.old_value == atomic_compare_value_5;
    atomic_compare_exchange_result_i32 r3 = atomic_result_7;
  }
}

[numthreads(16, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
