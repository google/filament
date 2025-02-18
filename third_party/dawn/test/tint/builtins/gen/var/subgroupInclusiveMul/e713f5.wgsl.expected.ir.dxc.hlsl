//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupInclusiveMul_e713f5() {
  int2 arg_0 = (int(1)).xx;
  int2 v = arg_0;
  int2 res = (WavePrefixProduct(v) * v);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveMul_e713f5()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupInclusiveMul_e713f5() {
  int2 arg_0 = (int(1)).xx;
  int2 v = arg_0;
  int2 res = (WavePrefixProduct(v) * v);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveMul_e713f5()));
}

