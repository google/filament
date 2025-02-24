//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupBroadcastFirst_6945f6() {
  float2 res = WaveReadLaneFirst((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcastFirst_6945f6()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupBroadcastFirst_6945f6() {
  float2 res = WaveReadLaneFirst((1.0f).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcastFirst_6945f6()));
}

