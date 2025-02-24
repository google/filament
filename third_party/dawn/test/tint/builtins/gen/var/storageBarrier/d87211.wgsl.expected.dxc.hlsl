void storageBarrier_d87211() {
  DeviceMemoryBarrierWithGroupSync();
}

[numthreads(1, 1, 1)]
void compute_main() {
  storageBarrier_d87211();
  return;
}
