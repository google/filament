
groupshared bool v;
int foo() {
  GroupMemoryBarrierWithGroupSync();
  bool v_1 = v;
  GroupMemoryBarrierWithGroupSync();
  if (v_1) {
    return int(42);
  }
  return int(0);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

