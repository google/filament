//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float quadBroadcast_e6d39d() {
  float res = QuadReadLaneAt(1.0f, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadBroadcast_e6d39d()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float quadBroadcast_e6d39d() {
  float res = QuadReadLaneAt(1.0f, 1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadBroadcast_e6d39d()));
  return;
}
