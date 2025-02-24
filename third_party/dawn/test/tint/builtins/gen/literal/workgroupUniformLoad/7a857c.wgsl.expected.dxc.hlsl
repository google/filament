groupshared float arg_0;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    arg_0 = 0.0f;
  }
  GroupMemoryBarrierWithGroupSync();
}

float tint_workgroupUniformLoad_arg_0() {
  GroupMemoryBarrierWithGroupSync();
  float result = arg_0;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

RWByteAddressBuffer prevent_dce : register(u0);

float workgroupUniformLoad_7a857c() {
  float res = tint_workgroupUniformLoad_arg_0();
  return res;
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  prevent_dce.Store(0u, asuint(workgroupUniformLoad_7a857c()));
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
