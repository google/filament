groupshared int g1;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    int atomic_result = 0;
    InterlockedExchange(g1, 0, atomic_result);
  }
  GroupMemoryBarrierWithGroupSync();
}

int tint_ftoi(float v) {
  return ((v <= 2147483520.0f) ? ((v < -2147483648.0f) ? -2147483648 : int(v)) : 2147483647);
}

RWByteAddressBuffer s : register(u0);

struct S {
  int a;
  int b;
};

int accept_value(int val) {
  return val;
}

int accept_ptr_deref_call_func(inout int val) {
  int tint_symbol_2 = val;
  int tint_symbol_3 = accept_value(val);
  return (tint_symbol_2 + tint_symbol_3);
}

int accept_ptr_deref_pass_through(inout int val) {
  int tint_symbol = val;
  int tint_symbol_1 = accept_ptr_deref_call_func(val);
  return (tint_symbol + tint_symbol_1);
}

int accept_ptr_to_struct_and_access(inout S val) {
  return (val.a + val.b);
}

int accept_ptr_to_struct_access_pass_ptr(inout S val) {
  val.a = 2;
  return val.a;
}

int accept_ptr_vec_access_elements(inout float3 v1) {
  v1.x = cross(v1, v1).x;
  return tint_ftoi(v1.x);
}

int call_builtin_with_mod_scope_ptr() {
  int atomic_result_1 = 0;
  InterlockedOr(g1, 0, atomic_result_1);
  return atomic_result_1;
}

struct tint_symbol_11 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  int v1 = 0;
  S v2 = (S)0;
  float3 v4 = (0.0f).xxx;
  int atomic_result_2 = 0;
  InterlockedOr(g1, 0, atomic_result_2);
  int t1 = atomic_result_2;
  int tint_symbol_4 = accept_ptr_deref_pass_through(v1);
  int tint_symbol_5 = accept_ptr_to_struct_and_access(v2);
  int tint_symbol_6 = accept_ptr_to_struct_and_access(v2);
  int tint_symbol_7 = accept_ptr_vec_access_elements(v4);
  int tint_symbol_8 = accept_ptr_to_struct_access_pass_ptr(v2);
  int tint_symbol_9 = call_builtin_with_mod_scope_ptr();
  s.Store(0u, asuint(((((((tint_symbol_4 + tint_symbol_5) + tint_symbol_6) + tint_symbol_7) + tint_symbol_8) + tint_symbol_9) + t1)));
}

[numthreads(1, 1, 1)]
void main(tint_symbol_11 tint_symbol_10) {
  main_inner(tint_symbol_10.local_invocation_index);
  return;
}
