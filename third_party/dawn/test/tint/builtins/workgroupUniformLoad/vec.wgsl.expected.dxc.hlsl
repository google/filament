[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

groupshared float4 v;

float4 tint_workgroupUniformLoad_v() {
  GroupMemoryBarrierWithGroupSync();
  float4 result = v;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

float4 foo() {
  return tint_workgroupUniformLoad_v();
}
