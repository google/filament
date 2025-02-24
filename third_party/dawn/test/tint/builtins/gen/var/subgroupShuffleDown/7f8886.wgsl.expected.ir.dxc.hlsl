//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float subgroupShuffleDown_7f8886() {
  float arg_0 = 1.0f;
  uint arg_1 = 1u;
  float res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() + arg_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleDown_7f8886()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float subgroupShuffleDown_7f8886() {
  float arg_0 = 1.0f;
  uint arg_1 = 1u;
  float res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() + arg_1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleDown_7f8886()));
}

