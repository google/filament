[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

groupshared bool v;

bool tint_workgroupUniformLoad_v() {
  GroupMemoryBarrierWithGroupSync();
  bool result = v;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

bool foo() {
  return tint_workgroupUniformLoad_v();
}
