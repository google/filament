//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupExclusiveMul_a23002() {
  int arg_0 = 1;
  int res = WavePrefixProduct(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveMul_a23002()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

int subgroupExclusiveMul_a23002() {
  int arg_0 = 1;
  int res = WavePrefixProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveMul_a23002()));
  return;
}
