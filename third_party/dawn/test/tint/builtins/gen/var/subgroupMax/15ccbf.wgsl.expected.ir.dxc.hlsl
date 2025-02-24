//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupMax_15ccbf() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WaveActiveMax(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupMax_15ccbf());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupMax_15ccbf() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WaveActiveMax(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupMax_15ccbf());
}

