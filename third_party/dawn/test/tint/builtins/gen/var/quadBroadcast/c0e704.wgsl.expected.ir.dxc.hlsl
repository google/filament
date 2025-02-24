//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 quadBroadcast_c0e704() {
  int2 arg_0 = (int(1)).xx;
  int2 res = QuadReadLaneAt(arg_0, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(quadBroadcast_c0e704()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 quadBroadcast_c0e704() {
  int2 arg_0 = (int(1)).xx;
  int2 res = QuadReadLaneAt(arg_0, int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(quadBroadcast_c0e704()));
}

