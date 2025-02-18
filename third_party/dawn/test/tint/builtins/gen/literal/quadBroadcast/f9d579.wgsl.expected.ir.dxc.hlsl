//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int quadBroadcast_f9d579() {
  int res = QuadReadLaneAt(int(1), int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(quadBroadcast_f9d579()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int quadBroadcast_f9d579() {
  int res = QuadReadLaneAt(int(1), int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(quadBroadcast_f9d579()));
}

