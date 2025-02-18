//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 quadBroadcast_bed00b() {
  int4 res = QuadReadLaneAt((int(1)).xxxx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadBroadcast_bed00b()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 quadBroadcast_bed00b() {
  int4 res = QuadReadLaneAt((int(1)).xxxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadBroadcast_bed00b()));
}

