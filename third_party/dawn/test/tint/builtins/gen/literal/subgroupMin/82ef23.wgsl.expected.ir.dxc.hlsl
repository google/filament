//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupMin_82ef23() {
  uint4 res = WaveActiveMin((1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupMin_82ef23());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupMin_82ef23() {
  uint4 res = WaveActiveMin((1u).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupMin_82ef23());
}

