struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


RWByteAddressBuffer a_u32 : register(u0);
RWByteAddressBuffer a_i32 : register(u1);
groupshared uint b_u32;
groupshared int b_i32;
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    uint v = 0u;
    InterlockedExchange(b_u32, 0u, v);
    int v_1 = int(0);
    InterlockedExchange(b_i32, int(0), v_1);
  }
  GroupMemoryBarrierWithGroupSync();
  uint value = 42u;
  uint v_2 = 0u;
  a_u32.InterlockedCompareExchange(0u, 0u, value, v_2);
  uint v_3 = v_2;
  atomic_compare_exchange_result_u32 r1 = {v_3, (v_3 == 0u)};
  uint v_4 = 0u;
  a_u32.InterlockedCompareExchange(0u, 0u, value, v_4);
  uint v_5 = v_4;
  atomic_compare_exchange_result_u32 r2 = {v_5, (v_5 == 0u)};
  uint v_6 = 0u;
  a_u32.InterlockedCompareExchange(0u, 0u, value, v_6);
  uint v_7 = v_6;
  atomic_compare_exchange_result_u32 r3 = {v_7, (v_7 == 0u)};
  int value_1 = int(42);
  int v_8 = value_1;
  int v_9 = int(0);
  a_i32.InterlockedCompareExchange(int(0u), int(0), v_8, v_9);
  int v_10 = v_9;
  atomic_compare_exchange_result_i32 r1_1 = {v_10, (v_10 == int(0))};
  int v_11 = value_1;
  int v_12 = int(0);
  a_i32.InterlockedCompareExchange(int(0u), int(0), v_11, v_12);
  int v_13 = v_12;
  atomic_compare_exchange_result_i32 r2_1 = {v_13, (v_13 == int(0))};
  int v_14 = value_1;
  int v_15 = int(0);
  a_i32.InterlockedCompareExchange(int(0u), int(0), v_14, v_15);
  int v_16 = v_15;
  atomic_compare_exchange_result_i32 r3_1 = {v_16, (v_16 == int(0))};
  uint value_2 = 42u;
  uint v_17 = 0u;
  InterlockedCompareExchange(b_u32, 0u, value_2, v_17);
  uint v_18 = v_17;
  atomic_compare_exchange_result_u32 r1_2 = {v_18, (v_18 == 0u)};
  uint v_19 = 0u;
  InterlockedCompareExchange(b_u32, 0u, value_2, v_19);
  uint v_20 = v_19;
  atomic_compare_exchange_result_u32 r2_2 = {v_20, (v_20 == 0u)};
  uint v_21 = 0u;
  InterlockedCompareExchange(b_u32, 0u, value_2, v_21);
  uint v_22 = v_21;
  atomic_compare_exchange_result_u32 r3_2 = {v_22, (v_22 == 0u)};
  int value_3 = int(42);
  int v_23 = int(0);
  InterlockedCompareExchange(b_i32, int(0), value_3, v_23);
  int v_24 = v_23;
  atomic_compare_exchange_result_i32 r1_3 = {v_24, (v_24 == int(0))};
  int v_25 = int(0);
  InterlockedCompareExchange(b_i32, int(0), value_3, v_25);
  int v_26 = v_25;
  atomic_compare_exchange_result_i32 r2_3 = {v_26, (v_26 == int(0))};
  int v_27 = int(0);
  InterlockedCompareExchange(b_i32, int(0), value_3, v_27);
  int v_28 = v_27;
  atomic_compare_exchange_result_i32 r3_3 = {v_28, (v_28 == int(0))};
}

[numthreads(16, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

