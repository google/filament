//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupInclusiveMul_769def() {
  int3 arg_0 = (int(1)).xxx;
  int3 v = arg_0;
  int3 res = (WavePrefixProduct(v) * v);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveMul_769def()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupInclusiveMul_769def() {
  int3 arg_0 = (int(1)).xxx;
  int3 v = arg_0;
  int3 res = (WavePrefixProduct(v) * v);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveMul_769def()));
}

