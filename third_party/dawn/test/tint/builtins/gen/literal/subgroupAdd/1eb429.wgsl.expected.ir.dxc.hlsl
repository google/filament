//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupAdd_1eb429() {
  int2 res = WaveActiveSum((int(1)).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupAdd_1eb429()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupAdd_1eb429() {
  int2 res = WaveActiveSum((int(1)).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupAdd_1eb429()));
}

