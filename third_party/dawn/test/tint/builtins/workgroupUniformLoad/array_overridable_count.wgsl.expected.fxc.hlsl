[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

groupshared int v[128];

typedef int tint_workgroupUniformLoad_v_ret[128];
tint_workgroupUniformLoad_v_ret tint_workgroupUniformLoad_v() {
  GroupMemoryBarrierWithGroupSync();
  int result[128] = v;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

int foo() {
  int tint_symbol[128] = tint_workgroupUniformLoad_v();
  return tint_symbol[0];
}
