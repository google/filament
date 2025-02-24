//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupInclusiveMul_9a54ec() {
  int arg_0 = int(1);
  int v = arg_0;
  int res = (WavePrefixProduct(v) * v);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveMul_9a54ec()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupInclusiveMul_9a54ec() {
  int arg_0 = int(1);
  int v = arg_0;
  int res = (WavePrefixProduct(v) * v);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveMul_9a54ec()));
}

