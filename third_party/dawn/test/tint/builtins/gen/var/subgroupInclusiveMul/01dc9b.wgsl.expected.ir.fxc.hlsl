SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupInclusiveMul_01dc9b() {
  float2 arg_0 = (1.0f).xx;
  float2 v = arg_0;
  float2 res = (WavePrefixProduct(v) * v);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveMul_01dc9b()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupInclusiveMul_01dc9b()));
}

FXC validation failure:
<scrubbed_path>(6,17-36): error X3004: undeclared identifier 'WavePrefixProduct'


tint executable returned error: exit status 1
