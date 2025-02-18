//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 quadBroadcast_641316() {
  uint2 res = QuadReadLaneAt((1u).xx, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadBroadcast_641316()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint2 quadBroadcast_641316() {
  uint2 res = QuadReadLaneAt((1u).xx, 1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadBroadcast_641316()));
  return;
}
