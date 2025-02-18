[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

groupshared int v[4];

typedef int tint_workgroupUniformLoad_v_ret[4];
tint_workgroupUniformLoad_v_ret tint_workgroupUniformLoad_v() {
  GroupMemoryBarrierWithGroupSync();
  int result[4] = v;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

typedef int foo_ret[4];
foo_ret foo() {
  return tint_workgroupUniformLoad_v();
}
