struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


RWByteAddressBuffer prevent_dce : register(u0);
groupshared int arg_0;
int workgroupUniformLoad_9d33de() {
  GroupMemoryBarrierWithGroupSync();
  int v = arg_0;
  GroupMemoryBarrierWithGroupSync();
  int res = v;
  return res;
}

void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    arg_0 = int(0);
  }
  GroupMemoryBarrierWithGroupSync();
  prevent_dce.Store(0u, asuint(workgroupUniformLoad_9d33de()));
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

