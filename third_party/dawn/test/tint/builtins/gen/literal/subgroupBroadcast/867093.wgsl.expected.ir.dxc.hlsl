//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float subgroupBroadcast_867093() {
  float res = WaveReadLaneAt(1.0f, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcast_867093()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float subgroupBroadcast_867093() {
  float res = WaveReadLaneAt(1.0f, int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcast_867093()));
}

