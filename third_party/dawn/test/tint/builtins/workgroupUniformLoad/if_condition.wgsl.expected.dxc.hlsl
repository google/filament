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

int foo() {
  if (tint_workgroupUniformLoad_v()) {
    return 42;
  }
  return 0;
}
