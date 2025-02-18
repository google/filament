//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupExclusiveMul_019660() {
  int4 arg_0 = (int(1)).xxxx;
  int4 res = WavePrefixProduct(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveMul_019660()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupExclusiveMul_019660() {
  int4 arg_0 = (int(1)).xxxx;
  int4 res = WavePrefixProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveMul_019660()));
}

