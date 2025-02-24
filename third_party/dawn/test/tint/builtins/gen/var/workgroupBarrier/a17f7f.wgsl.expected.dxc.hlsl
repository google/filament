void workgroupBarrier_a17f7f() {
  GroupMemoryBarrierWithGroupSync();
}

[numthreads(1, 1, 1)]
void compute_main() {
  workgroupBarrier_a17f7f();
  return;
}
