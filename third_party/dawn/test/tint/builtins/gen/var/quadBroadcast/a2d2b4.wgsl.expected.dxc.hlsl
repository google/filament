//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint quadBroadcast_a2d2b4() {
  uint arg_0 = 1u;
  uint res = QuadReadLaneAt(arg_0, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadBroadcast_a2d2b4()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint quadBroadcast_a2d2b4() {
  uint arg_0 = 1u;
  uint res = QuadReadLaneAt(arg_0, 1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadBroadcast_a2d2b4()));
  return;
}
