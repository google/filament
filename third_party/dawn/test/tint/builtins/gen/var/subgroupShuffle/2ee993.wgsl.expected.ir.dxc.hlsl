//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupShuffle_2ee993() {
  int4 arg_0 = (int(1)).xxxx;
  int arg_1 = int(1);
  int4 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_2ee993()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupShuffle_2ee993() {
  int4 arg_0 = (int(1)).xxxx;
  int arg_1 = int(1);
  int4 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_2ee993()));
}

