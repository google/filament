//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 quadBroadcast_f60448() {
  uint2 arg_0 = (1u).xx;
  uint2 res = QuadReadLaneAt(arg_0, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, quadBroadcast_f60448());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 quadBroadcast_f60448() {
  uint2 arg_0 = (1u).xx;
  uint2 res = QuadReadLaneAt(arg_0, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, quadBroadcast_f60448());
}

