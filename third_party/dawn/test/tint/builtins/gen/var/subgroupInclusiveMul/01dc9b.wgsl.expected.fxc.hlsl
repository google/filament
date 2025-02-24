SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

float2 subgroupInclusiveMul_01dc9b() {
  float2 arg_0 = (1.0f).xx;
  float2 res = (WavePrefixProduct(arg_0) * arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveMul_01dc9b()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveMul_01dc9b()));
  return;
}
