[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

groupshared float3x3 v;

float3x3 tint_workgroupUniformLoad_v() {
  GroupMemoryBarrierWithGroupSync();
  float3x3 result = v;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

float3x3 foo() {
  return tint_workgroupUniformLoad_v();
}
