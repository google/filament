//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupInclusiveMul_359176() {
  uint3 arg_0 = (1u).xxx;
  uint3 v = arg_0;
  uint3 res = (WavePrefixProduct(v) * v);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupInclusiveMul_359176());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupInclusiveMul_359176() {
  uint3 arg_0 = (1u).xxx;
  uint3 v = arg_0;
  uint3 res = (WavePrefixProduct(v) * v);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupInclusiveMul_359176());
}

