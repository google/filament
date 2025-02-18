//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupShuffle_5dfeab() {
  float4 arg_0 = (1.0f).xxxx;
  int arg_1 = int(1);
  float4 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_5dfeab()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupShuffle_5dfeab() {
  float4 arg_0 = (1.0f).xxxx;
  int arg_1 = int(1);
  float4 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_5dfeab()));
}

