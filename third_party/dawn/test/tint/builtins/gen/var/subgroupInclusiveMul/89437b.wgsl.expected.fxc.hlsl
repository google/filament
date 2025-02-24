SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupInclusiveMul_89437b() {
  uint arg_0 = 1u;
  uint res = (WavePrefixProduct(arg_0) * arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveMul_89437b()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupInclusiveMul_89437b()));
  return;
}
