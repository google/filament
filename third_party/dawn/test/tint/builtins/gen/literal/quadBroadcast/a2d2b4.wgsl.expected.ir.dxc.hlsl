//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint quadBroadcast_a2d2b4() {
  uint res = QuadReadLaneAt(1u, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, quadBroadcast_a2d2b4());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint quadBroadcast_a2d2b4() {
  uint res = QuadReadLaneAt(1u, int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, quadBroadcast_a2d2b4());
}

