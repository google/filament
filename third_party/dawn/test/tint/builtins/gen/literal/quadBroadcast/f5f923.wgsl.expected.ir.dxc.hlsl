//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 quadBroadcast_f5f923() {
  int2 res = QuadReadLaneAt((int(1)).xx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadBroadcast_f5f923()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 quadBroadcast_f5f923() {
  int2 res = QuadReadLaneAt((int(1)).xx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadBroadcast_f5f923()));
}

