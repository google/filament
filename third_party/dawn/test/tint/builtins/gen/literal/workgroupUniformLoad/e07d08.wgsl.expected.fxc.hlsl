SKIP: INVALID

groupshared float16_t arg_0;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    arg_0 = float16_t(0.0h);
  }
  GroupMemoryBarrierWithGroupSync();
}

float16_t tint_workgroupUniformLoad_arg_0() {
  GroupMemoryBarrierWithGroupSync();
  float16_t result = arg_0;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

RWByteAddressBuffer prevent_dce : register(u0);

float16_t workgroupUniformLoad_e07d08() {
  float16_t res = tint_workgroupUniformLoad_arg_0();
  return res;
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  prevent_dce.Store<float16_t>(0u, workgroupUniformLoad_e07d08());
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
FXC validation failure:
<scrubbed_path>(1,13-21): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
