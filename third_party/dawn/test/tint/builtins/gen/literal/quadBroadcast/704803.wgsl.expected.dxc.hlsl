//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int3 quadBroadcast_704803() {
  int3 res = QuadReadLaneAt((1).xxx, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadBroadcast_704803()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int3 quadBroadcast_704803() {
  int3 res = QuadReadLaneAt((1).xxx, 1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadBroadcast_704803()));
  return;
}
