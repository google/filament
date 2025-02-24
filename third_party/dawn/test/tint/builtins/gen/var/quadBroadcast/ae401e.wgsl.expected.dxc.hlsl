//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 quadBroadcast_ae401e() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = QuadReadLaneAt(arg_0, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadBroadcast_ae401e()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

uint3 quadBroadcast_ae401e() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = QuadReadLaneAt(arg_0, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadBroadcast_ae401e()));
  return;
}
