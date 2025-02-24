
groupshared bool v;
bool foo() {
  GroupMemoryBarrierWithGroupSync();
  bool v_1 = v;
  GroupMemoryBarrierWithGroupSync();
  return v_1;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

