
void textureBarrier_3d0f7e() {
  DeviceMemoryBarrierWithGroupSync();
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureBarrier_3d0f7e();
}

