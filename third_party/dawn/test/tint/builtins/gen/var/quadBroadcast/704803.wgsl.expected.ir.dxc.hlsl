//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 quadBroadcast_704803() {
  int3 arg_0 = (int(1)).xxx;
  int3 res = QuadReadLaneAt(arg_0, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadBroadcast_704803()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 quadBroadcast_704803() {
  int3 arg_0 = (int(1)).xxx;
  int3 res = QuadReadLaneAt(arg_0, int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadBroadcast_704803()));
}

