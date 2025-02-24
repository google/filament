SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupInclusiveMul_769def() {
  int3 arg_0 = (1).xxx;
  int3 res = (WavePrefixProduct(arg_0) * arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveMul_769def()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupInclusiveMul_769def()));
  return;
}
