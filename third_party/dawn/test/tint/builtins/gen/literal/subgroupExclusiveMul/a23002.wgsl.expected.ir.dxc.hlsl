//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupExclusiveMul_a23002() {
  int res = WavePrefixProduct(int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveMul_a23002()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupExclusiveMul_a23002() {
  int res = WavePrefixProduct(int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveMul_a23002()));
}

