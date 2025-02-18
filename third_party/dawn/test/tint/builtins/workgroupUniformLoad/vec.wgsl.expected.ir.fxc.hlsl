
groupshared float4 v;
float4 foo() {
  GroupMemoryBarrierWithGroupSync();
  float4 v_1 = v;
  GroupMemoryBarrierWithGroupSync();
  return v_1;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

