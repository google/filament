[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

groupshared int v[4];

int tint_workgroupUniformLoad_v_X(uint p[1]) {
  GroupMemoryBarrierWithGroupSync();
  int result = v[p[0]];
  GroupMemoryBarrierWithGroupSync();
  return result;
}

int foo_v_X(uint p[1]) {
  uint tint_symbol[1] = {p[0u]};
  return tint_workgroupUniformLoad_v_X(tint_symbol);
}

int bar() {
  uint tint_symbol_1[1] = (uint[1])0;
  return foo_v_X(tint_symbol_1);
}
