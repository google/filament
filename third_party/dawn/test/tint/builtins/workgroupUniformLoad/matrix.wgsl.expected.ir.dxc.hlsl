
groupshared float3x3 v;
float3x3 foo() {
  GroupMemoryBarrierWithGroupSync();
  float3x3 v_1 = v;
  GroupMemoryBarrierWithGroupSync();
  return v_1;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

