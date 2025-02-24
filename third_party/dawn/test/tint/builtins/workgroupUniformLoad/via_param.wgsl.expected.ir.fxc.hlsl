
groupshared int v[4];
int foo(uint p_indices[1]) {
  GroupMemoryBarrierWithGroupSync();
  int v_1 = v[p_indices[0u]];
  GroupMemoryBarrierWithGroupSync();
  return v_1;
}

int bar() {
  uint v_2[1] = {0u};
  return foo(v_2);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

