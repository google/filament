//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 quadBroadcast_f1e8ec() {
  uint3 res = QuadReadLaneAt((1u).xxx, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, quadBroadcast_f1e8ec());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 quadBroadcast_f1e8ec() {
  uint3 res = QuadReadLaneAt((1u).xxx, int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, quadBroadcast_f1e8ec());
}

