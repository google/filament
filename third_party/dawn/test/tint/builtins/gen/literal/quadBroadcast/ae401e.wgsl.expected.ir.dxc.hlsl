//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 quadBroadcast_ae401e() {
  uint3 res = QuadReadLaneAt((1u).xxx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, quadBroadcast_ae401e());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 quadBroadcast_ae401e() {
  uint3 res = QuadReadLaneAt((1u).xxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, quadBroadcast_ae401e());
}

