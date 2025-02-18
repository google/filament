//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint quadBroadcast_e6d948() {
  uint arg_0 = 1u;
  uint res = QuadReadLaneAt(arg_0, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, quadBroadcast_e6d948());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint quadBroadcast_e6d948() {
  uint arg_0 = 1u;
  uint res = QuadReadLaneAt(arg_0, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, quadBroadcast_e6d948());
}

