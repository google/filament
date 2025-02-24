struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


RWByteAddressBuffer prevent_dce : register(u0);
groupshared uint arg_0;
uint workgroupUniformLoad_37307c() {
  GroupMemoryBarrierWithGroupSync();
  uint v = arg_0;
  GroupMemoryBarrierWithGroupSync();
  uint res = v;
  return res;
}

void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    arg_0 = 0u;
  }
  GroupMemoryBarrierWithGroupSync();
  prevent_dce.Store(0u, workgroupUniformLoad_37307c());
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

