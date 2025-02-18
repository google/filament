//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupInclusiveMul_517979() {
  int4 arg_0 = (int(1)).xxxx;
  int4 v = arg_0;
  int4 res = (WavePrefixProduct(v) * v);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveMul_517979()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupInclusiveMul_517979() {
  int4 arg_0 = (int(1)).xxxx;
  int4 v = arg_0;
  int4 res = (WavePrefixProduct(v) * v);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupInclusiveMul_517979()));
}

