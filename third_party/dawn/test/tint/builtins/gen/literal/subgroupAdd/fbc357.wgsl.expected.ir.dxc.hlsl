//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupAdd_fbc357() {
  uint4 res = WaveActiveSum((1u).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupAdd_fbc357());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupAdd_fbc357() {
  uint4 res = WaveActiveSum((1u).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupAdd_fbc357());
}

