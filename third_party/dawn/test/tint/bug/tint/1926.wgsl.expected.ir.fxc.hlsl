struct main_inputs {
  uint3 local_id : SV_GroupThreadID;
  uint tint_local_index : SV_GroupIndex;
  uint3 global_id : SV_DispatchThreadID;
};


groupshared uint sh_atomic_failed;
RWByteAddressBuffer output : register(u4);
void main_inner(uint3 global_id, uint3 local_id, uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    sh_atomic_failed = 0u;
  }
  GroupMemoryBarrierWithGroupSync();
  GroupMemoryBarrierWithGroupSync();
  uint v = sh_atomic_failed;
  GroupMemoryBarrierWithGroupSync();
  uint failed = v;
  if ((local_id.x == 0u)) {
    output.Store(0u, failed);
  }
}

[numthreads(256, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.global_id, inputs.local_id, inputs.tint_local_index);
}

