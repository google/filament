//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float subgroupShuffle_4752bd() {
  float res = WaveReadLaneAt(1.0f, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_4752bd()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

float subgroupShuffle_4752bd() {
  float res = WaveReadLaneAt(1.0f, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_4752bd()));
  return;
}
