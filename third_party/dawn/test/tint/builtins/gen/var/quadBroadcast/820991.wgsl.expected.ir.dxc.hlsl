//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 quadBroadcast_820991() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = QuadReadLaneAt(arg_0, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(quadBroadcast_820991()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 quadBroadcast_820991() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = QuadReadLaneAt(arg_0, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(quadBroadcast_820991()));
}

